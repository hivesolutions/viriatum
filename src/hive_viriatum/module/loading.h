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

#define GET_ERROR_MODULE(module) getErrorMessageModule(module)

struct Module_t;
struct Environment_t;

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
typedef ERROR_CODE (*viriatumStartModule)(struct Environment_t *environment, struct Module_t *module);

/**
 * Function used for stopping a module.
 * During this destruction all the module
 * internal structures shall be closed and
 * destroyed.
 */
typedef ERROR_CODE (*viriatumStopModule)();

/**
 * Function used to retrieve information
 * about the module.
 * The module should be able to populate the module
 * structure accordingly.
 *
 * @param module The module structure to be
 * populated by the module.
 */
typedef ERROR_CODE (*viriatumInfoModule)(struct Module_t *module);

/**
 * Function used to retrieve information
 * about the module's last error.
 * The module should be able to populate the module
 * structure accordingly.
 *
 * @param messagePointer The pointer to the error message
 * to be set.
 */
typedef ERROR_CODE (*viriatumErrorModule)(unsigned char **messagePointer);

/**
 * Enumeration describing the
 * various possible types of modules.
 */
typedef enum ModuleType_e {
    MODULE_TYPE_HANDLER = 1
} ModuleType;

/**
 * Structure that describes the current
 * (service) environment.
 * This structure is used by the various
 * modules to identify the current environment
 * characteristics.
 */
typedef struct Environment_t {
    /**
     * The name used to describe
     * the environment.
     */
    unsigned char *name;
} Environment;

/**
 * Structure describing the module
 * facade specification.
 */
typedef struct Module_t {
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
     * The function called to start
     * the module.
     */
    viriatumStartModule start;

    /**
     * The function called to stop
     * the module.
     */
    viriatumStopModule stop;

    /**
     * The function called to retrieve
     * information about the module.
     */
    viriatumInfoModule info;

    /**
     * The function called to retrieve
     * information (message) about the
     * last occurring error in the module.
     */
    viriatumErrorModule error;
} Module;

/**
 * Constructor of the environment.
 *
 * @param environmentPointer The pointer to the environment to
 * be constructed.
 * @return The resulting error code.
 */
ERROR_CODE createEnvironment(struct Environment_t **environmentPointer);

/**
 * Destructor of the environment.
 *
 * @param environment The environment to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE deleteEnvironment(struct Environment_t *environment);

/**
 * Constructor of the module.
 *
 * @param modulePointer The pointer to the module to
 * be constructed.
 * @return The resulting error code.
 */
ERROR_CODE createModule(struct Module_t **modulePointer);

/**
 * Destructor of the module.
 *
 * @param module The module to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE deleteModule(struct Module_t *module);

/**
 * Loads the module in the given path.
 * The loading of the module implies finding
 * the appropriate file and loading the symbols.
 *
 * @param modulePath The path to the file
 * containing the module.
 * @return The resulting error code.
 */
ERROR_CODE loadModule(unsigned char *modulePath);

/**
 * Retrieves the (last) error message for the given module.
 * This method forwards the call to the error handler
 * method that retrieves the error message.
 *
 * @param module The module to be used to retrieve the error
 * message.
 * @return The retrieved error message.
 */
static __inline unsigned char *getErrorMessageModule(struct Module_t *module) {
    /* allocates the error message */
    unsigned char *errorMessage;

    /* in case no valid module structure
    is available */
    if(module == NULL) {
        /* sets invalid error message */
        errorMessage = "No valid module structure";
    }

    /* in case the module error handler
    is not defined */
    if(module->error == NULL) {
        /* sets invalid error message */
        errorMessage = "No error handler defined";
    }
    /* otherwise there is a valid error
    handler defined, and the error message
    can be retrieved */
    else {
        /* retrieves the error message from the module */
        module->error(&errorMessage);
    }

    /* returns the error message */
    return errorMessage;
}
