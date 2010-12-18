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

#include "system/system.h"

int main(int argc, char *argv[]) {
    /* allocates the socket data */
    SOCKET_DATA socketData;

    /* allocates the socket handle */
    SOCKET_HANDLE socketHandle;

    /* allocates the socket address structure */
    SOCKET_ADDRESS_INPUT socketAddress;

    /* allocates the client socket handle */
    SOCKET_HANDLE clientSocketHandle;

    /* allocates the client socket address structure */
    SOCKET_ADDRESS clientSocketAddress;

    /* allocates the client socket address size */
    int clientSocketAddressSize;

    SOCKET_ERROR_CODE result;

    char buffer[256];

    unsigned int n;

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the socket for the given types */
    socketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    // tests the socket for errors
    if(SOCKET_TEST_SOCKET(socketHandle)) {
        /* retrieves the socket error code */
        SOCKET_ERROR_CODE socketErrorCode = SOCKET_GET_ERROR_CODE(socketHandle);

        /* prints the error */
        printf("Problem creating socket: %d", socketErrorCode);

        /* runs the socket finish */
        SOCKET_FINISH();
    }

    /* sets the socket address attributes */
    socketAddress.sin_family = SOCKET_INTERNET_TYPE;
    socketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    socketAddress.sin_port = htons(8080);

    /* binds the socket */
    result = SOCKET_BIND(socketHandle, socketAddress);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(result)) {
        /* retrieves the binding error code */
        SOCKET_ERROR_CODE bindingErrorCode = SOCKET_GET_ERROR_CODE(result);

        /* prints the error */
        printf("Problem binding socket: %d", bindingErrorCode);

        /* closes the socket */
        SOCKET_CLOSE(socketHandle);

        /* runs the socket finish */
        SOCKET_FINISH();

        /* returns immediately */
        return;
    }

    /* listens for a socket change */
    SOCKET_LISTEN(socketHandle);

    /* iterates continuously */
    while(1) {
        /* calculates the size of the socket address */
        clientSocketAddressSize = sizeof(socketAddress);

        /* accepts the socket */
        clientSocketHandle = SOCKET_ACCEPT(socketHandle, &clientSocketAddress, clientSocketAddressSize);

        SOCKET_RECEIVE(clientSocketHandle, buffer, 255, 0);

        printf("Received %s", buffer);

        if (clientSocketHandle < 0)
            printf("ERROR on accept");

        n = SOCKET_SEND(clientSocketHandle, "tobias e fixe", strlen("tobias e fixe"), 0);
    }

    /* returns zero (valid) */
    return 0;
}
