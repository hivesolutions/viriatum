/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "handler.h"

ERROR_CODE create_mod_lua_http_handler(struct mod_lua_http_handler_t **mod_lua_http_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the mod Lua HTTP handler size */
    size_t mod_lua_http_handler_size = sizeof(struct mod_lua_http_handler_t);

    /* allocates space for the mod Lua HTTP handler */
    struct mod_lua_http_handler_t *mod_lua_http_handler =
        (struct mod_lua_http_handler_t *) MALLOC(mod_lua_http_handler_size);

    /* sets the mod Lua HTTP handler attributes (default) values */
    mod_lua_http_handler->lua_state = NULL;
    mod_lua_http_handler->file_path[0] = '\0';
    mod_lua_http_handler->file_dirty = 0;
    mod_lua_http_handler->locations = NULL;
    mod_lua_http_handler->locations_count = 0;
    mod_lua_http_handler->current_location = NULL;

    /* sets the mod Lua HTTP handler in the upper HTTP handler substrate */
    http_handler->lower = (void *) mod_lua_http_handler;

    /* sets the mod Lua HTTP handler in the mod Lua HTTP handler pointer */
    *mod_lua_http_handler_pointer = mod_lua_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_lua_http_handler(struct mod_lua_http_handler_t *mod_lua_http_handler) {
    /* allocates space for the index counter to be used to iterate
    around the various locations and space for the location in iteration */
    size_t index;
    struct mod_lua_location_t *location;

    /* iterates over all the locations to (possibly) close the
    various loaded Lua states in their control, this is considered
    a garbage collection operation */
    for(index = 0; index < mod_lua_http_handler->locations_count; index++) {
        location = &mod_lua_http_handler->locations[index];
        if(location->lua_state != NULL) { lua_close(location->lua_state); }
    }

    /* in case the locations buffer is set it must be released
    to avoid any memory leak (not going to be used) */
    if(mod_lua_http_handler->locations != NULL) { FREE(mod_lua_http_handler->locations); }

    /* releases the mod Lua HTTP handler */
    FREE(mod_lua_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_lua_context(struct handler_lua_context_t **handler_lua_context_pointer) {
    /* retrieves the handler Lua context size */
    size_t handler_lua_context_size = sizeof(struct handler_lua_context_t);

    /* allocates space for the handler Lua context */
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) MALLOC(handler_lua_context_size);

    /* sets the default value in the handler Lua context
    structure (resets the structure) */
    handler_lua_context->url[0] = '\0';
    handler_lua_context->file_name[0] = '\0';
    handler_lua_context->query[0] = '\0';
    handler_lua_context->prefix_name[0] = '\0';
    handler_lua_context->file_path[0] = '\0';
    handler_lua_context->content_type[0] = '\0';
    handler_lua_context->content_length_[0] = '\0';
    handler_lua_context->cookie[0] = '\0';
    handler_lua_context->host[0] = '\0';
    handler_lua_context->server_name[0] = '\0';
    handler_lua_context->header = NULL;
    handler_lua_context->flags = 0;
    handler_lua_context->_next_header = UNDEFINED_HEADER;
    handler_lua_context->_url_string.length = 0;
    handler_lua_context->_file_name_string.length = 0;
    handler_lua_context->_query_string.length = 0;
    handler_lua_context->_file_path_string.length = 0;
    handler_lua_context->_content_type_string.length = 0;
    handler_lua_context->_content_length_string.length = 0;
    handler_lua_context->_cookie_string.length = 0;
    handler_lua_context->_host_string.length = 0;
    handler_lua_context->_server_name_string.length = 0;
    handler_lua_context->_prefix_name_string.length = 0;

    /* sets the correct headers global reference in the Lua
    context and then resets the number of count to zero */
    handler_lua_context->headers = &_headers;
    handler_lua_context->headers->count = 0;

    /* sets the handler Lua context in the pointer */
    *handler_lua_context_pointer = handler_lua_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_lua_context(struct handler_lua_context_t *handler_lua_context) {
    /* releases the handler Lua context memory */
    FREE(handler_lua_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_lua(struct http_connection_t *http_connection) {
    /* sets the HTTP parser values */
    _set_http_parser_handler_lua(http_connection->http_parser);

    /* sets the HTTP settings values */
    _set_http_settings_handler_lua(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_lua(struct http_connection_t *http_connection) {
    /* unsets the HTTP parser values */
    _unset_http_parser_handler_lua(http_connection->http_parser);

    /* unsets the HTTP settings values */
    _unset_http_settings_handler_lua(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_module(struct http_parser_t *http_parser) {
    /* retrieves the connection from the parser and then uses it to retrieve the
    correct Lua HTTP handler reference from the HTTP connection */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct mod_lua_http_handler_t *mod_lua_http_handler =
        (struct mod_lua_http_handler_t *) http_connection->http_handler->lower;

    /* resets the current location reference so that the response
    handler falls back to the handler defaults until a location
    callback overrides it for the current request */
    mod_lua_http_handler->current_location = NULL;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler Lua context from the HTTP parser */
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;
    size_t query_size = pointer == NULL ? 0 : data_size - path_size - 1;
    query_size = query_size > 0 ? query_size : 0;

    /* copies the part of the data buffer relative to the file name
    this avoids copying the query part */
    memcpy(handler_lua_context->file_name, data, path_size);
    handler_lua_context->file_name[path_size] = '\0';
    normalize_path((char *) handler_lua_context->file_name);

    /* in case the pointer is defined (query separator found) copies
    the query contents into the target query buffer */
    if(pointer) { memcpy(handler_lua_context->query, pointer + 1, query_size); }
    handler_lua_context->query[query_size] = '\0';

    /* copies the URL to the URL reference in the handler file context to
    be used in the message handler */
    memcpy(handler_lua_context->url, data, data_size);
    handler_lua_context->url[data_size] = '\0';

    /* sets the prefix name as non existent this is the default behaviour
    to be overriden by the virtual URL handler */
    handler_lua_context->prefix_name[0] = '\0';

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    string_populate(&handler_lua_context->_file_name_string, handler_lua_context->file_name, path_size, 0);
    string_populate(&handler_lua_context->_query_string, handler_lua_context->query, query_size, 0);
    string_populate(&handler_lua_context->_url_string, handler_lua_context->url, path_size, 0);
    string_populate(&handler_lua_context->_prefix_name_string, handler_lua_context->prefix_name, 0, 0);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler Lua context from the HTTP parser */
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;

    /* copies the current header name into the appropriate structure
    and also updates the size of the name string in it */
    handler_lua_context->header = &handler_lua_context->headers->values[handler_lua_context->headers->count];
    memcpy(handler_lua_context->header->name, data, data_size);
    handler_lua_context->header->name[data_size] = '\0';
    handler_lua_context->header->name_size = data_size;

    /* checks if the current header is a valid "capturable"
    header in such case changes the next header value accordingly
    otherwise sets the undefined header */
    if(data_size == 12 && memcmp(data, CONTENT_TYPE_H, data_size) == 0) {
        handler_lua_context->_next_header = CONTENT_TYPE;
    } else if(data_size == 14 && memcmp(data, CONTENT_LENGTH_H, data_size) == 0) {
        handler_lua_context->_next_header = CONTENT_LENGTH;
    } else if(data_size == 6 && memcmp(data, COOKIE_H, data_size) == 0) {
        handler_lua_context->_next_header = COOKIE;
    } else if(data_size == 4 && memcmp(data, HOST_H, data_size) == 0) {
        handler_lua_context->_next_header = HOST;
    } else {
        handler_lua_context->_next_header = UNDEFINED_HEADER;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler Lua context from the HTTP parser */
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;

    /* allocates space for the pointer to be used for partial
    calculation on the header values */
    char *pointer;
    size_t _data_size;

    /* copies the current header value into the appropriate structure
    and also updates the size of the value string in it */
    memcpy(handler_lua_context->header->value, data, data_size);
    handler_lua_context->header->value[data_size] = '\0';
    handler_lua_context->header->value_size = data_size;

    /* adds the current header to the list of headers this is accomplished
    by incrementing the current headers counter by one value */
    handler_lua_context->headers->count++;

    /* switches over the next header possible values to
    copy the current header buffer into the appropriate place */
    switch(handler_lua_context->_next_header) {
        case CONTENT_TYPE:
            /* copies the content type header value into the
            appropriate buffer in the Lua context */
            memcpy(handler_lua_context->content_type, data, data_size);
            handler_lua_context->content_type[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_lua_context->_content_type_string, handler_lua_context->content_type, data_size, 0);

            /* breaks the switch */
            break;

        case CONTENT_LENGTH:
            /* copies the content length header value into the
            appropriate buffer in the Lua context */
            memcpy(handler_lua_context->content_length_, data, data_size);
            handler_lua_context->content_length_[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_lua_context->_content_length_string, handler_lua_context->content_length_, data_size, 0);

            /* breaks the switch */
            break;

        case COOKIE:
            /* copies the cookie header value into the
            appropriate buffer in the Lua context */
            memcpy(handler_lua_context->cookie, data, data_size);
            handler_lua_context->cookie[data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_lua_context->_cookie_string, handler_lua_context->cookie, data_size, 0);

            /* breaks the switch */
            break;

        case HOST:
            /* copies the host header value into the
            appropriate buffer in the Lua context */
            memcpy(handler_lua_context->host, data, data_size);
            handler_lua_context->host[data_size] = '\0';

            /* tries to find the port part of the host in case
            it's possible sets the base data size for the server name */
            pointer = strchr((char *) handler_lua_context->host, ':');
            if(pointer == NULL) {
                _data_size = data_size;
            } else {
                _data_size = pointer - (char *) handler_lua_context->host;
            }

            /* copies the server name header value into the
            appropriate buffer in the Lua context */
            memcpy(handler_lua_context->server_name, data, _data_size);
            handler_lua_context->server_name[_data_size] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            string_populate(&handler_lua_context->_host_string, handler_lua_context->host, data_size, 0);
            string_populate(&handler_lua_context->_server_name_string, handler_lua_context->server_name, _data_size, 0);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_lua(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_lua(struct http_parser_t *http_parser) {
    /* allocates space for the error return value from the sending
    of the response (loading, execution, etc.) */
    ERROR_CODE return_value;
    struct handler_lua_context_t *handler_lua_context;

    /* sends (and creates) the response and retrieves the (possible)
    error code from it then in such case sends the error code to
    the connection through the upstream pipe */
    return_value = _send_response_handler_lua(http_parser);
    if(IS_ERROR_CODE(return_value)) {
        /* retrieves the handler Lua context to access the URL for
        logging, then logs the error at warning level for visibility */
        handler_lua_context =
            (struct handler_lua_context_t *) http_parser->context;
        V_WARNING_CTX_F("mod_lua", "Lua error for %s: %s\n", handler_lua_context ? (char *) handler_lua_context->url : "/", (char *) GET_ERROR());
        _write_error_connection_lua(http_parser, (char *) GET_ERROR());
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_lua(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* allocates space for the temporary file path size variables
    to be used in internal string size calculations */
    size_t file_path_size;

    /* retrieves the handler Lua context from the HTTP parser */
    struct handler_lua_context_t *handler_lua_context =
        (struct handler_lua_context_t *) http_parser->context;

    /* retrieves the connection from the parser and then uses it to retrieve the
    correct Lua HTTP handler reference from the HTTP connection */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct mod_lua_http_handler_t *mod_lua_http_handler =
        (struct mod_lua_http_handler_t *) http_connection->http_handler->lower;

    /* retrieves the current location from the location buffer and checks if the
    file path value is correctly set */
    struct mod_lua_location_t *location = &mod_lua_http_handler->locations[index];
    if(location->file_path[0] == '\0') { RAISE_NO_ERROR; }

    /* retrieves the size of the file path associated with the location and copies
    it to the current context file path string then updates the file path string
    to the required size */
    file_path_size = strlen(location->file_path);
    memcpy(handler_lua_context->file_path, location->file_path, file_path_size);
    handler_lua_context->file_path[file_path_size] = '\0';
    string_populate(
        &handler_lua_context->_file_path_string,
        handler_lua_context->file_path,
        file_path_size,
        0
    );

    /* sets the current location reference in the handler so that the
    response handler uses the location-specific Lua state and script
    path instead of the handler defaults */
    mod_lua_http_handler->current_location = location;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates space for the variable that will hold the new size
    of the prefix path to be used, by removing the new virtual URL
    from the previously set file name (path) */
    size_t prefix_size;

    /* retrieves the handler Lua context from the HTTP parser and then
    uses it to retrieve the size of the file name (path) for prefix
    size calculation (to be used as the prefix path) */
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;
    size_t _path_size = handler_lua_context->_file_name_string.length;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;
    size_t query_size = pointer == NULL ? 0 : data_size - path_size - 1;
    query_size = query_size > 0 ? query_size : 0;
    prefix_size = _path_size - path_size;

    /* sets the prefix name from the previously set file name path
    to be used for virtual URL location guide */
    memcpy(handler_lua_context->prefix_name, handler_lua_context->file_name, prefix_size);
    handler_lua_context->prefix_name[prefix_size] = '\0';

    /* copies the part of the data buffer relative to the file name
    this avoids copying the query part */
    memcpy(handler_lua_context->file_name, data, path_size);
    handler_lua_context->file_name[path_size] = '\0';
    normalize_path((char *) handler_lua_context->file_name);

    /* in case the pointer is defined (query separator found) copies
    the query contents into the target query buffer */
    if(pointer) { memcpy(handler_lua_context->query, pointer + 1, query_size); }
    handler_lua_context->query[query_size] = '\0';

    /* copies the URL to the URL reference in the handler file context to
    be used in the message handler */
    memcpy(handler_lua_context->url, data, data_size);
    handler_lua_context->url[data_size] = '\0';

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    string_populate(&handler_lua_context->_file_name_string, handler_lua_context->file_name, path_size, 0);
    string_populate(&handler_lua_context->_query_string, handler_lua_context->query, query_size, 0);
    string_populate(&handler_lua_context->_url_string, handler_lua_context->url, path_size, 0);
    string_populate(&handler_lua_context->_prefix_name_string, handler_lua_context->prefix_name, prefix_size, 0);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_lua(struct http_parser_t *http_parser) {
    /* allocates space for the handler Lua context and
    then creates and populates the instance after that
    sets the handler Lua context as the context for
    the HTTP parser */
    struct handler_lua_context_t *handler_lua_context;
    create_handler_lua_context(&handler_lua_context);
    http_parser->context = handler_lua_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_lua(struct http_parser_t *http_parser) {
    /* retrieves the handler Lua context from the HTTP parser
    and then deletes (releases memory) */
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;
    delete_handler_lua_context(handler_lua_context);
    http_parser->context = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_lua(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the HTTP settings
    structure, these callbacks are going to be used in the runtime
    processing of HTTP parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_module;
    http_settings->on_url = url_callback_handler_lua;
    http_settings->on_header_field = header_field_callback_handler_lua;
    http_settings->on_header_value = header_value_callback_handler_lua;
    http_settings->on_headers_complete = headers_complete_callback_handler_lua;
    http_settings->on_body = body_callback_handler_lua;
    http_settings->on_message_complete = message_complete_callback_handler_lua;
    http_settings->on_path = path_callback_handler_lua;
    http_settings->on_location = location_callback_handler_lua;
    http_settings->on_virtual_url = virtual_url_callback_handler_lua;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_lua(struct http_settings_t *http_settings) {
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

ERROR_CODE _send_response_handler_lua(struct http_parser_t *http_parser) {
    /* retrieves the connection from the HTTP parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* retrieves the HTTP connection from the io connection and uses it to retrieve
    the correct (mod Lua) handler to operate around it */
    struct http_connection_t *http_connection = (struct http_connection_t *) ((struct io_connection_t *) connection->lower)->lower;
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;
    struct mod_lua_http_handler_t *mod_lua_http_handler = (struct mod_lua_http_handler_t *) http_connection->http_handler->lower;

    /* allocates space for the result code and for the pointers that
    will resolve to either the location or handler level values */
    ERROR_CODE result_code;
    lua_State **lua_state_pointer;
    char *file_path;
    unsigned int *file_dirty_pointer;

    /* allocates space for the variables used to construct
    the HTTP response from the WSAPI return values */
    int status_code;
    const char *status_message;
    char *headers_buffer;
    size_t count;
    const char *body_chunk;
    size_t body_chunk_size;
    size_t body_size;
    char *response_buffer;
    char has_content_length;
    char _header_name_lower[VIRIATUM_MAX_HEADER_SIZE];
    char *error_message;
    const char *header_name;
    const char *header_value;
    luaL_Buffer lua_buffer;

    /* acquires the lock on the HTTP connection, this will avoid further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* resolves the active Lua state, file path, and dirty flag using
    location-specific values when a location is matched, otherwise
    falls back to the handler defaults */
    if(mod_lua_http_handler->current_location != NULL) {
        lua_state_pointer = &mod_lua_http_handler->current_location->lua_state;
        file_path = mod_lua_http_handler->current_location->file_path;
        file_dirty_pointer = &mod_lua_http_handler->current_location->file_dirty;
    } else {
        lua_state_pointer = &mod_lua_http_handler->lua_state;
        file_path = mod_lua_http_handler->file_path;
        file_dirty_pointer = &mod_lua_http_handler->file_dirty;
    }

    /* in case the Lua state is not started an error must
    have occurred so need to return immediately in error */
    if(*lua_state_pointer == NULL) {
        /* logs the error at warning level for console visibility
        then returns in error (the caller writes the error response) */
        V_WARNING_CTX_F("mod_lua", "Lua error, no state available for %s\n", file_path);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem accessing Lua state");
    }

    /* sets the flags from the current HTTP parser in the Lua
    context for later reference */
    handler_lua_context->flags = http_parser->flags;

    /* runs the script in case the current file is considered to be
    dirty, this is the case for the loading of the module and reloading */
    if(*file_dirty_pointer) {
        result_code = luaL_dofile(*lua_state_pointer, file_path);
        *file_dirty_pointer = 0;

        /* in case there was an error in Lua */
        if(LUA_ERROR(result_code)) {
            /* retrieves the error message and logs it at warning level
            for console visibility before sending the error response */
            error_message = (char *) lua_tostring(*lua_state_pointer, -1);
            V_WARNING_CTX_F(
                "mod_lua", "Lua error in %s: %s\n",
                file_path,
                error_message ? error_message : "(unknown error)"
            );

            /* pops the error message from the Lua stack before reloading
            the state to avoid leaving orphaned values */
            lua_pop(*lua_state_pointer, 1);

            /* sets the file as dirty (forces reload) and then reloads the current
            internal Lua state, virtual machine reset (to avoid corruption) */
            *file_dirty_pointer = 1;
            _reload_lua_state(lua_state_pointer);

            /* raises the error to the upper levels (the caller
            writes the error response to the connection) */
            RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem executing script file");
        }

        /* if the script returned a table (WSAPI module convention) store
        it as the _wsapi_app global for subsequent requests */
        if(lua_istable(*lua_state_pointer, -1)) {
            lua_setglobal(*lua_state_pointer, "_wsapi_app");
        } else {
            lua_pop(*lua_state_pointer, lua_gettop(*lua_state_pointer) > 0 ? 1 : 0);

            /* clears any stale _wsapi_app reference so that a broken
            reload does not fall back to the previous module table */
            lua_pushnil(*lua_state_pointer);
            lua_setglobal(*lua_state_pointer, "_wsapi_app");
        }
    }

    /* retrieves the field reference to the run function, first trying
    the global scope then falling back to the _wsapi_app module table */
    lua_getfield(*lua_state_pointer, LUA_GLOBALSINDEX, "run");

    if(lua_isnil(*lua_state_pointer, -1)) {
        lua_pop(*lua_state_pointer, 1);

        /* tries to retrieve the run function from the module table
        that should have been returned by the script on first load */
        lua_getglobal(*lua_state_pointer, "_wsapi_app");
        if(!lua_isnil(*lua_state_pointer, -1)) {
            lua_getfield(*lua_state_pointer, -1, "run");
            lua_remove(*lua_state_pointer, -2);
        }
    }

    /* in case neither approach found a run function this is an error
    condition that must be reported back */
    if(!lua_isfunction(*lua_state_pointer, -1)) {
        lua_pop(*lua_state_pointer, 1);
        V_WARNING_CTX_F("mod_lua", "Lua error in %s: no 'run' function found\n", file_path);
        *file_dirty_pointer = 1;
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "No run function in script");
    }

    /* builds the WSAPI environ table on the Lua stack */
    _start_environ_lua(*lua_state_pointer, http_parser);

    /* calls the run function with 1 argument (environ) expecting 3 return
    values: status code, headers table, body iterator */
    result_code = lua_pcall(*lua_state_pointer, 1, 3, 0);

    /* in case there was an error in Lua */
    if(LUA_ERROR(result_code)) {
        /* retrieves the error message and logs it at warning level
        for console visibility before sending the error response */
        error_message = (char *) lua_tostring(*lua_state_pointer, -1);
        V_WARNING_CTX_F(
            "mod_lua", "Lua error in %s: %s\n",
            file_path,
            error_message ? error_message : "(unknown error)"
        );

        /* pops the error message from the Lua stack to avoid
        leaving orphaned values on the stack */
        lua_pop(*lua_state_pointer, 1);

        /* sets the file reference as dirty, this will force the script file
        to be reloaded on next request */
        *file_dirty_pointer = 1;

        /* raises the error to the upper levels (the caller
        writes the error response to the connection) */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem calling the run function");
    }

    /* retrieves the three return values from the WSAPI run function:
    status code (integer), headers (table), body iterator (function)
    the values are on the stack as: [-3]=status [-2]=headers [-1]=iterator */
    status_code = (int) lua_tointeger(*lua_state_pointer, -3);
    if(status_code == 0) { status_code = 200; }

    /* determines the status message based on the status code */
    switch(status_code) {
        case 200:
            status_message = "OK";
            break;
        case 201:
            status_message = "Created";
            break;
        case 204:
            status_message = "No Content";
            break;
        case 301:
            status_message = "Moved Permanently";
            break;
        case 302:
            status_message = "Found";
            break;
        case 304:
            status_message = "Not Modified";
            break;
        case 400:
            status_message = "Bad Request";
            break;
        case 401:
            status_message = "Unauthorized";
            break;
        case 403:
            status_message = "Forbidden";
            break;
        case 404:
            status_message = "Not Found";
            break;
        case 405:
            status_message = "Method Not Allowed";
            break;
        case 500:
            status_message = "Internal Server Error";
            break;
        case 502:
            status_message = "Bad Gateway";
            break;
        case 503:
            status_message = "Service Unavailable";
            break;
        default:
            status_message = "Unknown";
            break;
    }

    /* allocates space for the header buffer and then writes the default values
    into it the value is dynamically constructed based on the current header values */
    connection->alloc_data(connection, VIRIATUM_HTTP_MAX_SIZE, (void **) &headers_buffer);
    count = http_connection->write_headers(
        connection,
        headers_buffer,
        VIRIATUM_HTTP_SIZE,
        HTTP11,
        status_code,
        (char *) status_message,
        http_parser->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
        FALSE
    );

    /* tracks whether the script already provided a Content-Length
    header so that one can be auto-inserted when missing */
    has_content_length = FALSE;

    /* iterates over all the headers present in the Lua headers table
    (at stack position -2) and copies their content into the headers buffer */
    if(lua_istable(*lua_state_pointer, -2)) {
        lua_pushnil(*lua_state_pointer);
        while(lua_next(*lua_state_pointer, -3) != 0) {
            /* retrieves the header name and value from the table */
            header_name = lua_tostring(*lua_state_pointer, -2);
            header_value = lua_tostring(*lua_state_pointer, -1);

            /* copies the current header into the current position of the headers
            buffer (header copy), note that the trailing newlines are counted in size */
            if(header_name != NULL && header_value != NULL) {
                count += SPRINTF(
                    &headers_buffer[count],
                    VIRIATUM_MAX_HEADER_C_SIZE,
                    "%s: %s\r\n",
                    header_name,
                    header_value
                );

                /* checks if this is a Content-Length header (case-insensitive)
                to avoid inserting a duplicate one later */
                strncpy(_header_name_lower, header_name, VIRIATUM_MAX_HEADER_SIZE - 1);
                _header_name_lower[VIRIATUM_MAX_HEADER_SIZE - 1] = '\0';
                lowercase(_header_name_lower);
                if(strcmp(_header_name_lower, "content-length") == 0) {
                    has_content_length = TRUE;
                }
            }

            /* pops the value but keeps the key for the next iteration */
            lua_pop(*lua_state_pointer, 1);
        }
    }

    /* collects the body by calling the iterator function repeatedly until
    it returns nil, concatenating all the chunks into a single buffer */
    body_size = 0;

    if(lua_isfunction(*lua_state_pointer, -1)) {
        /* uses a temporary Lua buffer to collect all the body chunks
        from the iterator, this avoids multiple memory allocations */
        luaL_buffinit(*lua_state_pointer, &lua_buffer);

        while(TRUE) {
            /* pushes a copy of the iterator function and calls it
            with zero arguments expecting one result */
            lua_pushvalue(*lua_state_pointer, -1);
            result_code = lua_pcall(*lua_state_pointer, 0, 1, 0);

            /* in case of error breaks the iteration cycle */
            if(LUA_ERROR(result_code)) {
                lua_pop(*lua_state_pointer, 1);
                break;
            }

            /* in case the result is nil this is the end of the
            iteration cycle and so the loop must be broken */
            if(lua_isnil(*lua_state_pointer, -1)) {
                lua_pop(*lua_state_pointer, 1);
                break;
            }

            /* retrieves the current chunk as a string and adds
            it to the Lua buffer for later concatenation */
            body_chunk = lua_tolstring(*lua_state_pointer, -1, &body_chunk_size);
            if(body_chunk != NULL && body_chunk_size > 0) {
                luaL_addlstring(&lua_buffer, body_chunk, body_chunk_size);
                body_size += body_chunk_size;
            }

            /* pops the chunk value from the stack */
            lua_pop(*lua_state_pointer, 1);
        }

        /* pushes the final concatenated result */
        luaL_pushresult(&lua_buffer);
    }

    /* in case the script did not provide a Content-Length header
    auto-inserts one based on the collected body size */
    if(!has_content_length) {
        count += SPRINTF(
            &headers_buffer[count],
            VIRIATUM_MAX_HEADER_C_SIZE,
            "Content-Length: %lu\r\n",
            (unsigned long) body_size
        );
    }

    /* finishes the current headers sequence with the final carriage return newline
    character values to close the headers part of the envelope */
    memcpy(&headers_buffer[count], "\r\n", 2);
    count += 2;

    /* allocates the final response buffer containing both headers and
    body then copies the headers and body data into it */
    connection->alloc_data(connection, count + body_size, (void **) &response_buffer);
    memcpy(response_buffer, headers_buffer, count);

    /* releases the headers buffer as it has been copied into the
    response buffer and is no longer needed */
    FREE(headers_buffer);

    /* in case there is body data available copies it into the
    response buffer after the headers */
    if(body_size > 0) {
        body_chunk = lua_tolstring(*lua_state_pointer, -1, &body_chunk_size);
        if(body_chunk != NULL) { memcpy(&response_buffer[count], body_chunk, body_size); }
        lua_pop(*lua_state_pointer, 1);
    }

    /* pops the three WSAPI return values from the stack
    (status, headers, iterator) */
    lua_pop(*lua_state_pointer, 3);

    /* writes the complete response (headers + body) to the connection */
    connection->write_connection(
        connection,
        (unsigned char *) response_buffer,
        (unsigned int) (count + body_size),
        _send_response_callback_handler_lua,
        (void *) (size_t) http_parser->flags
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_lua(struct connection_t *connection, struct data_t *data, void *parameters) {
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

ERROR_CODE _write_error_connection_lua(struct http_parser_t *http_parser, char *message) {
    /* allocates space for the buffer to be used in the message */
    unsigned char *buffer;

    /* retrieves the connection from the HTTP parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* allocates the data buffer (in a safe manner) then
    writes the HTTP static headers to the response */
    connection->alloc_data(connection, VIRIATUM_HTTP_SIZE, (void **) &buffer);
    http_connection->write_error(
        connection,
        (char *) buffer,
        VIRIATUM_HTTP_SIZE,
        HTTP11,
        500,
        "Internal Server Error",
        message,
        http_parser->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
        _send_response_callback_handler_lua,
        (void *) (size_t) http_parser->flags
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _start_environ_lua(lua_State *lua_state, struct http_parser_t *http_parser) {
    /* allocates space for the counter to be used for iteration
    on the various header values */
    size_t index;

    /* allocates space for the final name of the exporting header,
    for the temporary header value structure pointer */
    char name[VIRIATUM_MAX_HEADER_SIZE + sizeof("HTTP_")];
    struct http_header_value_t *header;

    /* allocates the buffer reference to be used to store the
    data from the post part of the message */
    unsigned char *post_data;

    /* retrieves the connection from the HTTP parser parameters
    and then retrieves the handler Lua context */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct handler_lua_context_t *handler_lua_context = (struct handler_lua_context_t *) http_parser->context;

    /* retrieves the port (string representation) from the service
    options associated with the current connection */
    unsigned char *port = connection->service->options->_port;

    /* retrieves the HTTP method string value accessing the array of
    static string values */
    char *method = (char *) http_method_strings[http_parser->method - 1];

    /* in case there is contents to be read retrieves the appropriate
    reference to the start of the post data in the connection buffer */
    if(http_parser->_content_length > 0) {
        post_data = &http_connection->buffer[http_connection->buffer_offset - http_parser->_content_length];
    } else {
        post_data = NULL;
    }

    /* creates a new table to hold the WSAPI environ dictionary */
    lua_newtable(lua_state);

    /* iterates over all the headers in order to "export" them
    into the environ table to expose them to the application */
    for(index = 0; index < _headers.count; index++) {
        /* retrieves the current header structure for the
        iteration cycle in course */
        header = &_headers.values[index];

        /* copies the header name into a local buffer and converts
        it to uppercase, avoiding mutation of the shared headers */
        SPRINTF(name, VIRIATUM_MAX_HEADER_SIZE + sizeof("HTTP_"), "HTTP_%s", header->name);
        uppercase(&name[sizeof("HTTP_") - 1]);

        /* sets the value in the environ table using the
        HTTP_ prefixed header name */
        lua_pushstring(lua_state, header->value);
        lua_setfield(lua_state, -2, name);
    }

    /* sets the REQUEST_METHOD value in the environ table */
    lua_pushstring(lua_state, method);
    lua_setfield(lua_state, -2, "REQUEST_METHOD");

    /* sets the SCRIPT_NAME value in the environ table */
    lua_pushstring(lua_state, (char *) handler_lua_context->prefix_name);
    lua_setfield(lua_state, -2, "SCRIPT_NAME");

    /* sets the PATH_INFO value in the environ table */
    lua_pushstring(lua_state, (char *) handler_lua_context->file_name);
    lua_setfield(lua_state, -2, "PATH_INFO");

    /* sets the QUERY_STRING value in the environ table */
    lua_pushstring(lua_state, (char *) handler_lua_context->query);
    lua_setfield(lua_state, -2, "QUERY_STRING");

    /* sets the SERVER_NAME value in the environ table */
    lua_pushstring(lua_state, (char *) handler_lua_context->server_name);
    lua_setfield(lua_state, -2, "SERVER_NAME");

    /* sets the SERVER_PORT value in the environ table */
    lua_pushstring(lua_state, (char *) port);
    lua_setfield(lua_state, -2, "SERVER_PORT");

    /* sets the SERVER_PROTOCOL value in the environ table */
    lua_pushstring(lua_state, "HTTP/1.1");
    lua_setfield(lua_state, -2, "SERVER_PROTOCOL");

    /* sets the CONTENT_TYPE value in the environ table in case
    a content type header was captured during parsing */
    if(handler_lua_context->_content_type_string.length > 0) {
        lua_pushstring(lua_state, (char *) handler_lua_context->content_type);
        lua_setfield(lua_state, -2, "CONTENT_TYPE");
    }

    /* sets the CONTENT_LENGTH value in the environ table in case
    a content length header was captured during parsing */
    if(handler_lua_context->_content_length_string.length > 0) {
        lua_pushstring(lua_state, (char *) handler_lua_context->content_length_);
        lua_setfield(lua_state, -2, "CONTENT_LENGTH");
    }

    /* creates and pushes the input userdata object that provides
    a read() method for accessing the request body (WSAPI input) */
    lua_input_push(lua_state, post_data, http_parser->_content_length);
    lua_setfield(lua_state, -2, "input");

    /* creates a simple table with a write() method that logs to the
    server error log (WSAPI error stream) */
    lua_newtable(lua_state);
    lua_pushcfunction(lua_state, lua_error_write);
    lua_setfield(lua_state, -2, "write");
    lua_setfield(lua_state, -2, "error");

    /* raises no error */
    RAISE_NO_ERROR;
}
