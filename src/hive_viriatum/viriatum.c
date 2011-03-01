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

void runService() {
    /* allocates the service select */
    struct ServiceSelect_t *serviceSelect;

    /* allocates the socket data */
    SOCKET_DATA socketData;

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the service select */
    createServiceSelect(&serviceSelect);

    /* starts the service select */
    startServiceSelect(serviceSelect);

    /* runs the socket finish */
    SOCKET_FINISH();
}

int main(int argc, char *argv[]) {
    /* retrieves the version */
    unsigned char *version = versionViriatum();

    /* retrieves the description */
    unsigned char *description = descriptionViriatum();

    /* prints a debug message */
    DEBUG_F("%s %s (%s, %s) [%s %s %d bit (%s)] on %s\n", description, version, VIRIATUM_COMPILATION_DATE, VIRIATUM_COMPILATION_TIME, VIRIATUM_COMPILER, VIRIATUM_COMPILER_VERSION_STRING, VIRIATUM_PLATFORM_CPU_BITS, VIRIATUM_PLATFORM_CPU, VIRIATUM_PLATFORM_STRING);

    /* prints a debug message */
    DEBUG_F("Receiving %d arguments\n", argc);

    /* loads the module */
    loadModule("/opt/lib/libviriatum_mod_lua.so");

    /* runs the simple tests */
    runSimpleTests();

    /* runs the service */
    runService();

    /* prints a debug message */
    DEBUG("Finishing process");

    /* returns zero (valid) */
    return 0;
}
