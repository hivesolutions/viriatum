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

    /* sets the mod wsgi http handler attributes (default) values */
    mod_wsgi_http_handler->file_path = NULL;
    mod_wsgi_http_handler->reload = 0;
    mod_wsgi_http_handler->module = NULL;
	mod_wsgi_http_handler->locations = NULL;

    /* sets the mod wsgi http handler in the upper http handler substrate */
    http_handler->lower = (void *) mod_wsgi_http_handler;

    /* sets the mod wsgi http handler in the mod wsgi http handler pointer */
    *mod_wsgi_http_handler_pointer = mod_wsgi_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* in case the locations buffer is set it must be released
	to avoid any memory leak (not going to be used) */
    if(mod_wsgi_http_handler->locations != NULL) { FREE(mod_wsgi_http_handler->locations); }

    /* in case the execution module is defined for the wsgi handler
    it must be released by decrementing the reference count */
    if(mod_wsgi_http_handler->module != NULL) { Py_DECREF(mod_wsgi_http_handler->module); }

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
    handler_wsgi_context->_next_header = UNDEFINED_HEADER;
    handler_wsgi_context->_url_string.length = 0;
    handler_wsgi_context->_file_name_string.length = 0;
    handler_wsgi_context->_query_string.length = 0;
    handler_wsgi_context->_file_path_string.length = 0;
    handler_wsgi_context->_content_type_string.length = 0;
    handler_wsgi_context->_content_length_string.length = 0;
    handler_wsgi_context->_cookie_string.length = 0;
    handler_wsgi_context->_host_string.length = 0;
    handler_wsgi_context->_server_name_string.length = 0;
    handler_wsgi_context->iterator = NULL;

    /* sets the correct headers global reference in the wsgi
    context and then resets the number of count to zero */
    handler_wsgi_context->headers = &_headers;
    handler_wsgi_context->headers->count = 0;

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
    /* retrieves the handler wsgi context from the http parser */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;
    size_t query_size = pointer == NULL ? 0 : data_size - path_size - 1;
    query_size = query_size > 0 ? query_size : 0;

    /* copies the part of the data buffer relative to the file name
    this avoids copying the query part */
    memcpy(handler_wsgi_context->file_name, data, path_size);
    handler_wsgi_context->file_name[path_size] = '\0';
    normalize_path((char *) handler_wsgi_context->file_name);

    /* in case the pointer is defined (query separator found) copies
    the query contents into the target query buffer */
    if(pointer) { memcpy(handler_wsgi_context->query, pointer + 1, query_size); }
    handler_wsgi_context->query[query_size] = '\0';

    /* copies the url to the url reference in the handler file context to
	be used in the message handler */
    memcpy(handler_wsgi_context->url, data, data_size);
    handler_wsgi_context->url[data_size] = '\0';

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    string_populate(&handler_wsgi_context->_file_name_string, handler_wsgi_context->file_name, path_size, 0);
    string_populate(&handler_wsgi_context->_query_string, handler_wsgi_context->query, query_size, 0);
    string_populate(&handler_wsgi_context->_url_string, handler_wsgi_context->url, path_size, 0);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler wsgi context from the http parser */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;

    /* copies the current header name into the appropriate structure
    and also updates the size of the name string in it */
    handler_wsgi_context->header = &handler_wsgi_context->headers->values[handler_wsgi_context->headers->count];
    memcpy(handler_wsgi_context->header->name, data, data_size);
    handler_wsgi_context->header->name[data_size] = '\0';
    handler_wsgi_context->header->name_size = data_size;

    /* checks if the current header is a valid "capturable"
    header in such case changes the next header value accordingly
    otherwise sets the undefined header */
    if(memcmp(data, "Content-Type", data_size) == 0) { handler_wsgi_context->_next_header = CONTENT_TYPE; }
    else if(memcmp(data, "Content-Length", data_size) == 0) { handler_wsgi_context->_next_header = CONTENT_LENGTH; }
    else if(memcmp(data, "Cookie", data_size) == 0) { handler_wsgi_context->_next_header = COOKIE; }
    else if(memcmp(data, "Host", data_size) == 0) { handler_wsgi_context->_next_header = HOST; }
    else { handler_wsgi_context->_next_header = UNDEFINED_HEADER; }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler wsgi context from the http parser */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;

    /* allocates space for the pointer to be used for partial
    calculation on the header values */
    char *pointer;
    size_t _data_size;

    /* copies the current header value into the appropriate structure
    and alse updates the size of the value string in it */
    memcpy(handler_wsgi_context->header->value, data, data_size);
    handler_wsgi_context->header->value[data_size] = '\0';
    handler_wsgi_context->header->value_size = data_size;

    /* adds the current header to the list of headers this is accomplished
    by incrementing the current headers counter by one value*/
    handler_wsgi_context->headers->count++;

    /* switchs over the next header possible values to
    copy the current header buffer into the appropriate place */
    switch(handler_wsgi_context->_next_header) {
        case CONTENT_TYPE:
            /* copies the content type header value into the
            appropriate buffer in the wsgi context */
            memcpy(handler_wsgi_context->content_type, data, data_size);
            handler_wsgi_context->content_type[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_wsgi_context->_content_type_string, handler_wsgi_context->content_type, data_size, 0);

            /* breaks the switch */
            break;

        case CONTENT_LENGTH:
            /* copies the content length header value into the
            appropriate buffer in the wsgi context */
            memcpy(handler_wsgi_context->content_length_, data, data_size);
            handler_wsgi_context->content_length_[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_wsgi_context->_content_length_string, handler_wsgi_context->content_length_, data_size, 0);

            /* breaks the switch */
            break;

        case COOKIE:
            /* copies the cookie header value into the
            appropriate buffer in the wsgi context */
            memcpy(handler_wsgi_context->cookie, data, data_size);
            handler_wsgi_context->cookie[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_wsgi_context->_cookie_string, handler_wsgi_context->cookie, data_size, 0);

            /* breaks the switch */
            break;

        case HOST:
            /* copies the host header value into the
            appropriate buffer in the wsgi context */
            memcpy(handler_wsgi_context->host, data, data_size);
            handler_wsgi_context->host[data_size] = '\0';

            /* tries to find the port part of the host in case
            it's possible sets the base data size for the server name */
            pointer = strchr((char *) handler_wsgi_context->host, ':');
            if(pointer == NULL) { _data_size = data_size; }
            else { _data_size = pointer - (char *) handler_wsgi_context->host; }

            /* copies the server name header value into the
            appropriate buffer in the wsgi context */
            memcpy(handler_wsgi_context->server_name, data, _data_size);
            handler_wsgi_context->server_name[_data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_wsgi_context->_host_string, handler_wsgi_context->host, data_size, 0);
            string_populate(&handler_wsgi_context->_server_name_string, handler_wsgi_context->server_name, _data_size, 0);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

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

ERROR_CODE location_callback_handler_module(struct http_parser_t *http_parser, size_t index, size_t offset) {
	/* allocates space for the temporary file path size variables
	to be used in internal string size calculations */
	size_t file_path_size;

    /* retrieves the handler wsgi context from the http parser */
    struct handler_wsgi_context_t *handler_wsgi_context =
		(struct handler_wsgi_context_t *) http_parser->context;

	/* retrieves the connection from the parser and then used it to retreives the
	the correct wsgi http handler reference from the http connection */
	struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler =
		(struct mod_wsgi_http_handler_t *) http_connection->http_handler->lower;

	/* retrieves the current location from the location buffer and checks if the
	file path value is correctly set */
	struct mod_wsgi_location_t *location = &mod_wsgi_http_handler->locations[index];
	if(location->file_path == NULL) { RAISE_NO_ERROR; }

	/* retrieves the size of the file path associated with the location and copies
	it to the current context file path string then updates the file path string
	to the required size */
	file_path_size = strlen(location->file_path);
	memcpy(handler_wsgi_context->file_path, location->file_path, file_path_size);
	handler_wsgi_context->file_path[file_path_size] = '\0';
    string_populate(
		&handler_wsgi_context->_file_path_string,
		handler_wsgi_context->file_path,
		file_path_size,
		0
	);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler wsgi context from the http parser */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;
    size_t query_size = pointer == NULL ? 0 : data_size - path_size - 1;
    query_size = query_size > 0 ? query_size : 0;

    /* copies the part of the data buffer relative to the file name
    this avoids copying the query part */
    memcpy(handler_wsgi_context->file_name, data, path_size);
    handler_wsgi_context->file_name[path_size] = '\0';
    normalize_path((char *) handler_wsgi_context->file_name);

    /* in case the pointer is defined (query separator found) copies
    the query contents into the target query buffer */
    if(pointer) { memcpy(handler_wsgi_context->query, pointer + 1, query_size); }
    handler_wsgi_context->query[query_size] = '\0';

    /* copies the url to the url reference in the handler file context to
	be used in the message handler */
    memcpy(handler_wsgi_context->url, data, data_size);
    handler_wsgi_context->url[data_size] = '\0';

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    string_populate(&handler_wsgi_context->_file_name_string, handler_wsgi_context->file_name, path_size, 0);
    string_populate(&handler_wsgi_context->_query_string, handler_wsgi_context->query, query_size, 0);
    string_populate(&handler_wsgi_context->_url_string, handler_wsgi_context->url, path_size, 0);

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
    http_settings->on_location = location_callback_handler_module;
    http_settings->on_virtual_url = virtual_url_callback_handler_module;

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
    http_settings->on_location = NULL;
    http_settings->on_virtual_url = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_data_callback(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* allocates space for the buffers (both the python internal and
    the copy) and for the size of both of them */
    char *buffer;
    char *_buffer;
    size_t buffer_size;

    /* retrieves the current wsgi context object as the parameters object
    then uses it to retrieve the iterator and then retrieves the next
    element from the iterator (the next data to be sent) */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) parameters;
    PyObject *iterator = handler_wsgi_context->iterator;
    PyObject *item = PyIter_Next(iterator);

    /* in case the item is not defined (end of iteration) no more data is
    available to be sent in the response must cleanup the request */
    if(item == NULL) {
        /* decrements the iterator reference count (should release its memory)
        and then unsets it from the wsgi context (no more access) */
        Py_DECREF(iterator);
        handler_wsgi_context->iterator = NULL;

        /* redirect the handling to the send response callback handler module
        so that the proper cleanup is done (eg: closing connection check) */
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

    /* writes the buffer to the connection, this will write another
    chunk of data into the connection and return to this same callback
    function to try to write more data */
    connection->write_connection(connection, (unsigned char *) _buffer, buffer_size, _send_data_callback, parameters);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_module(struct http_parser_t *http_parser) {
    /* allocates space for the local (application) module, for the global
    wsgi module (containing util function) an then allocates also space
    for the application handler function and for the start response function */
    PyObject *module;
    PyObject *wsgi_module;
    PyObject *handler_function;
    PyObject *start_response_function;

    /* allocates space for the iterator reference to be used as a temporary
    variable holding the various message items for the response */
    PyObject *iterator;

    /* allocates space for the tuple to hold the arguments to be sent to the
    application handler and then allocates space for both arguments */
    PyObject *args;
    PyObject *environ;
    PyObject *result;

    /* allocates space for both the index to be used for iteration
    and for the count to be used in pointer increment */
    size_t index;
    size_t count;

    /* allocates space for the path to the file to be interpreted as the
    application execution module */
    char *file_path;

    /* allocates space for the reference to the buffer that will the initial
    (headers) part of the http message to be sent as response */
    char *headers_buffer;

    /* retrieves the connection from the http parser parameters
    and then retrieves the handler wsgi context and mod wsgi handler */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler = (struct mod_wsgi_http_handler_t *) http_connection->http_handler->lower;

    /* in case the reload flag is set and the module is already loaded must
    release its memory and unset it from the handler */
    if(mod_wsgi_http_handler->reload && mod_wsgi_http_handler->module) {
        mod_wsgi_http_handler->module = NULL;
    }

    /* in cae the module is not defined, must be loaded again from the
    file into the python interpreter */
    if(mod_wsgi_http_handler->module == NULL) {
        /* retrieves the correct file path for the module to be loaded
        defaulting to the preddefined path in case none is defined */
        file_path = mod_wsgi_http_handler->file_path == NULL
            ? DEFAULT_FILE_PATH : mod_wsgi_http_handler->file_path;
		file_path = handler_wsgi_context->_file_path_string.length > 0 ?
		handler_wsgi_context->file_path : file_path;

        /* loads the module as wsgi app from the provided file path and
        then updates the module variable to contain a reference to it */
        _load_module(
            &mod_wsgi_http_handler->module,
            "wsgi_app",
            file_path
        );
    }

    /* imports the wsgi module containing the util methos to be used by the
    application to access viriatum wsgi functions */
    wsgi_module = PyImport_ImportModule("viriatum_wsgi");
    if(wsgi_module == NULL) { RAISE_NO_ERROR; }

    /* retrieves the reference to the start response function from the wsgi module
    and then verifies that it's a valid python function */
    start_response_function = PyObject_GetAttrString(wsgi_module, "start_response");
    if(!start_response_function || !PyCallable_Check(start_response_function)) { RAISE_NO_ERROR; }

    /* imports the associated (handler) module and retrieves
    its reference to be used for the calling, in case the
    reference is invalid raises an error */
    module = mod_wsgi_http_handler->module;
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
    PyTuple_SET_ITEM(args, 0, environ);
    PyTuple_SET_ITEM(args, 1, start_response_function);

    /* starts the environ dictionary object with the various values
    indirectly associated with the parser of the current request */
    _start_environ(environ, http_parser);

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
        "Connection: Keep-Alive\r\n"
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

    /* retrieves the iterator associated with the result, that should be a sequence
    object (iterable) containg the various parts of the message to be returned, then
    sets the iterator in the wsgi context structure to be used in the callback */
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
    Py_DECREF(result);

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

ERROR_CODE _start_environ(PyObject *environ, struct http_parser_t *http_parser) {
    /* allocates space for the counter to be used for iteration
    on the various header values */
    size_t index;

    /* allocates space for the final name of the exporting header,
    for the temporary values to eb used in the exports and for the
    temporary header value structure pointer */
    char name[VIRIATUM_MAX_HEADER_SIZE + sizeof("HTTP_")];
    PyObject *_value;
    PyObject *_value_;
    struct http_header_value_t *header;

    /* allocates the buffer reference to the used to store the
    data from the post part of the message */
    unsigned char *post_data;

    /* retrieves the connection from the http parser parameters
    and then retrieves the handler wsgi context */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;

    /* retrieves the port (string representation) from the service
    options associated with the current connection */
    unsigned char *port = connection->service->options->_port;

    /* retrieves the http method string value accessing the array of
    static string values */
    char *method = (char *) http_method_strings[http_parser->method - 1];

    /* in case there is contents to be read retrieves the appropriate
    reference to the start of the post data in the connection buffer */
    if(http_parser->_content_length > 0) { post_data = &http_connection->buffer[http_connection->buffer_offset - http_parser->_content_length];
    } else { post_data = NULL; }

    /* iterates over all the heades in order to "export" them
    into the environ dictionary to expose them to the application */
    for(index = 0; index < _headers.count; index++) {
        /* retrieves the current header structure for the
        iteration cycle in course */
        header = &_headers.values[index];

        /* converts the current header name to uppercase
        and appends the http prefix into it, then converts
        the header value into a python string object */
        uppercase(header->name);
        SPRINTF(name, VIRIATUM_MAX_HEADER_SIZE + sizeof("HTTP_"), "HTTP_%s", header->name);
        _value = PyString_FromString(header->value);

        /* sets the value using the provided string name
        and decrements the reference count of the value */
        PyDict_SetItemString(environ, name, _value);
        Py_DECREF(_value);
    }

    _value_ = PyTuple_New(2);
    _value = PyInt_FromLong(1);
    PyTuple_SET_ITEM(_value_, 0, _value);
    _value = PyInt_FromLong(0);
    PyTuple_SET_ITEM(_value_, 1, _value);
    PyDict_SetItemString(environ, "wsgi.version", _value_);
    Py_DECREF(_value_);

    _value = PyString_FromString("http");
    PyDict_SetItemString(environ, "wsgi.url_scheme", _value);
    Py_DECREF(_value);

    _value = _new_wsgi_input(post_data, http_parser->_content_length);
    PyDict_SetItemString(environ, "wsgi.input", _value);
    Py_DECREF(_value);

    _value = PyBool_FromLong(0);
    PyDict_SetItemString(environ, "wsgi.multithread", _value);
    Py_DECREF(_value);

    _value = PyBool_FromLong(1);
    PyDict_SetItemString(environ, "wsgi.multiprocess", _value);
    Py_DECREF(_value);

    _value = PyBool_FromLong(0);
    PyDict_SetItemString(environ, "wsgi.run_once", _value);
    Py_DECREF(_value);

    _value = PyString_FromString(method);
    PyDict_SetItemString(environ, "REQUEST_METHOD", _value);
    Py_DECREF(_value);

    _value = PyString_FromString((char *) handler_wsgi_context->file_name);
    PyDict_SetItemString(environ, "SCRIPT_NAME", _value);
    Py_DECREF(_value);

    _value = PyString_FromString((char *) handler_wsgi_context->file_name);
    PyDict_SetItemString(environ, "PATH_INFO", _value);
    Py_DECREF(_value);

    _value = PyString_FromString((char *) handler_wsgi_context->query);
    PyDict_SetItemString(environ, "QUERY_STRING", _value);
    Py_DECREF(_value);

    _value = PyString_FromString((char *) handler_wsgi_context->server_name);
    PyDict_SetItemString(environ, "SERVER_NAME", _value);
    Py_DECREF(_value);

    _value = PyString_FromString((char *) port);
    PyDict_SetItemString(environ, "SERVER_PORT", _value);
    Py_DECREF(_value);

    _value = PyString_FromString("HTTP/1.1");
    PyDict_SetItemString(environ, "SERVER_PROTOCOL", _value);
    Py_DECREF(_value);

    if(handler_wsgi_context->_content_type_string.length > 0) {
        _value = PyString_FromString((char *) handler_wsgi_context->content_type);
        PyDict_SetItemString(environ, "CONTENT_TYPE", _value);
        Py_DECREF(_value);
    }

    if(handler_wsgi_context->_content_length_string.length > 0) {
        _value = PyString_FromString((char *) handler_wsgi_context->content_length_);
        PyDict_SetItemString(environ, "CONTENT_LENGTH", _value);
        Py_DECREF(_value);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_module(PyObject **module_pointer, char *name, char *file_path) {
    /* allocates space for the pointer to the file object to be
    used for reading the module file */
    FILE *file;

    /* allocates space for the node to hold the root node of the
    ast and to the code and module python objects */
    struct _node *node;
    PyObject *code;
    PyObject *module;

    /* allocates space for the number of bytes for the file size
    and for the buffer that will hold the file */
    size_t number_bytes;
    size_t file_size;
    char *file_buffer;

    /* resets the provided module pointer to the "default" invalid
    value to provide error detection */
    *module_pointer = NULL;

    /* opens the file for reading (in binary mode) and checks if
    there was a problem opening it, raising an error in such case */
    FOPEN(&file, file_path, "r");
    if(file == NULL) { RAISE_NO_ERROR; }

    /* seeks the file until the end of the file and then
    retrieves the current position as the size at the end
    restores the file position back the beginning */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocates space for the file buffer that will contain the
    complete python file, this should be a null terminated string */
    file_buffer = (char *) MALLOC(file_size + 1);
    number_bytes = fread(file_buffer, 1, file_size, file);
    file_buffer[number_bytes] = '\0';

    /* parses the "just" read file using the python parser and then
    closes the file to avoid any file memory leaking (possible problems) */
    node = PyParser_SimpleParseString(file_buffer, Py_file_input);
    fclose(file);
    FREE(file_buffer);

    /* in case the parsed node is not valid (something wrong occurred
    while parsing the file) raises an error */
    if(node == NULL) { RAISE_NO_ERROR; }

    /* compiles the top level node (ast) into a python code object
    so that it can be executed */
    code = (PyObject *) PyNode_Compile(node, file_path);
    PyNode_Free(node);
    if(code == NULL) { RAISE_NO_ERROR; }

    /* executes the code in the code object provided, retrieveing the
    module and setting it in the module pointer reference */
    module = PyImport_ExecCodeModuleEx(name, code, file_path);
    *module_pointer = module;

    /* decrements the reference count in the code object so that it's
    able to release itself */
    Py_XDECREF(code);

    /* raises no error */
    RAISE_NO_ERROR;
}
