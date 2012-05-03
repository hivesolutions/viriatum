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
typedef struct ModLuaModule_t {
    /**
     * The global lua state used over
     * all the operations in lua.
     */
    lua_State *luaState;

    struct HttpHandler_t *httpHandler;

    /**
     * The mod lua http handler associated
     * with the module.
     */
    struct ModLuaHttpHandler_t *modLuaHttpHandler;
} ModLuaModule;

VIRIATUM_EXPORT_PREFIX ERROR_CODE createModLuaModule(struct ModLuaModule_t **modLuaModulePointer, struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE deleteModLuaModule(struct ModLuaModule_t *modLuaModule);
VIRIATUM_EXPORT_PREFIX ERROR_CODE startModule(struct Environment_t *environment, struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stopModule(struct Environment_t *environment, struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE infoModule(struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE errorModule(unsigned char **messagePointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _loadConfiguration(struct Service_t *service, struct ModLuaHttpHandler_t *modLuaHttpHandler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _startLuaState(lua_State *luaState);
