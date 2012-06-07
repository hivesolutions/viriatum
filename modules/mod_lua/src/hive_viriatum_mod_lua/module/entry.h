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
 * mod lua module.
 */
typedef struct mod_lua_module_t {
    /**
     * The global lua state used over
     * all the operations in lua.
     */
    lua_State *lua_state;

    /**
     * The http handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod lua http handler associated
     * with the module.
     */
    struct mod_lua_http_handler_t *mod_lua_http_handler;
} mod_lua_module;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_lua_module(struct mod_lua_module_t **mod_lua_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_lua_module(struct mod_lua_module_t *mod_lua_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration(struct service_t *service, struct mod_lua_http_handler_t *mod_lua_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_lua_state(lua_State **lua_state_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_lua_state(lua_State **lua_state_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reload_lua_state(lua_State **lua_state_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_lua_state(lua_State *lua_state);
