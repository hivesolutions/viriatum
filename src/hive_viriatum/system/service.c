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

#include "service.h"

void createService(struct Service_t **servicePointer) {
    /* retrieves the service size */
    size_t serviceSize = sizeof(struct Service_t);

    /* allocates space for the service */
    struct Service_t *service = (struct Service_t *) malloc(serviceSize);

    /* creates the connections list */
    createLinkedList(&service->connectionsList);

    /* setst the service status as closed */
    service->status = 1;

    /* sets the service in the service pointer */
    *servicePointer = service;
}

void deleteService(struct Service_t *service) {
    /* releases the service */
    free(service);
}

void startService(struct Service_t *service) {
    /* allocates the socket address structure */
    SOCKET_ADDRESS_INPUT socketAddress;

    /* allocates the socket result */
    SOCKET_ERROR_CODE socketResult;

    /* allocates the option value */
    char optionValue;

    /* sets the socket address attributes */
    socketAddress.sin_family = SOCKET_INTERNET_TYPE;
    socketAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
    socketAddress.sin_port = htons(8181);

    /* creates the service socket for the given types */
    service->serviceSocketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    /* sets the option value to one (valid) */
    optionValue = 0,

    /* sets the socket reuse address option in the socket */
    SOCKET_SET_OPTIONS(service->serviceSocketHandle, SOCKET_OPTIONS_LEVEL_SOCKET, SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET, optionValue);

    /* binds the service socket */
    socketResult = SOCKET_BIND(service->serviceSocketHandle, socketAddress);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(socketResult)) {
        /* retrieves the binding error code */
        SOCKET_ERROR_CODE bindingErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints the error */
        V_DEBUG_F("Problem binding socket: %d", bindingErrorCode);

        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);

        /* returns immediately */
        return;
    }

    /* listens for a service socket change */
    socketResult = SOCKET_LISTEN(service->serviceSocketHandle);

    /* in case there was an error listening the socket */
    if(SOCKET_TEST_ERROR(socketResult)) {
        /* retrieves the listening error code */
        SOCKET_ERROR_CODE bindingErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints the error */
        V_DEBUG_F("Problem listening socket: %d", bindingErrorCode);

        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);

        /* returns immediately */
        return;
    }

    /* setst the service status as open */
    service->status = 2;
}

void addConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* adds the connection to the connections list */
    appendValueLinkedList(service->connectionsList, connection);
}

void removeConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* removes the connection from the connections list */
    removeValueLinkedList(service->connectionsList, connection);
}

void createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle) {
    /* retrieves the connection size */
    size_t connectionSize = sizeof(struct Connection_t);

    /* allocates space for the connection */
    struct Connection_t *connection = (struct Connection_t *) malloc(connectionSize);

    /* sets the socket handle in the connection */
    connection->socketHandle = socketHandle;

    /* sets teh write registered to false */
    connection->writeRegistered = 0;

    /* creates the read queue linked list */
    createLinkedList(&connection->readQueue);

    /* creates the write queue linked list */
    createLinkedList(&connection->writeQueue);

    /* sets the connection in the connection pointer */
    *connectionPointer = connection;
}

void deleteConnection(struct Connection_t *connection) {
    /* deletes the read queue linked list */
    deleteLinkedList(connection->readQueue);

    /* deletes the write queue linked list */
    deleteLinkedList(connection->writeQueue);

    /* releases the connection */
    free(connection);
}
