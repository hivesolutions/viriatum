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

#include "service.h"

void createService(struct Service_t **servicePointer) {
    /* retrieves the service size */
    size_t serviceSize = sizeof(struct Service_t);

    /* allocates space for the service */
    struct Service_t *service = (struct Service_t *) MALLOC(serviceSize);

    /* creates the polling (provider) */
    createPolling(&service->polling);

    /* creates the connections list */
    createLinkedList(&service->connectionsList);

    /* setst the service status as invalid */
    service->name = NULL;

    /* setst the service status as closed */
    service->status = STATUS_CLOSED;

    /* setst the service service socket handle as invalid */
    service->serviceSocketHandle = 0;

    /* sets the service in the service pointer */
    *servicePointer = service;
}

void deleteService(struct Service_t *service) {
    /* in case the service socket handle is defined */
    if(service->serviceSocketHandle) {
        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);
    }

    /* deletes the connections list */
    deleteLinkedList(service->connectionsList);

    /* deletes the polling (provider) */
    deletePolling(service->polling);

    /* releases the service */
    FREE(service);
}

void createData(struct Data_t **dataPointer) {
    /* retrieves the data size */
    size_t dataSize = sizeof(struct Data_t);

    /* allocates space for the data */
    struct Data_t *data = (struct Data_t *) MALLOC(dataSize);

    /* sets the data data */
    data->data = NULL;

    /* sets the data data base */
    data->dataBase = NULL;

    /* sets the data size */
    data->size = 0;

    /* sets the data callback */
    data->callback = NULL;

    /* sets the data callback parameters */
    data->callbackParameters = NULL;

    /* sets the data in the data pointer */
    *dataPointer = data;
}

void deleteData(struct Data_t *data) {
    /* releases the data */
    FREE(data->dataBase);

    /* releases the data */
    FREE(data);
}

void createPolling(struct Polling_t **pollingPointer) {
    /* retrieves the polling size */
    size_t pollingSize = sizeof(struct Polling_t);

    /* allocates space for the polling */
    struct Polling_t *polling = (struct Polling_t *) MALLOC(pollingSize);

    /* sets the polling service */
    polling->service = NULL;

    /* sets the polling open */
    polling->open = NULL;

    /* sets the polling close */
    polling->close = NULL;

    /* sets the polling register connection */
    polling->registerConnection = NULL;

    /* sets the polling unregister connection */
    polling->unregisterConnection = NULL;

    /* sets the polling register connection */
    polling->registerWrite = NULL;

    /* sets the polling unregister write connection */
    polling->unregisterWrite = NULL;

    /* sets the polling poll */
    polling->poll = NULL;

    /* sets the polling call */
    polling->call = NULL;

    /* sets the polling lower */
    polling->lower = NULL;

    /* sets the data in the polling pointer */
    *pollingPointer = polling;
}

void deletePolling(struct Polling_t *polling) {
    /* releases the polling */
    FREE(polling);
}

ERROR_CODE startService(struct Service_t *service) {
    /* allocates the socket address structure */
    SOCKET_ADDRESS_INPUT socketAddress;

    /* allocates the socket result */
    SOCKET_ERROR_CODE socketResult;

    /* allocates the service socket handle */
    SOCKET_HANDLE serviceSocketHandle;

    /* allocates the service connection */
    struct Connection_t *serviceConnection;

    /* allocates the polling (provider) */
    struct Polling_t *polling;

    /* allocates the process */
    PROCESS_TYPE process;

    /* allocates the memory information */
    MEMORY_INFORMATION_TYPE memoryInformation;

    /* allocates the memory ussage */
    size_t memoryUsage;

    /* allocates the option value and sets it to one (valid) */
    SOCKET_OPTION optionValue = 1;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* sets the socket address attributes */
    socketAddress.sin_family = SOCKET_INTERNET_TYPE;
    socketAddress.sin_addr.s_addr = inet_addr(VIRIATUM_DEFAULT_HOST);
    socketAddress.sin_port = htons(VIRIATUM_DEFAULT_PORT);

    /* creates the service socket for the given types */
    service->serviceSocketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    /* in case viriatum is set to non blocking */
    if(VIRIATUM_NON_BLOCKING) {
        /* sets the socket to non blocking mode */
        SOCKET_SET_NON_BLOCKING(service->serviceSocketHandle, flags);
    }

    /* sets the socket reuse address option in the socket */
    socketResult = SOCKET_SET_OPTIONS(service->serviceSocketHandle, SOCKET_OPTIONS_LEVEL_SOCKET, SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET, optionValue);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(socketResult)) {
        /* retrieves the option error code */
        SOCKET_ERROR_CODE optionErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints the error */
        V_ERROR_F("Problem setting socket option: %d\n", optionErrorCode);

        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem setting socket option");
    }

    /* binds the service socket */
    socketResult = SOCKET_BIND(service->serviceSocketHandle, socketAddress);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(socketResult)) {
        /* retrieves the binding error code */
        SOCKET_ERROR_CODE bindingErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints the error */
        V_ERROR_F("Problem binding socket: %d\n", bindingErrorCode);

        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem binding socket");
    }

    /* listens for a service socket change */
    socketResult = SOCKET_LISTEN(service->serviceSocketHandle);

    /* in case there was an error listening the socket */
    if(SOCKET_TEST_ERROR(socketResult)) {
        /* retrieves the listening error code */
        SOCKET_ERROR_CODE bindingErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints the error */
        V_ERROR_F("Problem listening socket: %d\n", bindingErrorCode);

        /* closes the service socket */
        SOCKET_CLOSE(service->serviceSocketHandle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem listening socket");
    }

    /* sets the service status as open */
    service->status = STATUS_OPEN;

    /* retrieves the service (required) elements */
    polling = service->polling;
    serviceSocketHandle = service->serviceSocketHandle;

    /* sets the service in the polling */
    polling->service = service;

    /* TODO: This value is hardcoded it should become softcoded */
    polling->open = openPollingSelect;
    polling->close = closePollingSelect;
    polling->registerConnection = registerConnectionPollingSelect;
    polling->unregisterConnection = unregisterConnectionPollingSelect;
    polling->registerWrite = registerWritePollingSelect;
    polling->unregisterWrite = unregisterWritePollingSelect;
    polling->poll = pollPollingSelect;
    polling->call = callPollingSelect;

    /* opens the polling (provider) */
    polling->open(polling);

    /* creates the (service) connection */
    createConnection(&serviceConnection, serviceSocketHandle);

    /* sets the service select service as the service in the service connection */
    serviceConnection->service = service;

    /* sets the base hanlding functions in the service connection */
    serviceConnection->openConnection = openConnection;
    serviceConnection->closeConnection = closeConnection;
    serviceConnection->registerWrite = registerWriteConnection;
    serviceConnection->unregisterWrite = unregisterWriteConnection;

    /* TODO: This setting is HARDCODED should be configurable */
    serviceConnection->onRead = acceptHandlerStreamIo;

    /* opens the (service) connection */
    serviceConnection->openConnection(serviceConnection);

    /* iterates continuously */
    while(service->status == STATUS_OPEN) {
        /* retrieves the (current) process, to be used
        to retrieves some memory information, and then closes it*/
        process = GET_PROCESS();
        GET_MEMORY_INFORMATION(process, memoryInformation);
        memoryUsage = GET_MEMORY_USAGE(memoryInformation);
        CLOSE_PROCESS(process);

        /* prints a debug message */
        V_DEBUG_F("Memory status: [%d objects] [%d KBytes]\n", ALLOCATIONS, memoryUsage / 1024);

        /* polls the connections using the polling (provider) */
        polling->poll(polling);

        /* calls the callbacks for the connection (events)
        using the polling (provider) */
        polling->call(polling);
    }

    /* closes the polling (provider) */
    polling->close(polling);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopService(struct Service_t *service) {
    /* sets the service status as closed */
    service->status = STATUS_CLOSED;

    /* raises no error */
    RAISE_NO_ERROR;
}

void addConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* retrieves the polling (provider) */
    struct Polling_t *polling = service->polling;

    /* adds the connection to the connections list */
    appendValueLinkedList(service->connectionsList, connection);

    /* registes the connection in the polling (provider) */
    polling->registerConnection(polling, connection);
}

void removeConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* retrieves the polling (provider) */
    struct Polling_t *polling = service->polling;

    /* unregisters the connection from the polling (provider) */
    polling->unregisterConnection(polling, connection);

    /* removes the connection from the connections list */
    removeValueLinkedList(service->connectionsList, connection, 1);
}

void createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle) {
    /* retrieves the connection size */
    size_t connectionSize = sizeof(struct Connection_t);

    /* allocates space for the connection */
    struct Connection_t *connection = (struct Connection_t *) MALLOC(connectionSize);

    /* sets the connection status as closed */
    connection->status = STATUS_CLOSED;

    /* sets the socket handle in the connection */
    connection->socketHandle = socketHandle;

    /* sets the service as not set */
    connection->service = NULL;

    /* sets the service reference as not set */
    connection->serviceReference = NULL;

    /* sets the write registered to false */
    connection->writeRegistered = 0;

    /* sets the open connection to unset */
    connection->openConnection = NULL;

    /* sets the close connection to unset */
    connection->closeConnection = NULL;

    /* sets the on read to unset */
    connection->onRead = NULL;

    /* sets the on write to unset */
    connection->onWrite = NULL;

    /* sets the on error to unset */
    connection->onError = NULL;

    /* sets the on open to unset */
    connection->onOpen = NULL;

    /* sets the on close to unset */
    connection->onClose = NULL;

    /* sets the lower to unset */
    connection->lower = NULL;

    /* creates the read queue linked list */
    createLinkedList(&connection->readQueue);

    /* creates the write queue linked list */
    createLinkedList(&connection->writeQueue);

    /* sets the connection in the connection pointer */
    *connectionPointer = connection;
}

void deleteConnection(struct Connection_t *connection) {
    /* allocates the data */
    struct Data_t *data;

    /* iterates continuously */
    while(1) {
        /* pops a value (data) from the linked list (write queue) */
        popValueLinkedList(connection->writeQueue, (void **) &data, 1);

        /* in case the data is invalid (list is empty) */
        if(data == NULL) {
            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG("Deleting data (cleanup structures)\n");

        /* deletes the data */
        deleteData(data);
    }

    /* deletes the read queue linked list */
    deleteLinkedList(connection->readQueue);

    /* deletes the write queue linked list */
    deleteLinkedList(connection->writeQueue);

    /* releases the connection */
    FREE(connection);
}

void writeConnection(struct Connection_t *connection, unsigned char *data, unsigned int size, serviceCallback callback, void *callbackParameters) {
    /* allocates the data */
    struct Data_t *_data;

    /* retrieves the (connection) service */
    struct Service_t *service = connection->service;

    /* creates the data */
    createData(&_data);

    /* sets the data contents */
    _data->data = data;
    _data->dataBase = data;
    _data->size = size;
    _data->callback = callback;
    _data->callbackParameters = callbackParameters;

    /* adds the file buffer to the write queue */
    appendValueLinkedList(connection->writeQueue, (void *) _data);

    /* registers the connection for write */
    connection->registerWrite(connection);
}

ERROR_CODE openConnection(struct Connection_t *connection) {
    /* in case the connection is (already) open */
    if(connection->status == STATUS_OPEN) {
        /* raises no error */
        RAISE_NO_ERROR;
    }

    /* prints a debug message */
    V_DEBUG_F("Opening connection: %d\n", connection->socketHandle);

    /* sets the connection status as open */
    connection->status = STATUS_OPEN;

    /* adds the connection to the service select */
    addConnectionService(connection->service, connection);

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

ERROR_CODE closeConnection(struct Connection_t *connection) {
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

    /* removes the connection from the service select */
    removeConnectionService(connection->service, connection);

    /* unsets the base hanlding functions from the connection */
    connection->openConnection = NULL;
    connection->closeConnection = NULL;
    connection->registerWrite = NULL;
    connection->unregisterWrite = NULL;

    /* closes the socket */
    SOCKET_CLOSE(connection->socketHandle);

    /* sets the connection status as closed */
    connection->status = STATUS_CLOSED;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE registerWriteConnection(struct Connection_t *connection) {
    /* retrieves the (connection) service */
    struct Service_t *service = connection->service;

    /* retrieves the polling (provider) */
    struct Polling_t *polling = service->polling;

    /* registers the connection for write */
    polling->registerWrite(polling, connection);

    /* sets the connection as write registered */
    connection->writeRegistered = 1;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterWriteConnection(struct Connection_t *connection) {
    /* retrieves the (connection) service */
    struct Service_t *service = connection->service;

    /* retrieves the polling (provider) */
    struct Polling_t *polling = service->polling;

    /* unregisters the connection for write */
    polling->unregisterWrite(polling, connection);

    /* sets the connection as not write registered */
    connection->writeRegistered = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}
