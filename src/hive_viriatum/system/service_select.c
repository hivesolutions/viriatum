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

#include "service_select.h"

void createServiceSelect(struct ServiceSelect_t **serviceSelectPointer) {
    /* retrieves the service select size */
    size_t serviceSelectSize = sizeof(struct ServiceSelect_t);

    /* allocates space for the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) malloc(serviceSelectSize);

    /* creates the service */
    createService(&serviceSelect->service);

    /* resets the sockets set highest value */
    serviceSelect->socketsSetHighest = 0;

    /* zeros the sockets read set */
    SOCKET_SET_ZERO(&serviceSelect->socketsReadSet);

    /* zeros the sockets write set */
    SOCKET_SET_ZERO(&serviceSelect->socketsWriteSet);

    /* zeros the sockets read set temporary */
    SOCKET_SET_ZERO(&serviceSelect->socketsReadSetTemporary);

    /* zeros the sockets write set temporary */
    SOCKET_SET_ZERO(&serviceSelect->socketsWriteSetTemporary);

    /* sets the default timeout */
    serviceSelect->selectTimeout.tv_sec = 10;
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
    char *responseBuffer;

    /* allocates the http settings */
    struct HttpSettings_t *httpSettings;

    /* allocates the http request */
    struct HttpRequest_t *httpRequest;

    /* receives from the socket */
    numberBytes = SOCKET_RECEIVE(connection->socketHandle, buffer, 10240, 0);

    /* in case there was an error receiving from the socket */
    if(SOCKET_TEST_ERROR(numberBytes)) {
        /* retrieves the receving error code */
        SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

        /* prints the error */
        V_DEBUG_F("Problem receiving from socket: %d\n", receivingErrorCode);

        /* removes the connection from the service select */
        removeConnectionServiceSelect(serviceSelect, connection);

        /* returns immediately */
        return;
    }

    /* sets the final string bytes */
    buffer[numberBytes] = '\0';

    /* allocates the response buffer */
    responseBuffer = (char *) malloc(1024);

    /* creates the http settings */
    createHttpSettings(&httpSettings);

    /* creates the http request */
    createHttpRequest(&httpRequest);

    /* process the http data for the http request */
    processDataHttpRequest(httpRequest, httpSettings, buffer, numberBytes);

    /* writes the http static headers to the response */
    SPRINTF(responseBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: viriatum/1.0.0 (%s)\r\nContent-Length: %lu\r\n\r\nhello world", VIRIATUM_PLATFORM_STRING, strlen("hello world"));

    /* adds the response buffer to the write queue */
    appendValueLinkedList(connection->writeQueue, (void *) responseBuffer);

    /* ESTE CONCEIDO DE ADICAO A LISTA DE ESCRITAS AINDA ESTA MUITO BADALHOCO TENHO DE PENSAR MELHOR */
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
        /* pops a value from the linked list (write queue) */
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
            V_DEBUG_F("Problem sending from socket: %d\n", receivingErrorCode);

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
        /* Tenho realmente de remover o ganho de estar a espera para escrita */
        /* mas nao neste handler de http (nao e da competencia dele) */
        SOCKET_SET_CLEAR(connection->socketHandle, &serviceSelect->socketsWriteSet);
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
    SOCKET_ADDRESS_SIZE clientSocketAddressSize = sizeof(SOCKET_ADDRESS);

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
            V_DEBUG_F("Accepted connection: %d\n", socketHandle);

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

    /* copies the socket sets to the socket sets temporary */
    serviceSelect->socketsReadSetTemporary = serviceSelect->socketsReadSet;
    serviceSelect->socketsWriteSetTemporary = serviceSelect->socketsWriteSet;

    /* copies the select timeout to the select timeout temporary */
    serviceSelect->selectTimeoutTemporary = serviceSelect->selectTimeout;

    /* creates the iterator for the linked list */
    createIteratorLinkedList(connectionsList, &connectionsListIterator);

    /* prints a debug message */
    V_DEBUG("Entering select statement\n");

    /* runs the select over the sockets set */
    selectCount = SOCKET_SELECT(serviceSelect->socketsSetHighest + 1, &serviceSelect->socketsReadSetTemporary, &serviceSelect->socketsWriteSetTemporary, NULL, &serviceSelect->selectTimeoutTemporary);

    /* prints a debug message */
    V_DEBUG_F("Exiting select statement with value: %d\n", selectCount);

    /* in case the select coutn is negative */
    if(selectCount < 0) {
        /* prints a debug message */
        V_DEBUG_F("Problem in select statement: %d\n", selectCount);
    }

    /* in case the service socket handle is set in
    the sockets read ready set */
    if(SOCKET_SET_IS_SET(serviceSelect->service->serviceSocketHandle, &serviceSelect->socketsReadSetTemporary) > 0)  {
        /* sets the service socket ready to one */
        *serviceSocketReady = 1;

        /* decrements the select count */
        selectCount--;
    } else {
        /* sets the service socket ready to zero */
        *serviceSocketReady = 0;
    }

    /* iterates continuously */
    while(1) {
        /* in case the select count is zero */
        if(selectCount == 0) {
            /* breaks the loop */
            break;
        }

        /* retrieves the next value from the iterator */
        getNextIterator(connectionsListIterator, (void **) &currentConnection);

        /* in case the current connection is null (end of iterator) */
        if(currentConnection == NULL) {
            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG_F("Testing file for select: %d\n", currentConnection->socketHandle);

        /* in case the current connection socket handle is set in
        the sockets read ready set */
        if(SOCKET_SET_IS_SET(currentConnection->socketHandle, &serviceSelect->socketsReadSetTemporary) > 0)  {
            /* sets the current connection in the read connections */
            readConnections[readIndex] = currentConnection;

            /* increments the read index */
            readIndex++;

            /* decrements the select count */
            selectCount--;
        }

        /* in case the current connection socket handle is set in
        the sockets write ready set */
        if(SOCKET_SET_IS_SET(currentConnection->socketHandle, &serviceSelect->socketsWriteSetTemporary) > 0)  {
            /* sets the current connection in the write connections */
            writeConnections[writeIndex] = currentConnection;

            /* increments the write index */
            writeIndex++;

            /* decrements the select count */
            selectCount--;
        }
    }

    /* in case the select count is bigger than zero */
    if(selectCount > 0) {
        /* prints a debug message */
        V_DEBUG_F("Extraordinary select file descriptors not found: %d\n", selectCount);
    }

    /* sets the read index in the read connections size */
    *readConnectionsSize = readIndex;

    /* sets the write index in the write connections size */
    *writeConnectionsSize = writeIndex;
}

void addSocketHandleSocketsSetServiceSelect(struct ServiceSelect_t *serviceSelect, SOCKET_HANDLE socketHandle, SOCKET_SET *socketsSet) {
    /* in case the current socket handle is bigger than the service
    select sockets set highest value */
    if(socketHandle > serviceSelect->socketsSetHighest) {
        /* sets the socket handle as the sockets set highest */
        serviceSelect->socketsSetHighest = socketHandle;
    }

    /* adds the socket handle to the sockets set */
    SOCKET_SET_SET(socketHandle, socketsSet);
}

void removeSocketHandleSocketsSetServiceSelect(struct ServiceSelect_t *serviceSelect, SOCKET_HANDLE socketHandle, SOCKET_SET *socketsSet) {
    /* removes the socket handle from the sockets set */
    SOCKET_SET_CLEAR(socketHandle, socketsSet);
}
