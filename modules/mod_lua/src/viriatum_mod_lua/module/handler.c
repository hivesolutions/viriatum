/*
 Hive Viriatum Modules
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
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

    /* sets the mod Lua HTTP handler in the upper HTTP handler substrate */
    http_handler->lower = (void *) mod_lua_http_handler;

    /* sets the mod Lua HTTP handler in the mod Lua HTTP handler pointer */
    *mod_lua_http_handler_pointer = mod_lua_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_lua_http_handler(struct mod_lua_http_handler_t *mod_lua_http_handler) {
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

    /* allocates space for the result code */
    ERROR_CODE result_code;

    /* acquires the lock on the HTTP connection, this will avoids further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* in case the Lua state is not started an error must
    have occured so need to return immediately in error */
    if(mod_lua_http_handler->lua_state == NULL) {
        /* writes the error to the connection and then returns
        in error to the caller function */
        _write_error_connection_lua(http_parser, "no Lua state available");
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem accessing Lua state");
    }

    /* registers the current connection in Lua */
    lua_pushlightuserdata(mod_lua_http_handler->lua_state, (void *) http_parser);
    lua_setglobal(mod_lua_http_handler->lua_state, "connection");

    /* registers the write connection function in Lua */
    lua_register(mod_lua_http_handler->lua_state, "write_connection", _lua_write_connection);

    /* runs the script in case the current file is considered to be
    dirty, this is the case for the loading of the module and reloading*/
    if(mod_lua_http_handler->file_dirty) { result_code = luaL_dofile(mod_lua_http_handler->lua_state, mod_lua_http_handler->file_path); mod_lua_http_handler->file_dirty = 0; }
    else { result_code = 0; }

    /* in case there was an error in Lua */
    if(LUA_ERROR(result_code)) {
        /* retrieves the error message and then writes it to the connection
        so that the end user may be able to respond to it */
        char *error_message = (char *) lua_tostring(mod_lua_http_handler->lua_state, -1);
        _write_error_connection_lua(http_parser, error_message);

        /* sets the file as dirty (forces reload) and then reloads the curernt
        internal Lua state, virtual machine reset (to avoid corruption) */
        mod_lua_http_handler->file_dirty = 1;
        _reload_lua_state(&mod_lua_http_handler->lua_state);

        /* prints a warning message, closes the Lua interpreter and then
        raises the error to the upper levels */
        V_WARNING_F("There was a problem executing: %s\n", mod_lua_http_handler->file_path);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem executing script file");
    }

    /* retrieves the field reference to the handle method that symbol
    to call the function (in protected mode) and retrieves the result */
    lua_getfield(mod_lua_http_handler->lua_state, LUA_GLOBALSINDEX, "handle");
    result_code = lua_pcall(mod_lua_http_handler->lua_state, 0, 0, 0);

    /* in case there was an error in Lua */
    if(LUA_ERROR(result_code)) {
        /* retrieves the error message and then writes it to the connection
        so that the end user may be able to respond to it */
        char *error_message = (char *) lua_tostring(mod_lua_http_handler->lua_state, -1);
        _write_error_connection_lua(http_parser, error_message);

        /* sets the file reference as dirty, this will force the script file
        to be reload on next request */
        mod_lua_http_handler->file_dirty = 1;

        /* prints a warning message, closes the Lua interpreter and then
        raises the error to the upper levels */
        V_WARNING_F("There was a problem running call on file: %s\n", mod_lua_http_handler->file_path);
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

    /* allocates the data buffer (in a safe maner) then
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
        V_WARNING("Invalid number of arguments\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Invalid number of arguments");
        lua_error(lua_state);
    }

    if(!lua_isnumber(lua_state, -1)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected number'\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Incorrect argument to 'expected number'");
        lua_error(lua_state);
    }

    if(!lua_isstring(lua_state, -2)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected string'\n");

        /* pushes an error message to Lua */
        lua_pushstring(lua_state, "Incorrect argument to 'expected string'");
        lua_error(lua_state);
    }

    if(!lua_islightuserdata(lua_state, -3)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected lightuserdata'\n");

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
