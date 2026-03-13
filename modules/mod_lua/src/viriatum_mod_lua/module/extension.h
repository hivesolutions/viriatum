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

#pragma once

#include "entry.h"

/**
 * The name used for the Lua module that exposes
 * the viriatum API to Lua scripts.
 */
#define VIRIATUM_LUA_MODULE_NAME "viriatum"

/**
 * The name used for the Lua input metatable that
 * provides the WSAPI input stream interface.
 */
#define VIRIATUM_LUA_INPUT_NAME "viriatum.input"

/**
 * Structure representing the WSAPI input stream
 * object, used to read the request body from Lua.
 */
typedef struct lua_input_t {
    /**
     * The pointer to the raw post data buffer
     * owned by the HTTP connection.
     */
    unsigned char *post_data;

    /**
     * The current read position in the post
     * data buffer.
     */
    size_t position;

    /**
     * The total size of the post data buffer
     * in bytes.
     */
    size_t size;
} lua_input;

VIRIATUM_EXTERNAL_PREFIX luaL_Reg viriatum_methods[6];

int lua_viriatum_connections(lua_State *lua_state);
int lua_viriatum_connections_l(lua_State *lua_state);
int lua_viriatum_connection_info(lua_State *lua_state);
int lua_viriatum_uptime(lua_State *lua_state);
int lua_viriatum_modules(lua_State *lua_state);

int lua_input_new(lua_State *lua_state);
int lua_input_read(lua_State *lua_state);
int lua_input_close(lua_State *lua_state);
int lua_input_readline(lua_State *lua_state);
int lua_input_readlines(lua_State *lua_state);
int lua_input_gc(lua_State *lua_state);
int lua_input_tostring(lua_State *lua_state);
void lua_input_push(lua_State *lua_state, unsigned char *post_data, size_t size);

int lua_error_write(lua_State *lua_state);
