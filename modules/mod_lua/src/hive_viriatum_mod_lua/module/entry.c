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

unsigned char *lastErrorMessage;

ERROR_CODE startModule(struct Environment_t *environment, struct Module_t *module) {
    /* allocates the lua state */
    lua_State *luaState;

    /* retrieves the name */
    unsigned char *name = nameViriatumModLua();

    /* retrieves the version */
    unsigned char *version = versionViriatumModLua();

    /* retrieves the description */
    unsigned char *description = descriptionViriatumModLua();


    struct HttpHandler_t *httpHandler;

    struct ModLuaHttpHandler_t *modLuaHttpHandler;


    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* populates the module structure */
    infoModule(module);

    /* initializes the lua interpreter */
    luaState = lua_open();

    /* load various lua libraries */
    luaL_openlibs(luaState);





    /* creates the http handler */
    createHttpHandler(&httpHandler);

    /* creates the mod lua http handler */
    createModLuaHttpHandler(&modLuaHttpHandler, httpHandler);


    httpHandler->set = setHandlerModule;
    httpHandler->unset = unsetHandlerModule;
    httpHandler->reset = NULL;

    modLuaHttpHandler->luaState = luaState;




    /* adds the http handler to the list of handlers in the service */
    appendValueLinkedList(environment->service->httpHandlersList, (void *) httpHandler);











    /* cleanup lua */
    /*lua_close(luaState);*/

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopModule() {
    /* retrieves the name */
    unsigned char *name = nameViriatumModLua();

    /* retrieves the version */
    unsigned char *version = versionViriatumModLua();

    /* retrieves the description */
    unsigned char *description = descriptionViriatumModLua();

    /* prints a debug message */
    V_DEBUG_F("Stoping the module '%s' (%s) v%s\n", name, description, version);

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
