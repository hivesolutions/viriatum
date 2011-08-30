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

struct Data_t;

/**
 * The "default" function used to update a state in the connection
 * for the given service context.
 */
typedef ERROR_CODE (*connectionUpdate) (struct Connection_t *);

typedef ERROR_CODE (*connectionCallback) (struct Connection_t *);

typedef struct Service_t {
    unsigned char *name;
    unsigned char status;
    SOCKET_HANDLE serviceSocketHandle;
    struct LinkedList_t *connectionsList;
    connectionUpdate registerWrite;
    connectionUpdate unregisterWrite;
} Service;

/**
 * Structure defining a connection
 * conceptually, including the read and write
 * queue.
 * The references to the socket handle is also
 * maintained.
 */
typedef struct Connection_t {
    unsigned char status;
    SOCKET_HANDLE socketHandle;
    struct Service_t *service;
    void *serviceReference;
    unsigned int writeRegistered;
    struct LinkedList_t *readQueue;
    struct LinkedList_t *writeQueue;
    connectionUpdate closeConnection;
    connectionCallback onRead;
    connectionCallback onWrite;
    connectionCallback onError;
    connectionCallback onClose;
} Connection;

typedef enum Operation_e {
    OPERATION_WRITE = 1,
    OPERATION_READ
} Operation;

typedef enum Status_e {
    STATUS_OPEN = 1,
    STATUS_CLOSED
} Status;

/**
 * The "default" callback function to be used, without
 * any extra arguments.
 */
typedef ERROR_CODE (*serviceCallback) (struct Connection_t *, struct Data_t *, void *);

/**
 * Structure defining a data element
 * to be used in the internal queues.
 * The queues are user normally for reading
 * and writing.
 */
typedef struct Data_t {
    unsigned char *data;
    size_t size;
    serviceCallback callback;
    void *callbackParameters;
} Data;

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

/**
 * Constructor of the data.
 *
 * @param dataPointer The pointer to the data to be constructed.
 */
void createData(struct Data_t **dataPointer);

/**
 * Destructor of the data.
 *
 * @param data The data to be destroyed.
 */
void deleteData(struct Data_t *data);

/**
 * Starts the given service, initializing the
 * internal structures and the main loop.
 *
 * @param service The service to be initialized.
 */
void startService(struct Service_t *service);
void addConnectionService(struct Service_t *service, struct Connection_t *connection);
void removeConnectionService(struct Service_t *service, struct Connection_t *connection);
void createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle);
void deleteConnection(struct Connection_t *connection);
void writeConnection(struct Connection_t *connection, unsigned char *data, unsigned int size, serviceCallback callback, void *callbackParameters);
void openConection(struct Connection_t *connection);
void closeConection(struct Connection_t *connection);
