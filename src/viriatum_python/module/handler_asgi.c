/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "handler_asgi.h"

static void _report_handler_asgi(void) {
    /* in case the pending exception is a system exit it must not be
    printed, as that would terminate the process that is hosting the
    server, it is cleared and reported as a plain error instead */
    if(PyErr_ExceptionMatches(PyExc_SystemExit)) {
        PyErr_Clear();
        V_WARNING("Application requested a system exit, ignoring\n");
        return;
    }

    /* in case the pending exception is a keyboard interrupt it is re
    armed so that the serving loop is able to notice it */
    if(PyErr_ExceptionMatches(PyExc_KeyboardInterrupt)) {
        PyErr_Clear();
        PyErr_SetInterrupt();
        return;
    }

    /* prints the pending exception into the standard error, this is the
    usual reporting for an application level problem */
    PyErr_Print();
}

static void _decode_handler_asgi(char *value) {
    /* allocates space for both the read and the write indexes, the
    decoding is run in place as it may only shrink the value */
    size_t read = 0;
    size_t write = 0;
    char first;
    char second;

    /* iterates over the complete set of characters looking for the
    percent escapes, any other character is copied unchanged */
    while(value[read] != '\0') {
        if(value[read] == '%' && isxdigit((unsigned char) value[read + 1]) &&
            isxdigit((unsigned char) value[read + 2])) {
            first = value[read + 1];
            second = value[read + 2];
            value[write] = (char) (
                (isdigit((unsigned char) first) ? first - '0' : (toupper(first) - 'A') + 10) * 16 +
                (isdigit((unsigned char) second) ? second - '0' : (toupper(second) - 'A') + 10)
            );
            read += 3;
        } else {
            value[write] = value[read];
            read++;
        }
        write++;
    }

    /* closes the resulting value with the end of string character */
    value[write] = '\0';
}

static char _is_valid_handler_asgi(const char *value, size_t size) {
    /* iterates over the complete set of characters of the value looking
    for a control one, their presence in a header would allow the
    response envelope to be split by the application */
    size_t index;
    for(index = 0; index < size; index++) {
        if((unsigned char) value[index] < 32 || value[index] == 127) { return FALSE; }
    }
    return TRUE;
}

static char _is_length_handler_asgi(const char *name, size_t size) {
    /* compares the provided name against the content length one in a
    case insensitive manner as header names are not case sensitive */
    size_t index;
    const char *reference = CONTENT_LENGTH_H;
    if(size != strlen(reference)) { return FALSE; }
    for(index = 0; index < size; index++) {
        if(toupper((unsigned char) name[index]) != toupper((unsigned char) reference[index])) {
            return FALSE;
        }
    }
    return TRUE;
}

static const char *_status_handler_asgi(int status_code) {
    /* calculates both the coordinates of the status message in the
    table of them, the code has already been validated as a proper
    one and so only the bounds of the table must be verified */
    const char *status_message;
    size_t major = (size_t) (status_code / 100) - 1;
    size_t minor = (size_t) (status_code % 100);
    if(major > 4 || minor > 63) { return "Unknown"; }

    /* retrieves the status message from the table, an unset entry
    means that the code is not one of the standard ones */
    status_message = _get_http_status_code(major, minor);
    return status_message == NULL ? "Unknown" : status_message;
}

static void _push_event_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, PyObject *event) {
    /* in case a receive call is currently pending the event is used to
    resolve the future that it returned, waking the application up on
    the next iteration of the loop, otherwise it is queued */
    if(handler_asgi_context->future != NULL) {
        resolve_future_loop_python(handler_asgi_context->future, event);
        Py_DECREF(handler_asgi_context->future);
        handler_asgi_context->future = NULL;
        return;
    }
    if(handler_asgi_context->events == NULL) { return; }
    if(PyList_Append(handler_asgi_context->events, event) < 0) { PyErr_Clear(); }
}

static PyObject *_receive_handler_asgi(PyObject *self, PyObject *args) {
    /* allocates space for both the future that is returned to the
    application and for the event used to resolve it */
    PyObject *future;
    PyObject *event;

    /* retrieves the context of the request from the capsule that has
    been set as the self value of the callable */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) PyCapsule_GetPointer(self, NULL);
    if(handler_asgi_context == NULL) { return NULL; }

    /* creates the future that is going to be awaited by the caller,
    every message of the specification is an awaitable one */
    future = create_future_loop_python(handler_asgi_context->handler->loop_python);
    if(future == NULL) { return NULL; }

    /* in case an event is already queued it is used to resolve the
    future immediately, no suspension of the application happens */
    if(handler_asgi_context->events != NULL &&
        PyList_Size(handler_asgi_context->events) > 0) {
        event = PyList_GetItem(handler_asgi_context->events, 0);
        Py_INCREF(event);
        if(PySequence_DelItem(handler_asgi_context->events, 0) < 0) { PyErr_Clear(); }
        resolve_future_loop_python(future, event);
        Py_DECREF(event);
        return future;
    }

    /* stores the future so that it may be resolved as soon as an
    event becomes available, replacing any previously pending one */
    Py_XDECREF(handler_asgi_context->future);
    Py_INCREF(future);
    handler_asgi_context->future = future;
    return future;
}

static ERROR_CODE _write_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, const char *data, size_t data_size, char last, PyObject *future) {
    /* allocates space for the buffer receiving the payload together
    with the counter of the bytes already written into it */
    char *buffer;
    size_t count = 0;
    size_t buffer_size;
    struct write_asgi_t *write_asgi;
    struct connection_t *connection = handler_asgi_context->connection;

    /* calculates the size of the buffer that is required for the
    payload, the chunked framing takes both a prefix carrying the
    size of the chunk and the terminator of the stream */
    buffer_size = data_size;
    if(handler_asgi_context->has_length == FALSE) {
        if(data_size > 0) { buffer_size += VIRIATUM_MAX_HEADER_C_SIZE + 2; }
        if(last == TRUE) { buffer_size += 5; }
    }

    /* allocates the buffer for the complete payload and writes both
    the chunk and the terminator of the stream into it */
    connection->alloc_data(connection, buffer_size + 1, (void **) &buffer);
    if(handler_asgi_context->has_length == FALSE) {
        if(data_size > 0) {
            count = SPRINTF(
                buffer,
                VIRIATUM_MAX_HEADER_C_SIZE,
                "%lx\r\n",
                (long unsigned int) data_size
            );
            memcpy(&buffer[count], data, data_size);
            count += data_size;
            memcpy(&buffer[count], "\r\n", 2);
            count += 2;
        }
        if(last == TRUE) {
            memcpy(&buffer[count], "0\r\n\r\n", 5);
            count += 5;
        }
    } else if(data_size > 0) {
        memcpy(buffer, data, data_size);
        count = data_size;
    }

    /* creates the state carrying both the future to be resolved once
    the payload reaches the wire and the flag marking the end of the
    response, it is released by the callback itself */
    write_asgi = (struct write_asgi_t *) MALLOC(sizeof(struct write_asgi_t));
    write_asgi->handler_asgi_context = handler_asgi_context;
    write_asgi->last = last;
    write_asgi->future = future;
    write_asgi->next = handler_asgi_context->writes;
    handler_asgi_context->writes = write_asgi;
    Py_XINCREF(future);

    /* writes the payload into the connection, registering the callback
    that resolves the future and tears the request down */
    connection->write_connection(
        connection,
        (unsigned char *) buffer,
        (unsigned int) count,
        _send_response_callback_handler_asgi,
        (void *) write_asgi
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _send_headers_handler_asgi(struct handler_asgi_context_t *handler_asgi_context) {
    /* allocates space for the buffer receiving the envelope together
    with the counter of the bytes already written into it */
    char *buffer;
    size_t count;
    size_t index;
    size_t buffer_size;
    size_t headers_size = 0;

    /* retrieves the underlying connection references in order to be
    able to reach the writing of the default headers */
    struct connection_t *connection = handler_asgi_context->connection;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* calculates the amount of space taken by the headers set by the
    application, these are unbounded in size and so they must be
    accounted for in the allocation of the buffer */
    for(index = 0; index < handler_asgi_context->response_header_count; index++) {
        headers_size += strlen((char *) handler_asgi_context->response_headers[index]) + 2;
    }

    /* allocates space for the complete envelope, the default headers
    are bounded by the maximum HTTP size and the application ones
    take the remaining part of the buffer */
    buffer_size = VIRIATUM_HTTP_MAX_SIZE + headers_size;
    connection->alloc_data(connection, buffer_size, (void **) &buffer);

    /* writes the default set of headers into the buffer, the connection
    is kept alive according to the flags of the current request */
    count = http_connection->write_headers(
        connection,
        buffer,
        VIRIATUM_HTTP_SIZE,
        HTTP11,
        handler_asgi_context->status_code,
        (char *) _status_handler_asgi(handler_asgi_context->status_code),
        handler_asgi_context->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
        FALSE
    );

    /* in case the application has set no content length of its own the
    payload is framed using the chunked transfer encoding, as its size
    is only known once the last of the chunks is received */
    if(handler_asgi_context->has_length == FALSE) {
        count += SPRINTF(
            &buffer[count],
            buffer_size - count,
            "%s: chunked\r\n",
            TRANSFER_ENCODING_H
        );
    }

    /* iterates over the complete set of headers set by the application
    copying each one of them into the headers buffer */
    for(index = 0; index < handler_asgi_context->response_header_count; index++) {
        count += SPRINTF(
            &buffer[count],
            buffer_size - count,
            "%s\r\n",
            handler_asgi_context->response_headers[index]
        );
    }

    /* closes the headers part of the envelope and writes it into the
    connection, no callback is required as it is never the last part */
    memcpy(&buffer[count], "\r\n", 2);
    count += 2;
    connection->write_connection(
        connection,
        (unsigned char *) buffer,
        (unsigned int) count,
        NULL,
        NULL
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

static PyObject *_send_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, PyObject *message, const char *type_value);
static PyObject *_send_lifespan_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, PyObject *message, const char *type_value);
static ERROR_CODE _write_raw_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, const unsigned char *data, size_t data_size, char close);
static ERROR_CODE _close_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, unsigned short code, const char *reason);

static PyObject *_send_handler_asgi(PyObject *self, PyObject *args) {
    /* allocates space for the various objects used during the parsing
    of the message provided by the application */
    PyObject *message;
    PyObject *object;
    PyObject *future;
    PyObject *headers;
    PyObject *header;
    PyObject *name;
    PyObject *value;
    const char *type_value;
    char *name_value;
    char *value_value;
    Py_ssize_t name_size;
    Py_ssize_t value_size;
    Py_ssize_t index;
    Py_ssize_t count;
    char *body_data = NULL;
    Py_ssize_t body_size = 0;
    char more_body = FALSE;
    long status_code;
    size_t header_size;

    /* retrieves the context of the request from the capsule that has
    been set as the self value of the callable */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) PyCapsule_GetPointer(self, NULL);
    if(handler_asgi_context == NULL) { return NULL; }

    /* parses the single message argument of the call and retrieves the
    type of it, the value that discriminates the various messages */
    if(!PyArg_ParseTuple(args, "O", &message)) { return NULL; }
    if(!PyDict_Check(message)) {
        PyErr_SetString(PyExc_TypeError, "message must be a dictionary");
        return NULL;
    }
    object = PyDict_GetItemString(message, "type");
    if(object == NULL || (type_value = PyUnicode_AsUTF8(object)) == NULL) {
        PyErr_SetString(PyExc_ValueError, "message carries no valid type");
        return NULL;
    }

    /* the messages of the lifespan protocol carry no payload and so
    they are handled apart from the ones of a request */
    if(handler_asgi_context->lifespan == TRUE) {
        return _send_lifespan_handler_asgi(handler_asgi_context, message, type_value);
    }

    /* in case the connection has been upgraded the message belongs to
    the websocket protocol and so it is handled apart */
    if(handler_asgi_context->websocket_state != ASGI_WEBSOCKET_NONE) {
        return _send_websocket_handler_asgi(handler_asgi_context, message, type_value);
    }

    /* creates the future that is going to be awaited by the caller,
    every message of the specification is an awaitable one */
    future = create_future_loop_python(handler_asgi_context->handler->loop_python);
    if(future == NULL) { return NULL; }

    /* handles the message that starts the response, it carries both
    the status code and the complete set of headers of it */
    if(strcmp(type_value, "http.response.start") == 0) {
        if(handler_asgi_context->state != ASGI_STATE_PENDING) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_RuntimeError, "response has already been started");
            return NULL;
        }

        /* retrieves the status code verifying that it falls inside the
        range of the valid HTTP ones, a malformed value would otherwise
        reach the wire as a zero code */
        object = PyDict_GetItemString(message, "status");
        status_code = object == NULL ? -1 : PyLong_AsLong(object);
        if(PyErr_Occurred()) { PyErr_Clear(); status_code = -1; }
        if(status_code < 100 || status_code > 599) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_ValueError, "status must be a valid code");
            return NULL;
        }
        handler_asgi_context->status_code = (int) status_code;

        /* the statuses that are defined as carrying no payload may not
        be framed, as a client would otherwise wait for a body */
        if(status_code == 204 || status_code == 304) {
            handler_asgi_context->has_body = FALSE;
        }

        /* iterates over the complete set of headers provided by the
        application storing them as complete header lines */
        headers = PyDict_GetItemString(message, "headers");
        count = headers == NULL ? 0 : PySequence_Length(headers);
        if(count < 0) { PyErr_Clear(); count = 0; }
        for(index = 0; index < count; index++) {
            /* in case the maximum number of headers has been reached the
            remaining ones must be discarded (avoids overflow) */
            if(handler_asgi_context->response_header_count >= VIRIATUM_ASGI_MAX_HEADERS) { break; }

            header = PySequence_GetItem(headers, index);
            if(header == NULL) { Py_DECREF(future); return NULL; }
            name = PySequence_GetItem(header, 0);
            value = PySequence_GetItem(header, 1);
            Py_DECREF(header);
            if(name == NULL || value == NULL) {
                Py_XDECREF(name);
                Py_XDECREF(value);
                Py_DECREF(future);
                return NULL;
            }

            /* the specification defines both the name and the value of
            a header as byte strings, no encoding is ever assumed */
            if(PyBytes_AsStringAndSize(name, &name_value, &name_size) < 0 ||
                PyBytes_AsStringAndSize(value, &value_value, &value_size) < 0) {
                Py_DECREF(name);
                Py_DECREF(value);
                Py_DECREF(future);
                return NULL;
            }

            /* rejects any control character in either the name or the
            value of the header, as they would allow the response to be
            split by an application that reflects received data */
            if(_is_valid_handler_asgi(name_value, (size_t) name_size) == FALSE ||
                _is_valid_handler_asgi(value_value, (size_t) value_size) == FALSE) {
                Py_DECREF(name);
                Py_DECREF(value);
                Py_DECREF(future);
                PyErr_SetString(PyExc_ValueError, "header carries a control character");
                return NULL;
            }

            /* verifies if the application has set a content length of
            its own, in which case no chunked framing is used */
            if(_is_length_handler_asgi(name_value, (size_t) name_size) == TRUE) {
                handler_asgi_context->has_length = TRUE;
            }

            /* allocates space for the complete header line and formats
            it into the newly allocated buffer (name and value pair) */
            header_size = (size_t) name_size + (size_t) value_size + 3;
            handler_asgi_context->response_headers[handler_asgi_context->response_header_count] =
                (unsigned char *) MALLOC(header_size);
            SPRINTF(
                (char *) handler_asgi_context->response_headers[handler_asgi_context->response_header_count],
                header_size,
                "%.*s: %.*s",
                (int) name_size,
                name_value,
                (int) value_size,
                value_value
            );
            handler_asgi_context->response_header_count++;

            Py_DECREF(name);
            Py_DECREF(value);
        }

        /* a response that may carry no payload is never framed using
        the chunked transfer encoding, nothing follows the envelope */
        if(handler_asgi_context->has_body == FALSE) {
            handler_asgi_context->has_length = TRUE;
        }

        /* marks the response as started and resolves the future, no
        writing happens until the first of the body messages */
        handler_asgi_context->state = ASGI_STATE_STARTED;
        resolve_future_loop_python(future, Py_None);
        return future;
    }

    /* handles the message that carries a part of the payload of the
    response, the last one is the one that closes it */
    if(strcmp(type_value, "http.response.body") == 0) {
        if(handler_asgi_context->state == ASGI_STATE_PENDING) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_RuntimeError, "response has not been started");
            return NULL;
        }
        if(handler_asgi_context->state == ASGI_STATE_COMPLETE) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_RuntimeError, "response has already been completed");
            return NULL;
        }

        /* retrieves both the payload of the message and the flag that
        marks it as one of the intermediate ones */
        object = PyDict_GetItemString(message, "body");
        if(object != NULL && object != Py_None) {
            if(PyBytes_AsStringAndSize(object, &body_data, &body_size) < 0) {
                Py_DECREF(future);
                return NULL;
            }
        }
        object = PyDict_GetItemString(message, "more_body");
        if(object != NULL) { more_body = PyObject_IsTrue(object) ? TRUE : FALSE; }

        /* writes the envelope of the response in case it has not been
        written yet, it precedes the first of the chunks */
        if(handler_asgi_context->state == ASGI_STATE_STARTED) {
            _send_headers_handler_asgi(handler_asgi_context);
            handler_asgi_context->state = ASGI_STATE_HEADERS;
        }

        /* the responses that may carry no payload have their body
        silently discarded, only the envelope reaches the wire */
        if(handler_asgi_context->has_body == FALSE) { body_size = 0; }

        /* marks the response as complete in case this is the last of
        the chunks and writes the payload into the connection */
        if(more_body == FALSE) { handler_asgi_context->state = ASGI_STATE_COMPLETE; }
        _write_handler_asgi(
            handler_asgi_context,
            body_data,
            (size_t) body_size,
            more_body == FALSE ? TRUE : FALSE,
            future
        );
        return future;
    }

    /* no other message type is defined for the http scope and so the
    application is reporting a problem of its own */
    Py_DECREF(future);
    PyErr_Format(PyExc_ValueError, "unexpected message type '%s'", type_value);
    return NULL;
}

/**
 * The method definition for the receive callable that is handed to
 * the application, the context of the request is carried in the
 * self argument through a capsule.
 */
static PyMethodDef receive_method = {
    "receive",
    (PyCFunction) _receive_handler_asgi,
    METH_NOARGS,
    NULL
};

/**
 * The method definition for the send callable that is handed to the
 * application, it shares the capsule carrying the context.
 */
static PyMethodDef send_method = {
    "send",
    (PyCFunction) _send_handler_asgi,
    METH_VARARGS,
    NULL
};

ERROR_CODE create_handler_asgi_context(struct handler_asgi_context_t **handler_asgi_context_pointer) {
    /* retrieves the context size and allocates space for it, then
    resets the complete set of values so that no invalid reference
    is kept in the newly created structure */
    size_t context_size = sizeof(struct handler_asgi_context_t);
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) MALLOC(context_size);
    PyGILState_STATE gil_state;
    memset(handler_asgi_context, 0, context_size);

    /* sets the various default values of the context, the status is
    defaulted to an internal error so that a failure of the
    application is properly reported to the client */
    handler_asgi_context->state = ASGI_STATE_PENDING;
    handler_asgi_context->websocket_state = ASGI_WEBSOCKET_NONE;
    handler_asgi_context->status_code = 500;
    handler_asgi_context->has_body = TRUE;

    /* creates the list that is going to hold the events that are
    pending delivery to the application */
    gil_state = PyGILState_Ensure();
    handler_asgi_context->events = PyList_New(0);
    if(handler_asgi_context->events == NULL) { PyErr_Clear(); }
    PyGILState_Release(gil_state);

    /* sets the context in the context pointer */
    *handler_asgi_context_pointer = handler_asgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_asgi_context(struct handler_asgi_context_t *handler_asgi_context) {
    /* allocates space for the index to be used in the
    iteration over the various header sequences */
    size_t index;
    struct write_asgi_t *write_asgi;
    PyGILState_STATE gil_state;

    /* acquires the global interpreter lock as the releasing of the
    various references interacts with the interpreter */
    gil_state = PyGILState_Ensure();

    /* cancels the task that is still running the application, this is
    the usual way of ending the handling of a request as it unblocks
    the application at whatever point it is suspended */
    if(handler_asgi_context->task != NULL) {
        Py_XDECREF(PyObject_CallMethod(handler_asgi_context->task, "cancel", NULL));
        PyErr_Clear();
        Py_DECREF(handler_asgi_context->task);
        handler_asgi_context->task = NULL;
    }

    /* invalidates the capsule by renaming it and then releases the
    reference held on it, a callable retained by the application then
    fails the name check instead of reaching this released context */
    if(handler_asgi_context->capsule != NULL) {
        PyCapsule_SetName(handler_asgi_context->capsule, "invalid");
        Py_DECREF(handler_asgi_context->capsule);
        handler_asgi_context->capsule = NULL;
    }

    /* releases the references held on both the pending events and the
    future of a receive call that is still pending */
    Py_CLEAR(handler_asgi_context->events);
    Py_CLEAR(handler_asgi_context->future);

    /* releases the writes that have been queued in the connection but
    never reached the wire, a dropped connection releases the queued
    data without ever raising the callbacks associated with it */
    while(handler_asgi_context->writes != NULL) {
        write_asgi = handler_asgi_context->writes;
        handler_asgi_context->writes = write_asgi->next;
        Py_XDECREF(write_asgi->future);
        FREE(write_asgi);
    }

    /* releases the global interpreter lock, no more interpreter usage
    happens for the remaining of the destruction */
    PyGILState_Release(gil_state);

    /* releases the url and the body values in case they
    have been set during the parsing of the request */
    if(handler_asgi_context->url != NULL) { FREE(handler_asgi_context->url); }
    if(handler_asgi_context->body != NULL) { FREE(handler_asgi_context->body); }

    /* releases the buffers that have been used for the reception of
    the frames of an upgraded connection */
    if(handler_asgi_context->websocket_buffer != NULL) {
        FREE(handler_asgi_context->websocket_buffer);
    }
    if(handler_asgi_context->websocket_message != NULL) {
        FREE(handler_asgi_context->websocket_message);
    }

    /* releases the various request headers that have been
    gathered from the request */
    for(index = 0; index < handler_asgi_context->header_count; index++) {
        FREE(handler_asgi_context->header_fields[index]);
        FREE(handler_asgi_context->header_values[index]);
    }

    /* releases the field of an incomplete header pair, one that has
    never been closed by the corresponding value callback */
    if(handler_asgi_context->header_count < VIRIATUM_ASGI_MAX_HEADERS &&
        handler_asgi_context->header_fields[handler_asgi_context->header_count] != NULL) {
        FREE(handler_asgi_context->header_fields[handler_asgi_context->header_count]);
    }

    /* releases the various response headers that have been
    set by the application */
    for(index = 0; index < handler_asgi_context->response_header_count; index++) {
        FREE(handler_asgi_context->response_headers[index]);
    }

    /* releases the context structure itself */
    FREE(handler_asgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

static PyObject *_invoke_handler_asgi(struct handler_asgi_t *handler_asgi, PyObject *scope, PyObject *receive, PyObject *send) {
    /* allocates space for both the instance of the application and
    for the coroutine that is returned by it */
    PyObject *instance;
    PyObject *coroutine;

    /* the single callable applications, the ones of the third version
    of the specification, take everything in a single call */
    if(handler_asgi->double_callable == FALSE) {
        return PyObject_CallFunctionObjArgs(
            handler_asgi->application,
            scope,
            receive,
            send,
            NULL
        );
    }

    /* the double callable ones are first called with the scope and the
    resulting instance is the one that takes the pair of callables */
    instance = PyObject_CallFunctionObjArgs(handler_asgi->application, scope, NULL);
    if(instance == NULL) { return NULL; }
    coroutine = PyObject_CallFunctionObjArgs(instance, receive, send, NULL);
    Py_DECREF(instance);
    return coroutine;
}

static const char *_version_handler_asgi(struct handler_asgi_t *handler_asgi) {
    /* the version reported in the scope is the one of the interface
    that is being used for the calling of the application */
    return handler_asgi->double_callable == TRUE ?
        VIRIATUM_ASGI_VERSION_LEGACY : VIRIATUM_ASGI_VERSION;
}

ERROR_CODE register_handler_asgi(struct service_t *service, PyObject *application, struct loop_python_t *loop_python, char double_callable) {
    /* allocates the HTTP handler and the asgi handler that is
    going to be kept as the lower substrate of it */
    struct http_handler_t *http_handler;
    size_t handler_size = sizeof(struct handler_asgi_t);
    struct handler_asgi_t *handler_asgi =
        (struct handler_asgi_t *) MALLOC(handler_size);
    memset(handler_asgi, 0, handler_size);

    /* increments the reference count of the application so that it
    remains valid for the complete lifetime of the handler */
    Py_INCREF(application);
    handler_asgi->application = application;
    handler_asgi->loop_python = loop_python;
    handler_asgi->double_callable = double_callable;

    /* creates the HTTP handler and sets its attributes, note that
    no index resolution is performed as the application is the one
    responsible for the complete routing of the requests */
    service->create_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    http_handler->resolve_index = FALSE;
    http_handler->set = set_handler_asgi;
    http_handler->unset = unset_handler_asgi;
    http_handler->reset = NULL;
    http_handler->lower = (void *) handler_asgi;
    handler_asgi->http_handler = http_handler;

    /* adds the HTTP handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_asgi(struct service_t *service) {
    /* allocates the HTTP handler and the associated lower
    substrate to be released */
    struct http_handler_t *http_handler;
    struct handler_asgi_t *handler_asgi;

    /* retrieves the HTTP handler from the service, in case none is
    found returns immediately (nothing to be unregistered) */
    service->get_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    if(http_handler == NULL) { RAISE_NO_ERROR; }

    /* releases the reference to the application and then releases
    the lower substrate structure itself */
    handler_asgi = (struct handler_asgi_t *) http_handler->lower;
    if(handler_asgi != NULL) {
        Py_XDECREF(handler_asgi->application);
        FREE(handler_asgi);
    }

    /* removes the HTTP handler from the service and then
    deletes the handler structure */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

static struct handler_asgi_t *_get_handler_asgi(struct service_t *service) {
    /* retrieves the HTTP handler from the service and then the lower
    substrate of it, the one carrying the application */
    struct http_handler_t *http_handler;
    service->get_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    if(http_handler == NULL) { return NULL; }
    return (struct handler_asgi_t *) http_handler->lower;
}

static PyObject *_build_lifespan_scope_handler_asgi(const char *version) {
    /* creates the scope of the lifespan protocol, it carries only the
    versions as no request is associated with it */
    PyObject *scope = PyDict_New();
    PyObject *object;
    if(scope == NULL) { return NULL; }
    PyDict_SetItemString(scope, "type", PyUnicode_FromString("lifespan"));
    object = Py_BuildValue(
        "{s:s,s:s}",
        "version", version,
        "spec_version", VIRIATUM_ASGI_SPEC_VERSION
    );
    if(object != NULL) {
        PyDict_SetItemString(scope, "asgi", object);
        Py_DECREF(object);
    }
    return scope;
}

static ERROR_CODE _wait_lifespan_handler_asgi(struct handler_asgi_t *handler_asgi, char *state) {
    /* allocates space for the flag controlling if the task is already
    done and for the deadline of the complete operation, the latter is
    measured against the clock of the loop so that a slice that fails
    to let time pass is never mistaken for a completed wait */
    PyObject *done;
    int is_done = 0;
    size_t index;
    double initial = time_loop_python(handler_asgi->loop_python);
    double elapsed = 0.0;

    /* advances the loop until the application reports the completion
    of the event, the task dies or the deadline is reached, each of
    the slices blocks so that the timers scheduled by the application
    are given the chance to come due, the iteration count bounds the
    wait as well so that a clock that never advances is not able to
    keep the loop running forever */
    for(index = 0; index < VIRIATUM_ASGI_LIFESPAN_ITERATIONS; index++) {
        elapsed = time_loop_python(handler_asgi->loop_python) - initial;
        if(elapsed >= VIRIATUM_ASGI_LIFESPAN_TIMEOUT) { break; }
        if(*state != 0) { RAISE_NO_ERROR; }
        if(IS_ERROR_CODE(run_slice_loop_python(
            handler_asgi->loop_python,
            VIRIATUM_ASGI_LIFESPAN_SLICE
        ))) {
            V_WARNING_F("Problem advancing the loop: %s\n", (char *) GET_ERROR());
            RAISE_NO_ERROR;
        }
        if(*state != 0) { RAISE_NO_ERROR; }
        done = PyObject_CallMethod(handler_asgi->lifespan_context->task, "done", NULL);
        if(done == NULL) { PyErr_Clear(); break; }
        is_done = PyObject_IsTrue(done);
        Py_DECREF(done);
        if(is_done != 0) { break; }
    }

    /* reports the giving up on a task that is still alive, this is
    what tells apart an application that never answers from a loop
    that is unable to let any time pass at all */
    if(*state == 0 && is_done == 0) {
        V_WARNING_F(
            "Lifespan gave up after %d iterations and %.3f seconds\n",
            (int) index,
            elapsed
        );
    }

    /* raises no error, the caller is the one that interprets both the
    state and the liveness of the task */
    RAISE_NO_ERROR;
}

ERROR_CODE startup_handler_asgi(struct service_t *service) {
    /* allocates space for the various objects used during the running
    of the startup event of the lifespan protocol */
    PyObject *scope;
    PyObject *receive;
    PyObject *send;
    PyObject *coroutine;
    PyObject *event;
    PyGILState_STATE gil_state;
    struct handler_asgi_context_t *handler_asgi_context;
    struct handler_asgi_t *handler_asgi = _get_handler_asgi(service);
    if(handler_asgi == NULL) { RAISE_NO_ERROR; }

    /* acquires the global interpreter lock as the complete set of
    operations that follow interact with the interpreter */
    gil_state = PyGILState_Ensure();

    /* creates the context of the lifespan protocol, it lives for the
    complete duration of the serving operation */
    create_handler_asgi_context(&handler_asgi_context);
    handler_asgi_context->lifespan = TRUE;
    handler_asgi_context->handler = handler_asgi;
    handler_asgi->lifespan_context = handler_asgi_context;

    /* builds the scope of the protocol and the callables that are
    handed to the application, both carrying the context */
    scope = _build_lifespan_scope_handler_asgi(_version_handler_asgi(handler_asgi));
    handler_asgi_context->capsule = PyCapsule_New((void *) handler_asgi_context, NULL, NULL);
    receive = PyCFunction_New(&receive_method, handler_asgi_context->capsule);
    send = PyCFunction_New(&send_method, handler_asgi_context->capsule);

    /* queues the startup event so that the first of the receive calls
    of the application resolves immediately */
    event = Py_BuildValue("{s:s}", "type", "lifespan.startup");
    if(event != NULL) {
        _push_event_handler_asgi(handler_asgi_context, event);
        Py_DECREF(event);
    }

    /* calls the application with the lifespan scope, the resulting
    coroutine is wrapped in a task driven by the serving loop */
    coroutine = scope == NULL || receive == NULL || send == NULL ? NULL :
        _invoke_handler_asgi(handler_asgi, scope, receive, send);
    Py_XDECREF(scope);
    Py_XDECREF(receive);
    Py_XDECREF(send);
    if(coroutine != NULL) {
        handler_asgi_context->task = create_task_loop_python(handler_asgi->loop_python, coroutine);
        Py_DECREF(coroutine);
    }

    /* in case the application refused the scope altogether it does not
    implement the protocol, which is a valid situation, the serving
    proceeds without any kind of lifespan handling */
    if(handler_asgi_context->task == NULL) {
        PyErr_Clear();
        V_DEBUG("Application does not support the lifespan protocol\n");
        delete_handler_asgi_context(handler_asgi_context);
        handler_asgi->lifespan_context = NULL;
        PyGILState_Release(gil_state);
        RAISE_NO_ERROR;
    }

    /* advances the loop until the startup completes, fails or the task
    dies, the latter meaning that the protocol is not supported */
    _wait_lifespan_handler_asgi(handler_asgi, &handler_asgi->lifespan_startup);

    /* in case the application reported a failure the opening of the
    service must be aborted, the problem is reported to the caller */
    if(handler_asgi->lifespan_startup == 2) {
        delete_handler_asgi_context(handler_asgi_context);
        handler_asgi->lifespan_context = NULL;
        PyGILState_Release(gil_state);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Application failed the lifespan startup"
        );
    }

    /* in case the startup never completed the application does not
    implement the protocol, the context is released as no shutdown
    is ever going to be handled by it */
    if(handler_asgi->lifespan_startup == 0) {
        PyErr_Clear();
        V_DEBUG("Application does not support the lifespan protocol\n");
        delete_handler_asgi_context(handler_asgi_context);
        handler_asgi->lifespan_context = NULL;
    }

    /* releases the global interpreter lock */
    PyGILState_Release(gil_state);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE shutdown_handler_asgi(struct service_t *service) {
    /* allocates space for the event that is sent to the application
    and for the state of the interpreter */
    PyObject *event;
    PyGILState_STATE gil_state;
    struct handler_asgi_t *handler_asgi = _get_handler_asgi(service);
    if(handler_asgi == NULL || handler_asgi->lifespan_context == NULL) { RAISE_NO_ERROR; }

    /* acquires the global interpreter lock as the complete set of
    operations that follow interact with the interpreter */
    gil_state = PyGILState_Ensure();

    /* pushes the shutdown event and advances the loop until the
    application reports the completion of it */
    event = Py_BuildValue("{s:s}", "type", "lifespan.shutdown");
    if(event != NULL) {
        _push_event_handler_asgi(handler_asgi->lifespan_context, event);
        Py_DECREF(event);
    }
    _wait_lifespan_handler_asgi(handler_asgi, &handler_asgi->lifespan_shutdown);
    if(handler_asgi->lifespan_shutdown == 2) {
        V_WARNING("Application failed the lifespan shutdown\n");
    }

    /* releases the context of the protocol, cancelling the task that
    is running the application in case it is still alive */
    delete_handler_asgi_context(handler_asgi->lifespan_context);
    handler_asgi->lifespan_context = NULL;

    /* releases the global interpreter lock */
    PyGILState_Release(gil_state);

    /* raises no error */
    RAISE_NO_ERROR;
}

static PyObject *_send_lifespan_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, PyObject *message, const char *type_value) {
    /* allocates space for both the future that is returned to the
    application and for the message describing a failure */
    PyObject *future;
    PyObject *object;
    const char *message_value;

    /* creates the future that is going to be awaited by the caller,
    every message of the specification is an awaitable one */
    future = create_future_loop_python(handler_asgi_context->handler->loop_python);
    if(future == NULL) { return NULL; }

    /* records the completion or the failure of either the startup or
    the shutdown events, the serving loop is the one waiting on them */
    if(strcmp(type_value, "lifespan.startup.complete") == 0) {
        handler_asgi_context->handler->lifespan_startup = 1;
    } else if(strcmp(type_value, "lifespan.startup.failed") == 0) {
        handler_asgi_context->handler->lifespan_startup = 2;
    } else if(strcmp(type_value, "lifespan.shutdown.complete") == 0) {
        handler_asgi_context->handler->lifespan_shutdown = 1;
    } else if(strcmp(type_value, "lifespan.shutdown.failed") == 0) {
        handler_asgi_context->handler->lifespan_shutdown = 2;
    } else {
        Py_DECREF(future);
        PyErr_Format(PyExc_ValueError, "unexpected message type '%s'", type_value);
        return NULL;
    }

    /* prints the message that has been provided together with the
    failure, it describes the problem faced by the application */
    object = PyDict_GetItemString(message, "message");
    if(object != NULL && (message_value = PyUnicode_AsUTF8(object)) != NULL) {
        V_WARNING_F("Lifespan reported: %s\n", message_value);
    }
    PyErr_Clear();

    /* resolves the future and returns it, no writing is ever
    associated with the messages of the lifespan protocol */
    resolve_future_loop_python(future, Py_None);
    return future;
}

static ERROR_CODE _send_status_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, int status_code, const char *body) {
    /* allocates space for both the index used in the iteration over
    the headers already set by the application and for the header
    carrying the length of the payload of the response */
    size_t index;
    char header[VIRIATUM_MAX_HEADER_C_SIZE];
    SPRINTF(
        header,
        VIRIATUM_MAX_HEADER_C_SIZE,
        "%s: %lu",
        CONTENT_LENGTH_H,
        (long unsigned int) strlen(body)
    );

    /* discards the complete set of headers that have been set by the
    application, none of them describes the error response */
    for(index = 0; index < handler_asgi_context->response_header_count; index++) {
        FREE(handler_asgi_context->response_headers[index]);
    }
    handler_asgi_context->response_header_count = 0;

    /* sets the provided status together with the length of the payload,
    no chunked framing is required for a response of a known size */
    handler_asgi_context->status_code = status_code;
    handler_asgi_context->has_length = TRUE;
    handler_asgi_context->has_body = TRUE;
    handler_asgi_context->response_headers[0] =
        (unsigned char *) MALLOC(strlen(header) + 1);
    STRCPY(
        (char *) handler_asgi_context->response_headers[0],
        strlen(header) + 1,
        header
    );
    handler_asgi_context->response_header_count = 1;

    /* writes both the envelope and the payload of the error response,
    the latter is the one that tears the request down */
    _send_headers_handler_asgi(handler_asgi_context);
    handler_asgi_context->state = ASGI_STATE_COMPLETE;
    _write_handler_asgi(handler_asgi_context, body, strlen(body), TRUE, NULL);

    /* raises no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _send_error_handler_asgi(struct handler_asgi_context_t *handler_asgi_context) {
    /* produces the internal error response, the one used whenever the
    application is unable to produce a response of its own */
    return _send_status_handler_asgi(
        handler_asgi_context,
        500,
        VIRIATUM_ASGI_ERROR_BODY
    );
}

static PyObject *_done_handler_asgi(PyObject *self, PyObject *args) {
    /* allocates space for the various objects used during the
    inspection of the task that has just finished */
    PyObject *task;
    PyObject *object;
    PyObject *exception;
    int is_cancelled;

    /* retrieves the context of the request from the capsule, in case
    it has already been invalidated the request has been torn down
    in the meantime and so nothing is left to be done */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) PyCapsule_GetPointer(self, NULL);
    if(handler_asgi_context == NULL) {
        PyErr_Clear();
        Py_INCREF(Py_None);
        return Py_None;
    }

    /* parses the single task argument of the call, it is the one that
    has just reached its final state */
    if(!PyArg_ParseTuple(args, "O", &task)) {
        PyErr_Clear();
        Py_INCREF(Py_None);
        return Py_None;
    }

    /* a cancelled task is the usual way of ending the handling of a
    request and so nothing is ever reported for it */
    object = PyObject_CallMethod(task, "cancelled", NULL);
    if(object == NULL) { PyErr_Clear(); }
    else {
        is_cancelled = PyObject_IsTrue(object);
        Py_DECREF(object);
        if(is_cancelled != 0) {
            Py_INCREF(Py_None);
            return Py_None;
        }
    }

    /* retrieves the exception that has been raised by the application
    and reports it into the standard error, as the usual reporting */
    exception = PyObject_CallMethod(task, "exception", NULL);
    if(exception == NULL) { PyErr_Clear(); }
    else if(exception != Py_None) {
        PyErr_SetObject((PyObject *) Py_TYPE(exception), exception);
        _report_handler_asgi();
        V_WARNING_F(
            "Problem handling request %s\n",
            handler_asgi_context->url == NULL ?
                (unsigned char *) "" : handler_asgi_context->url
        );
    }
    Py_XDECREF(exception);

    /* the lifespan protocol owns no connection and so no response is
    ever produced for the ending of its task */
    if(handler_asgi_context->lifespan == TRUE) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    /* an upgraded connection carries no http response and so the
    ending of the application closes it through the protocol, a
    handshake that was never accepted is refused instead */
    if(handler_asgi_context->websocket_state != ASGI_WEBSOCKET_NONE) {
        if(handler_asgi_context->websocket_state == ASGI_WEBSOCKET_CONNECTING) {
            handler_asgi_context->websocket_state = ASGI_WEBSOCKET_CLOSED;
            _write_raw_handler_asgi(
                handler_asgi_context,
                (unsigned char *) VIRIATUM_ASGI_REJECTED,
                strlen(VIRIATUM_ASGI_REJECTED),
                TRUE
            );
        } else {
            _close_websocket_handler_asgi(
                handler_asgi_context,
                WEBSOCKET_CLOSE_NORMAL,
                NULL
            );
        }
        Py_INCREF(Py_None);
        return Py_None;
    }

    /* in case the application never started a response one must be
    produced for it, otherwise the client would wait forever */
    if(handler_asgi_context->state == ASGI_STATE_PENDING) {
        _send_error_handler_asgi(handler_asgi_context);
    } else if(handler_asgi_context->state != ASGI_STATE_COMPLETE) {
        /* in case the response has been started but never closed the
        stream is terminated so that the client is released, the
        envelope is written first when no payload ever reached it */
        if(handler_asgi_context->state == ASGI_STATE_STARTED) {
            _send_headers_handler_asgi(handler_asgi_context);
        }
        handler_asgi_context->state = ASGI_STATE_COMPLETE;
        _write_handler_asgi(handler_asgi_context, NULL, 0, TRUE, NULL);
    }

    /* returns the none value as the callback provides no meaningful
    result to the loop that raised it */
    Py_INCREF(Py_None);
    return Py_None;
}

/**
 * The method definition for the callback that is raised once the
 * task running the application reaches its final state.
 */
static PyMethodDef done_method = {
    "done",
    (PyCFunction) _done_handler_asgi,
    METH_VARARGS,
    NULL
};

ERROR_CODE set_handler_asgi(struct http_connection_t *http_connection) {
    /* sets the HTTP parser values */
    _set_http_parser_handler_asgi(http_connection->http_parser);

    /* sets the HTTP settings values */
    _set_http_settings_handler_asgi(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_asgi(struct http_connection_t *http_connection) {
    /* unsets the HTTP parser values */
    _unset_http_parser_handler_asgi(http_connection->http_parser);

    /* unsets the HTTP settings values */
    _unset_http_settings_handler_asgi(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_asgi(struct http_parser_t *http_parser) {
    /* prints a debug message about the request reception */
    V_DEBUG("HTTP request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and then allocates
    space for the url copying the received data into it */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) http_parser->context;
    if(handler_asgi_context->url != NULL) { FREE(handler_asgi_context->url); }
    handler_asgi_context->url = (unsigned char *) MALLOC(data_size + 1);
    memcpy(handler_asgi_context->url, data, data_size);
    handler_asgi_context->url[data_size] = '\0';

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and in case the maximum
    number of headers has been reached ignores the current one */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) http_parser->context;
    if(handler_asgi_context->header_count >= VIRIATUM_ASGI_MAX_HEADERS) { RAISE_NO_ERROR; }

    /* releases any field that is still pending, this happens when a
    header line carries no value and would otherwise be leaked */
    if(handler_asgi_context->header_fields[handler_asgi_context->header_count] != NULL) {
        FREE(handler_asgi_context->header_fields[handler_asgi_context->header_count]);
    }

    /* allocates space for the header field and copies the received
    data into it, the value is set on the following callback */
    handler_asgi_context->header_fields[handler_asgi_context->header_count] =
        (unsigned char *) MALLOC(data_size + 1);
    memcpy(
        handler_asgi_context->header_fields[handler_asgi_context->header_count],
        data,
        data_size
    );
    handler_asgi_context->header_fields[handler_asgi_context->header_count][data_size] = '\0';
    handler_asgi_context->header_values[handler_asgi_context->header_count] = NULL;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and in case the maximum
    number of headers has been reached ignores the current one */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) http_parser->context;
    if(handler_asgi_context->header_count >= VIRIATUM_ASGI_MAX_HEADERS) { RAISE_NO_ERROR; }

    /* in case no field is currently pending the value belongs to a folded
    header line, as the field may not be determined the value is ignored
    instead of closing a pair with an unset field name */
    if(handler_asgi_context->header_fields[handler_asgi_context->header_count] == NULL) {
        RAISE_NO_ERROR;
    }

    /* allocates space for the header value and copies the received
    data into it, then closes the current header pair */
    handler_asgi_context->header_values[handler_asgi_context->header_count] =
        (unsigned char *) MALLOC(data_size + 1);
    memcpy(
        handler_asgi_context->header_values[handler_asgi_context->header_count],
        data,
        data_size
    );
    handler_asgi_context->header_values[handler_asgi_context->header_count][data_size] = '\0';
    handler_asgi_context->header_count++;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_asgi(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and then grows the body
    buffer so that the newly received payload fits into it, note that
    the body is accumulated as the parser may raise this callback
    multiple times for a single request */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) http_parser->context;
    unsigned char *body;
    size_t body_capacity;

    /* in case the payload would exceed the maximum allowed size the
    remaining data is discarded, avoiding an unbounded growth driven
    by the client */
    if(handler_asgi_context->body_size + data_size > VIRIATUM_ASGI_MAX_BODY) {
        handler_asgi_context->overflow = TRUE;
        RAISE_NO_ERROR;
    }

    /* grows the buffer geometrically whenever the payload no longer fits
    it, this keeps the accumulation linear over the various callbacks */
    if(handler_asgi_context->body_size + data_size > handler_asgi_context->body_capacity) {
        body_capacity = handler_asgi_context->body_capacity == 0 ?
            VIRIATUM_ASGI_BODY_CAPACITY : handler_asgi_context->body_capacity;
        while(body_capacity < handler_asgi_context->body_size + data_size) {
            body_capacity *= 2;
        }
        body = (unsigned char *) MALLOC(body_capacity);
        if(handler_asgi_context->body != NULL) {
            memcpy(body, handler_asgi_context->body, handler_asgi_context->body_size);
            FREE(handler_asgi_context->body);
        }
        handler_asgi_context->body = body;
        handler_asgi_context->body_capacity = body_capacity;
    }

    /* copies the newly received payload into the tail of the buffer */
    memcpy(&handler_asgi_context->body[handler_asgi_context->body_size], data, data_size);
    handler_asgi_context->body_size += data_size;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_asgi(struct http_parser_t *http_parser) {
    /* prints a debug message about the request parsing and then
    schedules the application that is going to answer it */
    V_DEBUG("HTTP request parsed\n");
    _call_application_handler_asgi(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_asgi(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_asgi(struct http_parser_t *http_parser) {
    /* allocates space for the context to be used during the
    handling of the request and sets it in the parser */
    struct handler_asgi_context_t *handler_asgi_context;
    create_handler_asgi_context(&handler_asgi_context);
    http_parser->context = (void *) handler_asgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_asgi(struct http_parser_t *http_parser) {
    /* retrieves the context from the parser and in case it's set
    releases it, unsetting the reference afterwards */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) http_parser->context;
    if(handler_asgi_context != NULL) {
        delete_handler_asgi_context(handler_asgi_context);
        http_parser->context = NULL;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_asgi(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the HTTP settings
    structure, these callbacks are going to be used in the runtime
    processing of HTTP parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_asgi;
    http_settings->on_url = url_callback_handler_asgi;
    http_settings->on_header_field = header_field_callback_handler_asgi;
    http_settings->on_header_value = header_value_callback_handler_asgi;
    http_settings->on_headers_complete = headers_complete_callback_handler_asgi;
    http_settings->on_body = body_callback_handler_asgi;
    http_settings->on_message_complete = message_complete_callback_handler_asgi;
    http_settings->on_path = path_callback_handler_asgi;
    http_settings->on_location = location_callback_handler_asgi;
    http_settings->on_virtual_url = virtual_url_callback_handler_asgi;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_asgi(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the HTTP settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;
    http_settings->on_path = NULL;
    http_settings->on_location = NULL;
    http_settings->on_virtual_url = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

static const char *_find_header_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, const char *name) {
    /* iterates over the complete set of headers gathered from the
    request looking for the one with the provided name, the
    comparison is case insensitive as the names are not sensitive */
    size_t index;
    size_t sub_index;
    size_t name_size = strlen(name);
    for(index = 0; index < handler_asgi_context->header_count; index++) {
        if(handler_asgi_context->header_fields[index] == NULL) { continue; }
        if(handler_asgi_context->header_values[index] == NULL) { continue; }
        if(strlen((char *) handler_asgi_context->header_fields[index]) != name_size) { continue; }
        for(sub_index = 0; sub_index < name_size; sub_index++) {
            if(toupper(handler_asgi_context->header_fields[index][sub_index]) !=
                toupper((unsigned char) name[sub_index])) { break; }
        }
        if(sub_index == name_size) {
            return (const char *) handler_asgi_context->header_values[index];
        }
    }
    return NULL;
}

static char _is_upgrade_handler_asgi(struct handler_asgi_context_t *handler_asgi_context) {
    /* retrieves both the upgrade header and the key of the handshake,
    the presence of the latter is what distinguishes a websocket
    upgrade from any other one that may be requested */
    const char *upgrade = _find_header_handler_asgi(handler_asgi_context, "Upgrade");
    const char *key = _find_header_handler_asgi(handler_asgi_context, "Sec-WebSocket-Key");
    size_t index;
    const char *reference = VIRIATUM_ASGI_WEBSOCKET;
    if(upgrade == NULL || key == NULL) { return FALSE; }
    if(strlen(upgrade) != strlen(reference)) { return FALSE; }
    for(index = 0; reference[index] != '\0'; index++) {
        if(toupper((unsigned char) upgrade[index]) !=
            toupper((unsigned char) reference[index])) { return FALSE; }
    }
    return TRUE;
}

static PyObject *_build_scope_handler_asgi(
    struct handler_asgi_context_t *handler_asgi_context,
    struct http_parser_t *http_parser,
    struct connection_t *connection,
    char websocket
) {
    /* allocates space for the various objects that are going to be
    created as part of the scope construction */
    PyObject *scope;
    PyObject *object;
    PyObject *headers;
    PyObject *header;
    size_t index;
    size_t sub_index;

    /* allocates space for the buffers used to split the url into the
    path and the query string parts of it */
    char path[VIRIATUM_MAX_URL_SIZE];
    char name[VIRIATUM_MAX_HEADER_SIZE];
    char version[16];
    char *pointer;
    size_t path_size;
    size_t name_size;

    /* unpacks the service and the associated options from the connection
    as they are required for some of the scope values */
    struct service_t *service = connection->service;
    struct service_options_t *service_options = service->options;

    /* creates the scope map that is going to be populated with the
    complete set of values describing the current request */
    scope = PyDict_New();
    if(scope == NULL) { return NULL; }

    /* splits the url around the get parameters divisor, the first part
    is the path and the remaining one the query string */
    pointer = handler_asgi_context->url == NULL ?
        NULL : strchr((char *) handler_asgi_context->url, '?');
    path_size = handler_asgi_context->url == NULL ? 0 :
        (pointer == NULL ? strlen((char *) handler_asgi_context->url) :
        (size_t) (pointer - (char *) handler_asgi_context->url));
    if(path_size >= VIRIATUM_MAX_URL_SIZE) { path_size = VIRIATUM_MAX_URL_SIZE - 1; }
    if(path_size > 0) { memcpy(path, handler_asgi_context->url, path_size); }
    path[path_size] = '\0';

    /* sets the raw path before the decoding of it, the specification
    defines it as the one that reached the wire */
    object = PyBytes_FromStringAndSize(path, (Py_ssize_t) path_size);
    if(object != NULL) {
        PyDict_SetItemString(scope, "raw_path", object);
        Py_DECREF(object);
    }

    /* decodes the percent escapes of the path in place, the operation is
    run after the query string split so that an encoded separator is not
    able to forge one */
    _decode_handler_asgi(path);

    /* sets the various values that describe both the protocol and the
    interface that is being implemented by the server */
    object = PyUnicode_FromString(websocket == TRUE ? "websocket" : "http");
    if(object != NULL) { PyDict_SetItemString(scope, "type", object); Py_DECREF(object); }
    object = Py_BuildValue(
        "{s:s,s:s}",
        "version", _version_handler_asgi(handler_asgi_context->handler),
        "spec_version", VIRIATUM_ASGI_SPEC_VERSION
    );
    if(object != NULL) { PyDict_SetItemString(scope, "asgi", object); Py_DECREF(object); }

    /* sets the version of the protocol as gathered from the request
    line, it is the one that framed the current request */
    SPRINTF(
        version,
        sizeof(version),
        "%d.%d",
        (int) http_parser->http_major,
        (int) http_parser->http_minor
    );
    object = PyUnicode_FromString(version);
    if(object != NULL) { PyDict_SetItemString(scope, "http_version", object); Py_DECREF(object); }

    /* sets the various request oriented values in the scope, note that
    the root path is always empty as the application owns the routing */
    object = PyUnicode_FromString(path);
    if(object != NULL) { PyDict_SetItemString(scope, "path", object); Py_DECREF(object); }
    object = PyBytes_FromString(pointer == NULL ? "" : pointer + 1);
    if(object != NULL) { PyDict_SetItemString(scope, "query_string", object); Py_DECREF(object); }
    object = PyUnicode_FromString("");
    if(object != NULL) { PyDict_SetItemString(scope, "root_path", object); Py_DECREF(object); }

    /* the method is only part of the scope of the http requests, the
    websocket ones are always the result of a get */
    if(websocket == FALSE) {
        object = PyUnicode_FromString(get_http_method_string(http_parser->method));
        if(object != NULL) { PyDict_SetItemString(scope, "method", object); Py_DECREF(object); }
    }

    /* sets the scheme taking the ssl flag of the service into account
    so that it reflects the real channel of the connection */
    if(websocket == TRUE) {
        object = PyUnicode_FromString(service_options->ssl ? "wss" : "ws");
    } else {
        object = PyUnicode_FromString(service_options->ssl ? "https" : "http");
    }
    if(object != NULL) { PyDict_SetItemString(scope, "scheme", object); Py_DECREF(object); }

    /* sets both the client and the server addresses, they are provided
    as a pair of the host and of the port of each of the peers */
    object = Py_BuildValue("(si)", (char *) connection->host, (int) connection->port);
    if(object != NULL) { PyDict_SetItemString(scope, "client", object); Py_DECREF(object); }
    object = Py_BuildValue("(si)", (char *) service_options->address, (int) service_options->port);
    if(object != NULL) { PyDict_SetItemString(scope, "server", object); Py_DECREF(object); }

    /* creates the sequence of headers of the request, each of them is
    a pair of byte strings with the name lower cased */
    headers = PyList_New(0);
    if(headers == NULL) { Py_DECREF(scope); return NULL; }
    for(index = 0; index < handler_asgi_context->header_count; index++) {
        if(handler_asgi_context->header_fields[index] == NULL) { continue; }
        if(handler_asgi_context->header_values[index] == NULL) { continue; }
        name_size = strlen((char *) handler_asgi_context->header_fields[index]);
        if(name_size >= VIRIATUM_MAX_HEADER_SIZE) { continue; }
        for(sub_index = 0; sub_index < name_size; sub_index++) {
            name[sub_index] = (char) tolower(handler_asgi_context->header_fields[index][sub_index]);
        }
        header = Py_BuildValue(
            "(y#y#)",
            name,
            (Py_ssize_t) name_size,
            (char *) handler_asgi_context->header_values[index],
            (Py_ssize_t) strlen((char *) handler_asgi_context->header_values[index])
        );
        if(header == NULL) { PyErr_Clear(); continue; }
        PyList_Append(headers, header);
        Py_DECREF(header);
    }
    PyDict_SetItemString(scope, "headers", headers);
    Py_DECREF(headers);

    /* sets the sequence of subprotocols that have been proposed by the
    client, only the websocket connections may carry them */
    if(websocket == TRUE) {
        const char *protocols = _find_header_handler_asgi(
            handler_asgi_context,
            "Sec-WebSocket-Protocol"
        );
        object = PyList_New(0);
        if(object != NULL && protocols != NULL) {
            PyObject *value = PyUnicode_FromString(protocols);
            PyObject *split = value == NULL ?
                NULL : PyObject_CallMethod(value, "split", "s", ",");
            Py_XDECREF(value);
            if(split != NULL) {
                Py_ssize_t count = PyList_Size(split);
                Py_ssize_t sub;
                for(sub = 0; sub < count; sub++) {
                    PyObject *item = PyObject_CallMethod(PyList_GetItem(split, sub), "strip", NULL);
                    if(item == NULL) { PyErr_Clear(); continue; }
                    PyList_Append(object, item);
                    Py_DECREF(item);
                }
                Py_DECREF(split);
            }
            PyErr_Clear();
        }
        if(object != NULL) {
            PyDict_SetItemString(scope, "subprotocols", object);
            Py_DECREF(object);
        }
    }

    /* returns the newly created scope map */
    return scope;
}

ERROR_CODE _call_application_handler_asgi(struct http_parser_t *http_parser) {
    /* allocates space for the various python objects used during the
    scheduling of the application for the current request */
    PyObject *scope;
    PyObject *receive;
    PyObject *send;
    PyObject *coroutine;
    PyObject *event;
    PyObject *done;
    PyGILState_STATE gil_state;
    char websocket;

    /* retrieves the connection from the HTTP parser parameters and then
    the underlying connection references in order to operate over them */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* retrieves both the context of the request and the handler that owns
    it, the handler carries the application to be called */
    struct handler_asgi_context_t *handler_asgi_context =
        (struct handler_asgi_context_t *) http_parser->context;
    struct handler_asgi_t *handler_asgi =
        (struct handler_asgi_t *) http_connection->http_handler->lower;

    /* sets the references that are required for the production of the
    response, they are reached from the callables of the application */
    handler_asgi_context->connection = connection;
    handler_asgi_context->handler = handler_asgi;
    handler_asgi_context->flags = (unsigned char) http_parser->flags;

    /* the head requests carry no payload in their response, only the
    envelope of it is ever written into the connection */
    if(http_parser->method == HTTP_HEAD) { handler_asgi_context->has_body = FALSE; }

    /* acquires the global interpreter lock as the complete set of
    operations that follow interact with the interpreter */
    gil_state = PyGILState_Ensure();

    /* verifies if the request is asking for an upgrade of the protocol,
    in which case the websocket scope is the one to be built */
    websocket = _is_upgrade_handler_asgi(handler_asgi_context);
    if(websocket == TRUE) {
        handler_asgi_context->websocket_state = ASGI_WEBSOCKET_CONNECTING;
    }

    /* builds the scope of the request and creates both the receive and
    the send callables, all of them carrying the context */
    scope = _build_scope_handler_asgi(
        handler_asgi_context,
        http_parser,
        connection,
        websocket
    );
    handler_asgi_context->capsule = PyCapsule_New((void *) handler_asgi_context, NULL, NULL);
    receive = PyCFunction_New(&receive_method, handler_asgi_context->capsule);
    send = PyCFunction_New(&send_method, handler_asgi_context->capsule);

    /* queues the first of the events so that the initial receive call
    of the application resolves immediately */
    if(websocket == TRUE) {
        event = Py_BuildValue("{s:s}", "type", "websocket.connect");
    } else {
        event = Py_BuildValue(
            "{s:s,s:y#,s:O}",
            "type", "http.request",
            "body", handler_asgi_context->body == NULL ?
                "" : (char *) handler_asgi_context->body,
            (Py_ssize_t) handler_asgi_context->body_size,
            "more_body", Py_False
        );
    }
    if(event != NULL) {
        _push_event_handler_asgi(handler_asgi_context, event);
        Py_DECREF(event);
    }

    /* calls the application with the scope that has just been built,
    the resulting coroutine is wrapped in a task */
    coroutine = scope == NULL || receive == NULL || send == NULL ? NULL :
        _invoke_handler_asgi(handler_asgi, scope, receive, send);
    Py_XDECREF(scope);
    Py_XDECREF(receive);
    Py_XDECREF(send);
    if(coroutine != NULL) {
        handler_asgi_context->task = create_task_loop_python(
            handler_asgi->loop_python,
            coroutine
        );
        Py_DECREF(coroutine);
    }

    /* acquires the lock on the HTTP connection, this will avoid further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* in case the payload exceeded the maximum allowed size it may not
    be handed to the application, as it would be a truncated one, the
    request is refused instead (payload too large) */
    if(handler_asgi_context->overflow == TRUE) {
        _send_status_handler_asgi(
            handler_asgi_context,
            413,
            VIRIATUM_ASGI_LARGE_BODY
        );
        PyGILState_Release(gil_state);
        RAISE_NO_ERROR;
    }

    /* in case the application refused the request altogether the error
    response is produced for it, nothing else may be done */
    if(handler_asgi_context->task == NULL) {
        _report_handler_asgi();
        _send_error_handler_asgi(handler_asgi_context);
        PyGILState_Release(gil_state);
        RAISE_NO_ERROR;
    }

    /* registers the callback that inspects the task once it reaches
    its final state, both for the reporting and for the completion */
    done = PyCFunction_New(&done_method, handler_asgi_context->capsule);
    if(done != NULL) {
        Py_XDECREF(PyObject_CallMethod(
            handler_asgi_context->task,
            "add_done_callback",
            "O",
            done
        ));
        Py_DECREF(done);
        PyErr_Clear();
    }

    /* releases the global interpreter lock, the task is advanced by
    the iterations of the serving loop from this point on */
    PyGILState_Release(gil_state);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_asgi(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the state of the write that has just been completed
    and unpacks the various values from it */
    struct write_asgi_t *write_asgi = (struct write_asgi_t *) parameters;
    struct write_asgi_t **current;
    struct handler_asgi_context_t *handler_asgi_context = write_asgi->handler_asgi_context;
    unsigned char flags = handler_asgi_context->flags;
    char last = write_asgi->last;
    PyGILState_STATE gil_state;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* unlinks the state from the context, from this point on it is
    no longer released by the destruction of the latter */
    current = &handler_asgi_context->writes;
    while(*current != NULL) {
        if(*current == write_asgi) { *current = write_asgi->next; break; }
        current = &(*current)->next;
    }

    /* resolves the future of the send call that produced the payload,
    this is what applies the back pressure to the application */
    gil_state = PyGILState_Ensure();
    if(write_asgi->future != NULL) {
        resolve_future_loop_python(write_asgi->future, Py_None);
        Py_DECREF(write_asgi->future);
    }
    PyGILState_Release(gil_state);
    FREE(write_asgi);

    /* in case this is not the last part of the response the connection
    must be kept as it is, more payload is still to be written */
    if(last == FALSE) { RAISE_NO_ERROR; }

    /* in case there is an HTTP handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current HTTP connection and then sets the reference
        to it in the HTTP connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive */
    if(!(flags & FLAG_KEEP_ALIVE)) {
        /* closes the connection */
        connection->close_connection(connection);
    } else {
        /* releases the lock on the HTTP connection, this will allow further
        messages to be processed, an update event should raised following this
        lock releasing call */
        http_connection->release(http_connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _close_callback_handler_asgi(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* in case there is an HTTP handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* closes the connection, an upgraded one is never reused as the
    protocol running on top of it has already been terminated */
    connection->close_connection(connection);

    /* raise no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _write_raw_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, const unsigned char *data, size_t data_size, char close) {
    /* allocates space for the buffer that is going to receive the
    payload to be written into the connection */
    char *buffer;
    struct connection_t *connection = handler_asgi_context->connection;

    /* allocates the buffer through the connection so that it is
    released once the payload reaches the wire */
    connection->alloc_data(connection, data_size + 1, (void **) &buffer);
    memcpy(buffer, data, data_size);

    /* writes the payload into the connection, closing it afterwards
    in case that has been requested by the caller */
    connection->write_connection(
        connection,
        (unsigned char *) buffer,
        (unsigned int) data_size,
        close == TRUE ? _close_callback_handler_asgi : NULL,
        NULL
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _close_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, unsigned short code, const char *reason) {
    /* allocates space for the buffer receiving the close frame that
    is going to be written into the connection */
    unsigned char *buffer;
    size_t buffer_size;

    /* in case the connection has already been closed nothing else may
    be written into it (graceful return) */
    if(handler_asgi_context->websocket_state == ASGI_WEBSOCKET_CLOSED) { RAISE_NO_ERROR; }
    handler_asgi_context->websocket_state = ASGI_WEBSOCKET_CLOSED;

    /* builds the close frame and writes it into the connection, the
    closing of it happens once the frame reaches the wire */
    if(IS_ERROR_CODE(build_close_websocket(code, reason, &buffer, &buffer_size))) {
        RAISE_NO_ERROR;
    }
    _write_raw_handler_asgi(handler_asgi_context, buffer, buffer_size, TRUE);
    FREE(buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _accept_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, const char *subprotocol, const char *headers) {
    /* allocates space for both the accept value of the handshake and
    for the buffer receiving the response to be written */
    unsigned char accept_key[VIRIATUM_WEBSOCKET_ACCEPT_SIZE];
    unsigned char buffer[VIRIATUM_HTTP_SIZE];
    size_t count;

    /* retrieves the underlying connection references so that the
    reception of data may be redirected to the frame reader */
    struct connection_t *connection = handler_asgi_context->connection;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;

    /* retrieves the key provided by the client and derives the accept
    value of the handshake from it */
    const char *key = _find_header_handler_asgi(handler_asgi_context, "Sec-WebSocket-Key");
    if(key == NULL) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Request carries no websocket key"
        );
    }
    if(IS_ERROR_CODE(accept_key_websocket((unsigned char *) key, accept_key, sizeof(accept_key)))) {
        RAISE_AGAIN(RUNTIME_EXCEPTION_ERROR_CODE);
    }

    /* formats the response of the handshake, the subprotocol is only
    part of it in case the application selected one */
    count = SPRINTF(
        (char *) buffer,
        VIRIATUM_HTTP_SIZE,
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "%s: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n",
        CONNECTION_H,
        (char *) accept_key
    );
    if(subprotocol != NULL) {
        count += SPRINTF(
            (char *) &buffer[count],
            VIRIATUM_HTTP_SIZE - count,
            "Sec-WebSocket-Protocol: %s\r\n",
            subprotocol
        );
    }

    /* copies the headers that have been set by the application, they
    are already formatted as complete lines by the caller */
    if(headers != NULL) {
        count += SPRINTF(
            (char *) &buffer[count],
            VIRIATUM_HTTP_SIZE - count,
            "%s",
            headers
        );
    }
    memcpy(&buffer[count], "\r\n", 2);
    count += 2;

    /* writes the response of the handshake and redirects the reception
    of data to the reader of frames, the connection is upgraded */
    _write_raw_handler_asgi(handler_asgi_context, buffer, count, FALSE);
    io_connection->on_data = data_handler_websocket_asgi;

    /* raises no error */
    RAISE_NO_ERROR;
}

static PyObject *_send_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, PyObject *message, const char *type_value) {
    /* allocates space for the various objects used during the parsing
    of the message provided by the application */
    PyObject *future;
    PyObject *object;
    PyObject *headers;
    PyObject *header;
    PyObject *name;
    PyObject *value;
    PyObject *encoded = NULL;
    unsigned char *frame;
    size_t frame_size;
    char *data = NULL;
    char *value_data = NULL;
    Py_ssize_t data_size = 0;
    Py_ssize_t value_size = 0;
    Py_ssize_t index;
    Py_ssize_t count;
    const char *subprotocol = NULL;
    const char *reason = NULL;
    char accept_headers[VIRIATUM_HTTP_SIZE];
    size_t accept_size = 0;
    unsigned char opcode = WEBSOCKET_OPCODE_BINARY;
    long code = WEBSOCKET_CLOSE_NORMAL;

    /* creates the future that is going to be awaited by the caller,
    every message of the specification is an awaitable one */
    future = create_future_loop_python(handler_asgi_context->handler->loop_python);
    if(future == NULL) { return NULL; }

    /* handles the acceptance of the connection, it is the message that
    completes the handshake and upgrades the connection */
    if(strcmp(type_value, "websocket.accept") == 0) {
        if(handler_asgi_context->websocket_state != ASGI_WEBSOCKET_CONNECTING) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_RuntimeError, "connection has already been accepted");
            return NULL;
        }
        object = PyDict_GetItemString(message, "subprotocol");
        if(object != NULL && object != Py_None) {
            subprotocol = PyUnicode_AsUTF8(object);
            if(subprotocol == NULL) { Py_DECREF(future); return NULL; }
            if(_is_valid_handler_asgi(subprotocol, strlen(subprotocol)) == FALSE) {
                Py_DECREF(future);
                PyErr_SetString(PyExc_ValueError, "subprotocol carries a control character");
                return NULL;
            }
            if(strlen(subprotocol) >= VIRIATUM_ASGI_MAX_PROTOCOL) {
                Py_DECREF(future);
                PyErr_SetString(PyExc_ValueError, "subprotocol is too long");
                return NULL;
            }
        }
        /* gathers the headers that the application wants to be part of
        the response of the handshake, they are optional and defined
        as a sequence of pairs of byte strings */
        headers = PyDict_GetItemString(message, "headers");
        count = headers == NULL ? 0 : PySequence_Length(headers);
        if(count < 0) { PyErr_Clear(); count = 0; }
        for(index = 0; index < count; index++) {
            header = PySequence_GetItem(headers, index);
            if(header == NULL) { Py_DECREF(future); return NULL; }
            name = PySequence_GetItem(header, 0);
            value = PySequence_GetItem(header, 1);
            Py_DECREF(header);
            if(name == NULL || value == NULL) {
                Py_XDECREF(name);
                Py_XDECREF(value);
                Py_DECREF(future);
                return NULL;
            }
            if(PyBytes_AsStringAndSize(name, &data, &data_size) < 0 ||
                PyBytes_AsStringAndSize(value, &value_data, &value_size) < 0) {
                Py_DECREF(name);
                Py_DECREF(value);
                Py_DECREF(future);
                return NULL;
            }

            /* rejects any control character, as it would allow the
            response of the handshake to be split by the application */
            if(_is_valid_handler_asgi(data, (size_t) data_size) == FALSE ||
                _is_valid_handler_asgi(value_data, (size_t) value_size) == FALSE) {
                Py_DECREF(name);
                Py_DECREF(value);
                Py_DECREF(future);
                PyErr_SetString(PyExc_ValueError, "header carries a control character");
                return NULL;
            }

            /* formats the header line into the buffer, the ones that no
            longer fit it are discarded (avoids an overflow) */
            if(accept_size + (size_t) data_size + (size_t) value_size + 4 <
                VIRIATUM_HTTP_SIZE) {
                accept_size += SPRINTF(
                    &accept_headers[accept_size],
                    VIRIATUM_HTTP_SIZE - accept_size,
                    "%.*s: %.*s\r\n",
                    (int) data_size,
                    data,
                    (int) value_size,
                    value_data
                );
            }
            Py_DECREF(name);
            Py_DECREF(value);
        }
        accept_headers[accept_size] = '\0';

        if(IS_ERROR_CODE(_accept_websocket_handler_asgi(
            handler_asgi_context,
            subprotocol,
            accept_size > 0 ? accept_headers : NULL
        ))) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_RuntimeError, (char *) GET_ERROR());
            return NULL;
        }
        handler_asgi_context->websocket_state = ASGI_WEBSOCKET_CONNECTED;
        resolve_future_loop_python(future, Py_None);
        return future;
    }

    /* handles the sending of a message, it may carry either a textual
    or a binary payload as defined by the specification */
    if(strcmp(type_value, "websocket.send") == 0) {
        if(handler_asgi_context->websocket_state != ASGI_WEBSOCKET_CONNECTED) {
            Py_DECREF(future);
            PyErr_SetString(PyExc_RuntimeError, "connection is not accepted");
            return NULL;
        }
        object = PyDict_GetItemString(message, "text");
        if(object != NULL && object != Py_None) {
            encoded = PyUnicode_AsUTF8String(object);
            if(encoded == NULL) { Py_DECREF(future); return NULL; }
            PyBytes_AsStringAndSize(encoded, &data, &data_size);
            opcode = WEBSOCKET_OPCODE_TEXT;
        } else {
            object = PyDict_GetItemString(message, "bytes");
            if(object != NULL && object != Py_None) {
                if(PyBytes_AsStringAndSize(object, &data, &data_size) < 0) {
                    Py_DECREF(future);
                    return NULL;
                }
            }
        }

        /* builds the frame carrying the payload and writes it into the
        connection, a message is never fragmented by the server */
        if(IS_ERROR_CODE(build_frame_websocket(
            opcode,
            TRUE,
            (unsigned char *) data,
            (size_t) data_size,
            &frame,
            &frame_size
        ))) {
            Py_XDECREF(encoded);
            Py_DECREF(future);
            PyErr_SetString(PyExc_ValueError, "message payload is too large");
            return NULL;
        }
        _write_raw_handler_asgi(handler_asgi_context, frame, frame_size, FALSE);
        FREE(frame);
        Py_XDECREF(encoded);
        resolve_future_loop_python(future, Py_None);
        return future;
    }

    /* handles the closing of the connection, before the handshake it
    is a rejection of it and afterwards a proper closing */
    if(strcmp(type_value, "websocket.close") == 0) {
        object = PyDict_GetItemString(message, "code");
        if(object != NULL) {
            code = PyLong_AsLong(object);
            if(PyErr_Occurred()) { PyErr_Clear(); code = WEBSOCKET_CLOSE_NORMAL; }
        }
        object = PyDict_GetItemString(message, "reason");
        if(object != NULL && object != Py_None) {
            reason = PyUnicode_AsUTF8(object);
            if(reason == NULL) { PyErr_Clear(); }
        }
        if(handler_asgi_context->websocket_state == ASGI_WEBSOCKET_CONNECTING) {
            handler_asgi_context->websocket_state = ASGI_WEBSOCKET_CLOSED;
            _write_raw_handler_asgi(
                handler_asgi_context,
                (unsigned char *) VIRIATUM_ASGI_REJECTED,
                strlen(VIRIATUM_ASGI_REJECTED),
                TRUE
            );
        } else {
            _close_websocket_handler_asgi(
                handler_asgi_context,
                (unsigned short) code,
                reason
            );
        }
        resolve_future_loop_python(future, Py_None);
        return future;
    }

    /* no other message type is defined for the websocket scope and so
    the application is reporting a problem of its own */
    Py_DECREF(future);
    PyErr_Format(PyExc_ValueError, "unexpected message type '%s'", type_value);
    return NULL;
}

static void _deliver_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context) {
    /* allocates space for the various objects used during the creation
    of the event that carries the reassembled message */
    PyObject *event;
    PyObject *object;

    /* creates the payload of the event according to the operation code
    of the first frame of the message that has been reassembled */
    if(handler_asgi_context->websocket_opcode == WEBSOCKET_OPCODE_TEXT) {
        object = PyUnicode_DecodeUTF8(
            (char *) handler_asgi_context->websocket_message,
            (Py_ssize_t) handler_asgi_context->websocket_message_size,
            "strict"
        );

        /* a textual message that is not properly encoded is a violation
        of the protocol and so the connection must be closed */
        if(object == NULL) {
            PyErr_Clear();
            _close_websocket_handler_asgi(
                handler_asgi_context,
                WEBSOCKET_CLOSE_INVALID,
                "invalid utf-8 payload"
            );
            handler_asgi_context->websocket_message_size = 0;
            handler_asgi_context->websocket_opcode = 0;
            return;
        }
        event = Py_BuildValue(
            "{s:s,s:O,s:O}",
            "type", "websocket.receive",
            "text", object,
            "bytes", Py_None
        );
        Py_DECREF(object);
    } else {
        event = Py_BuildValue(
            "{s:s,s:y#,s:O}",
            "type", "websocket.receive",
            "bytes", handler_asgi_context->websocket_message == NULL ?
                "" : (char *) handler_asgi_context->websocket_message,
            (Py_ssize_t) handler_asgi_context->websocket_message_size,
            "text", Py_None
        );
    }

    /* pushes the event so that it reaches the application through the
    receive callable and resets the reassembly state */
    if(event == NULL) { PyErr_Clear(); }
    else {
        _push_event_handler_asgi(handler_asgi_context, event);
        Py_DECREF(event);
    }
    handler_asgi_context->websocket_message_size = 0;
    handler_asgi_context->websocket_opcode = 0;
}

static void _frame_websocket_handler_asgi(struct handler_asgi_context_t *handler_asgi_context, struct websocket_frame_t *websocket_frame) {
    /* allocates space for the objects used during the handling of the
    frame and for the growth of the reassembly buffer */
    PyObject *event;
    unsigned char *message;
    unsigned char *buffer;
    size_t message_capacity;
    size_t frame_size;

    /* answers a ping with the corresponding pong carrying the same
    payload, this happens without the application ever noticing */
    if(websocket_frame->opcode == WEBSOCKET_OPCODE_PING) {
        if(IS_ERROR_CODE(build_frame_websocket(
            WEBSOCKET_OPCODE_PONG,
            TRUE,
            websocket_frame->payload,
            websocket_frame->payload_size,
            &buffer,
            &frame_size
        ))) { return; }
        _write_raw_handler_asgi(handler_asgi_context, buffer, frame_size, FALSE);
        FREE(buffer);
        return;
    }

    /* a pong is either the answer to a ping of the server or an
    unsolicited one, neither of them requires any handling */
    if(websocket_frame->opcode == WEBSOCKET_OPCODE_PONG) { return; }

    /* the closing of the connection is reported to the application and
    answered with the corresponding frame, as the specification
    mandates for the closing handshake */
    if(websocket_frame->opcode == WEBSOCKET_OPCODE_CLOSE) {
        event = Py_BuildValue(
            "{s:s,s:i}",
            "type", "websocket.disconnect",
            "code", (int) close_code_websocket(
                websocket_frame->payload,
                websocket_frame->payload_size
            )
        );
        if(event == NULL) { PyErr_Clear(); }
        else {
            _push_event_handler_asgi(handler_asgi_context, event);
            Py_DECREF(event);
        }
        _close_websocket_handler_asgi(
            handler_asgi_context,
            WEBSOCKET_CLOSE_NORMAL,
            NULL
        );
        return;
    }

    /* a continuation frame may only follow the first frame of a
    message, otherwise there is nothing to be continued */
    if(websocket_frame->opcode == WEBSOCKET_OPCODE_CONTINUATION) {
        if(handler_asgi_context->websocket_opcode == 0) {
            _close_websocket_handler_asgi(
                handler_asgi_context,
                WEBSOCKET_CLOSE_PROTOCOL,
                "unexpected continuation frame"
            );
            return;
        }
    } else {
        /* the first frame of a message may not arrive while another
        one is still being reassembled, they may not be interleaved */
        if(handler_asgi_context->websocket_opcode != 0) {
            _close_websocket_handler_asgi(
                handler_asgi_context,
                WEBSOCKET_CLOSE_PROTOCOL,
                "unexpected data frame"
            );
            return;
        }
        handler_asgi_context->websocket_opcode = websocket_frame->opcode;
    }

    /* in case the message would exceed the maximum allowed size the
    connection is closed, avoiding an unbounded growth of the buffer */
    if(handler_asgi_context->websocket_message_size + websocket_frame->payload_size >
        VIRIATUM_WEBSOCKET_MAX_PAYLOAD) {
        _close_websocket_handler_asgi(
            handler_asgi_context,
            WEBSOCKET_CLOSE_TOO_LARGE,
            "message is too large"
        );
        return;
    }

    /* grows the buffer geometrically whenever the payload no longer fits
    it, this keeps the accumulation linear over the various frames */
    if(handler_asgi_context->websocket_message_size + websocket_frame->payload_size >
        handler_asgi_context->websocket_message_capacity) {
        message_capacity = handler_asgi_context->websocket_message_capacity == 0 ?
            VIRIATUM_ASGI_BODY_CAPACITY : handler_asgi_context->websocket_message_capacity;
        while(message_capacity < handler_asgi_context->websocket_message_size +
            websocket_frame->payload_size) {
            message_capacity *= 2;
        }
        message = (unsigned char *) MALLOC(message_capacity);
        if(handler_asgi_context->websocket_message != NULL) {
            memcpy(
                message,
                handler_asgi_context->websocket_message,
                handler_asgi_context->websocket_message_size
            );
            FREE(handler_asgi_context->websocket_message);
        }
        handler_asgi_context->websocket_message = message;
        handler_asgi_context->websocket_message_capacity = message_capacity;
    }

    /* copies the payload of the frame into the tail of the buffer that
    is reassembling the complete message */
    if(websocket_frame->payload_size > 0) {
        memcpy(
            &handler_asgi_context->websocket_message[handler_asgi_context->websocket_message_size],
            websocket_frame->payload,
            websocket_frame->payload_size
        );
        handler_asgi_context->websocket_message_size += websocket_frame->payload_size;
    }

    /* in case this is the final frame of the message it may be handed
    to the application, otherwise more frames are still expected */
    if(websocket_frame->fin == TRUE) {
        _deliver_websocket_handler_asgi(handler_asgi_context);
    }
}

ERROR_CODE data_handler_websocket_asgi(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the frame that is parsed out of the buffer
    together with the offset of the already consumed part of it */
    struct websocket_frame_t websocket_frame;
    struct handler_asgi_context_t *handler_asgi_context;
    ERROR_CODE error;
    PyGILState_STATE gil_state;
    unsigned char *websocket_buffer;
    size_t websocket_buffer_capacity;
    size_t offset = 0;

    /* retrieves the context of the connection from the parser, it is
    the one that survives the upgrade of the connection */
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    if(http_connection == NULL || http_connection->http_parser == NULL) { RAISE_NO_ERROR; }
    handler_asgi_context =
        (struct handler_asgi_context_t *) http_connection->http_parser->context;
    if(handler_asgi_context == NULL) { RAISE_NO_ERROR; }
    if(buffer == NULL || buffer_size == 0) { RAISE_NO_ERROR; }

    /* grows the buffer geometrically whenever the received data no longer
    fits it, it holds at most one incomplete frame at a time */
    if(handler_asgi_context->websocket_buffer_size + buffer_size >
        handler_asgi_context->websocket_buffer_capacity) {
        websocket_buffer_capacity = handler_asgi_context->websocket_buffer_capacity == 0 ?
            VIRIATUM_ASGI_BODY_CAPACITY : handler_asgi_context->websocket_buffer_capacity;
        while(websocket_buffer_capacity < handler_asgi_context->websocket_buffer_size + buffer_size) {
            websocket_buffer_capacity *= 2;
        }
        websocket_buffer = (unsigned char *) MALLOC(websocket_buffer_capacity);
        if(handler_asgi_context->websocket_buffer != NULL) {
            memcpy(
                websocket_buffer,
                handler_asgi_context->websocket_buffer,
                handler_asgi_context->websocket_buffer_size
            );
            FREE(handler_asgi_context->websocket_buffer);
        }
        handler_asgi_context->websocket_buffer = websocket_buffer;
        handler_asgi_context->websocket_buffer_capacity = websocket_buffer_capacity;
    }

    /* copies the newly received data into the tail of the buffer */
    memcpy(
        &handler_asgi_context->websocket_buffer[handler_asgi_context->websocket_buffer_size],
        buffer,
        buffer_size
    );
    handler_asgi_context->websocket_buffer_size += buffer_size;

    /* acquires the global interpreter lock as the handling of the
    various frames interacts with the interpreter */
    gil_state = PyGILState_Ensure();

    /* consumes as many complete frames as the buffer carries, an
    incomplete one is kept for the following reception */
    while(offset < handler_asgi_context->websocket_buffer_size) {
        error = parse_frame_websocket(
            &handler_asgi_context->websocket_buffer[offset],
            handler_asgi_context->websocket_buffer_size - offset,
            &websocket_frame
        );

        /* a malformed frame is a violation of the protocol and so the
        connection must be closed reporting it to the peer */
        if(IS_ERROR_CODE(error)) {
            _close_websocket_handler_asgi(
                handler_asgi_context,
                WEBSOCKET_CLOSE_PROTOCOL,
                (char *) GET_ERROR()
            );
            offset = handler_asgi_context->websocket_buffer_size;
            break;
        }

        /* an incomplete frame is kept in the buffer so that it may be
        parsed once the remaining part of it is received */
        if(websocket_frame.size == 0) { break; }
        _frame_websocket_handler_asgi(handler_asgi_context, &websocket_frame);
        offset += websocket_frame.size;
        if(handler_asgi_context->websocket_state == ASGI_WEBSOCKET_CLOSED) {
            offset = handler_asgi_context->websocket_buffer_size;
            break;
        }
    }

    /* releases the global interpreter lock and compacts the buffer so
    that only the incomplete frame remains in it */
    PyGILState_Release(gil_state);
    if(offset > 0) {
        handler_asgi_context->websocket_buffer_size -= offset;
        if(handler_asgi_context->websocket_buffer_size > 0) {
            memmove(
                handler_asgi_context->websocket_buffer,
                &handler_asgi_context->websocket_buffer[offset],
                handler_asgi_context->websocket_buffer_size
            );
        }
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
