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

ERROR_CODE create_mod_php_http_handler(struct mod_php_http_handler_t **mod_php_http_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the mod php http handler size */
    size_t mod_php_http_handler_size = sizeof(struct mod_php_http_handler_t);

    /* allocates space for the mod php http handler */
    struct mod_php_http_handler_t *mod_php_http_handler = (struct mod_php_http_handler_t *) MALLOC(mod_php_http_handler_size);

    /* sets the mod php http handler attributes (default) values */
    mod_php_http_handler->base_path = NULL;

    /* sets the mod php http handler in the upper http handler substrate */
    http_handler->lower = (void *) mod_php_http_handler;

    /* sets the mod php http handler in the mod php http handler pointer */
    *mod_php_http_handler_pointer = mod_php_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_php_http_handler(struct mod_php_http_handler_t *mod_php_http_handler) {
    /* releases the mod php http handler */
    FREE(mod_php_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_php_context(struct handler_php_context_t **handler_php_context_pointer) {
    /* retrieves the handler php context size */
    size_t handler_php_context_size = sizeof(struct handler_php_context_t);

    /* allocates space for the handler php context */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) MALLOC(handler_php_context_size);

    /* sets the handler php default values */
    handler_php_context->method = NULL;
    handler_php_context->post_data = NULL;
    handler_php_context->flags = 0;
    handler_php_context->content_length = 0;
    handler_php_context->output_buffer = NULL;
    handler_php_context->_next_header = UNDEFINED_HEADER;

    /* sets the handler php context in the  pointer */
    *handler_php_context_pointer = handler_php_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_php_context(struct handler_php_context_t *handler_php_context) {
    /* in case there is a valid output buffer defined in the current
    handler php context it must be removed (linked buffer removal)
    this way serious memory leaks are avoided */
    if(handler_php_context->output_buffer) { delete_linked_buffer(handler_php_context->output_buffer); }

    /* releases the handler php context memory */
    FREE(handler_php_context);

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
    /* retrieves the handler php context from the http parser
    in order tu use it to update the content type to the default
    empty value (avoids possible problems in php interpreter)*/
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;
    handler_php_context->content_type[0] = '\0';

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    string_populate(&handler_php_context->_content_type_string, handler_php_context->content_type, 0, 0);
    string_populate(&handler_php_context->_cookie_string, handler_php_context->cookie, 0, 0);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler php context from the http parser */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;
    size_t query_size = pointer == NULL ? 0 : data_size - path_size - 1;
    query_size = query_size > 0 ? query_size : 0;

    /* copies the part of the data buffer relative to the file name
    this avoids copying the query part */
    memcpy(handler_php_context->file_name, data, path_size);
    handler_php_context->file_name[path_size] = '\0';

    /* in case the pointer is defined (query separator found) copies
    the query contents into the target query buffer */
    if(pointer) { memcpy(handler_php_context->query, pointer + 1, query_size); }
    handler_php_context->query[query_size] = '\0';

    /* copies the url to the url reference in the handler file context then
    creates the file path from using the base viriatum path */
    memcpy(handler_php_context->url, data, data_size);
    handler_php_context->url[path_size] = '\0';
    SPRINTF((char *) handler_php_context->file_path, VIRIATUM_MAX_PATH_SIZE, "%s%s%s", VIRIATUM_CONTENTS_PATH, VIRIATUM_BASE_PATH, handler_php_context->file_name);

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
	string_populate(&handler_php_context->_file_name_string, handler_php_context->file_name, path_size, 0);
    string_populate(&handler_php_context->_query_string, handler_php_context->query, query_size, 0);
    string_populate(&handler_php_context->_url_string, handler_php_context->url, path_size, 0);
    string_populate(&handler_php_context->_file_path_string, handler_php_context->file_path, 0, 1);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler php context from the http parser */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;

    /* checks if the current header is a valid "capturable"
    header in such case changes the next header value accordingly
    otherwise sets the undefined header */
    if(memcmp(data, "Content-Type", data_size) == 0) { handler_php_context->_next_header = CONTENT_TYPE; }
    else if(memcmp(data, "Cookie", data_size) == 0) { handler_php_context->_next_header = COOKIE; }
	else if(memcmp(data, "Host", data_size) == 0) { handler_php_context->_next_header = HOST; }
    else { handler_php_context->_next_header = UNDEFINED_HEADER; }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler php context from the http parser */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;

	char *pointer;
	size_t _data_size;

    /* switchs over the next header possible values to
    copy the current header buffer into the appropriate place */
    switch(handler_php_context->_next_header) {
        case CONTENT_TYPE:
            /* copies the content type header value into the
            appropriate buffer in the php context */
            memcpy(handler_php_context->content_type, data, data_size);
            handler_php_context->content_type[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_php_context->_content_type_string, handler_php_context->content_type, data_size, 0);

            /* breaks the switch */
            break;

        case COOKIE:
            /* copies the cookie header value into the
            appropriate buffer in the php context */
            memcpy(handler_php_context->cookie, data, data_size);
            handler_php_context->cookie[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_php_context->_cookie_string, handler_php_context->cookie, data_size, 0);

            /* breaks the switch */
            break;

        case HOST:
            /* copies the host header value into the
            appropriate buffer in the php context */
            memcpy(handler_php_context->host, data, data_size);
            handler_php_context->host[data_size] = '\0';

			/* tries to find the port part of the host in case
			it's possible sets the base data size for the server name */
			pointer = strchr(handler_php_context->host, ':');
			if(pointer == NULL) { _data_size = data_size; }
			else { _data_size = pointer - handler_php_context->host; }

			/* copies the server name header value into the
            appropriate buffer in the php context */
            memcpy(handler_php_context->server_name, data, _data_size);
            handler_php_context->server_name[_data_size] = '\0';

		    /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
			string_populate(&handler_php_context->_host_string, handler_php_context->host, data_size, 0);
            string_populate(&handler_php_context->_server_name_string, handler_php_context->server_name, _data_size, 0);

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

ERROR_CODE _set_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* allocates space for the handler php context and
    then creates and populates the instance after that
    sets the handler file context as the context for
    the http parser*/
    struct handler_php_context_t *handler_php_context;
    create_handler_php_context(&handler_php_context);
    http_parser->context = handler_php_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* retrieves the handler php context from the http parser
    and then deletes (releases memory) */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;
    delete_handler_php_context(handler_php_context);

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
    /* allocates the buffer that will hod the message to be sent
    through the connection and then allocates the buffer to hold
    the joined buffer from the linked buffer rerference */
    char *buffer;
    char *output_data;

    /* retrieves the current php context and then uses it to retrieve
    the proper output buffer for the current connection (the context) */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) parameters;
    struct linked_buffer_t *output_buffer = handler_php_context->output_buffer;

    /* retrieves the size of the output buffer, this is going to
    be used to measure the size of the output stream */
    size_t output_length = output_buffer->buffer_length;

    /* joins the output buffer so that the buffer is set as continguous
    and then deletes the output buffer (no more need to use it) */
    join_linked_buffer(output_buffer, (unsigned char **) &output_data);
    delete_linked_buffer(output_buffer);

    /* allocates data for the current connection and then copues the
    current output buffer data into it (for writing into the connection) */
    connection->alloc_data(connection, output_length, (void **) &buffer);
    memcpy(buffer, output_data, output_length);

    /* writes the response to the connection, this should flush the current
    data in the output buffer to the network */
    connection->write_connection(connection, (unsigned char *) buffer, output_length, _send_response_callback_handler_module, parameters);

    /* unsets the output buffer from the context (it's not going) to
    be used anymore (must be released) */
    handler_php_context->output_buffer = NULL;

    /* releases the output data no more need to use it */
    FREE(output_data);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_module(struct http_parser_t *http_parser) {
    /* allocates space for the script file structure to be
    during for script execution */
    zend_file_handle script;

    /* allocates space for both the index to be used for iteration
    and for the count to be used in pointer increment  */
    size_t index;
    size_t count;

    /* allocates space for the buffer that will hold the headers and
    the pointer that will hold the reference to the buffer containing
    the post data information */
    char *headers_buffer;
    unsigned char *post_data;

    /* allocates space for the linked buffer to be used for the
    standard ouput resulting from the php interpreter execution */
    struct linked_buffer_t *output_buffer = NULL;

    /* retrieves the connection from the http parser parameters
    and then retrieves the handler php context*/
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;

    /* retrieves the http method string value accessing the array of
    static string values */
    char *method = (char *) http_method_strings[http_parser->method - 1];

    /* in case there is contents to be read retrieves the appropriate
    reference to the start of the post data in the connection buffer */
    if(http_parser->_content_length > 0) { post_data = &http_connection->buffer[http_connection->buffer_offset - http_parser->_content_length];
    } else { post_data = NULL; }

    /* sets the global reference to the connection currently being
    used for the php interpreter this is the reference that is going
    to be used while accessing global server values */
    _connection = connection;

    /* creates the linked buffer to be used to store
    the complete output of the php interpreter */
    create_linked_buffer(&output_buffer);

    /* sets the output buffer reference in the global output buffer
    variable so that the php write handler is able to store the
    current output stream values, then sets the output buffer also
    in the current http parser structure reference */
    _output_buffer = output_buffer;
    handler_php_context->method = method;
    handler_php_context->post_data = post_data;
    handler_php_context->flags = http_parser->flags;
    handler_php_context->content_length = http_parser->_content_length;
    handler_php_context->output_buffer = output_buffer;

    /* resets the number of headers for the current php request
    to be processed (this is a new php request) */
    _php_request.header_count = 0;

    /* sets the php context in the php request for global reference
    this should be able to allow global access from the handler methods */
    _php_request.php_context = handler_php_context;

    /* updates the current global php reqest information
    this is the main interface for the sapi modules */
    _update_request(handler_php_context);

    /* populates the "base" script reference structure
    with the required value for execution */
    script.type = ZEND_HANDLE_FILENAME;
    script.filename = (char *) handler_php_context->file_path;
    script.opened_path = NULL;
    script.free_filename = 0;

    zend_try {
        /* tries to start the request handling and in case it
        succeedes continues with the execution of it */
        if(php_request_startup(TSRMLS_C) == SUCCESS) {
            /* executes the script in the current instantiated virtual
            machine, this is a blocking call so it will block the current
            general loop (care is required), then after the execution
            closes the current request information */
            php_execute_script(&script TSRMLS_CC);
            php_request_shutdown(NULL);
        }
    } zend_catch {
    } zend_end_try();

    /* allocates space fot the header buffer and then writes the default values
    into it the value is dynamicaly contructed based on the current header values */
    connection->alloc_data(connection, 25602, (void **) &headers_buffer);
    count = SPRINTF(headers_buffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) output_buffer->buffer_length);

    /* iterates over all the headers present in the current php request to copy
    their content into the current headers buffer */
    for(index = 0; index < _php_request.header_count; index++) {
        /* copies the current php header into the current position of the headers
        buffer (header copy) */
        count += SPRINTF(&headers_buffer[count], 1026, "%s\r\n", _php_request.headers[index]);
    }

    /* finishes the current headers sequence with the final carriage return newline
    character values to closes the headers part of the envelope */
    memcpy(&headers_buffer[count], "\r\n", 2);
    count += 2;

    /* writes the response to the connection, this will only write
    the headers the remaining message will be sent on the callback */
    connection->write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) count, _send_data_callback, (void *) handler_php_context);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_module(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current php context for the parameters */
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

	/* checks if the current connection should be kept alive, this must
	be done prior to the unseting of the connection as the current php
	context structrue will be destroyed there */
	unsigned char keep_alive = handler_php_context->flags & FLAG_CONNECTION_KEEP_ALIVE;

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

ERROR_CODE _write_error_connection(struct http_parser_t *http_parser, char *message) {
    /* allocates space for the buffer to be used in the message */
    unsigned char *buffer;

    /* retrieves the connection from the http parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct handler_php_context_t *handler_php_context = (struct handler_php_context_t *) http_parser->context;

    /* retrieves the length of the message so that it's possible to print
    the proper error */
    size_t message_length = strlen(message);

    /* updates the flags value in the php context, this is required
    to avoid problems in the callback handler */
    handler_php_context->flags = http_parser->flags;

    /* allocates the data buffer (in a safe maner) then
    writes the http static headers to the response */
    connection->alloc_data(connection, 1024 * sizeof(unsigned char), (void **) &buffer);
    SPRINTF((char *) buffer, 1024, "HTTP/1.1 500 Internal Server Error\r\nServer: %s/%s (%s @ %s)\r\nConnection: Keep-Alive\r\nContent-Length: %d\r\n\r\n%s", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (unsigned int) message_length, message);

    /* writes the response to the connection, registers for the appropriate callbacks */
    connection->write_connection(connection, buffer, (unsigned int) strlen((char *) buffer), _send_response_callback_handler_module, (void *) handler_php_context);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _update_request(struct handler_php_context_t *handler_php_context) {
    /* sets the various sapi headers and request info parameters
    from the current php context object values */
    SG(sapi_headers).http_response_code = 200;
    SG(sapi_headers).http_status_line = "OK";
    SG(request_info).content_type = (char *) handler_php_context->content_type;
    SG(request_info).query_string = (char *) handler_php_context->query;
    SG(request_info).request_method = handler_php_context->method;
    SG(request_info).proto_num = 1001;
    SG(request_info).request_uri = (char *) handler_php_context->url;
    SG(request_info).path_translated = (char *) handler_php_context->file_path;
    SG(request_info).content_length = handler_php_context->content_length;

    /* updates the global values with the previously defined (static)
    values the server context is set as one to allow correct php running */
    SG(global_request_time) = 0;
    SG(read_post_bytes) = 1;
    SG(server_context) = (void *) 1;

    /* raises no error */
    RAISE_NO_ERROR;
}
