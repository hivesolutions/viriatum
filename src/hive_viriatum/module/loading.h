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

/**
 * Enumeration describing the
 * various possible types of modules.
 */
typedef enum ModuleType_e {
    MODULE_TYPE_HANDLER = 1
} ModuleType;

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
} Module;

/**
 * Function used for starting a module.
 * During this initialization all the module
 * internal structures shall be initialized.
 * The module should be able to populate the module
 * structure accordingly.
 *
 * @param module The module structure to be
 * populated by the module.
 */
typedef ERROR_CODE (*viriatumStartModule)(struct Module_t *module);

/**
 * Function used for stopping a module.
 * During this destruction all the module
 * internal structures shall be closed and
 * destroyed.
 */
typedef ERROR_CODE (*viriatumStopModule)(void);

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
 * Constructor of the module.
 *
 * @param servicePointer The pointer to the module to
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
