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

#include "handler.h"

/**
 * The path to the file to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_FILE_PATH "default.lua"

/**
 * Structure describing the internal
 * structures and information for the
 * mod Lua module.
 */
typedef struct mod_lua_module_t {
    /**
     * The global Lua state used over
     * all the operations in Lua.
     */
    lua_State *lua_state;

    /**
     * The HTTP handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod Lua HTTP handler associated
     * with the module.
     */
    struct mod_lua_http_handler_t *mod_lua_http_handler;
} mod_lua_module;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_lua_module(struct mod_lua_module_t **mod_lua_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_lua_module(struct mod_lua_module_t *mod_lua_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module_lua(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module_lua(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module_lua(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module_lua(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration_lua(struct service_t *service, struct mod_lua_http_handler_t *mod_lua_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_lua_state(lua_State **lua_state_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_lua_state(lua_State **lua_state_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reload_lua_state(lua_State **lua_state_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_lua_state(lua_State *lua_state);
