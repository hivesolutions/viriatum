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

#include "../http/http.h"
#include "../handlers/handlers.h"
#include "service.h"

typedef struct ServiceSelect_t {
    /**
     * The service reference, for top
     * level reference.
     */
    struct Service_t *service;

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
     * The timeout value used for the
     * select call.
     */
    struct timeval selectTimeout;

    /**
     * The temporary select timeout value.
     */
    struct timeval selectTimeoutTemporary;
} ServiceSelect_t;

void createServiceSelect(struct ServiceSelect_t **serviceSelectPointer);
void deleteServiceSelect(struct ServiceSelect_t *serviceSelect);
void startServiceSelect(struct ServiceSelect_t *serviceSelect);
void addConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection);
void removeConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection);
void pollServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t **readConnections, struct Connection_t **writeConnections, unsigned int *readConnectionsSize, unsigned int *writeConnectionsSize, unsigned int *serviceSocketReady);
void addSocketHandleSocketsSetServiceSelect(struct ServiceSelect_t *serviceSelect, SOCKET_HANDLE socketHandle, SOCKET_SET *socketsSet);
void removeSocketHandleSocketsSetServiceSelect(struct ServiceSelect_t *serviceSelect, SOCKET_HANDLE socketHandle, SOCKET_SET *socketsSet);



void registerConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection, char operation);
void unregisterConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection, char operation);
