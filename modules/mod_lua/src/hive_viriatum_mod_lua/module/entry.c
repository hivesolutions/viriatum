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

#include "entry.h"

ERROR_CODE createModLuaModule(struct ModLuaModule_t **modLuaModulePointer, struct Module_t *module) {
    /* retrieves the mod lua module size */
    size_t modLuaModuleSize = sizeof(struct ModLuaModule_t);

    /* allocates space for the mod lua module */
    struct ModLuaModule_t *modLuaModule = (struct ModLuaModule_t *) MALLOC(modLuaModuleSize);

     /* sets the mod lua module attributes (default) values */
    modLuaModule->luaState = NULL;
    modLuaModule->httpHandler = NULL;
    modLuaModule->modLuaHttpHandler = NULL;

    /* sets the mod lua module in the (upper) module substrate */
    module->lower = (void *) modLuaModule;

    /* sets the mod lua module in the module pointer */
    *modLuaModulePointer = modLuaModule;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteModLuaModule(struct ModLuaModule_t *modLuaModule) {
    /* releases the mod lua module */
    FREE(modLuaModule);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE startModule(struct Environment_t *environment, struct Module_t *module) {
    /* allocates the lua state */
    lua_State *luaState;

    /* allocates the mod lua module */
    struct ModLuaModule_t *modLuaModule;

    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* allocates the mod lua http handler */
    struct ModLuaHttpHandler_t *modLuaHttpHandler;

    /* retrieves the name */
    unsigned char *name = nameViriatumModLua();

    /* retrieves the version */
    unsigned char *version = versionViriatumModLua();

    /* retrieves the description */
    unsigned char *description = descriptionViriatumModLua();

    /* retrieves the (environment) service */
    struct Service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* creates the mod lua module */
    createModLuaModule(&modLuaModule, module);

    /* populates the module structure */
    infoModule(module);

    /* initializes the lua interpreter */
    luaState = lua_open();

    /* load various lua libraries */
    luaL_openlibs(luaState);

    /* creates the http handler */
    service->createHttpHandler(service, &httpHandler);

    /* creates the mod lua http handler */
    createModLuaHttpHandler(&modLuaHttpHandler, httpHandler);

    /* sets the http handler attributes */
    httpHandler->set = setHandlerModule;
    httpHandler->unset = unsetHandlerModule;
    httpHandler->reset = NULL;

    /* sets the mod lua handler attributes */
    modLuaHttpHandler->luaState = luaState;

    /* sets the mod lua module attributes */
    modLuaModule->luaState = luaState;
    modLuaModule->httpHandler = httpHandler;
    modLuaModule->modLuaHttpHandler = modLuaHttpHandler;

    /* adds the http handler to the service */
    service->addHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopModule(struct Environment_t *environment, struct Module_t *module) {
    /* retrieves the name */
    unsigned char *name = nameViriatumModLua();

    /* retrieves the version */
    unsigned char *version = versionViriatumModLua();

    /* retrieves the description */
    unsigned char *description = descriptionViriatumModLua();

    /* retrieves the (environment) service */
    struct Service_t *service = environment->service;

    /* retrieves the mod lua module (from the module) */
    struct ModLuaModule_t *modLuaModule = (struct  ModLuaModule_t *) module->lower;

    /* retrieves the lua state from the mod lua module */
    lua_State *luaState = modLuaModule->luaState;

    /* retrieves the http handler from the mod lua module */
    struct HttpHandler_t *httpHandler = modLuaModule->httpHandler;

    /* retrieves the mod lua http handler from the mod lua module */
    struct ModLuaHttpHandler_t *modLuaHttpHandler = modLuaModule->modLuaHttpHandler;

    /* prints a debug message */
    V_DEBUG_F("Stoping the module '%s' (%s) v%s\n", name, description, version);

    /* removes the http handler from the service */
    service->removeHttpHandler(service, httpHandler);

    /* in case the mod lua http handler is valid and
    initialized (correct state) */
    if(modLuaHttpHandler != NULL) {
        /* deletes the mod lua http handler */
        deleteModLuaHttpHandler(modLuaHttpHandler);
    }

    /* in case the http handler is valid and
    initialized (correct state) */
    if(httpHandler != NULL) {
        /* deletes the http handler */
        service->deleteHttpHandler(service, httpHandler);
    }

    /* in case the lua state is valid and
    initialized (correct state) */
    if(luaState != NULL) {
        /* cleanup lua */
        lua_close(luaState);
    }

    /* deletes the mod lua module */
    deleteModLuaModule(modLuaModule);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE infoModule(struct Module_t *module) {
    /* retrieves the name */
    unsigned char *name = nameViriatumModLua();

    /* retrieves the version */
    unsigned char *version = versionViriatumModLua();

    /* populates the module structure */
    module->name = name;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = startModule;
    module->stop = stopModule;
    module->info = infoModule;
    module->error = errorModule;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE errorModule(unsigned char **messagePointer) {
    /* sets the error message in the (erro) message pointer */
    *messagePointer = lastErrorMessage;

    /* raises no error */
    RAISE_NO_ERROR;
}
