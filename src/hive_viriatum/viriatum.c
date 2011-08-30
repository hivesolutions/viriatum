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

#include "viriatum.h"

ERROR_CODE runService() {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* allocates the service select */
    struct ServiceSelect_t *serviceSelect;

    /* allocates the socket data */
    SOCKET_DATA socketData;

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the service select */
    createServiceSelect(&serviceSelect);

    /* starts the service select */
    returnValue = startServiceSelect(serviceSelect);

	/* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
		 /* runs the socket finish */
		SOCKET_FINISH();

		/* raises the error again */
		RAISE_AGAIN(returnValue);
    }

    /* runs the socket finish */
    SOCKET_FINISH();

	/* raises no error */
	RAISE_NO_ERROR;
}

int main(int argc, char *argv[]) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* retrieves the version */
    unsigned char *version = versionViriatum();

    /* retrieves the description */
    unsigned char *description = descriptionViriatum();

    /* prints a message */
    V_PRINT_F("%s %s (%s, %s) [%s %s %d bit (%s)] on %s\n", description, version, VIRIATUM_COMPILATION_DATE, VIRIATUM_COMPILATION_TIME, VIRIATUM_COMPILER, VIRIATUM_COMPILER_VERSION_STRING, (int) VIRIATUM_PLATFORM_CPU_BITS, VIRIATUM_PLATFORM_CPU, VIRIATUM_PLATFORM_STRING);

    /* prints a message */
    V_PRINT_F("%s\n", VIRIATUM_COPYRIGHT);

    /* prints a debug message */
    V_DEBUG_F("Receiving %d arguments\n", argc);

    /* loads the module */
    returnValue = loadModule((unsigned char *) "C:/Users/joamag/Desktop/repositories/viriatum/bin/hive_viriatum_mod_lua/i386/win32/Debug/hive_viriatum_mod_lua.dll");

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
        /* prints a warning message */
        V_WARNING_F("%s\n", getLastErrorMessageSafe());
    }

    /* runs the service */
    returnValue = runService();

	/* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
        /* prints an error message */
        V_ERROR_F("%s\n", getLastErrorMessageSafe());
    }

    /* prints a debug message */
    V_DEBUG("Finishing process");

    /* returns zero (valid) */
    return 0;
}
