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

#include "../stream/stream.h"
#include "../polling/polling.h"

struct Data_t;
struct Polling_t;
struct Connection_t;

/**
 * The "default" function used to update a state in the connection
 * for the given service context.
 */
typedef ERROR_CODE (*connectionUpdate) (struct Connection_t *);

/**
 * The function to be used for callbacks associated with the
 * connection status updates.
 */
typedef ERROR_CODE (*connectionCallback) (struct Connection_t *);

/**
 * The "default" callback function to be used, without
 * any extra arguments.
 */
typedef ERROR_CODE (*serviceCallback) (struct Connection_t *, struct Data_t *, void *);




typedef ERROR_CODE (*pollingCallback) (struct Polling_t *);

typedef ERROR_CODE (*pollingConnectionCallback) (struct Polling_t *, struct Connection_t *);




typedef struct Polling_t {
    struct Service_t *service;
    pollingCallback open;
    pollingCallback close;
    pollingConnectionCallback registerConnection;
    pollingConnectionCallback unregisterConnection;
    pollingConnectionCallback registerWrite;
    pollingConnectionCallback unregisterWrite;
    pollingCallback poll;
    pollingCallback call;

    /**
     * Reference to the lower level
     * connection substrate.
     */
    void *lower;
} Polling;

typedef struct Service_t {
    unsigned char *name;
    unsigned char status;
    SOCKET_HANDLE serviceSocketHandle;
    struct Polling_t *polling;
    struct LinkedList_t *connectionsList;
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
    unsigned char writeRegistered;
    struct LinkedList_t *readQueue;
    struct LinkedList_t *writeQueue;
    connectionUpdate openConnection;
    connectionUpdate closeConnection;
    connectionUpdate registerWrite;
    connectionUpdate unregisterWrite;
    connectionCallback onRead;
    connectionCallback onWrite;
    connectionCallback onError;
    connectionCallback onOpen;
    connectionCallback onClose;

    /**
     * Reference to the lower level
     * connection substrate.
     */
    void *lower;
} Connection;

typedef enum Operation_e {
    OPERATION_WRITE = 1,
    OPERATION_READ
} Operation;

/**
 * Enumeration defining the various
 * status to be used in a workflow driven
 * object or structure.
 */
typedef enum Status_e {
    STATUS_OPEN = 1,
    STATUS_CLOSED
} Status;

/**
 * Structure defining a data element
 * to be used in the internal queues.
 * The queues are user normally for reading
 * and writing.
 */
typedef struct Data_t {
    unsigned char *data;
    unsigned char *dataBase;
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
 * Constructor of the polling.
 *
 * @param dataPointer The pointer to the polling to be constructed.
 */
void createPolling(struct Polling_t **pollingPointer);

/**
 * Destructor of the polling.
 *
 * @param polling The polling to be destroyed.
 */
void deletePolling(struct Polling_t *polling);

/**
 * Starts the given service, initializing the
 * internal structures and the main loop.
 *
 * @param service The service to be initialized.
 * @return The resulting error code.
 */
ERROR_CODE startService(struct Service_t *service);

/**
 * Stops the given service, stopping and cleaning
 * all the internal structures.
 * This method stops the main loop.
 *
 * @param service The service to be stopped.
 * @return The resulting error code.
 */
ERROR_CODE stopService(struct Service_t *service);

void addConnectionService(struct Service_t *service, struct Connection_t *connection);
void removeConnectionService(struct Service_t *service, struct Connection_t *connection);
void createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle);
void deleteConnection(struct Connection_t *connection);
void writeConnection(struct Connection_t *connection, unsigned char *data, unsigned int size, serviceCallback callback, void *callbackParameters);
ERROR_CODE openConnection(struct Connection_t *connection);
ERROR_CODE closeConnection(struct Connection_t *connection);
ERROR_CODE registerWriteConnection(struct Connection_t *connection);
ERROR_CODE unregisterWriteConnection(struct Connection_t *connection);
