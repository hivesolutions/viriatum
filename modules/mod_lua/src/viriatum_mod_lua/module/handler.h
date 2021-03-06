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

#pragma once

#include "entry.h"

#define LUA_ERROR(error) error > 0

/**
 * The structure that holds the internal
 * structure to support the context
 * of the Lua module.
 */
typedef struct mod_lua_http_handler_t {
    /**
     * The global Lua state used over
     * all the operations in Lua.
     */
    lua_State *lua_state;

    /**
     * The path to the default file to
     * be used for the parsing.
     */
    char *file_path;

    /**
     * Flag that controls if the script file
     * to be executed is currently dirty.
     */
    unsigned int file_dirty;
} mod_lua_http_handler;

ERROR_CODE create_mod_lua_http_handler(struct mod_lua_http_handler_t **mod_lua_http_handler_pointer, struct http_handler_t *http_handler_pointer);
ERROR_CODE delete_mod_lua_http_handler(struct mod_lua_http_handler_t *mod_lua_http_handler);
ERROR_CODE set_handler_lua(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_lua(struct http_connection_t *http_connection);
ERROR_CODE url_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_lua(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_lua(struct http_parser_t *http_parser);
ERROR_CODE path_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_lua(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_lua(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_parser_handler_lua(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_lua(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_lua(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_lua(struct http_settings_t *http_settings);
ERROR_CODE _message_begin_callback_handler_lua(struct http_parser_t *http_parser);
ERROR_CODE _send_response_handler_lua(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_lua(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _write_error_connection_lua(struct http_parser_t *http_parser, char *message);
int _lua_write_connection(lua_State *lua_state);
