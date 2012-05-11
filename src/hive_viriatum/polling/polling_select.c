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

#include "polling_select.h"

void createPollingSelect(struct PollingSelect_t **pollingSelectPointer, struct Polling_t *polling) {
    /* retrieves the polling select size */
    size_t pollingSelectSize = sizeof(struct PollingSelect_t);

    /* retrieves the connection pointer size */
    size_t connectionPointerSize = sizeof(struct Connection_t *);

    /* allocates space for the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) MALLOC(pollingSelectSize);

    /* resets the polling in the polling select */
    pollingSelect->polling = polling;

    /* resets the sockets set highest value */
    pollingSelect->socketsSetHighest = 0;

    /* zeros the sockets read set */
    SOCKET_SET_ZERO(&pollingSelect->socketsReadSet);

    /* zeros the sockets write set */
    SOCKET_SET_ZERO(&pollingSelect->socketsWriteSet);

    /* zeros the sockets error set */
    SOCKET_SET_ZERO(&pollingSelect->socketsErrorSet);

    /* zeros the sockets read set temporary */
    SOCKET_SET_ZERO(&pollingSelect->socketsReadSetTemporary);

    /* zeros the sockets write set temporary */
    SOCKET_SET_ZERO(&pollingSelect->socketsWriteSetTemporary);

    /* zeros the sockets error set temporary */
    SOCKET_SET_ZERO(&pollingSelect->socketsErrorSetTemporary);

    /* allocates the read connection for internal
    polling select usage */
    pollingSelect->readConnections = (struct Connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connectionPointerSize);

    /* allocates the write connection for internal
    polling select usage */
    pollingSelect->writeConnections = (struct Connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connectionPointerSize);

    /* allocates the error connection for internal
    polling select usage */
    pollingSelect->errorConnections = (struct Connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connectionPointerSize);

    /* allocates the remove connection for internal
    polling select usage */
    pollingSelect->removeConnections = (struct Connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connectionPointerSize);

    /* sets the default timeout */
    pollingSelect->selectTimeout.tv_sec = VIRIATUM_SELECT_TIMEOUT;
    pollingSelect->selectTimeout.tv_usec = 0;

    /* sets the polling select in the polling select pointer */
    *pollingSelectPointer = pollingSelect;
}

void deletePollingSelect(struct PollingSelect_t *pollingSelect) {
    /* releases the remove connections */
    FREE(pollingSelect->removeConnections);

    /* releases the error connection */
    FREE(pollingSelect->errorConnections);

    /* releases the write connection */
    FREE(pollingSelect->writeConnections);

    /* releases the read connection */
    FREE(pollingSelect->readConnections);

    /* releases the polling select */
    FREE(pollingSelect);
}

ERROR_CODE openPollingSelect(struct Polling_t *polling) {
    /* allocates the polling select */
    struct PollingSelect_t *pollingSelect;

    /* creates the polling select */
    createPollingSelect(&pollingSelect, polling);

    /* sets the polling select in the polling as
    the lower substrate */
    polling->lower = (void *) pollingSelect;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closePollingSelect(struct Polling_t *polling) {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* deletes the polling select */
    deletePollingSelect(pollingSelect);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE registerConnectionPollingSelect(struct Polling_t *polling, struct Connection_t *connection) {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* registers the socket handle in the sockets read set */
    _registerSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsReadSet);

    /* in case the socket error are meant to be processed */
    if(VIRIATUM_SOCKET_ERROR) {
        /* registers the socket handle in the sockets error set */
        _registerSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsReadSet);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterConnectionPollingSelect(struct Polling_t *polling, struct Connection_t *connection)  {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* unregister the socket handle from the sockets read set */
    _unregisterSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsReadSet);

    /* in case the socket error are meant to be processed */
    if(VIRIATUM_SOCKET_ERROR) {
        /* unregister the socket handle from the sockets error set */
        _unregisterSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsErrorSet);
    }

    /* in case the connection write is registered */
    if(connection->writeRegistered == 1) {
        /* unregister the socket handle from the sockets write set */
        _unregisterSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsWriteSet);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE registerWritePollingSelect(struct Polling_t *polling, struct Connection_t *connection) {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* in case the connection is write registered */
    if(connection->writeRegistered == 1) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Connection already registered");
    }

    /* register the socket handle from the sockets write set */
    _registerSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsWriteSet);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterWritePollingSelect(struct Polling_t *polling, struct Connection_t *connection) {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* in case the connection is not write registered */
    if(connection->writeRegistered == 0) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Connection already unregistered");
    }

    /* unregister the socket handle from the sockets write set */
    _unregisterSocketsSetPollingSelect(pollingSelect, connection->socketHandle, &pollingSelect->socketsWriteSet);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE pollPollingSelect(struct Polling_t *polling) {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* polls the polling select */
    _pollPollingSelect(pollingSelect, pollingSelect->readConnections, pollingSelect->writeConnections, pollingSelect->errorConnections, &pollingSelect->readConnectionsSize, &pollingSelect->writeConnectionsSize, &pollingSelect->errorConnectionsSize);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE callPollingSelect(struct Polling_t *polling) {
    /* retrieves the polling select */
    struct PollingSelect_t *pollingSelect = (struct PollingSelect_t *) polling->lower;

    /* calls the polling select */
    _callPollingSelect(pollingSelect, pollingSelect->readConnections, pollingSelect->writeConnections, pollingSelect->errorConnections, pollingSelect->removeConnections, pollingSelect->readConnectionsSize, pollingSelect->writeConnectionsSize, pollingSelect->errorConnectionsSize);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _pollPollingSelect(struct PollingSelect_t *pollingSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, struct Connection_t **errorConnections, unsigned int *readConnectionsSize, unsigned int *writeConnectionsSize, unsigned int *errorConnectionsSize) {
    /* allocates space for the select count */
    int selectCount;

    /* allocates space for the current connection */
    struct Connection_t *currentConnection;

    /* allocates space for the connections list iterator */
    struct Iterator_t *connectionsListIterator;

    /* retrieves the service */
    struct Service_t *service = pollingSelect->polling->service;

    /* retrieves the service connections list */
    struct LinkedList_t *connectionsList = service->connectionsList;

    /* initializes the read index */
    unsigned int readIndex = 0;

    /* initializes the write index */
    unsigned int writeIndex = 0;

    /* initializes the error index */
    unsigned int errorIndex = 0;

    /* copies the socket sets to the socket sets temporary */
    pollingSelect->socketsReadSetTemporary = pollingSelect->socketsReadSet;
    pollingSelect->socketsWriteSetTemporary = pollingSelect->socketsWriteSet;

    /* copies the select timeout to the select timeout temporary */
    pollingSelect->selectTimeoutTemporary = pollingSelect->selectTimeout;

    /* creates the iterator for the linked list */
    createIteratorLinkedList(connectionsList, &connectionsListIterator);

    /* prints a debug message */
    V_DEBUG("Entering select statement\n");

    /* runs the select over the sockets set */
    selectCount = SOCKET_SELECT(pollingSelect->socketsSetHighest + 1, &pollingSelect->socketsReadSetTemporary, &pollingSelect->socketsWriteSetTemporary, &pollingSelect->socketsErrorSetTemporary, &pollingSelect->selectTimeoutTemporary);

    /* prints a debug message */
    V_DEBUG_F("Exiting select statement with value: %d\n", selectCount);

    /* in case there was an error in select */
    if(SOCKET_TEST_ERROR(selectCount)) {
        /* retrieves the select error code */
        SOCKET_ERROR_CODE selectErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints an info message */
        V_INFO_F("Problem running select: %d\n", selectErrorCode);

        /* resets the values for the various read values,
        this avoid possible problems in next actions */
        *readConnectionsSize = 0;
        *writeConnectionsSize = 0;
        *errorConnectionsSize = 0;

        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem running select");
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
        if(SOCKET_SET_IS_SET(currentConnection->socketHandle, &pollingSelect->socketsReadSetTemporary) > 0)  {
            /* sets the current connection in the read connections */
            readConnections[readIndex] = currentConnection;

            /* increments the read index */
            readIndex++;

            /* decrements the select count */
            selectCount--;
        }

        /* in case the current connection socket handle is set in
        the sockets write ready set */
        if(SOCKET_SET_IS_SET(currentConnection->socketHandle, &pollingSelect->socketsWriteSetTemporary) > 0)  {
            /* sets the current connection in the write connections */
            writeConnections[writeIndex] = currentConnection;

            /* increments the write index */
            writeIndex++;

            /* decrements the select count */
            selectCount--;
        }

        /* in case the current connection socket handle is set in
        the sockets error ready set */
        if(SOCKET_SET_IS_SET(currentConnection->socketHandle, &pollingSelect->socketsErrorSetTemporary) > 0)  {
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

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _callPollingSelect(struct PollingSelect_t *pollingSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, struct Connection_t **errorConnections, struct Connection_t **removeConnections, unsigned int readConnectionsSize, unsigned int writeConnectionsSize, unsigned int errorConnectionsSize) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current connection */
    struct Connection_t *currentConnection;

    /* resets the remove connections size */
    unsigned int removeConnectionsSize = 0;

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

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _registerSocketsSetPollingSelect(struct PollingSelect_t *pollingSelect, SOCKET_HANDLE socketHandle,  SOCKET_SET *socketsSet) {
    /* in case the current socket handle is bigger than the polling
    select sockets set highest value */
    if(socketHandle > pollingSelect->socketsSetHighest) {
        /* sets the socket handle as the sockets set highest */
        pollingSelect->socketsSetHighest = socketHandle;
    }

    /* adds the socket handle to the sockets set */
    SOCKET_SET_SET(socketHandle, socketsSet);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unregisterSocketsSetPollingSelect(struct PollingSelect_t *pollingSelect, SOCKET_HANDLE socketHandle,  SOCKET_SET *socketsSet) {
    /* removes the socket handle from the sockets set */
    SOCKET_SET_CLEAR(socketHandle, socketsSet);

    /* raises no error */
    RAISE_NO_ERROR;
}
