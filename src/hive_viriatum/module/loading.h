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

#pragma once

#include "../system/system.h"

#define GET_ERROR_MODULE(module) get_error_message_module(module)

/* forward references (avoids loop) */
struct module_t;
struct environment_t;

/**
 * Function used for starting a module.
 * During this initialization all the module
 * internal structures shall be initialized.
 * The module should be able to populate the module
 * structure accordingly.
 *
 * @param environment Module environment structures used
 * to help starting the module.
 * @param module The module structure to be
 * populated by the module.
 */
typedef ERROR_CODE (*viriatum_start_module) (struct environment_t *environment, struct module_t *module);

/**
 * Function used for stopping a module.
 * During this destruction all the module
 * internal structures shall be closed and
 * destroyed.
 *
 * @param environment Module environment structures used
 * to help stoping the module.
 * @param The model structure to be used in
 * the stoping of the module.
 */
typedef ERROR_CODE (*viriatum_stop_module) (struct environment_t *environment, struct module_t *module);

/**
 * Function used to retrieve information
 * about the module.
 * The module should be able to populate the module
 * structure accordingly.
 *
 * @param module The module structure to be
 * populated by the module.
 */
typedef ERROR_CODE (*viriatum_info_module) (struct module_t *module);

/**
 * Function used to retrieve information
 * about the module's last error.
 * The module should be able to populate the module
 * structure accordingly.
 *
 * @param message_pointer The pointer to the error message
 * to be set.
 */
typedef ERROR_CODE (*viriatum_error_module)(unsigned char **message_pointer);

/**
 * Enumeration describing the
 * various possible types of modules.
 */
typedef enum module_type_e {
    MODULE_TYPE_HTTP_HANDLER = 1
} module_type;

/**
 * Structure that describes the current
 * (service) environment.
 * This structure is used by the various
 * modules to identify the current environment
 * characteristics.
 */
typedef struct environment_t {
    /**
     * The name used to describe
     * the environment.
     */
    unsigned char *name;

    /**
     * The current environment service.
     */
    struct service_t *service;
} environment;

/**
 * Structure describing the module
 * facade specification.
 */
typedef struct module_t {
    /**
     * The name or description of
     * the module.
     */
    unsigned char *name;

    /**
     * A string representing the
     * the version of the module
     */
    unsigned char *version;

    /**
     * The type of module.
     * This value should be according
     * to the module type enumeration.
     */
    unsigned short type;

    /**
     * The environment used to load
     * the module.
     */
    struct environment_t *environment;

    /**
     * The function called to start
     * the module.
     */
    viriatum_start_module start;

    /**
     * The function called to stop
     * the module.
     */
    viriatum_stop_module stop;

    /**
     * The function called to retrieve
     * information about the module.
     */
    viriatum_info_module info;

    /**
     * The function called to retrieve
     * information (message) about the
     * last occurring error in the module.
     */
    viriatum_error_module error;

    /**
     * The reference to the library that
     * represents the current module.
     * This should be a dynamic library reference.
     */
    LIBRARY_REFERENCE library;

    /**
     * Reference to the lower level
     * module substrate (child).
     */
    void *lower;
} module;

/**
 * Constructor of the environment.
 *
 * @param environment_pointer The pointer to the environment to
 * be constructed.
 * @return The resulting error code.
 */
ERROR_CODE create_environment(struct environment_t **environment_pointer);

/**
 * Destructor of the environment.
 *
 * @param environment The environment to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE delete_environment(struct environment_t *environment);

/**
 * Constructor of the module.
 *
 * @param module_pointer The pointer to the module to
 * be constructed.
 * @return The resulting error code.
 */
ERROR_CODE create_module(struct module_t **module_pointer);

/**
 * Destructor of the module.
 *
 * @param module The module to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE delete_module(struct module_t *module);

/**
 * Loads the module in the given path.
 * The loading of the module implies finding
 * the appropriate file and loading the symbols.
 *
 * @param service The base service to load the module
 * into.
 * @param module_path The path to the file
 * containing the module.
 * @return The resulting error code.
 */
ERROR_CODE load_module(struct service_t *service, unsigned char *module_path);

/**
 * Unloads the module in the given module structure.
 * The loading of the module implies the unregistering
 * from the service.
 *
 * @param service The base service to unload the module
 * from.
 * @param module The structure describing the module
 * to be unloaded.
 * @return The resulting error code.
 */
ERROR_CODE unload_module(struct service_t *service, struct module_t *module);

/**
 * Retrieves the (last) error message for the given module.
 * This method forwards the call to the error handler
 * method that retrieves the error message.
 *
 * @param module The module to be used to retrieve the error
 * message.
 * @return The retrieved error message.
 */
static __inline unsigned char *get_error_message_module(struct module_t *module) {
    /* allocates the error message */
    unsigned char *error_message;

    /* in case no valid module structure
    is available */
    if(module == NULL) {
        /* sets invalid error message */
        error_message = (unsigned char *) "No valid module structure";
    }

    /* in case the module error handler
    is not defined */
    if(module->error == NULL) {
        /* sets invalid error message */
        error_message = (unsigned char *) "No error handler defined";
    }
    /* otherwise there is a valid error
    handler defined, and the error message
    can be retrieved */
    else {
        /* retrieves the error message from the module */
        module->error(&error_message);
    }

    /* returns the error message */
    return error_message;
}
