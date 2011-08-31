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

ERROR_CODE loadModule(unsigned char *modulePath) {
    /* the mod library reference */
    LIBRARY_REFERENCE modLibrary;

    /* the holder of the library symbol */
    LIBRARY_SYMBOL symbol;

    /* the start module function reference */
    viriatumStartModule startModuleFunction;

    /* loads the mod library */
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
    symbol = GET_LIBRARY_SYMBOL(modLibrary, "startModule");

    /* retrieves the start nodule function reference */
    startModuleFunction = *((viriatumStartModule*)(&symbol));

    /* in case the start module function was not found */
    if(startModuleFunction == NULL) {
        /* prints a warning message */
        V_WARNING_F("No such symbol %s in library\n", "startModule");

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem finding symbol");
    } else {
        /* prints a debug message */
        V_DEBUG_F("Found symbol %s in library\n", "startModule");
    }

    /* calls the start module function */
    startModuleFunction();

    /* unloads the library */
    UNLOAD_LIBRARY(modLibrary);

    /* raise no error */
    RAISE_NO_ERROR;
}
