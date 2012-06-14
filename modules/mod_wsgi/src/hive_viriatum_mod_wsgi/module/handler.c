/*
 Hive Viriatum Modules
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "handler.h"

ERROR_CODE create_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t **mod_wsgi_http_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the mod wsgi http handler size */
    size_t mod_wsgi_http_handler_size = sizeof(struct mod_wsgi_http_handler_t);

    /* allocates space for the mod wsgi http handler */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler = (struct mod_wsgi_http_handler_t *) MALLOC(mod_wsgi_http_handler_size);

    /* sets the mod wsgi http handler in the upper http handler substrate */
    http_handler->lower = (void *) mod_wsgi_http_handler;

    /* sets the mod wsgi http handler in the mod wsgi http handler pointer */
    *mod_wsgi_http_handler_pointer = mod_wsgi_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* releases the mod wsgi http handler */
    FREE(mod_wsgi_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_wsgi_context(struct handler_wsgi_context_t **handler_wsgi_context_pointer) {
    /* retrieves the handler wsgi context size */
    size_t handler_wsgi_context_size = sizeof(struct handler_wsgi_context_t);

    /* allocates space for the handler wsgi context */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) MALLOC(handler_wsgi_context_size);

    /* set the default value in the handler wsgi context
    structure (resets the structure) */
    handler_wsgi_context->flags = 0;
    handler_wsgi_context->iterator = NULL;

    /* sets the handler wsgi context in the  pointer */
    *handler_wsgi_context_pointer = handler_wsgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_wsgi_context(struct handler_wsgi_context_t *handler_wsgi_context) {
    /* in case the iterator in the wsgi is not null it must be
    released by decrementing the reference count */
    if(handler_wsgi_context->iterator != NULL) { Py_DECREF(handler_wsgi_context->iterator); }

    /* releases the handler wsgi context memory */
    FREE(handler_wsgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_module(struct http_connection_t *http_connection) {
    /* sets the http parser values */
    _set_http_parser_handler_module(http_connection->http_parser);

    /* sets the http settings values */
    _set_http_settings_handler_module(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_module(struct http_connection_t *http_connection) {
    /* unsets the http parser values */
    _unset_http_parser_handler_module(http_connection->http_parser);

    /* unsets the http settings values */
    _unset_http_settings_handler_module(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_module(struct http_parser_t *http_parser) {
    /* sends (and creates) the reponse */
    _send_response_handler_module(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* allocates space for the handler wsgi context and
    then creates and populates the instance after that
    sets the handler file context as the context for
    the http parser*/
    struct handler_wsgi_context_t *handler_wsgi_context;
    create_handler_wsgi_context(&handler_wsgi_context);
    http_parser->context = handler_wsgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* retrieves the handler wsgi context from the http parser
    and then deletes (releases memory) */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;
    delete_handler_wsgi_context(handler_wsgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_module(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_module;
    http_settings->on_url = url_callback_handler_module;
    http_settings->on_header_field = header_field_callback_handler_module;
    http_settings->on_header_value = header_value_callback_handler_module;
    http_settings->on_headers_complete = headers_complete_callback_handler_module;
    http_settings->on_body = body_callback_handler_module;
    http_settings->on_message_complete = message_complete_callback_handler_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_module(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the http settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_data_callback(struct connection_t *connection, struct data_t *data, void *parameters) {
    char *buffer;
    char *_buffer;
    size_t buffer_size;

    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) parameters;
    PyObject *iterator = handler_wsgi_context->iterator;
    PyObject *item = PyIter_Next(iterator);

    if(item == NULL) {
        Py_DECREF(iterator);
        handler_wsgi_context->iterator = NULL;
        _send_response_callback_handler_module(connection, data, parameters);

        /* raises no error */
        RAISE_NO_ERROR;
    }

    /* retieves the buffer from the current item and then retrieves the
    size of it (to be used for the allocation of the connection buffer) */
    buffer = PyString_AsString(item);
    buffer_size = PyString_Size(item);

    /* allocates data for the current connection and then copies the
    current python buffer data into it (for writing into the connection) */
    connection->alloc_data(connection, buffer_size, (void **) &_buffer);
    memcpy(_buffer, buffer, buffer_size);

    /* decrements the item reference it's no longer required to exist
    the data has been already copied */
    Py_DECREF(item);

    connection->write_connection(connection, (unsigned char *) _buffer, buffer_size, _send_data_callback, parameters);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_module(struct http_parser_t *http_parser) {
    PyObject *module;
    PyObject *wsgi_module;
    PyObject *handler_function;
    PyObject *start_response_function;

    PyObject *iterator;

    PyObject *args;
    PyObject *environ;
    PyObject *result;


    /* allocates space for both the index to be used for iteration
    and for the count to be used in pointer increment  */
    size_t index;
    size_t count;


    char *headers_buffer;

    /* retrieves the connection from the http parser parameters
    and then retrieves the handler wsgi context*/
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;

    wsgi_module = PyImport_ImportModule("viriatum_wsgi");
    if(wsgi_module == NULL) { RAISE_NO_ERROR; }

    start_response_function = PyObject_GetAttrString(wsgi_module, "start_response");
    if(!start_response_function || !PyCallable_Check(start_response_function)) { RAISE_NO_ERROR; }

    /* imports the associated (handler) module and retrieves
    its reference to be used for the calling, in case the
    reference is invalid raises an error */
    module = PyImport_ImportModule("wsgi_demo");
    if(module == NULL) { RAISE_NO_ERROR; }

    /* retrieves the function to be used as handler for the
    wsgi request, then check if the reference is valid and
    if it refers a valid function */
    handler_function = PyObject_GetAttrString(module, "application");
    if(!handler_function || !PyCallable_Check(handler_function)) { RAISE_NO_ERROR; }

    /* updates the global reference to the connection to be
    used for the wsgi request handling */
    _connection = connection;

    /* sets the flags from the current http parser in the wsgi
    context for latter reference */
    handler_wsgi_context->flags = http_parser->flags;

    /* resets the number of headers for the current wsgi request
    to be processed (this is a new wsgi request) */
    _wsgi_request.header_count = 0;

    /* sets the wsgi context in the wsgi request for global reference
    this should be able to allow global access from the handler methods */
    _wsgi_request.wsgi_context = handler_wsgi_context;

    /* creates a new tuple to hold the various arguments to
    the call and then populates it with the environment variables
    dictionary and with the start response function */
    args = PyTuple_New(2);
    environ = PyDict_New();
    PyTuple_SetItem(args, 0, environ);
    PyTuple_SetItem(args, 1, start_response_function);

    /* calls the handler function retrieving the result and releasing
    the resources immediately in case the result is not valid */
    result = PyObject_CallObject(handler_function, args);
    if(result == NULL) {
        Py_DECREF(handler_function);
        Py_DECREF(args);
        Py_DECREF(module);
        Py_DECREF(wsgi_module);
        RAISE_NO_ERROR;
    }

    /* allocates space for the header buffer and then writes the default values
    into it the value is dynamicaly contructed based on the current header values */
    connection->alloc_data(connection, 25602, (void **) &headers_buffer);
    count = SPRINTF(
        headers_buffer,
        1024,
        "HTTP/1.1 %d %s\r\n"
        "Server: %s/%s (%s - %s)\r\n",
        _wsgi_request.status_code,
        _wsgi_request.status_message,
        VIRIATUM_NAME,
        VIRIATUM_VERSION,
        VIRIATUM_PLATFORM_STRING,
        VIRIATUM_PLATFORM_CPU
    );

    /* iterates over all the headers present in the current wsgi request to copy
    their content into the current headers buffer */
    for(index = 0; index < _wsgi_request.header_count; index++) {
        /* copies the current wsgi header into the current position of the headers
        buffer (header copy) */
        count += SPRINTF(&headers_buffer[count], 1026, "%s\r\n", _wsgi_request.headers[index]);
    }

    /* finishes the current headers sequence with the final carriage return newline
    character values to closes the headers part of the envelope */
    memcpy(&headers_buffer[count], "\r\n", 2);
    count += 2;

    iterator = PyObject_GetIter(result);
    if(iterator == NULL) { RAISE_NO_ERROR; }
    handler_wsgi_context->iterator = iterator;

    /* writes the response to the connection, this will only write
    the headers the remaining message will be sent on the callback */
    connection->write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) count, _send_data_callback, (void *) handler_wsgi_context);

    /* releases the references on the various resources used in this
    function (memory will be deallocated if necessary) */
    Py_DECREF(handler_function);
    Py_DECREF(args);
    Py_DECREF(module);
    Py_DECREF(wsgi_module);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_module(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current wsgi context for the parameters */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* checks if the current connection should be kept alive, this must
    be done prior to the unseting of the connection as the current wsgi
    context structrue will be destroyed there */
    unsigned char keep_alive = handler_wsgi_context->flags & FLAG_CONNECTION_KEEP_ALIVE;

    /* in case there is an http handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current http connection and then sets the reference
        to it in the http connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive must be closed
    in the normal manner (using the close connection function) */
    if(!keep_alive) { connection->close_connection(connection); }

    /* raise no error */
    RAISE_NO_ERROR;
}
