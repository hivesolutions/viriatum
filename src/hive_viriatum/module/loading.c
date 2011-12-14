/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "loading.h"

ERROR_CODE createEnvironment(struct Environment_t **environmentPointer) {
    /* retrieves the environment size */
    size_t environmentSize = sizeof(struct Environment_t);

    /* allocates space for the environment */
    struct Environment_t *environment = (struct Environment_t *) MALLOC(environmentSize);

    /* sets the environment attributes (default) values */
    environment->name = NULL;

    /* sets the environment in the environment pointer */
    *environmentPointer = environment;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteEnvironment(struct Environment_t *environment) {
    /* releases the environment */
    FREE(environment);

    /* raises no environment */
    RAISE_NO_ERROR;
}

ERROR_CODE createModule(struct Module_t **modulePointer) {
    /* retrieves the module size */
    size_t moduleSize = sizeof(struct Module_t);

    /* allocates space for the module */
    struct Module_t *module = (struct Module_t *) MALLOC(moduleSize);

     /* sets the module attributes (default) values */
    module->name = NULL;
    module->version = NULL;
    module->type = 0;
    module->start = NULL;
    module->stop = NULL;
    module->info = NULL;
    module->error = NULL;
    module->lower = NULL;

    /* sets the module in the module pointer */
    *modulePointer = module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteModule(struct Module_t *module) {
    /* releases the module */
    FREE(module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE loadModule(struct Service_t *service, unsigned char *modulePath) {
    /* error code to be used for testing */
    ERROR_CODE errorCode;

    /* the mod library reference */
    LIBRARY_REFERENCE modLibrary;

    /* the holder of the library symbol */
    LIBRARY_SYMBOL symbol;

    /* the environment parameters reference */
    struct Environment_t *environment;

    /* the module structure reference */
    struct Module_t *module;

    /* the info module function reference */
    viriatumInfoModule infoModuleFunction;

    /* loads the mod library (tries to find the file) */
    modLibrary = LOAD_LIBRARY((const char *) modulePath);

    /* in case the mod library was not loaded */
    if(modLibrary == NULL) {
        /* prints a warning message */
        V_WARNING("Error loading library (File not found)\n");

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Error loading library");
    } else {
        /* prints a debug message */
        V_DEBUG("Loaded library\n");
    }

    /* retrieves the symbol from the mod library */
    symbol = GET_LIBRARY_SYMBOL(modLibrary, "infoModule");

    /* retrieves the info module function reference */
    infoModuleFunction = *(viriatumInfoModule *) (void *) &symbol;

    /* in case the start module function was not found */
    if(infoModuleFunction == NULL) {
        /* prints a warning message */
        V_WARNING_F("No such symbol '%s' in library\n", "infoModule");

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem finding symbol");
    } else {
        /* prints a debug message */
        V_DEBUG_F("Found symbol '%s' in library\n", "infoModule");
    }

    /* creates the environment */
    createEnvironment(&environment);

    /* creates the module */
    createModule(&module);

    /* sets the module attributes */
    module->environment = environment;

    /* sets the environment attributes */
    environment->service = service;

    /* calls the info module function */
    errorCode = infoModuleFunction(module);

    /* tests the error code for error */
    if(IS_ERROR_CODE(errorCode)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", GET_ERROR_MODULE(module));
    }

    /* calls the start module function */
    errorCode = module->start(environment, module);

    /* tests the error code for error */
    if(IS_ERROR_CODE(errorCode)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", GET_ERROR_MODULE(module));
    }

    /* adds the module to the list of modlues handlers in the service */
    appendValueLinkedList(service->modulesList, (void *) module);

    /* calls the stop module function */
    /*errorCode = module->stop(environment, module);*/

    /* tests the error code for error */
    /*if(IS_ERROR_CODE(errorCode)) {*/
        /* prints a warning message */
        /*V_WARNING_F("%s\n", GET_ERROR_MODULE(module));*/
    /*}*/

    /* deletes the module */
    /*deleteModule(module);*/

    /* deletes the environment */
    /*deleteEnvironment(environment);*/

    /* unloads the library */
    /*UNLOAD_LIBRARY(modLibrary);*/

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unloadModule(struct Service_t *service, struct Module_t *module) {
    /* allocates the error code */
    ERROR_CODE errorCode;

    /* retrieves the environment from the module */
    struct Environment_t *environment = module->environment;

    /* removes the module from the list of modlues handlers in the service */
    removeValueLinkedList(service->modulesList, (void *) module, 1);

    /* calls the stop module function */
    errorCode = module->stop(environment, module);

    /* tests the error code for error */
    if(IS_ERROR_CODE(errorCode)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", GET_ERROR_MODULE(module));
    }

    /* deletes the module */
    deleteModule(module);

    /* deletes the environment */
    deleteEnvironment(environment);

    /* unloads the library */
    /*UNLOAD_LIBRARY(modLibrary);*/

    /* raise no error */
    RAISE_NO_ERROR;
}
