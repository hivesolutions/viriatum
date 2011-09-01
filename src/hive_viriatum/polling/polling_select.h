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

#pragma once

#include "../system/system.h"

typedef struct PollingSelect_t {
    /**
     * The polling reference, for top
     * level reference.
     */
    struct Polling_t *polling;

    /**
     * The value of the highest socket
     * reference (performance issue).
     */
    SOCKET_HANDLE socketsSetHighest;

    /**
     * The socket set for the read sockets.
     */
    SOCKET_SET socketsReadSet;

    /**
     * The socket set for the write sockets.
     */
    SOCKET_SET socketsWriteSet;

    /**
     * The socket set for the error sockets.
     */
    SOCKET_SET socketsErrorSet;

    /**
     * The temporary socket set for the
     * read sockets.
     */
    SOCKET_SET socketsReadSetTemporary;

    /**
     * The temporary socket set for the
     * write sockets.
     */
    SOCKET_SET socketsWriteSetTemporary;

    /**
     * The temporary socket set for the
     * error sockets.
     */
    SOCKET_SET socketsErrorSetTemporary;


    struct Connection_t **readConnections;
    struct Connection_t **writeConnections;
    struct Connection_t **errorConnections;
    struct Connection_t **removeConnections;
    unsigned int readConnectionsSize;
    unsigned int writeConnectionsSize;
    unsigned int errorConnectionsSize;
    unsigned int removeConnectionsSize;




    /**
     * The timeout value used for the
     * select call.
     */
    struct timeval selectTimeout;

    /**
     * The temporary select timeout value.
     */
    struct timeval selectTimeoutTemporary;
} PollingSelect;

ERROR_CODE openPollingSelect(struct Polling_t *polling);
ERROR_CODE closePollingSelect(struct Polling_t *polling);
ERROR_CODE registerConnectionPollingSelect(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE unregisterConnectionPollingSelect(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE registerWritePollingSelect(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE unregisterWritePollingSelect(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE pollPollingSelect(struct Polling_t *polling);
ERROR_CODE callPollingSelect(struct Polling_t *polling);
ERROR_CODE _pollPollingSelect(struct PollingSelect_t *pollingSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, struct Connection_t **errorConnections, unsigned int *readConnectionsSize, unsigned int *writeConnectionsSize, unsigned int *errorConnectionsSize);
ERROR_CODE _callPollingSelect(struct PollingSelect_t *pollingSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, struct Connection_t **errorConnections, struct Connection_t **removeConnections, unsigned int readConnectionsSize, unsigned int writeConnectionsSize, unsigned int errorConnectionsSize);
ERROR_CODE _registerSocketsSetPollingSelect(struct PollingSelect_t *pollingSelect, SOCKET_HANDLE socketHandle, SOCKET_SET *socketsSet);
ERROR_CODE _unregisterSocketsSetPollingSelect(struct PollingSelect_t *pollingSelect, SOCKET_HANDLE socketHandle, SOCKET_SET *socketsSet);

/**
 * Removes a connection from the remove connections array.
 * This method checks for duplicates and in case they exist, no
 * connection is added.
 *
 * @param removeConnections The list of connection for removal.
 * @param removeConnectionsSizePointer A pointer to the sisze
 * of the remove connections array.
 * @param connection The connection to be removed (deleted).
 */
static __inline void removeConnection(struct Connection_t **removeConnections, unsigned int *removeConnectionsSizePointer, struct Connection_t *connection) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current connection */
    struct Connection_t *currentConnection;

    /* retrieves the remove connections size */
    unsigned int removeConnectionsSize = *removeConnectionsSizePointer;

    /* iterates over the remove connections */
    for(index = 0; index < removeConnectionsSize; index++) {
        /* retrieves the current connection */
        currentConnection = removeConnections[index];

        /* in case the current connection already exists */
        if(currentConnection == connection) {
            /* returns immediately */
            return;
        }
    }

    /* adds the connection to the remove connections */
    removeConnections[removeConnectionsSize] = connection;

    /* increments the remove connections size */
    (*removeConnectionsSizePointer)++;
}
