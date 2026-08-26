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

#include "handler.h"

static PyObject *_start_response_handler_python(PyObject *self, PyObject *args);

/**
 * The method definition for the start response callable that
 * is handed to the application, the context of the request is
 * carried in the self argument through a capsule.
 */
static PyMethodDef start_response_method = {
    "start_response",
    (PyCFunction) _start_response_handler_python,
    METH_VARARGS,
    NULL
};

ERROR_CODE create_handler_python_context(struct handler_python_context_t **handler_python_context_pointer) {
    /* retrieves the context size and allocates space for it, then
    resets the complete set of values so that no invalid reference
    is kept in the newly created structure */
    size_t context_size = sizeof(struct handler_python_context_t);
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) MALLOC(context_size);
    memset(handler_python_context, 0, context_size);

    /* sets the context in the context pointer */
    *handler_python_context_pointer = handler_python_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_python_context(struct handler_python_context_t *handler_python_context) {
    /* allocates space for the index to be used in the
    iteration over the various header sequences */
    size_t index;

    /* releases the url and the body values in case they
    have been set during the parsing of the request */
    if(handler_python_context->url != NULL) { FREE(handler_python_context->url); }
    if(handler_python_context->body != NULL) { FREE(handler_python_context->body); }
    if(handler_python_context->status_message != NULL) {
        FREE(handler_python_context->status_message);
    }

    /* releases the various request headers that have been
    gathered from the request */
    for(index = 0; index < handler_python_context->header_count; index++) {
        FREE(handler_python_context->header_fields[index]);
        FREE(handler_python_context->header_values[index]);
    }

    /* releases the field of an incomplete header pair, one that has
    never been closed by the corresponding value callback */
    if(handler_python_context->header_count < VIRIATUM_PYTHON_MAX_HEADERS &&
        handler_python_context->header_fields[handler_python_context->header_count] != NULL) {
        FREE(handler_python_context->header_fields[handler_python_context->header_count]);
    }

    /* releases the various response headers that have been
    set by the application */
    for(index = 0; index < handler_python_context->response_header_count; index++) {
        FREE(handler_python_context->response_headers[index]);
    }

    /* releases the context structure itself */
    FREE(handler_python_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_handler_python(struct service_t *service, PyObject *application) {
    /* allocates the HTTP handler and the python handler that is
    going to be kept as the lower substrate of it */
    struct http_handler_t *http_handler;
    size_t handler_size = sizeof(struct handler_python_t);
    struct handler_python_t *handler_python =
        (struct handler_python_t *) MALLOC(handler_size);

    /* increments the reference count of the application so that it
    remains valid for the complete lifetime of the handler */
    Py_INCREF(application);
    handler_python->application = application;

    /* creates the HTTP handler and sets its attributes, note that
    no index resolution is performed as the application is the one
    responsible for the complete routing of the requests */
    service->create_http_handler(service, &http_handler, VIRIATUM_PYTHON_HANDLER_NAME);
    http_handler->resolve_index = FALSE;
    http_handler->set = set_handler_python;
    http_handler->unset = unset_handler_python;
    http_handler->reset = NULL;
    http_handler->lower = (void *) handler_python;
    handler_python->http_handler = http_handler;

    /* adds the HTTP handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_python(struct service_t *service) {
    /* allocates the HTTP handler and the associated lower
    substrate to be released */
    struct http_handler_t *http_handler;
    struct handler_python_t *handler_python;

    /* retrieves the HTTP handler from the service, in case none is
    found returns immediately (nothing to be unregistered) */
    service->get_http_handler(service, &http_handler, VIRIATUM_PYTHON_HANDLER_NAME);
    if(http_handler == NULL) { RAISE_NO_ERROR; }

    /* releases the reference to the application and then releases
    the lower substrate structure itself */
    handler_python = (struct handler_python_t *) http_handler->lower;
    if(handler_python != NULL) {
        Py_XDECREF(handler_python->application);
        FREE(handler_python);
    }

    /* removes the HTTP handler from the service and then
    deletes the handler structure */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_python(struct http_connection_t *http_connection) {
    /* sets the HTTP parser values */
    _set_http_parser_handler_python(http_connection->http_parser);

    /* sets the HTTP settings values */
    _set_http_settings_handler_python(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_python(struct http_connection_t *http_connection) {
    /* unsets the HTTP parser values */
    _unset_http_parser_handler_python(http_connection->http_parser);

    /* unsets the HTTP settings values */
    _unset_http_settings_handler_python(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_python(struct http_parser_t *http_parser) {
    /* prints a debug message about the request reception */
    V_DEBUG("HTTP request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and then allocates
    space for the url copying the received data into it */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) http_parser->context;
    if(handler_python_context->url != NULL) { FREE(handler_python_context->url); }
    handler_python_context->url = (unsigned char *) MALLOC(data_size + 1);
    memcpy(handler_python_context->url, data, data_size);
    handler_python_context->url[data_size] = '\0';

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and in case the maximum
    number of headers has been reached ignores the current one */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) http_parser->context;
    if(handler_python_context->header_count >= VIRIATUM_PYTHON_MAX_HEADERS) { RAISE_NO_ERROR; }

    /* releases any field that is still pending, this happens when a
    header line carries no value and would otherwise be leaked */
    if(handler_python_context->header_fields[handler_python_context->header_count] != NULL) {
        FREE(handler_python_context->header_fields[handler_python_context->header_count]);
    }

    /* allocates space for the header field and copies the received
    data into it, the value is set on the following callback */
    handler_python_context->header_fields[handler_python_context->header_count] =
        (unsigned char *) MALLOC(data_size + 1);
    memcpy(
        handler_python_context->header_fields[handler_python_context->header_count],
        data,
        data_size
    );
    handler_python_context->header_fields[handler_python_context->header_count][data_size] = '\0';
    handler_python_context->header_values[handler_python_context->header_count] = NULL;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and in case the maximum
    number of headers has been reached ignores the current one */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) http_parser->context;
    if(handler_python_context->header_count >= VIRIATUM_PYTHON_MAX_HEADERS) { RAISE_NO_ERROR; }

    /* in case no field is currently pending the value belongs to a folded
    header line, as the field may not be determined the value is ignored
    instead of closing a pair with an unset field name */
    if(handler_python_context->header_fields[handler_python_context->header_count] == NULL) {
        RAISE_NO_ERROR;
    }

    /* allocates space for the header value and copies the received
    data into it, then closes the current header pair */
    handler_python_context->header_values[handler_python_context->header_count] =
        (unsigned char *) MALLOC(data_size + 1);
    memcpy(
        handler_python_context->header_values[handler_python_context->header_count],
        data,
        data_size
    );
    handler_python_context->header_values[handler_python_context->header_count][data_size] = '\0';
    handler_python_context->header_count++;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_python(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the context from the parser and then grows the body
    buffer so that the newly received payload fits into it, note that
    the body is accumulated as the parser may raise this callback
    multiple times for a single request */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) http_parser->context;
    unsigned char *body = (unsigned char *) MALLOC(handler_python_context->body_size + data_size);
    if(handler_python_context->body != NULL) {
        memcpy(body, handler_python_context->body, handler_python_context->body_size);
        FREE(handler_python_context->body);
    }
    memcpy(&body[handler_python_context->body_size], data, data_size);
    handler_python_context->body = body;
    handler_python_context->body_size += data_size;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_python(struct http_parser_t *http_parser) {
    /* prints a debug message about the request parsing and then
    sends (and creates) the response for it */
    V_DEBUG("HTTP request parsed\n");
    _send_response_handler_python(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_python(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_python(struct http_parser_t *http_parser) {
    /* allocates space for the context to be used during the
    handling of the request and sets it in the parser */
    struct handler_python_context_t *handler_python_context;
    create_handler_python_context(&handler_python_context);
    http_parser->context = (void *) handler_python_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_python(struct http_parser_t *http_parser) {
    /* retrieves the context from the parser and in case it's set
    releases it, unsetting the reference afterwards */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) http_parser->context;
    if(handler_python_context != NULL) {
        delete_handler_python_context(handler_python_context);
        http_parser->context = NULL;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_python(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the HTTP settings
    structure, these callbacks are going to be used in the runtime
    processing of HTTP parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_python;
    http_settings->on_url = url_callback_handler_python;
    http_settings->on_header_field = header_field_callback_handler_python;
    http_settings->on_header_value = header_value_callback_handler_python;
    http_settings->on_headers_complete = headers_complete_callback_handler_python;
    http_settings->on_body = body_callback_handler_python;
    http_settings->on_message_complete = message_complete_callback_handler_python;
    http_settings->on_path = path_callback_handler_python;
    http_settings->on_location = location_callback_handler_python;
    http_settings->on_virtual_url = virtual_url_callback_handler_python;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_python(struct http_settings_t *http_settings) {
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

static void _set_environ_handler_python(PyObject *environ_map, const char *key, const char *value) {
    /* creates the unicode object from the provided value using the latin 1
    codec, as mandated by the WSGI specification for the native strings, and
    then sets it in the environ_map releasing the local reference afterwards */
    PyObject *object = PyUnicode_DecodeLatin1(value, strlen(value), "replace");
    if(object == NULL) { PyErr_Clear(); return; }
    PyDict_SetItemString(environ_map, key, object);
    Py_DECREF(object);
}

static void _set_environ_header_handler_python(PyObject *environ_map, const char *field, const char *value) {
    /* allocates space for the name of the header to be created and for
    the index to be used in the iteration over the field characters */
    char name[VIRIATUM_MAX_HEADER_SIZE];
    size_t index;
    size_t offset;
    size_t field_size = strlen(field);

    /* the content type and the content length headers are the only ones
    that are not prefixed, as required by the WSGI specification */
    char is_content = strcasecmp(field, "Content-Type") == 0 ||
                      strcasecmp(field, "Content-Length") == 0;

    /* in case the resulting name does not fit the buffer the header must
    be discarded, avoiding an overflow of the name buffer */
    offset = is_content ? 0 : 5;
    if(field_size + offset + 1 > VIRIATUM_MAX_HEADER_SIZE) { return; }
    if(offset > 0) { memcpy(name, "HTTP_", 5); }

    /* converts the header name into the upper case underscore separated
    form that is used by the WSGI environ_map keys */
    for(index = 0; index < field_size; index++) {
        name[offset + index] = field[index] == '-' ?
            '_' : (char) toupper((unsigned char) field[index]);
    }
    name[offset + field_size] = '\0';

    /* sets the resulting header value in the environ_map map */
    _set_environ_handler_python(environ_map, name, value);
}

static PyObject *_build_environ_handler_python(
    struct handler_python_context_t *handler_python_context,
    struct http_parser_t *http_parser,
    struct connection_t *connection
) {
    /* allocates space for the various objects that are going to be
    created as part of the environ_map construction */
    PyObject *environ_map;
    PyObject *object;
    PyObject *io_module;
    size_t index;

    /* allocates space for the buffers used to split the url into the
    path and the query string parts of it */
    char path[VIRIATUM_MAX_URL_SIZE];
    char port[64];
    char *pointer;
    size_t path_size;

    /* unpacks the service and the associated options from the connection
    as they are required for some of the environ_map values */
    struct service_t *service = connection->service;
    struct service_options_t *service_options = service->options;

    /* creates the environ_map map that is going to be populated with the
    complete set of values describing the current request */
    environ_map = PyDict_New();
    if(environ_map == NULL) { return NULL; }

    /* splits the url around the get parameters divisor, the first part
    is the path and the remaining one the query string */
    pointer = handler_python_context->url == NULL ?
        NULL : strchr((char *) handler_python_context->url, '?');
    path_size = handler_python_context->url == NULL ? 0 :
        (pointer == NULL ? strlen((char *) handler_python_context->url) :
        (size_t) (pointer - (char *) handler_python_context->url));
    if(path_size >= VIRIATUM_MAX_URL_SIZE) { path_size = VIRIATUM_MAX_URL_SIZE - 1; }
    if(path_size > 0) { memcpy(path, handler_python_context->url, path_size); }
    path[path_size] = '\0';

    /* sets the various request oriented values in the environ_map, note that
    the script name is always empty as the application owns the routing */
    _set_environ_handler_python(environ_map, "REQUEST_METHOD", get_http_method_string(http_parser->method));
    _set_environ_handler_python(environ_map, "SCRIPT_NAME", "");
    _set_environ_handler_python(environ_map, "PATH_INFO", path);
    _set_environ_handler_python(environ_map, "QUERY_STRING", pointer == NULL ? "" : pointer + 1);
    _set_environ_handler_python(environ_map, "SERVER_PROTOCOL", "HTTP/1.1");
    _set_environ_handler_python(environ_map, "SERVER_SOFTWARE", VIRIATUM_AGENT);
    _set_environ_handler_python(environ_map, "SERVER_NAME", (char *) service_options->address);
    _set_environ_handler_python(environ_map, "REMOTE_ADDR", (char *) connection->host);
    SPRINTF(port, 64, "%d", (int) service_options->port);
    _set_environ_handler_python(environ_map, "SERVER_PORT", port);

    /* sets the various headers gathered from the request in the environ_map
    using the prefixed and upper cased name form */
    for(index = 0; index < handler_python_context->header_count; index++) {
        if(handler_python_context->header_fields[index] == NULL) { continue; }
        if(handler_python_context->header_values[index] == NULL) { continue; }
        _set_environ_header_handler_python(
            environ_map,
            (char *) handler_python_context->header_fields[index],
            (char *) handler_python_context->header_values[index]
        );
    }

    /* sets the WSGI specific values, the url scheme takes the ssl flag
    of the service into account so that it reflects the real channel */
    object = Py_BuildValue("(ii)", 1, 0);
    PyDict_SetItemString(environ_map, "wsgi.version", object);
    Py_DECREF(object);
    _set_environ_handler_python(environ_map, "wsgi.url_scheme", service_options->ssl ? "https" : "http");
    PyDict_SetItemString(environ_map, "wsgi.multithread", Py_False);
    PyDict_SetItemString(environ_map, "wsgi.multiprocess", Py_False);
    PyDict_SetItemString(environ_map, "wsgi.run_once", Py_False);

    /* creates the input stream from the payload of the request using a
    bytes io object, this provides the complete file like interface */
    io_module = PyImport_ImportModule("io");
    if(io_module == NULL) { Py_DECREF(environ_map); return NULL; }
    object = PyObject_CallMethod(
        io_module,
        "BytesIO",
        "y#",
        handler_python_context->body == NULL ? "" : (char *) handler_python_context->body,
        (Py_ssize_t) handler_python_context->body_size
    );
    Py_DECREF(io_module);
    if(object == NULL) { Py_DECREF(environ_map); return NULL; }
    PyDict_SetItemString(environ_map, "wsgi.input", object);
    Py_DECREF(object);

    /* sets the error stream as the standard error of the interpreter so
    that the application may report its problems in the usual way */
    object = PySys_GetObject("stderr");
    if(object != NULL) { PyDict_SetItemString(environ_map, "wsgi.errors", object); }

    /* returns the newly created environ_map map */
    return environ_map;
}

static PyObject *_start_response_handler_python(PyObject *self, PyObject *args) {
    /* allocates space for the various objects used during the parsing
    of the status and of the headers sequence */
    PyObject *status;
    PyObject *headers;
    PyObject *exc_info = NULL;
    PyObject *header;
    PyObject *name;
    PyObject *value;
    const char *status_value;
    const char *name_value;
    const char *value_value;
    char *pointer;
    size_t index;
    size_t count;
    size_t header_size;

    /* retrieves the context of the request from the capsule that has
    been set as the self value of the callable */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) PyCapsule_GetPointer(self, NULL);
    if(handler_python_context == NULL) { return NULL; }

    /* parses the arguments of the call, the exception information is
    optional as defined by the WSGI specification */
    if(!PyArg_ParseTuple(args, "OO|O", &status, &headers, &exc_info)) { return NULL; }

    /* in case the exception information is set and the response has
    already been started the exception must be re raised, otherwise the
    previously set headers are discarded and the response restarted */
    if(exc_info != NULL && exc_info != Py_None) {
        if(handler_python_context->started == TRUE) {
            PyObject *type;
            PyObject *object;
            PyObject *traceback;
            if(!PyArg_ParseTuple(exc_info, "OOO", &type, &object, &traceback)) { return NULL; }
            Py_INCREF(type);
            Py_INCREF(object);
            Py_INCREF(traceback);
            PyErr_Restore(type, object, traceback);
            return NULL;
        }
        for(index = 0; index < handler_python_context->response_header_count; index++) {
            FREE(handler_python_context->response_headers[index]);
        }
        handler_python_context->response_header_count = 0;
    }

    /* retrieves the status line as a string value and then splits it
    around the first space, the first part is the code and the second
    one the complete (possibly multi word) status message */
    status_value = PyUnicode_AsUTF8(status);
    if(status_value == NULL) { return NULL; }
    handler_python_context->status_code = atoi(status_value);
    pointer = strchr(status_value, ' ');
    if(handler_python_context->status_message != NULL) {
        FREE(handler_python_context->status_message);
    }
    handler_python_context->status_message =
        (unsigned char *) MALLOC(strlen(status_value) + 1);
    STRCPY(
        (char *) handler_python_context->status_message,
        strlen(status_value) + 1,
        pointer == NULL ? "" : pointer + 1
    );

    /* iterates over the complete set of headers provided by the
    application storing them as complete header lines */
    count = (size_t) PySequence_Length(headers);
    for(index = 0; index < count; index++) {
        /* in case the maximum number of headers has been reached the
        remaining ones must be discarded (avoids overflow) */
        if(handler_python_context->response_header_count >= VIRIATUM_PYTHON_MAX_HEADERS) { break; }

        header = PySequence_GetItem(headers, index);
        if(header == NULL) { return NULL; }
        name = PySequence_GetItem(header, 0);
        value = PySequence_GetItem(header, 1);
        Py_DECREF(header);
        if(name == NULL || value == NULL) {
            Py_XDECREF(name);
            Py_XDECREF(value);
            return NULL;
        }

        name_value = PyUnicode_AsUTF8(name);
        value_value = PyUnicode_AsUTF8(value);
        if(name_value == NULL || value_value == NULL) {
            Py_DECREF(name);
            Py_DECREF(value);
            return NULL;
        }

        /* allocates space for the complete header line and formats it
        into the newly allocated buffer (name and value pair) */
        header_size = strlen(name_value) + strlen(value_value) + 3;
        handler_python_context->response_headers[handler_python_context->response_header_count] =
            (unsigned char *) MALLOC(header_size);
        SPRINTF(
            (char *) handler_python_context->response_headers[handler_python_context->response_header_count],
            header_size,
            "%s: %s",
            name_value,
            value_value
        );
        handler_python_context->response_header_count++;

        Py_DECREF(name);
        Py_DECREF(value);
    }

    /* marks the response as started so that a latter call carrying the
    exception information is able to detect it */
    handler_python_context->started = TRUE;

    /* returns the write callable, that is unsupported and so returns
    the none value, applications should use the iterable instead */
    Py_INCREF(Py_None);
    return Py_None;
}

ERROR_CODE _send_response_handler_python(struct http_parser_t *http_parser) {
    /* allocates space for the various python objects used during the
    calling of the application and the gathering of its response */
    PyObject *environ_map;
    PyObject *capsule;
    PyObject *start_response;
    PyObject *result;
    PyObject *list;
    PyObject *empty;
    PyObject *body_object = NULL;
    PyGILState_STATE gil_state;

    /* allocates space for the buffer holding the complete response and
    for the counter of the bytes already written into it */
    char *buffer;
    char *body_data = NULL;
    Py_ssize_t body_size = 0;
    size_t count;
    size_t index;
    size_t headers_size;

    /* retrieves the connection from the HTTP parser parameters and then
    the underlying connection references in order to operate over them */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* retrieves both the context of the request and the handler that owns
    it, the handler carries the application to be called */
    struct handler_python_context_t *handler_python_context =
        (struct handler_python_context_t *) http_parser->context;
    struct handler_python_t *handler_python =
        (struct handler_python_t *) http_connection->http_handler->lower;

    /* acquires the global interpreter lock as the complete set of
    operations that follow interact with the interpreter */
    gil_state = PyGILState_Ensure();

    /* defaults the response status to an internal error so that a
    failure in the application is properly reported */
    handler_python_context->status_code = 500;

    /* builds the environ_map map for the request and creates the start
    response callable carrying the context of the request */
    environ_map = _build_environ_handler_python(handler_python_context, http_parser, connection);
    if(environ_map == NULL) {
        PyErr_Print();
        result = NULL;
    } else {
        capsule = PyCapsule_New((void *) handler_python_context, NULL, NULL);
        start_response = PyCFunction_New(&start_response_method, capsule);
        result = PyObject_CallFunctionObjArgs(
            handler_python->application,
            environ_map,
            start_response,
            NULL
        );
        Py_XDECREF(start_response);
        Py_XDECREF(capsule);
        Py_DECREF(environ_map);
    }

    /* in case the application raised an exception prints it into the
    standard error and keeps the internal error status */
    if(result == NULL) {
        PyErr_Print();
        V_WARNING_F(
            "Problem handling request %s\n",
            handler_python_context->url == NULL ?
                (unsigned char *) "" : handler_python_context->url
        );
    } else {
        /* joins the complete set of items of the resulting iterable into
        a single bytes object, this avoids the need for a chunked based
        response at the cost of buffering the complete payload */
        list = PySequence_List(result);
        if(list != NULL) {
            empty = PyBytes_FromStringAndSize("", 0);
            body_object = PyObject_CallMethod(empty, "join", "O", list);
            Py_DECREF(empty);
            Py_DECREF(list);
        }
        if(body_object == NULL) {
            PyErr_Print();
            handler_python_context->status_code = 500;
        } else {
            PyBytes_AsStringAndSize(body_object, &body_data, &body_size);
        }

        /* closes the iterable in case it provides the close method, as
        required by the WSGI specification for resource cleanup */
        if(PyObject_HasAttrString(result, "close")) {
            PyObject *closed = PyObject_CallMethod(result, "close", NULL);
            if(closed == NULL) { PyErr_Clear(); } else { Py_DECREF(closed); }
        }
        Py_DECREF(result);
    }

    /* calculates the amount of space taken by the status message and by
    the headers set by the application, these are unbounded in size and
    so they must be accounted for in the allocation of the buffer */
    headers_size = handler_python_context->status_message == NULL ?
        0 : strlen((char *) handler_python_context->status_message);
    for(index = 0; index < handler_python_context->response_header_count; index++) {
        headers_size += strlen((char *) handler_python_context->response_headers[index]) + 2;
    }

    /* allocates space for the complete response, the default headers are
    bounded by the maximum HTTP size and both the application headers and
    the body take the remaining part of the buffer */
    connection->alloc_data(
        connection,
        VIRIATUM_HTTP_MAX_SIZE + headers_size + (size_t) body_size,
        (void **) &buffer
    );

    /* writes the default set of headers into the buffer, the connection
    is kept alive according to the flags of the current request */
    count = http_connection->write_headers(
        connection,
        buffer,
        VIRIATUM_HTTP_SIZE,
        HTTP11,
        handler_python_context->status_code,
        handler_python_context->status_message == NULL ?
            "Internal Server Error" : (char *) handler_python_context->status_message,
        http_parser->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
        FALSE
    );

    /* writes the content length header with the size of the payload
    that has been gathered from the application response */
    count += SPRINTF(
        &buffer[count],
        VIRIATUM_MAX_HEADER_C_SIZE,
        "%s: %lu\r\n",
        CONTENT_LENGTH_H,
        (long unsigned int) body_size
    );

    /* iterates over the complete set of headers set by the application
    copying each one of them into the headers buffer */
    for(index = 0; index < handler_python_context->response_header_count; index++) {
        count += SPRINTF(
            &buffer[count],
            VIRIATUM_MAX_HEADER_C_SIZE,
            "%s\r\n",
            handler_python_context->response_headers[index]
        );
    }

    /* closes the headers part of the envelope and then copies the
    complete payload into the remaining part of the buffer */
    memcpy(&buffer[count], "\r\n", 2);
    count += 2;
    if(body_size > 0) { memcpy(&buffer[count], body_data, (size_t) body_size); }
    count += (size_t) body_size;

    /* releases the reference to the payload object and then releases
    the global interpreter lock, no more interpreter usage */
    Py_XDECREF(body_object);
    PyGILState_Release(gil_state);

    /* acquires the lock on the HTTP connection, this will avoid further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* writes the complete response into the connection, registering the
    appropriate callback for the cleanup of the connection */
    connection->write_connection(
        connection,
        (unsigned char *) buffer,
        (unsigned int) count,
        _send_response_callback_handler_python,
        (void *) (size_t) http_parser->flags
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_python(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current HTTP flags */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

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
