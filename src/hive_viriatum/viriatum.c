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

/* starts the memory structures */
START_MEMORY;

static struct Service_t *service;

ERROR_CODE runService(struct HashMap_t *arguments) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* allocates the socket data */
    SOCKET_DATA socketData;

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the service and loads the options
    taking into account the arguments */
    createService(&service);
    loadOptionsService(service, arguments);

    /* starts the service */
    returnValue = startService(service);

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
         /* runs the socket finish */
        SOCKET_FINISH();

        /* raises the error again */
        RAISE_AGAIN(returnValue);
    }

    /* deletes the service */
    deleteService(service);

    /* runs the socket finish */
    SOCKET_FINISH();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE ranService() {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* in case the service status is open */
    if(service->status == STATUS_CLOSED) {
        /* prints a warning message */
        V_DEBUG("No service to be stopped\n");
    } else {
        /* prints a warning message */
        V_DEBUG("Stopping service\n");

        /* stops the service */
        returnValue = stopService(service);

        /* tests the error code for error */
        if(IS_ERROR_CODE(returnValue)) {
            /* runs the socket finish */
            SOCKET_FINISH();

            /* raises the error again */
            RAISE_AGAIN(returnValue);
        }

        /* prints a warning message */
        V_DEBUG("Finished stopping service\n");
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

void killHandler(int signalNumber) {
    /* defaults the signal handler (only one handling) */
    signal(signalNumber, SIG_DFL);

    /* runs the "ran" service */
    ranService();
}

ERROR_CODE printInformation() {
    /* retrieves the viriatum version and description */
    unsigned char *version = versionViriatum();
    unsigned char *description = descriptionViriatum();

    /* registers the kill handler for the int signal */
    signal(SIGINT, killHandler);

    /* prints a message */
    V_PRINT_F("%s %s (%s, %s) [%s %s %d bit (%s)] on %s\n", description, version, VIRIATUM_COMPILATION_DATE, VIRIATUM_COMPILATION_TIME, VIRIATUM_COMPILER, VIRIATUM_COMPILER_VERSION_STRING, (int) VIRIATUM_PLATFORM_CPU_BITS, VIRIATUM_PLATFORM_CPU, VIRIATUM_PLATFORM_STRING);

    /* prints a message */
    V_PRINT_F("%s\n", VIRIATUM_COPYRIGHT);

    /* raises no error */
    RAISE_NO_ERROR;
}

#ifndef VIRIATUM_PLATFORM_IPHONE
int main(int argc, char *argv[]) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* allocates the map that will contain the various
    processed arguments, indexed by name */
    struct HashMap_t *arguments;

    /* prints the viriatum information into the standard
    output "file", the label should be standard */
    printInformation();

    /* prints a debug message */
    V_DEBUG_F("Receiving %d argument(s)\n", argc);

    /* processes the various arguments into a map */
    processArguments(argc, argv, &arguments);
    printArguments(arguments);

    /* runs the service, with the given arguments */
    returnValue = runService(arguments);

    /* deletes the processed arguments */
    deleteArguments(arguments);

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
        /* prints an error message */
        V_ERROR_F("Problem running service (%s)\n", (char *) GET_ERROR());
    }

    /* prints a debug message */
    V_DEBUG("Finishing process\n");

    /* returns zero (valid) */
    return 0;
}
#endif
