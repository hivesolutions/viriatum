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

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "service_select.h"

void createServiceSelect(struct ServiceSelect_t **serviceSelectPointer) {
    /* retrieves the service select size */
    size_t serviceSelectSize = sizeof(struct ServiceSelect_t);

    /* retrieves the service size */
    size_t serviceSize = sizeof(struct Service_t);

    /* allocates space for the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) malloc(serviceSelectSize);

    /* creates the service */
    createService(&serviceSelect->service);

    /* zeros the sockets read set */
    FD_ZERO(&serviceSelect->socketsReadSet);

    /* zeros the sockets write set */
    FD_ZERO(&serviceSelect->socketsWriteSet);

    /* zeros the sockets read temporary set */
    FD_ZERO(&serviceSelect->socketsReadTemporarySet);

    /* zeros the sockets write temporary set */
    FD_ZERO(&serviceSelect->socketsWriteTemporarySet);

    /* sets the default timeout */
    serviceSelect->selectTimeout.tv_sec = 1;
    serviceSelect->selectTimeout.tv_usec = 0;

    /* sets the service select in the service select pointer */
    *serviceSelectPointer = serviceSelect;
}

void deleteServiceSelect(struct ServiceSelect_t *serviceSelect) {
    /* deletes the service */
    deleteService(serviceSelect->service);

    /* releases the service select */
    free(serviceSelect);
}

void httpReadHandler(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* allocates the number of bytes */
    size_t numberBytes;

    /* allocates the "simple" buffer */
    unsigned char buffer[10240];

    /* allocates the response buffer */
    unsigned char *responseBuffer;

    /* receives from the socket */
    numberBytes = SOCKET_RECEIVE(connection->socketHandle, buffer, 10240, 0);

    /* in case there was an error receiving from the socket */
    if(SOCKET_TEST_ERROR(numberBytes)) {
        /* retrieves the receving error code */
        SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

        /* prints the error */
        debug("Problem receiving from socket: %d\n", receivingErrorCode);

        /* removes the connection from the service select */
        removeConnectionServiceSelect(serviceSelect, connection);

        /* returns immediately */
        return;
    }

    /* sets the final string bytes */
    buffer[numberBytes] = '\0';

    /* prints the received message */
    debug("%s", buffer);

    responseBuffer = (unsigned char *) malloc(1024);

    /* writes the http static headers to the response */
    SPRINTF(responseBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: viriatum/1.0.0\r\nContent-Length: %d\r\n\r\nhello world", strlen("hello world"));

    /* adds the response buffer to the write queue */
    appendValueLinkedList(connection->writeQueue, (void *) responseBuffer);

    if(connection->writeRegistered == 0) {
        /* ESTA HARDCODADO TENHO DE ARRANJAR UMA MANEIIRA MAIS SOFT DE FAZER ISTO !!! */
        addSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsWriteSet);
        connection->writeRegistered = 1;
    }
}

void httpWriteHandler(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* allocates the number of bytes */
    int numberBytes;

    void *value;

    size_t valueSize;

    unsigned int error = 0;

    /* iterates continuously */
    while(1) {
        /* pops a value from the linked list */
        popValueLinkedList(connection->writeQueue, &value);

        /* in case the value is invalid */
        if(value == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrieves the value size */
        valueSize = strlen(value);

        /* sends the value retrieving the number of bytes sent */
        numberBytes = SOCKET_SEND(connection->socketHandle, value, valueSize, 0);

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(numberBytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

            /* prints the error */
            debug("Problem sending from socket: %d\n", receivingErrorCode);

            /* removes the connection from the service select */
            removeConnectionServiceSelect(serviceSelect, connection);

            /* breaks the loop */
            break;
        }

        /* in case the number of bytes sent is the same as the value size */
        if(numberBytes != valueSize) {
            /* sets the error flag */
            error = 1;

            /* breaks the loop */
            break;
        }
    }

    /* in case there is no error */
    if(error == 0) {
        /* VALOR HARDCODAD FAZER ISTO DE UMA MELHOR MANEIRA */
        FD_CLR(connection->socketHandle, &serviceSelect->socketsWriteSet);
        connection->writeRegistered = 0;
    }
    /* otherwise an error occurred */
    else {
    }




    /* closes the socket */
    SOCKET_CLOSE(connection->socketHandle);

    /* removes the connection from the service select */
    removeConnectionServiceSelect(serviceSelect, connection);
}

void startServiceSelect(struct ServiceSelect_t *serviceSelect) {
    /* allocates the index */
    unsigned int index;

    /* allocates the read connections */
    struct Connection_t **readConnections = (struct Connection_t **) malloc(120 * sizeof(struct Connection_t *));

    /* allocates the write connections */
    struct Connection_t **writeConnections = (struct Connection_t **) malloc(120 * sizeof(struct Connection_t *));

    /* allocates the read connections size */
    unsigned int readConnectionsSize;

    /* allocates the write connections size */
    unsigned int writeConnectionsSize;

    /* allocates the service socket ready */
    unsigned int serviceSocketReady;

    /* allocates the current connection */
    struct Connection_t *currentConnection;

    /* allocates the connection */
    struct Connection_t *connection;

    /* allocates the socket handle */
    SOCKET_HANDLE socketHandle;

    /* allocates the service socket handle */
    SOCKET_HANDLE serviceSocketHandle;

    /* allocates the socket address */
    SOCKET_ADDRESS socketAddress;

    /* calculates the size of the socket address */
    size_t clientSocketAddressSize = sizeof(SOCKET_ADDRESS);

    /* starts the service */
    startService(serviceSelect->service);

    /* retrieves the service socket handle */
    serviceSocketHandle = serviceSelect->service->serviceSocketHandle;

    /* adds the service socket handle to the sockets read set */
    addSocketHandleSocketsSetServiceSelect(serviceSelect, serviceSocketHandle, &serviceSelect->socketsReadSet);

    /* iterates continuously */
    while(1) {
        /* polls the service select */
        pollServiceSelect(serviceSelect, readConnections, writeConnections, &readConnectionsSize, &writeConnectionsSize, &serviceSocketReady);

        /* in case the service socket is ready (for read) */
        if(serviceSocketReady == 1) {
            /* accepts the socket, retrieving the socket handle */
            socketHandle = SOCKET_ACCEPT(serviceSocketHandle, &socketAddress, clientSocketAddressSize);

            /* prints a debug message */
            debug("Accepted connection: %d\n", socketHandle);

            /* creates the connection */
            createConnection(&connection, socketHandle);

            /* adds the connection to the service select */
            addConnectionServiceSelect(serviceSelect, connection);
        }

        /* iterates over the read connections */
        for(index = 0; index < readConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = readConnections[index];

            /* calls the read handler */
            httpReadHandler(serviceSelect, currentConnection);
        }

        /* iterates over the write connections */
        for(index = 0; index < writeConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = writeConnections[index];

            /* calls the write handler */
            httpWriteHandler(serviceSelect, currentConnection);
        }
    }
}

void addConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* calls the add connection service (super) */
    addConnectionService(serviceSelect->service, connection);

    /* adds the socket handle to the sockets read set */
    addSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsReadSet);
}

void removeConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* calls the remove connection service (super) */
    removeConnectionService(serviceSelect->service, connection);

    /* removes the socket handle from the sockets read set */
    removeSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsReadSet);

    /* in case the connection write is registered */
    if(connection->writeRegistered == 1) {
        /* removes the socket handle from the sockets write set */
        removeSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsWriteSet);
    }
}

void pollServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, unsigned int *readConnectionsSize, unsigned int *writeConnectionsSize, unsigned int *serviceSocketReady) {
    /* allocates space for the select count */
    int selectCount;

    /* allocates space for the current connection */
    struct Connection_t *currentConnection;

    /* allocates space for the connections list iterator */
    struct Iterator_t *connectionsListIterator;

    /* retrieves the service connections list */
    struct LinkedList_t *connectionsList = serviceSelect->service->connectionsList;

    /* initializes the read index */
    unsigned int readIndex = 0;

    /* initializes the write index */
    unsigned int writeIndex = 0;

    /* copies the socket sets to the socket temporary sets */
    serviceSelect->socketsReadTemporarySet = serviceSelect->socketsReadSet;
    serviceSelect->socketsWriteTemporarySet = serviceSelect->socketsWriteSet;

    /* creates the iterator for the linked list */
    createIteratorLinkedList(connectionsList, &connectionsListIterator);

	/* prints a debug message */
	debug("Entering select statement\n");

    /* runs the select over the sockets set */
    selectCount = select(serviceSelect->socketsSetHighest, &serviceSelect->socketsReadTemporarySet, &serviceSelect->socketsWriteTemporarySet, NULL, &serviceSelect->selectTimeout);

	/* prints a debug message */
	debug("Exiting select statement with value: %d\n", selectCount);

    if(FD_ISSET(serviceSelect->service->serviceSocketHandle, &serviceSelect->socketsReadTemporarySet) == 1)  {
        /* sets the service socket ready to one */
        *serviceSocketReady = 1;
    } else {
        /* sets the service socket ready to zero */
        *serviceSocketReady = 0;
    }

    /* iterator continuously */
    while(1) {
        /* retrieves the next value from the iterator */
        getNextIterator(connectionsListIterator, (void **) &currentConnection);

        /* in case the current connection is null (end of iterator) */
        if(currentConnection == NULL) {
            /* breaks the loop */
            break;
        }

        /* in case the current connection socket handle is set in
        the sockets read ready set */
        if(FD_ISSET(currentConnection->socketHandle, &serviceSelect->socketsReadTemporarySet) == 1)  {
            /* sets the current connection in the read connections */
            readConnections[readIndex] = currentConnection;

            /* increments the read index */
            readIndex++;
        }

        /* in case the current connection socket handle is set in
        the sockets write ready set */
        if(FD_ISSET(currentConnection->socketHandle, &serviceSelect->socketsWriteTemporarySet) == 1)  {
            /* sets the current connection in the write connections */
            writeConnections[writeIndex] = currentConnection;

            /* increments the write index */
            writeIndex++;
        }
    }

    /* sets the read index in the read connections size */
    *readConnectionsSize = readIndex;

    /* sets the write index in the write connections size */
    *writeConnectionsSize = writeIndex;
}

void addSocketHandleSocketsSetServiceSelect(struct ServiceSelect_t *serviceSelect, SOCKET_HANDLE socketHandle, fd_set *socketsSet) {
    /* in case the current socket handle is bigger than the service
    select sockets set highest value */
    if(socketHandle > serviceSelect->socketsSetHighest) {
        /* sets the socket handle as the sockets set highest */
        serviceSelect->socketsSetHighest = socketHandle;
    }

    /* adds the socket handle to the sockets set */
    FD_SET(socketHandle, socketsSet);
}

void removeSocketHandleSocketsSetServiceSelect(struct ServiceSelect_t *serviceSelect, SOCKET_HANDLE socketHandle, fd_set *socketsSet) {
    /* removes the socket handle from the sockets set */
    FD_CLR(socketHandle, socketsSet);
}
