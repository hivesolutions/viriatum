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

    /* zeros the sockets error set */
    SOCKET_SET_ZERO(&serviceSelect->socketsErrorSet);

    /* zeros the sockets read set temporary */
    SOCKET_SET_ZERO(&serviceSelect->socketsReadSetTemporary);

    /* zeros the sockets write set temporary */
    SOCKET_SET_ZERO(&serviceSelect->socketsWriteSetTemporary);

    /* zeros the sockets error set temporary */
    SOCKET_SET_ZERO(&serviceSelect->socketsErrorSetTemporary);

    /* sets the default timeout */
    serviceSelect->selectTimeout.tv_sec = VIRIATUM_SELECT_TIMEOUT;
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

ERROR_CODE startServiceSelect(struct ServiceSelect_t *serviceSelect) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* allocates the index */
    unsigned int index;

    /* allocates the read connections */
    struct Connection_t **readConnections = (struct Connection_t **) malloc(VIRIATUM_MAXIMUM_CONNECTIONS * sizeof(struct Connection_t *));

    /* allocates the write connections */
    struct Connection_t **writeConnections = (struct Connection_t **) malloc(VIRIATUM_MAXIMUM_CONNECTIONS * sizeof(struct Connection_t *));

    /* allocates the error connections */
    struct Connection_t **errorConnections = (struct Connection_t **) malloc(VIRIATUM_MAXIMUM_CONNECTIONS * sizeof(struct Connection_t *));

    /* allocates the remove connections */
    struct Connection_t **removeConnections = (struct Connection_t **) malloc(VIRIATUM_MAXIMUM_CONNECTIONS * sizeof(struct Connection_t *));

    /* allocates the read connections size */
    unsigned int readConnectionsSize;

    /* allocates the write connections size */
    unsigned int writeConnectionsSize;

    /* allocates the error connections size */
    unsigned int errorConnectionsSize;

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

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* starts the service */
    returnValue = startService(serviceSelect->service);

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
        /* raises the error again */
        RAISE_AGAIN(returnValue);
    }

    /* retrieves the service socket handle */
    serviceSocketHandle = serviceSelect->service->serviceSocketHandle;

    /* adds the service socket handle to the sockets read set */
    addSocketHandleSocketsSetServiceSelect(serviceSelect, serviceSocketHandle, &serviceSelect->socketsReadSet);

    /* iterates while the status is open */
    while(serviceSelect->service->status == STATUS_OPEN) {
        /* resets the remove connections size */
        removeConnectionsSize = 0;

        /* polls the service select */
        pollServiceSelect(serviceSelect, readConnections, writeConnections, errorConnections, &readConnectionsSize, &writeConnectionsSize, &errorConnectionsSize, &serviceSocketReady);

        /* in case the service socket is ready (for read) */
        if(serviceSocketReady == 1) {
            /* accepts the socket, retrieving the socket handle */
            socketHandle = SOCKET_ACCEPT(serviceSocketHandle, &socketAddress, clientSocketAddressSize);

            /* in case viriatum is set to non blocking */
            if(VIRIATUM_NON_BLOCKING) {
                /* sets the socket to non blocking mode */
                SOCKET_SET_NON_BLOCKING(socketHandle, flags);
            }

            /* prints a debug message */
            V_DEBUG_F("Accepted connection: %d\n", socketHandle);

            /* creates the connection */
            createConnection(&connection, socketHandle);

            /* sets the service select service as the service in the connection */
            connection->service = serviceSelect->service;

            /* sets the service select as the service in the connection */
            connection->serviceReference = serviceSelect;

            /* sets the open connection fucntion in the connection */
            connection->openConnection = openConnectionServiceSelect;

            /* sets the close connection fucntion in the connection */
            connection->closeConnection = closeConnectionServiceSelect;

            /* opens the connection */
            connection->openConnection(connection);
        }

        /* prints a debug message */
        V_DEBUG_F("Processing %d read connections\n", readConnectionsSize);

        /* iterates over the read connections */
        for(index = 0; index < readConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = readConnections[index];

            /* prints a debug message */
            V_DEBUG_F("Processing read connection: %d\n", currentConnection->socketHandle);

            /* in case the current connection is open */
            if(currentConnection->status == STATUS_OPEN && currentConnection->onRead != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on read handler\n");

                /* calls the on read handler */
                currentConnection->onRead(currentConnection);

                /* prints a debug message */
                V_DEBUG("Finished calling on read handler\n");
            }

            /* in case the current connection is closed */
            if(currentConnection->status == STATUS_CLOSED) {
                /* tries to add the current connection to the remove connections list */
                removeConnection(removeConnections, &removeConnectionsSize, currentConnection);
            }
        }

        /* prints a debug message */
        V_DEBUG_F("Processing %d write connections\n", writeConnectionsSize);

        /* iterates over the write connections */
        for(index = 0; index < writeConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = writeConnections[index];

            /* prints a debug message */
            V_DEBUG_F("Processing write connection: %d\n", currentConnection->socketHandle);

            /* in case the current connection is open */
            if(currentConnection->status == STATUS_OPEN && currentConnection->onWrite != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on write handler\n");

                /* calls the on write handler */
                currentConnection->onWrite(currentConnection);

                /* prints a debug message */
                V_DEBUG("Finished calling on write handler\n");
            }

            /* in case the current connection is closed */
            if(currentConnection->status == STATUS_CLOSED) {
                /* tries to add the current connection to the remove connections list */
                removeConnection(removeConnections, &removeConnectionsSize, currentConnection);
            }
        }

        /* prints a debug message */
        V_DEBUG_F("Processing %d error connections\n", errorConnectionsSize);

        /* iterates over the error connections */
        for(index = 0; index < errorConnectionsSize; index++) {
            /* retrieves the current connection */
            currentConnection = errorConnections[index];

            /* prints a debug message */
            V_DEBUG_F("Processing error connection: %d\n", currentConnection->socketHandle);

            /* in case the current connection is open */
            if(currentConnection->status == STATUS_OPEN && currentConnection->onError != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on error handler\n");

                /* calls the on error handler */
                currentConnection->onError(currentConnection);

                /* prints a debug message */
                V_DEBUG("Finished calling on error handler\n");
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

    /* releases the error connections */
    free(errorConnections);

    /* releases the remove connections */
    free(removeConnections);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopServiceSelect(struct ServiceSelect_t *serviceSelect) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* stops the service */
    returnValue = stopService(serviceSelect->service);

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
        /* raises the error again */
        RAISE_AGAIN(returnValue);
    }

    /* raises no error */
    RAISE_NO_ERROR;
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

void pollServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, struct Connection_t **errorConnections, unsigned int *readConnectionsSize, unsigned int *writeConnectionsSize, unsigned int *errorConnectionsSize, unsigned int *serviceSocketReady) {
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

    /* initializes the error index */
    unsigned int errorIndex = 0;

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
    selectCount = SOCKET_SELECT(serviceSelect->socketsSetHighest + 1, &serviceSelect->socketsReadSetTemporary, &serviceSelect->socketsWriteSetTemporary, &serviceSelect->socketsErrorSetTemporary, &serviceSelect->selectTimeoutTemporary);

    /* prints a debug message */
    V_DEBUG_F("Exiting select statement with value: %d\n", selectCount);

    /* in case the select coutn is negative */
    if(selectCount < 0) {
        /* prints a warning message */
        V_WARNING_F("Problem in select statement: %d\n", selectCount);
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

        /* in case the current connection socket handle is set in
        the sockets error ready set */
        if(SOCKET_SET_IS_SET(currentConnection->socketHandle, &serviceSelect->socketsErrorSetTemporary) > 0)  {
            /* sets the current connection in the error connections */
            errorConnections[errorIndex] = currentConnection;

            /* increments the error index */
            errorIndex++;

            /* decrements the select count */
            selectCount--;
        }
    }

    /* prints a debug message */
    V_DEBUG("Finished file testing\n");

    /* deletes the iterator linked list */
    deleteIteratorLinkedList(connectionsList, connectionsListIterator);

    /* prints a debug message */
    V_DEBUG("Deleted iterator linked list\n");

    /* in case the select count is bigger than zero */
    if(selectCount > 0) {
        /* prints a debug message */
        V_DEBUG_F("Extraordinary select file descriptors not found: %d\n", selectCount);
    }

    /* sets the read index in the read connections size */
    *readConnectionsSize = readIndex;

    /* sets the write index in the write connections size */
    *writeConnectionsSize = writeIndex;

    /* sets the error index in the error connections size */
    *errorConnectionsSize = errorIndex;
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

ERROR_CODE openConnectionServiceSelect(struct Connection_t *connection) {
    /* retrieves the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) connection->serviceReference;

    /* in case the connection is (already) open */
    if(connection->status == STATUS_OPEN) {
        /* raises no error */
        RAISE_NO_ERROR;
    }

    /* opens the connection */
    openConection(connection);

    /* adds the connection to the service select */
    addConnectionServiceSelect(serviceSelect, connection);

    /* TODO: This setting is HARDCODED should be configured */
    connection->onRead = readHandlerStreamIo;
    connection->onWrite = writeHandlerStreamIo;
    connection->onError = errorHandlerStreamIo;
    connection->onOpen = openHandlerStreamIo;
    connection->onClose = closeHandlerStreamIo;

    /* in case the on open handler is defined */
    if(connection->onOpen != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on open handler\n");

        /* calls the on open handler */
        connection->onOpen(connection);

        /* prints a debug message */
        V_DEBUG("Finished calling on open handler\n");
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

    /* prints a debug message */
    V_DEBUG_F("Closing connection: %d\n", connection->socketHandle);

    /* in case the on close handler is defined */
    if(connection->onClose != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on close handler\n");

        /* calls the on close handler */
        connection->onClose(connection);

        /* prints a debug message */
        V_DEBUG("Finished calling on close handler\n");
    }

    /* unsets the open connection fucntion in the connection */
    connection->openConnection = NULL;

    /* unsets the close connection fucntion in the connection */
    connection->closeConnection = NULL;

    /* removes the connection from the service select */
    removeConnectionServiceSelect(serviceSelect, connection);

    /* closes the connection */
    closeConection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
