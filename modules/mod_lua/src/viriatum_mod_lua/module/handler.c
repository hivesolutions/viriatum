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
    mod_lua_http_handler->file_path = NULL;
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
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
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
    /* sends (and creates) the reponse */
    _send_response_handler_lua(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_lua(struct http_parser_t *http_parser, size_t index, size_t offset) {
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
    if(location->file_path == NULL) { RAISE_NO_ERROR; }

    /* sets the current location reference in the handler so that the
    response handler uses the location-specific Lua state and script
    path instead of the handler defaults */
    mod_lua_http_handler->current_location = location;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_lua(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_lua(struct http_parser_t *http_parser) {
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
    struct mod_lua_http_handler_t *mod_lua_http_handler = (struct mod_lua_http_handler_t *) http_connection->http_handler->lower;

    /* allocates space for the result code and for the pointers that
    will resolve to either the location or handler level values */
    ERROR_CODE result_code;
    lua_State **lua_state_pointer;
    char *file_path;
    unsigned int *file_dirty_pointer;

    /* acquires the lock on the HTTP connection, this will avoids further
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
        then writes the error to the connection and returns in error */
        V_WARNING_CTX_F("mod_lua", "Lua error, no state available for %s\n", file_path);
        _write_error_connection_lua(http_parser, "no Lua state available");
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem accessing Lua state");
    }

    /* registers the current connection in Lua */
    lua_pushlightuserdata(*lua_state_pointer, (void *) http_parser);
    lua_setglobal(*lua_state_pointer, "connection");

    /* registers the write connection function in Lua */
    lua_register(*lua_state_pointer, "write_connection", _lua_write_connection);

    /* runs the script in case the current file is considered to be
    dirty, this is the case for the loading of the module and reloading*/
    if(*file_dirty_pointer) { result_code = luaL_dofile(*lua_state_pointer, file_path); *file_dirty_pointer = 0; }
    else { result_code = 0; }

    /* in case there was an error in Lua */
    if(LUA_ERROR(result_code)) {
        /* retrieves the error message and logs it at warning level
        for console visibility before sending the error response */
        char *error_message = (char *) lua_tostring(*lua_state_pointer, -1);
        V_WARNING_CTX_F("mod_lua", "Lua error in %s: %s\n", file_path, error_message);
        _write_error_connection_lua(http_parser, error_message);

        /* sets the file as dirty (forces reload) and then reloads the curernt
        internal Lua state, virtual machine reset (to avoid corruption) */
        *file_dirty_pointer = 1;
        _reload_lua_state(lua_state_pointer);

        /* raises the error to the upper levels */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem executing script file");
    }

    /* retrieves the field reference to the handle method that symbol
    to call the function (in protected mode) and retrieves the result */
    lua_getfield(*lua_state_pointer, LUA_GLOBALSINDEX, "handle");
    result_code = lua_pcall(*lua_state_pointer, 0, 0, 0);

    /* in case there was an error in Lua */
    if(LUA_ERROR(result_code)) {
        /* retrieves the error message and logs it at warning level
        for console visibility before sending the error response */
        char *error_message = (char *) lua_tostring(*lua_state_pointer, -1);
        V_WARNING_CTX_F("mod_lua", "Lua error in %s: %s\n", file_path, error_message);
        _write_error_connection_lua(http_parser, error_message);

        /* sets the file reference as dirty, this will force the script file
        to be reload on next request */
        *file_dirty_pointer = 1;

        /* raises the error to the upper levels */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem calling the handle method");
    }

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

int _lua_write_connection(lua_State *lua_state) {
    /* allocates the data (original message data) */
    const char *data;

    /* allocates the (final) buffer to send the message */
    unsigned char *buffer;

    /* allocates the data size */
    unsigned int data_size;

    /* allocates the HTTP parser */
    struct http_parser_t *http_parser;

    /* allocates the connection */
    struct connection_t *connection;

    /* retrieves the number of (received) arguments */
    unsigned int number_arguments = lua_gettop(lua_state);

    /* in case the number of arguments is invalid */
    if(number_arguments != 3) {
        /* prints a warning message */
        V_WARNING_CTX("mod_lua", "Invalid number of arguments\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Invalid number of arguments");
        lua_error(lua_state);
    }

    if(!lua_isnumber(lua_state, -1)) {
        /* prints a warning message */
        V_WARNING_CTX("mod_lua", "Incorrect argument 'expected number'\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Incorrect argument to 'expected number'");
        lua_error(lua_state);
    }

    if(!lua_isstring(lua_state, -2)) {
        /* prints a warning message */
        V_WARNING_CTX("mod_lua", "Incorrect argument 'expected string'\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Incorrect argument to 'expected string'");
        lua_error(lua_state);
    }

    if(!lua_islightuserdata(lua_state, -3)) {
        /* prints a warning message */
        V_WARNING_CTX("mod_lua", "Incorrect argument 'expected lightuserdata'\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Incorrect argument 'expected lightuserdata'");
        lua_error(lua_state);
    }

    /* converts the third argument into an integer */
    data_size = lua_tointeger(lua_state, -1);

    /* converts the second argument into a string */
    data = lua_tostring(lua_state, -2);

    /* converts the first argument into HTTP parser structure */
    http_parser = (struct http_parser_t *) lua_touserdata(lua_state, -3);

    /* retrieves the connection from the HTTP parser parameters */
    connection = (struct connection_t *) http_parser->parameters;

    /* allocates the data buffer (in a safe maner) then
    copies the data (from Lua) into the buffer */
    connection->alloc_data(connection, data_size * sizeof(unsigned char), (void **) &buffer);
    memcpy(buffer, data, data_size);

    /* writes the response to the connection */
    connection->write_connection(
        connection,
        buffer,
        data_size,
        _send_response_callback_handler_lua,
        (void *) (size_t) http_parser->flags
    );

    /* return the number of results */
    return 0;
}
