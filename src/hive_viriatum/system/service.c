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

ERROR_CODE createHttpHandlerService(struct Service_t *service, struct HttpHandler_t **httpHandlerPointer) {
    /* creates the http handler */
    createHttpHandler(httpHandlerPointer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpHandlerService(struct Service_t *service, struct HttpHandler_t *httpHandler) {
    /* deletes the http handler */
    deleteHttpHandler(httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE addHttpHandlerService(struct Service_t *service, struct HttpHandler_t *httpHandler) {
    /* adds the http handler to the list of http handlers in the service */
    appendValueLinkedList(service->httpHandlersList, (void *) httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE removeHttpHandlerService(struct Service_t *service, struct HttpHandler_t *httpHandler) {
    /* removes the http handler from the list of http handlers in the service */
    removeValueLinkedList(service->httpHandlersList, (void *) httpHandler, 1);

    /* raises no error */
    RAISE_NO_ERROR;
}










void createService(struct Service_t **servicePointer) {
    /* retrieves the service size */
    size_t serviceSize = sizeof(struct Service_t);

    /* allocates space for the service */
    struct Service_t *service = (struct Service_t *) MALLOC(serviceSize);

    /* sets the service attributes (default) values */
    service->name = NULL;
    service->status = STATUS_CLOSED;
    service->serviceSocketHandle = 0;
    service->createHttpHandler = createHttpHandlerService;
    service->deleteHttpHandler = deleteHttpHandlerService;
    service->addHttpHandler = addHttpHandlerService;
    service->removeHttpHandler = removeHttpHandlerService;

    /* creates the polling (provider) */
    createPolling(&service->polling);

    /* creates the connections list */
    createLinkedList(&service->connectionsList);

    /* creates the http modules list */
    createLinkedList(&service->modulesList);



    /* creates the http handlers list */
    createLinkedList(&service->httpHandlersList);




    /* sets the service in the service pointer */
    *servicePointer = service;
}

void deleteService(struct Service_t *service) {
    /* in case the service socket handle is defined */
    if(service->serviceSocketHandle) {
        /* closes the service socket (secure closing) */
        SOCKET_CLOSE(service->serviceSocketHandle);
    }


    /* deletes the http handlers list */
    deleteLinkedList(service->httpHandlersList);


    /* deletes the http modules list */
    deleteLinkedList(service->modulesList);

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

    /* sets the data attributes (default) values */
    data->data = NULL;
    data->dataBase = NULL;
    data->size = 0;
    data->callback = NULL;
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

    /* sets the polling attributes (default) values */
    polling->service = NULL;
    polling->open = NULL;
    polling->close = NULL;
    polling->registerConnection = NULL;
    polling->unregisterConnection = NULL;
    polling->registerWrite = NULL;
    polling->unregisterWrite = NULL;
    polling->poll = NULL;
    polling->call = NULL;
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

    /* loads (all) the currently available modules */
    loadModulesService(service);

    /* sets the socket address attributes */
    socketAddress.sin_family = SOCKET_INTERNET_TYPE;
    socketAddress.sin_addr.s_addr = inet_addr(VIRIATUM_DEFAULT_HOST);
    socketAddress.sin_port = htons(VIRIATUM_DEFAULT_PORT);

    /* creates the service socket for the given types */
    service->serviceSocketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    /* in case there was an error creating the service socket */
    if(SOCKET_TEST_ERROR(service->serviceSocketHandle)) {
        /* retrieves the creating error code */
        SOCKET_ERROR_CODE creatingErrorCode = SOCKET_GET_ERROR_CODE(socketResult);

        /* prints the error */
        V_ERROR_F("Problem creating socket: %d\n", creatingErrorCode);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem creating socket");
    }

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
    serviceConnection->writeConnection = writeConnection;
    serviceConnection->registerWrite = registerWriteConnection;
    serviceConnection->unregisterWrite = unregisterWriteConnection;

    /* TODO: This setting is HARDCODED should be configurable */
    serviceConnection->onRead = acceptHandlerStreamIo;

    /* opens the (service) connection */
    serviceConnection->openConnection(serviceConnection);

    /* iterates continuously, while the service is open */
    while(service->status == STATUS_OPEN) {
        /* retrieves the (current) process, to be used
        to retrieves some memory information, and then closes it*/
        process = GET_PROCESS();
        GET_MEMORY_INFORMATION(process, memoryInformation);
        memoryUsage = GET_MEMORY_USAGE(memoryInformation);
        CLOSE_PROCESS(process);

        /* prints a debug message */
        V_DEBUG_F("Memory status: [%ld objects] [%ld KBytes]\n", (long int) ALLOCATIONS, (long int) memoryUsage / 1024);

        /* polls the connections using the polling (provider) */
        polling->poll(polling);

        /* calls the callbacks for the connection (events)
        using the polling (provider) */
        polling->call(polling);
    }

    /* closes (all) the service connections */
    closeConnectionsService(service);

    /* closes the polling (provider) */
    polling->close(polling);

    /* unloads the modules for the service */
    unloadModulesService(service);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopService(struct Service_t *service) {
    /* sets the service status as closed */
    service->status = STATUS_CLOSED;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeConnectionsService(struct Service_t *service) {
    /* allocates space for the current connection */
    struct Connection_t *currentConnection;

    /* retrieves the service connections list */
    struct LinkedList_t *connectionsList = service->connectionsList;

    /* prints a debug message */
    V_DEBUG("Closing the service connections\n");

    /* iterates continuously */
    while(1) {
        /* pops a value from the connections list (and deletes the node) */
        popValueLinkedList(connectionsList, (void **) &currentConnection, 1);

        /* in case the current connection is null (connections list is empty) */
        if(currentConnection == NULL) {
            /* breaks the loop */
            break;
        }

        /* closes the current connection */
        currentConnection->closeConnection(currentConnection);

        /* deletes the current connection */
        deleteConnection(currentConnection);
    }

    /* prints a debug message */
    V_DEBUG("Finished closing the service connections\n");

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE loadModulesService(struct Service_t *service) {
    /* allocates the error code */
    ERROR_CODE errorCode;

	/* allocates space for the linked list for the entries
	and for the iterator to iterate "around" them */
	struct LinkedList_t *entries;
	struct Iterator_t *entriesIterator;

	/* allocates space for an entry of the directory */
	struct File_t *entry;

	/* allocates space for the path to be used to load the module */
	unsigned char modulePath[VIRIATUM_MAX_PATH_SIZE];

	/* creates the linked list for the entries and populates
	it with the entries from the plugins path */
	createLinkedList(&entries);
	listDirectoryFile(VIRIATUM_PLUGINS_PATH, entries);
	
	/* creates the iterator for the entries */
	createIteratorLinkedList(entries, &entriesIterator);

	/* iterates continuously */
    while(1) {
        /* retrieves the next value from the iterator */
        getNextIterator(entriesIterator, (void **) &entry);

        /* in case the current module is null (end of iterator) */
        if(entry == NULL) {
            /* breaks the loop */
            break;
        }

		/* in case the entry name does not ends with the shared object extension
		it must not be a module to be loaded */
		if(endsWithString(entry->name, VIRIATUM_SHARED_OBJECT_EXTENSION) == 0) {
			/* continue with the loop */
			continue;
		}

		/* creates the complete module path for the loading of it */
		SPRINTF(modulePath, VIRIATUM_MAX_PATH_SIZE, "%s/%s", VIRIATUM_PLUGINS_PATH, entry->name);

		/* loads the module, retrieving a possible error code */
		errorCode = loadModule(service, modulePath);

		/* tests the error code for error */
		if(IS_ERROR_CODE(errorCode)) {
			/* prints a warning message */
			V_WARNING_F("Problem loading module (%s)\n", (char *) GET_ERROR());

			/* raises again the error */
			RAISE_AGAIN(errorCode);
		}
    }

	/* deletes the iterator used for the entries */
	deleteIteratorLinkedList(entries, entriesIterator);

	/* deletes both the entries internal structures
	and the entries linked list */
	deleteDirectoryEntriesFile(entries);
	deleteLinkedList(entries);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unloadModulesService(struct Service_t *service) {
    /* allocates the current module */
    struct Module_t *currentModule;

    /* allocates space for the modules list iterator */
    struct Iterator_t *modulesListIterator;

    /* creates the iterator for the linked list */
    createIteratorLinkedList(service->modulesList, &modulesListIterator);

    /* iterates continuously */
    while(1) {
        /* retrieves the next value from the iterator */
        getNextIterator(modulesListIterator, (void **) &currentModule);

        /* in case the current module is null (end of iterator) */
        if(currentModule == NULL) {
            /* breaks the loop */
            break;
        }

        /* unloads the current module */
        unloadModule(service, currentModule);
    }

    /* deletes the iterator linked list */
    deleteIteratorLinkedList(service->modulesList, modulesListIterator);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE addConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* retrieves the polling (provider) */
    struct Polling_t *polling = service->polling;

    /* adds the connection to the connections list */
    appendValueLinkedList(service->connectionsList, connection);

    /* registes the connection in the polling (provider) */
    polling->registerConnection(polling, connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE removeConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* retrieves the polling (provider) */
    struct Polling_t *polling = service->polling;

    /* unregisters the connection from the polling (provider) */
    polling->unregisterConnection(polling, connection);

    /* removes the connection from the connections list */
    removeValueLinkedList(service->connectionsList, connection, 1);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle) {
    /* retrieves the connection size */
    size_t connectionSize = sizeof(struct Connection_t);

    /* allocates space for the connection */
    struct Connection_t *connection = (struct Connection_t *) MALLOC(connectionSize);

    /* sets the connection attributes (default) values */
    connection->status = STATUS_CLOSED;
    connection->socketHandle = socketHandle;
    connection->service = NULL;
    connection->writeRegistered = 0;
    connection->openConnection = NULL;
    connection->closeConnection = NULL;
    connection->writeConnection = NULL;
    connection->registerWrite = NULL;
    connection->unregisterWrite = NULL;
    connection->allocData = allocConnection;
    connection->onRead = NULL;
    connection->onWrite = NULL;
    connection->onError = NULL;
    connection->onOpen = NULL;
    connection->onClose = NULL;
    connection->lower = NULL;

    /* creates the read queue linked list */
    createLinkedList(&connection->readQueue);

    /* creates the write queue linked list */
    createLinkedList(&connection->writeQueue);

    /* sets the connection in the connection pointer */
    *connectionPointer = connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteConnection(struct Connection_t *connection) {
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

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE writeConnection(struct Connection_t *connection, unsigned char *data, unsigned int size, connectionDataCallback callback, void *callbackParameters) {
    /* allocates the data */
    struct Data_t *_data;

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

    /* raises no error */
    RAISE_NO_ERROR;
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

ERROR_CODE allocConnection(struct Connection_t *connection, size_t size, void **dataPointer) {
    /* allocates a data chunk with the required size */
    *dataPointer = MALLOC(size);

    /* raises no error */
    RAISE_NO_ERROR;
}
