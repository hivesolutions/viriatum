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

typedef struct Service_t {
    unsigned char *name;
    unsigned int status;
    SOCKET_HANDLE serviceSocketHandle;
    struct LinkedList_t *connectionsList;
} Service;

typedef struct Connection_t {
    unsigned int writeRegistered;
    SOCKET_HANDLE socketHandle;
    struct LinkedList_t *readQueue;
    struct LinkedList_t *writeQueue;
} Connection;

/**
 * Constructor of the service.
 *
 * @param servicePointer The pointer to the service to be constructed.
 */
void createService(struct Service_t **servicePointer);

/**
 * Destructor of the service.
 *
 * @param service The service to be destroyed.
 */
void deleteService(struct Service_t *service);
void startService(struct Service_t *service);
void addConnectionService(struct Service_t *service, struct Connection_t *connection);
void removeConnectionService(struct Service_t *service, struct Connection_t *connection);
void createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle);
void deleteConnection(struct Connection_t *connection);
