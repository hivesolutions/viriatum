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

ERROR_CODE create_environment(struct environment_t **environment_pointer) {
    /* retrieves the environment size */
    size_t environment_size = sizeof(struct environment_t);

    /* allocates space for the environment */
    struct environment_t *environment = (struct environment_t *) MALLOC(environment_size);

    /* sets the environment attributes (default) values */
    environment->name = NULL;

    /* sets the environment in the environment pointer */
    *environment_pointer = environment;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_environment(struct environment_t *environment) {
    /* releases the environment */
    FREE(environment);

    /* raises no environment */
    RAISE_NO_ERROR;
}

ERROR_CODE create_module(struct module_t **module_pointer) {
    /* retrieves the module size */
    size_t module_size = sizeof(struct module_t);

    /* allocates space for the module */
    struct module_t *module = (struct module_t *) MALLOC(module_size);

     /* sets the module attributes (default) values */
    module->name = NULL;
    module->version = NULL;
    module->type = 0;
    module->start = NULL;
    module->stop = NULL;
    module->info = NULL;
    module->error = NULL;
    module->library = NULL;
    module->lower = NULL;

    /* sets the module in the module pointer */
    *module_pointer = module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_module(struct module_t *module) {
    /* releases the module */
    FREE(module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE load_module(struct service_t *service, unsigned char *module_path) {
    /* error code to be used for testing */
    ERROR_CODE error_code;

    /* the library reference */
    LIBRARY_REFERENCE library;

    /* the holder of the library symbol */
    LIBRARY_SYMBOL symbol;

    /* the environment parameters reference */
    struct environment_t *environment;

    /* the module structure reference */
    struct module_t *module;

    /* the info module function reference */
    viriatum_info_module info_module_function;

    /* prints a debug message */
    V_DEBUG_F("Loading module (%s)\n", module_path);

    /* loads the library (tries to find the file) */
    library = LOAD_LIBRARY((const char *) module_path);

    /* in case the library was not loaded */
    if(library == NULL) {
        /* retrieves the library error message */
        const char *error_message = GET_LIBRARY_ERROR_MESSAGE();

        /* in case no error message is found, must
        set the default errro message */
        if(error_message == NULL) {
            /* sets the default assumed error message */
            error_message = "File not found";
        }

        /* prints a warning message */
        V_WARNING_F("Error loading library (%s)\n", error_message);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Error loading library");
    } else {
        /* prints a debug message */
        V_DEBUG("Loaded library\n");
    }

    /* retrieves the symbol from the library */
    symbol = GET_LIBRARY_SYMBOL(library, "infoModule");

    /* retrieves the info module function reference */
    info_module_function = *((viriatum_info_module *)(&symbol));

    /* in case the start module function was not found */
    if(info_module_function == NULL) {
        /* prints a warning message */
        V_WARNING_F("No such symbol '%s' in library\n", "infoModule");

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem finding symbol");
    } else {
        /* prints a debug message */
        V_DEBUG_F("Found symbol '%s' in library\n", "infoModule");
    }

    /* creates the environment */
    create_environment(&environment);

    /* creates the module */
    create_module(&module);

    /* sets the module attributes */
    module->environment = environment;
    module->library = library;

    /* sets the environment attributes */
    environment->service = service;

    /* calls the info module function */
    error_code = info_module_function(module);

    /* tests the error code for error */
    if(IS_ERROR_CODE(error_code)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", GET_ERROR_MODULE(module));
    }

    /* calls the start module function */
    error_code = module->start(environment, module);

    /* tests the error code for error */
    if(IS_ERROR_CODE(error_code)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", GET_ERROR_MODULE(module));
    }

    /* adds the module to the list of modlues handlers in the service */
    append_value_linked_list(service->modules_list, (void *) module);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unload_module(struct service_t *service, struct module_t *module) {
    /* allocates the error code */
    ERROR_CODE error_code;

    /* retrieves the environment from the module */
    struct environment_t *environment = module->environment;

    /* retrieves the library from the module */
    LIBRARY_REFERENCE library = module->library;

    /* removes the module from the list of modlues handlers in the service */
    remove_value_linked_list(service->modules_list, (void *) module, 1);

    /* calls the stop module function */
    error_code = module->stop(environment, module);

    /* tests the error code for error */
    if(IS_ERROR_CODE(error_code)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", GET_ERROR_MODULE(module));
    }

    /* deletes the module */
    delete_module(module);

    /* deletes the environment */
    delete_environment(environment);

    /* unloads the library */
    UNLOAD_LIBRARY(library);

    /* raise no error */
    RAISE_NO_ERROR;
}
