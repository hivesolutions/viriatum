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
#include "../module/module.h"
#include "../polling/polling.h"

/* forward references (avoids loop) */
struct Data_t;
struct Polling_t;
struct Connection_t;
struct HttpHandler_t;

/**
 * The function used to create a new handler instance
 * with a name and for the service context.
 */
typedef ERROR_CODE (*serviceHttpHandlerAccess) (struct Service_t *, struct HttpHandler_t **, unsigned char *name);

/**
 * The function used to update an handler state or value.
 */
typedef ERROR_CODE (*serviceHttpHandlerUpdate) (struct Service_t *, struct HttpHandler_t *);

/**
 * The "default" function used to update a state in the connection
 * for the given service context.
 */
typedef ERROR_CODE (*connectionUpdate) (struct Connection_t *);

/**
 * The function used to allocate data in the context
 * of a connection (for safe usage).
 */
typedef ERROR_CODE (*connectionAlloc) (struct Connection_t *, size_t size, void **dataPointer);

/**
 * The function to be used for callbacks associated with the
 * connection status updates.
 */
typedef ERROR_CODE (*connectionCallback) (struct Connection_t *);

/**
 * The function to be used for callbacks associated with the
 * connection data updates.
 */
typedef ERROR_CODE (*connectionDataCallback) (struct Connection_t *, struct Data_t *, void *);

/**
 * Function used to write data into a connection, optional
 * parameters allow a callback uppon the end of writing.
 */
typedef ERROR_CODE (*connectionWrite) (struct Connection_t *connection, unsigned char *data, unsigned int size, connectionDataCallback callback, void *callbackParameters);

/**
 * The "default" function used to update a state in the polling
 * for the given service context.
 */
typedef ERROR_CODE (*pollingUpdate) (struct Polling_t *);

/**
 * The function used to update a state in the polling
 * using the given connection as reference.
 */
typedef ERROR_CODE (*pollingConnectionUpdate) (struct Polling_t *, struct Connection_t *);

/**
 * Structure used to describe a polling (provider)
 * with all the action functions and callbacks.
 */
typedef struct Polling_t {
    /**
     * The reference to the "owning" service.
     * The "owning" service should be the
     * only using this polling (provider).
     */
    struct Service_t *service;

    /**
     * Function called for opening the polling
     * (provider).
     * This function should initialize all the
     * polling context structures.
     */
    pollingUpdate open;

    /**
     * Function called for closing the polling
     * (provider).
     * This function should destroy all the
     * polling context structures.
     */
    pollingUpdate close;

    /**
     * Function used to register a connection
     * into the polling (provider).
     * The polling system structures should
     * be updated to allow the connection events
     * to be detected.
     */
    pollingConnectionUpdate registerConnection;

    /**
     * Function used to unregister a connection
     * from the polling (provider).
     */
    pollingConnectionUpdate unregisterConnection;

    /**
     * Function used to the start "listening"
     * to the write events in the connection.
     */
    pollingConnectionUpdate registerWrite;

    /**
     * Function used to the stop "listening"
     * to the write events in the connection.
     */
    pollingConnectionUpdate unregisterWrite;

    /**
     * Function used to poll the connections
     * checking if new "events" are available.
     * This function should not block for a long
     * time (exiting may be compromised).
     */
    pollingUpdate poll;

    /**
     * Function to be used to process and call
     * the callbacks associated with the current
     * events.
     * This function may be called after a polling.
     */
    pollingUpdate call;

    /**
     * Reference to the lower level
     * connection substrate (child).
     */
    void *lower;
} Polling;

/**
 * Structure used to describe a service structure.
 * The main service socket is set and shold be
 * maintained for house-keeping purposes.
 * The list of connections should represent the
 * current set of active connections.
 */
typedef struct Service_t {
    /**
     * The descriptive name of the
     * service.
     * For textual representation.
     */
    unsigned char *name;

    /**
     * The current status of the service.
     * Used for service life-cycle control.
     */
    unsigned char status;

    /**
     * The set of options that configure
     * the current service instance.
     * These options are used both at the
     * startup and during the runtime.
     */
    struct ServiceOptions_t *options;

    /**
     * The map containing a set of configuration
     * loaded primarly from the configuration files.
     * This map is structured by domains and the
     * first level of keys represent these domains.
     */
    struct HashMap_t *configuration;

    /**
     * The socket handle to the service
     * connection.
     */
    SOCKET_HANDLE serviceSocketHandle;

    /**
     * The http handler currently in use.
     * Only one http (parser) handler can
     * be used at one given time.
     */
    struct HttpHandler_t *httpHandler;

    /**
     * The reference to the polling (provider)
     * used by the service.
     */
    struct Polling_t *polling;

    /**
     * The list of currenly available (active)
     * connections in the service.
     */
    struct LinkedList_t *connectionsList;

    /**
     * The list of modules available for the
     * service.
     * This list represents only the loaded
     * modules any unloaded or failed module
     * should not be present in this list.
     */
    struct LinkedList_t *modulesList;

    /**
     * The map of http handlers available
     * for the service.
     * The map associates the name of the handler
     * with the handler instance.
     */
    struct HashMap_t *httpHandlersMap;

    serviceHttpHandlerAccess createHttpHandler;
    serviceHttpHandlerUpdate deleteHttpHandler;
    serviceHttpHandlerUpdate addHttpHandler;
    serviceHttpHandlerUpdate removeHttpHandler;
    serviceHttpHandlerAccess getHttpHandler;
} Service;

/**
 * Structure defining the set of options/configuration
 * for an associated service.
 */
typedef struct ServiceOptions_t {
    /**
     * The "default" tcp port to bind the service
     * associated with these options.
     */
    unsigned short port;

    /**
     * The "default" address to bind the service
     * associated with these options.
     */
    unsigned char *address;

    /**
     * The name of the "default" handler to be used
     * in the runtime environment.
     * Change this name carefully.
     */
    unsigned char *handlerName;

    /**
     * If the default index file should be used in case
     * the root path file is requested (index.html is
     * the default file to be served).
     */
    unsigned int defaultIndex;

    /**
     * The default virtual host to be used in any
     * non matched host request.
     */
    struct VirtualHost_t *defaultVirtualHost;

    /**
     * The set of virtual hosts associated with the
     * current service.
     * The map key is the (virtual) host name to be
     * used, default host name is not contained in
     * the list for performance.
     */
    struct HashMap_t *virtualHosts;
} ServiceOptions;

typedef struct VirtualHost_t {
    /**
     * The descriptive name of the viratual
     * host reference.
     * For textual representation.
     */
    unsigned char *name;

    /**
     * The default rule for the virtual to be
     * used when no other rule is matched or
     * when the list of rules is empty.
     * This value is maintained separate for
     * performance reasons.
     */
    struct Rule_t *defaultRule;

    /**
     * The list of rules to be used for matching
     * the request, for gathering the correct
     * handler.
     */
    struct LinkedList_t *rules;
} VirtualHost_t;

/**
 * Enumeration defining the various types
 * of possible rules for a given rule
 * aggregation.
 */
typedef enum RuleType_e {
    DIRECTORY_RULE = 1,
    REGEX_RULE
} RuleType;

typedef struct Rule_t {
    enum RuleType_e type;
    void *value;
} Rule;

typedef struct RuleDirectory_t {
    char *path;
} RuleDirectory;

/**
 * Structure defining a connection
 * conceptually, including the read and write
 * queue.
 * The references to the socket handle is also
 * maintained.
 */
typedef struct Connection_t {
    /**
     * The current status of the connection.
     * Used for connection control.
     */
    unsigned char status;

    /**
     * The socket handle associated with
     * the connection.
     * Typically this represent a file
     * descriptor.
     */
    SOCKET_HANDLE socketHandle;

    /**
     * The reference to the service controlling
     * (managing) this connection (owner).
     */
    struct Service_t *service;

    /**
     * "Flag" controlling if the write operations
     * are currently being monitored in the polling
     * (provider).
     * This flag shall be used carefully.
     */
    unsigned char writeRegistered;

    /**
     * Queue containing the set of connections with
     * data pending to be read.
     */
    struct LinkedList_t *readQueue;

    /**
     * Queue containing the set of connections which
     * ar ready for reading.
     */
    struct LinkedList_t *writeQueue;

    /**
     * Function to be used for opening a
     * connection.
     */
    connectionUpdate openConnection;

    /**
     * Function to be used for closing a
     * connection.
     */
    connectionUpdate closeConnection;

    /**
     * Function to be used to write into
     * a connection.
     */
    connectionWrite writeConnection;

    /**
     * Function to be used for registering
     * writing detection in the polling (provider).
     */
    connectionUpdate registerWrite;

    /**
     * Function to be used for uregistering
     * writing detection in the polling (provider).
     */
    connectionUpdate unregisterWrite;

    /**
     * Function to be used to allocated (in a
     * safe away) message data that may be
     * later released safely
     */
    connectionAlloc allocData;

    /**
     * Callback function reference to be called
     * when data is available for reading.
     */
    connectionCallback onRead;

    /**
     * Callback function reference to be called
     * when the connection is ready for writing.
     */
    connectionCallback onWrite;

    /**
     * Callback function reference to be called
     * when the connection is an erroneous state.
     */
    connectionCallback onError;

    /**
     * Callback function reference to be called
     * when the connection has just been opened.
     */
    connectionCallback onOpen;

    /**
     * Callback function reference to be called
     * when the connection is going to be closed.
     */
    connectionCallback onClose;

    /**
     * Reference to the lower level
     * connection substrate (child).
     */
    void *lower;
} Connection;

/**
 * Enumeration defining the various
 * status to be used in a workflow driven
 * object or structure.
 */
typedef enum Status_e {
    /**
     * Status where the object is open
     * or in a running "like" state.
     */
    STATUS_OPEN = 1,

    /**
     * Status where an object is closed
     * or in a stopped "like" state.
     */
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
    connectionDataCallback callback;
    void *callbackParameters;
} Data;

/**
 * Constructor of the service.
 *
 * @param servicePointer The pointer to the service to
 * be constructed.
 * @param name The name of the service to be constructed.
 */
void createService(struct Service_t **servicePointer, unsigned char *name);

/**
 * Destructor of the service.
 *
 * @param service The service to be destroyed.
 */
void deleteService(struct Service_t *service);

/**
 * Constructor of the service options.
 *
 * @param serviceOptionsPointer The pointer to the service options to
 * be constructed.
 */
void createServiceOptions(struct ServiceOptions_t **serviceOptionsPointer);

/**
 * Destructor of the service options.
 *
 * @param serviceOptions The service options to be destroyed.
 */
void deleteServiceOptions(struct ServiceOptions_t *serviceOptions);

/**
 * Constructor of the data.
 *
 * @param dataPointer The pointer to the data to be
 * constructed.
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
 * Destructor of the configuration map.
 *
 * @param configuration The configuration to be destroyed.
 * @param isTop If the current configuration object refers a
 * top level configuration (section) or an inner one.
 */
void deleteConfiguration(struct HashMap_t *configuration, int isTop);

/**
 * Loads the various options, from the various data sources
 * into the service internal structures.
 *
 * @param service The service to be loaded with options.
 * @param arguments The (command line) argument map to be used
 * during the options loading.
 * @return The resulting error code.
 */
ERROR_CODE loadOptionsService(struct Service_t *service, struct HashMap_t *arguments);

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

/**
 * Closes all the current active connection in the
 * given service.
 *
 * @param service The service to have the connections
 * stopped.
 * @return The resulting error code.
 */
ERROR_CODE closeConnectionsService(struct Service_t *service);

/**
 * Loads all the currently avaialble modules
 * into the given service context.
 *
 * @param service The service to load the modules.
 * @return The resulting error code.
 */
ERROR_CODE loadModulesService(struct Service_t *service);

/**
 * Unloads all the currently avaialble modules
 * from the given service context.
 *
 * @param service The service to unload the modules.
 * @return The resulting error code.
 */
ERROR_CODE unloadModulesService(struct Service_t *service);

/**
 * Adds a connection to the given service.
 * Adding a connection implies the changing
 * of the polling (provider).
 *
 * @param service The service to be used.
 * @param connection The connection to be added.
 * @return The resulting error code.
 */
ERROR_CODE addConnectionService(struct Service_t *service, struct Connection_t *connection);

/**
 * Removes a connection from the given service.
 * Removing a connection implies the changing
 * of the polling (provider).
 *
 * @param service The service to be used.
 * @param connection The connection to be removed.
 * @return The resulting error code.
 */
ERROR_CODE removeConnectionService(struct Service_t *service, struct Connection_t *connection);

/**
 * Constructor of the connection.
 *
 * @param servicePointer The pointer to the connetcion to
 * be constructed.
 * @param socketHandle The socket handle to be associated
 * with the connection.
 * @return The resulting error code.
 */
ERROR_CODE createConnection(struct Connection_t **connectionPointer, SOCKET_HANDLE socketHandle);

/**
 * Destructor of the connection.
 *
 * @param connection The connection to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE deleteConnection(struct Connection_t *connection);

/**
 * Writes the given data to the connection.
 * A callback may be provided and if provided
 * it will be called after the data has been successfully
 * sent through the connection
 *
 * @param connection The connection to be used to
 * send the data.
 * @param data The data to be send throught the connection.
 * @param size The size of the data to be sent.
 * @param callback The callback to be called after the
 * successful write of data.
 * @parm callbackParameters The parameters to be sent
 * back to the callback.
 * @return The resulting error code.
 */
ERROR_CODE writeConnection(struct Connection_t *connection, unsigned char *data, unsigned int size, connectionDataCallback callback, void *callbackParameters);

/**
 * Opens the given connection, creating (and starting) all
 * the required structures.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be open.
 * @return The resulting error code.
 */
ERROR_CODE openConnection(struct Connection_t *connection);

/**
 * Closes the given connection, destroying all of the
 * created structures.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be closed.
 * @return The resulting error code.
 */
ERROR_CODE closeConnection(struct Connection_t *connection);

/**
 * Registers the given connection for write events.
 * This registration is done according to the polling
 * provider specification.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be registered
 * for writing "events".
 * @return The resulting error code.
 */
ERROR_CODE registerWriteConnection(struct Connection_t *connection);

/**
 * Unregisters the given connection for write events.
 * This unregistration is done according to the polling
 * provider specification.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be unregistered
 * for writing "events".
 * @return The resulting error code.
 */
ERROR_CODE unregisterWriteConnection(struct Connection_t *connection);

/**
 * Allocates a chunk of memory for the context of the given
 * connection.
 * This chunk of memory is safe in the context of multiple
 * dynamic libraries.
 *
 * @param connection The connection context to be used
 * in the allocation.
 * @param size The size of the chunk to be allocated.
 * @return The pointer to the allocated (data) chunk.
 * @return The resulting error code.
 */
ERROR_CODE allocConnection(struct Connection_t *connection, size_t size, void **dataPointer);
ERROR_CODE createHttpHandlerService(struct Service_t *service, struct HttpHandler_t **httpHandlerPointer, unsigned char *name);
ERROR_CODE deleteHttpHandlerService(struct Service_t *service, struct HttpHandler_t *httpHandler);
ERROR_CODE addHttpHandlerService(struct Service_t *service, struct HttpHandler_t *httpHandler);
ERROR_CODE removeHttpHandlerService(struct Service_t *service, struct HttpHandler_t *httpHandler);
ERROR_CODE getHttpHandlerService(struct Service_t *service, struct HttpHandler_t **httpHandlerPointer, unsigned char *name);
ERROR_CODE _defaultOptionsService(struct Service_t *service, struct HashMap_t *arguments);
ERROR_CODE _fileOptionsService(struct Service_t *service, struct HashMap_t *arguments);
ERROR_CODE _comandLineOptionsService(struct Service_t *service, struct HashMap_t *arguments);
