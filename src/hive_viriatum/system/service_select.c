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

    /* sets the register write in the service */
    serviceSelect->service->registerWrite = registerWriteServiceSelect;

    /* sets the unregister write in the service */
    serviceSelect->service->unregisterWrite = unregisterWriteServiceSelect;

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

void httpDataHandler(struct Connection_t *connection, unsigned char *buffer, size_t bufferSize) {
    /* allocates the http settings */
    struct HttpSettings_t *httpSettings;

    /* allocates the http parser */
    struct HttpParser_t *httpParser;

    /* creates the http settings */
    createHttpSettings(&httpSettings);

    /* creates the http parser */
    createHttpParser(&httpParser);

    /* sets the connection as the parser parameter(s) */
    httpParser->parameters = connection;

    /* sets the structures for the handler */
    setHandlerFile(httpParser, httpSettings);


    // TODO: tenho de testar quantos bytes processei !!!


    /* process the http data for the http parser */
    processDataHttpParser(httpParser, httpSettings, buffer, bufferSize);



    /* unsets the structures for the handler */
    unsetHandlerFile(httpParser, httpSettings);

    /* deletes the http parser */
    deleteHttpParser(httpParser);

    /* deletes the http settings */
    deleteHttpSettings(httpSettings);
}

ERROR_CODE httpReadHandler(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* allocates the number of bytes */
    size_t numberBytes;

    /* allocates the "simple" buffer */
    unsigned char buffer[10240];

    /* retrieves the buffer pointer */
    unsigned char *bufferPointer = (unsigned char *) buffer;

    /* allocates the buffer size */
    size_t bufferSize = 0;

    /* flag and value controlling the state of the read */
    ERROR_CODE error = 0;

    /* iterates continuously */
    while(1) {
        /* in case the buffer size is so big that may
        overflow the current allocated buffer */
        if(bufferSize + 1024 > 10240) {
            /* breaks the loop */
            break;
        }

        /* receives from the socket */
        numberBytes = SOCKET_RECEIVE(connection->socketHandle, bufferPointer, 1024, 0);

        /* in case the number of bytes is zero (connection closed) */
        if(numberBytes == 0) {
            /* sets the error flag */
            error = 1;

            /* breaks the loop */
            break;
        }

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(numberBytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

            if(receivingErrorCode == WSAEWOULDBLOCK) {
                /* sets the error flag (non fatal) */
                error = 2;
            } else {
                /* prints the error */
                V_DEBUG_F("Problem receiving from socket: %d\n", receivingErrorCode);

                /* sets the error flag */
                error = 1;
            }

            /* breaks the loop */
            break;
        }

        /* increments the buffer position */
        bufferPointer += numberBytes;

        /* increments the buffer size with the number of bytes */
        bufferSize += numberBytes;
    }

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* calls the http data handler */
            httpDataHandler(connection, buffer, bufferSize);

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection */
            connection->closeConnection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* calls the http data handler */
            httpDataHandler(connection, buffer, bufferSize);

            /* breaks the switch */
            break;

        /* default case */
        default:
            /* breaks the switch */
            break;
    }

    /* returns the error code */
    return error;
}

ERROR_CODE httpWriteHandler(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* allocates the number of bytes */
    int numberBytes;

    /* allocates the data */
    struct Data_t *data;

    /* flag and value controlling the state of the write */
    ERROR_CODE error = 0;

    /* iterates continuously */
    while(1) {
        /* peeks a value (data) from the linked list (write queue) */
        peekValueLinkedList(connection->writeQueue, (void **) &data);

        /* in case the data is invalid (list is empty) */
        if(data == NULL) {
            /* breaks the loop */
            break;
        }

        /* sends the value retrieving the number of bytes sent */
        numberBytes = SOCKET_SEND(connection->socketHandle, data->data, data->size, 0);

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(numberBytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

            if(receivingErrorCode == WSAEWOULDBLOCK) {
                V_DEBUG_F("RECEIVED EWOULDBLOCK!!!!\n");

                /* sets the error flag (non fatal) */
                error = 2;
            } else {
                /* prints the error */
                V_DEBUG_F("Problem sending from socket: %d\n", receivingErrorCode);

                /* sets the error flag */
                error = 1;
            }

            /* breaks the loop */
            break;
        }

        /* in case the number of bytes sent is the same as the value size
        (not all data has been sent) */
        if(numberBytes != data->size) {
            /* updates the data internal structure
            to allow the sending of the pending partial data */
            data->data += numberBytes;
            data->size += data->size - numberBytes;

            /* sets the error flag (non fatal) */
            error = 2;

            /* breaks the loop */
            break;
        }

        /* pops a value (data) from the linked list (write queue) */
        popValueLinkedList(connection->writeQueue, (void **) &data, 1);

        /* in case the data callback is set */
        if(data->callback != NULL) {
            /* calls the callback with the callback parameters */
            data->callback(connection, data, data->callbackParameters);
        }

        /* deletes the data */
        deleteData(data);
    }

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* unregisters the connection for write */
            connection->service->unregisterWrite(connection);

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection */
            connection->closeConnection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* breaks the switch */
            break;

        /* default case */
        default:
            /* breaks the switch */
            break;
    }

    /* returns the error code */
    return error;
}

void startServiceSelect(struct ServiceSelect_t *serviceSelect) {
    /* allocates the index */
    unsigned int index;

    /* allocates the read connections */
    struct Connection_t **readConnections = (struct Connection_t **) malloc(120 * sizeof(struct Connection_t *));

    /* allocates the write connections */
    struct Connection_t **writeConnections = (struct Connection_t **) malloc(120 * sizeof(struct Connection_t *));

    /* allocates the remove connections */
    struct Connection_t **removeConnections = (struct Connection_t **) malloc(120 * sizeof(struct Connection_t *));

    /* allocates the read connections size */
    unsigned int readConnectionsSize;

    /* allocates the write connections size */
    unsigned int writeConnectionsSize;

    /* allocates the remove connections size */
    unsigned int removeConnectionsSize;

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



    unsigned long iMode = 1;



    /* starts the service */
    startService(serviceSelect->service);

    /* retrieves the service socket handle */
    serviceSocketHandle = serviceSelect->service->serviceSocketHandle;

    /* adds the service socket handle to the sockets read set */
    addSocketHandleSocketsSetServiceSelect(serviceSelect, serviceSocketHandle, &serviceSelect->socketsReadSet);

    /* iterates continuously */
    while(1) {
        /* resets the remove connections size */
        removeConnectionsSize = 0;

        /* polls the service select */
        pollServiceSelect(serviceSelect, readConnections, writeConnections, &readConnectionsSize, &writeConnectionsSize, &serviceSocketReady);

        /* in case the service socket is ready (for read) */
        if(serviceSocketReady == 1) {
            /* accepts the socket, retrieving the socket handle */
            socketHandle = SOCKET_ACCEPT(serviceSocketHandle, &socketAddress, clientSocketAddressSize);





            ioctlsocket(socketHandle, FIONBIO, &iMode);



            /* prints a debug message */
            V_DEBUG_F("Accepted connection: %d\n", socketHandle);

            /* creates the connection */
            createConnection(&connection, socketHandle);

            /* opens the connection */
            openConection(connection);

            /* sets the service select service as the service in the connection */
            connection->service = serviceSelect->service;

            /* sets the service select as the service in the connection */
            connection->serviceReference = serviceSelect;

            /* adds the connection to the service select */
            addConnectionServiceSelect(serviceSelect, connection);
        }

        /* iterates over the read connections */
        for(index = 0; index < readConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = readConnections[index];

            /* in case the current connection is open */
            if(currentConnection->status == STATUS_OPEN) {
                /* calls the read handler */
                httpReadHandler(serviceSelect, currentConnection);
            }

            /* in case the current connection is closed */
            if(currentConnection->status == STATUS_CLOSED) {
                /* tries to add the current connection to the remove connections list */
                removeConnection(removeConnections, &removeConnectionsSize, currentConnection);
            }
        }

        /* iterates over the write connections */
        for(index = 0; index < writeConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = writeConnections[index];

            /* in case the current connection is open */
            if(currentConnection->status == STATUS_OPEN) {
                /* calls the write handler */
                httpWriteHandler(serviceSelect, currentConnection);
            }

            /* in case the current connection is closed */
            if(currentConnection->status == STATUS_CLOSED) {
                /* tries to add the current connection to the remove connections list */
                removeConnection(removeConnections, &removeConnectionsSize, currentConnection);
            }
        }

        /* iterates over the remove connections */
        for(index = 0; index < removeConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = removeConnections[index];

            /* deletes the current connection (house keeping) */
            deleteConnection(currentConnection);
        }
    }

    /* releases the read connections */
    free(readConnections);

    /* releases the write connections */
    free(writeConnections);

    /* releases the remove connections */
    free(removeConnections);
}

void addConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* calls the add connection service (super) */
    addConnectionService(serviceSelect->service, connection);

    /* adds the socket handle to the sockets read set */
    addSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsReadSet);

    /* sets the close connection fucntion in the connection */
    connection->closeConnection = closeConnectionServiceSelect;
}

void removeConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* unsets the close connection fucntion in the connection */
    connection->closeConnection = NULL;

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

    /* deletes the iterator linked list */
    deleteIteratorLinkedList(connectionsList, connectionsListIterator);

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

ERROR_CODE registerWriteServiceSelect(struct Connection_t *connection) {
    /* retrieves the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) connection->serviceReference;

    /* in case the connection is not write registered */
    if(connection->writeRegistered == 0) {
        /* adds the connection socket handle to the sockets write set */
        addSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsWriteSet);

        /* sets the connection as write registered */
        connection->writeRegistered = 1;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterWriteServiceSelect(struct Connection_t *connection) {
    /* retrieves the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) connection->serviceReference;

    /* in case the connection is write registered */
    if(connection->writeRegistered == 1) {
        /* removes the connection socket handle from the sockets write set */
        removeSocketHandleSocketsSetServiceSelect(serviceSelect, connection->socketHandle, &serviceSelect->socketsWriteSet);

        /* unsets the connection as write registered */
        connection->writeRegistered = 0;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeConnectionServiceSelect(struct Connection_t *connection) {
    /* retrieves the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) connection->serviceReference;

    /* in case the connection is (already) closed */
    if(connection->status == STATUS_CLOSED) {
        /* raises no error */
        RAISE_NO_ERROR;
    }

    /* removes the connection from the service select */
    removeConnectionServiceSelect(serviceSelect, connection);

    /* closes the connection */
    closeConection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
