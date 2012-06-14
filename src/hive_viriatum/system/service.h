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
struct data_t;
struct polling_t;
struct connection_t;
struct http_handler_t;

/**
 * The function used to create a new handler instance
 * with a name and for the service context.
 */
typedef ERROR_CODE (*service_http_handler_access) (struct service_t *, struct http_handler_t **, unsigned char *name);

/**
 * The function used to update an handler state or value.
 */
typedef ERROR_CODE (*service_http_handler_update) (struct service_t *, struct http_handler_t *);

/**
 * The "default" function used to update a state in the connection
 * for the given service context.
 */
typedef ERROR_CODE (*connection_update) (struct connection_t *);

/**
 * The function used to allocate data in the context
 * of a connection (for safe usage).
 */
typedef ERROR_CODE (*connection_alloc) (struct connection_t *, size_t size, void **data_pointer);

/**
 * The function to be used for callbacks associated with the
 * connection status updates.
 */
typedef ERROR_CODE (*connection_callback) (struct connection_t *);

/**
 * The function to be used for callbacks associated with the
 * connection data updates.
 */
typedef ERROR_CODE (*connection_data_callback) (struct connection_t *, struct data_t *, void *);

/**
 * Function used to write data into a connection, optional
 * parameters allow a callback uppon the end of writing.
 */
typedef ERROR_CODE (*connection_write) (struct connection_t *connection, unsigned char *, unsigned int, connection_data_callback, void *);

/**
 * The "default" function used to update a state in the polling
 * for the given service context.
 */
typedef ERROR_CODE (*polling_update) (struct polling_t *);

/**
 * The function used to update a state in the polling
 * using the given connection as reference.
 */
typedef ERROR_CODE (*polling_connection_update) (struct polling_t *, struct connection_t *);

/**
 * Structure used to describe a polling (provider)
 * with all the action functions and callbacks.
 */
typedef struct polling_t {
    /**
     * The reference to the "owning" service.
     * The "owning" service should be the
     * only using this polling (provider).
     */
    struct service_t *service;

    /**
     * Function called for opening the polling
     * (provider).
     * This function should initialize all the
     * polling context structures.
     */
    polling_update open;

    /**
     * Function called for closing the polling
     * (provider).
     * This function should destroy all the
     * polling context structures.
     */
    polling_update close;

    /**
     * Function used to register a connection
     * into the polling (provider).
     * The polling system structures should
     * be updated to allow the connection events
     * to be detected.
     */
    polling_connection_update register_connection;

    /**
     * Function used to unregister a connection
     * from the polling (provider).
     */
    polling_connection_update unregister_connection;

    /**
     * Function used to the start "listening"
     * to the write events in the connection.
     */
    polling_connection_update register_write;

    /**
     * Function used to the stop "listening"
     * to the write events in the connection.
     */
    polling_connection_update unregister_write;

    /**
     * Function used to poll the connections
     * checking if new "events" are available.
     * This function should not block for a long
     * time (exiting may be compromised).
     */
    polling_update poll;

    /**
     * Function to be used to process and call
     * the callbacks associated with the current
     * events.
     * This function may be called after a polling.
     */
    polling_update call;

    /**
     * Reference to the lower level
     * connection substrate (child).
     */
    void *lower;
} polling;

/**
 * Structure used to describe a service structure.
 * The main service socket is set and shold be
 * maintained for house-keeping purposes.
 * The list of connections should represent the
 * current set of active connections.
 */
typedef struct service_t {
    /**
     * The descriptive name of the
     * service.
     * For textual representation.
     */
    unsigned char *name;

    /**
     * The name (path) to the current program
     * (process) in execution for the service context.
     */
    unsigned char *program_name;

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
    struct service_options_t *options;

    /**
     * The map containing a set of configuration
     * loaded primarly from the configuration files.
     * This map is structured by domains and the
     * first level of keys represent these domains.
     */
    struct hash_map_t *configuration;

    /**
     * The socket handle to the service
     * connection.
     */
    SOCKET_HANDLE service_socket_handle;

    /**
     * The http handler currently in use.
     * Only one http (parser) handler can
     * be used at one given time.
     */
    struct http_handler_t *http_handler;

    /**
     * The reference to the polling (provider)
     * used by the service.
     */
    struct polling_t *polling;

    /**
     * The list of currenly available (active)
     * connections in the service.
     */
    struct linked_list_t *connections_list;

    /**
     * The list of modules available for the
     * service.
     * This list represents only the loaded
     * modules any unloaded or failed module
     * should not be present in this list.
     */
    struct linked_list_t *modules_list;

    /**
     * The map of http handlers available
     * for the service.
     * The map associates the name of the handler
     * with the handler instance.
     */
    struct hash_map_t *http_handlers_map;

    service_http_handler_access create_http_handler;
    service_http_handler_update delete_http_handler;
    service_http_handler_update add_http_handler;
    service_http_handler_update remove_http_handler;
    service_http_handler_access get_http_handler;
} service_;

/**
 * Structure defining the set of options/configuration
 * for an associated service.
 */
typedef struct service_options_t {
    /**
     * The "default" tcp port to bind the service
     * associated with these options.
     */
    unsigned short port;

    /**
     * The "default" tcp port to bind the service
     * associated with these options, this is the
     * string value corresponding to the integer.
     */
    unsigned char _port[128];

    /**
     * The string value structure to the port buffer
     * to be used, this value will be used for caching
     * length values.
     */
    struct string_t _port_string;

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
    unsigned char *handler_name;

    /**
     * If the current service is running in
     * the local version (local file system).
     */
    unsigned char local;

    /**
     * The number of worker processes to be
     * used for handling connection (should
     * approximate to the number of cpus).
     */
    unsigned char workers;

    /**
     * If the default index file should be used in case
     * the root path file is requested (index.html is
     * the default file to be served).
     */
    unsigned char default_index;

    /**
     * If the instance should use templates for error
     * messages providing a better look and feel at the
     * cost of more computer power (may create problems
     * for denial of service (dos) attacks).
     */
    unsigned char use_template;

    /**
     * The default virtual host to be used in any
     * non matched host request.
     */
    struct virtual_host_t *default_virtual_host;

    /**
     * The set of virtual hosts associated with the
     * current service.
     * The map key is the (virtual) host name to be
     * used, default host name is not contained in
     * the list for performance.
     */
    struct hash_map_t *virtual_hosts;
} service_options;

typedef struct virtual_host_t {
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
    struct rule_t *default_rule;

    /**
     * The list of rules to be used for matching
     * the request, for gathering the correct
     * handler.
     */
    struct linked_list_t *rules;
} virtual_host;

/**
 * Enumeration defining the various types
 * of possible rules for a given rule
 * aggregation.
 */
typedef enum rule_type_e {
    DIRECTORY_RULE = 1,
    REGEX_RULE
} rule_type;

/**
 * Enumeration defining the various types
 * of possible protocols, this is an open
 * enumeration and registration is required.
 */
typedef enum connection_protocol_e {
    UNDEFINED_PROTOCOL = 1,
    UNKNOWN_PROTOCOL,
    HTTP_PROTOCOL,
    HTTP_CLIENT_PROTOCOL,
    TORRENT_PROTOCOL,
} connection_protocol;

typedef struct rule_t {
    enum rule_type_e type;
    void *value;
} rule;

typedef struct rule_directory_t {
    char *path;
} rule_directory;

/**
 * Structure defining a connection
 * conceptually, including the read and write
 * queue.
 * The references to the socket handle is also
 * maintained.
 */
typedef struct connection_t {
    /**
     * The current status of the connection.
     * Used for connection control.
     */
    unsigned char status;

    /**
     * The type of the connection to
     */
    enum connection_protocol_e protocol;

    /**
     * The socket handle associated with
     * the connection.
     * Typically this represent a file
     * descriptor.
     */
    SOCKET_HANDLE socket_handle;

    /**
     * The address for the socket associated
     * with the connection.
     * This is a somehow complex structure and
     * access to it should be made carrefully.
     */
    SOCKET_ADDRESS socket_address;

    /**
     * The reference to the service controlling
     * (managing) this connection (owner).
     */
    struct service_t *service;

    /**
     * "Flag" controlling if the write operations
     * are currently being monitored in the polling
     * (provider).
     * This flag shall be used carefully.
     */
    unsigned char write_registered;

    /**
     * Queue containing the set of connections with
     * data pending to be read.
     */
    struct linked_list_t *read_queue;

    /**
     * Queue containing the set of connections which
     * ar ready for reading.
     */
    struct linked_list_t *write_queue;

    /**
     * Function to be used for opening a
     * connection.
     */
    connection_update open_connection;

    /**
     * Function to be used for closing a
     * connection.
     */
    connection_update close_connection;

    /**
     * Function to be used to write into
     * a connection.
     */
    connection_write write_connection;

    /**
     * Function to be used for registering
     * writing detection in the polling (provider).
     */
    connection_update register_write;

    /**
     * Function to be used for uregistering
     * writing detection in the polling (provider).
     */
    connection_update unregister_write;

    /**
     * Function to be used to allocated (in a
     * safe away) message data that may be
     * later released safely
     */
    connection_alloc alloc_data;

    /**
     * Callback function reference to be called
     * when data is available for reading.
     */
    connection_callback on_read;

    /**
     * Callback function reference to be called
     * when the connection is ready for writing.
     */
    connection_callback on_write;

    /**
     * Callback function reference to be called
     * when the connection is an erroneous state.
     */
    connection_callback on_error;

    /**
     * Callback function reference to be called
     * when the connection has just been opened.
     */
    connection_callback on_open;

    /**
     * Callback function reference to be called
     * when the connection is going to be closed.
     */
    connection_callback on_close;

    /**
     * Global wide parameters for interaction with
     * the lower layer of the connection.
     * This reference is aimed at configuration
     * only (do not use it otherwise).
     */
    void *parameters;

    /**
     * Reference to the lower level
     * connection substrate (child).
     */
    void *lower;
} connection;

/**
 * Enumeration defining the various
 * status to be used in a workflow driven
 * object or structure.
 */
typedef enum status_e {
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
} status;

/**
 * Structure defining a data element
 * to be used in the internal queues.
 * The queues are user normally for reading
 * and writing.
 */
typedef struct data_t {
    unsigned char *data;
    unsigned char *data_base;
    size_t size;
    connection_data_callback callback;
    void *callback_parameters;
} data_;

/**
 * Constructor of the service.
 *
 * @param service_pointer The pointer to the service to
 * be constructed.
 * @param name The name of the service to be constructed.
 * @param program_name The name (path) to the current program
 * (process) in execution for the service context.
 */
void create_service(struct service_t **service_pointer, unsigned char *name, unsigned char *program_name);

/**
 * Destructor of the service.
 *
 * @param service The service to be destroyed.
 */
void delete_service(struct service_t *service);

/**
 * Constructor of the service options.
 *
 * @param service_options_pointer The pointer to the service options to
 * be constructed.
 */
void create_service_options(struct service_options_t **service_options_pointer);

/**
 * Destructor of the service options.
 *
 * @param service_options The service options to be destroyed.
 */
void delete_service_options(struct service_options_t *service_options);

/**
 * Constructor of the data.
 *
 * @param data_pointer The pointer to the data to be
 * constructed.
 */
void create_data(struct data_t **data_pointer);

/**
 * Destructor of the data.
 *
 * @param data The data to be destroyed.
 */
void delete_data(struct data_t *data);

/**
 * Constructor of the polling.
 *
 * @param data_pointer The pointer to the polling to be constructed.
 */
void create_polling(struct polling_t **polling_pointer);

/**
 * Destructor of the polling.
 *
 * @param polling The polling to be destroyed.
 */
void delete_polling(struct polling_t *polling);

/**
 * Destructor of the configuration map.
 *
 * @param configuration The configuration to be destroyed.
 * @param is_top If the current configuration object refers a
 * top level configuration (section) or an inner one.
 */
void delete_configuration(struct hash_map_t *configuration, int is_top);

/**
 * Loads the various options, from the various data sources
 * into the service internal structures.
 *
 * @param service The service to be loaded with options.
 * @param arguments The (command line) argument map to be used
 * during the options loading.
 * @return The resulting error code.
 */
ERROR_CODE load_options_service(struct service_t *service, struct hash_map_t *arguments);

/**
 * Calculates the various options, these values correspond to the
 * various calculated attributes in the options.
 *
 * @param service The service to hasve the options calculated.
 * during the options loading.
 * @return The resulting error code.
 */
ERROR_CODE calculate_options_service(struct service_t *service);

/**
 * Starts the given service, initializing the
 * internal structures and the main loop.
 *
 * @param service The service to be initialized.
 * @return The resulting error code.
 */
ERROR_CODE start_service(struct service_t *service);

/**
 * Stops the given service, stopping and cleaning
 * all the internal structures.
 * This method stops the main loop.
 *
 * @param service The service to be stopped.
 * @return The resulting error code.
 */
ERROR_CODE stop_service(struct service_t *service);

/**
 * Closes all the current active connection in the
 * given service.
 *
 * @param service The service to have the connections
 * stopped.
 * @return The resulting error code.
 */
ERROR_CODE close_connections_service(struct service_t *service);

/**
 * Loads all the currently avaialble modules
 * into the given service context.
 *
 * @param service The service to load the modules.
 * @return The resulting error code.
 */
ERROR_CODE load_modules_service(struct service_t *service);

/**
 * Unloads all the currently avaialble modules
 * from the given service context.
 *
 * @param service The service to unload the modules.
 * @return The resulting error code.
 */
ERROR_CODE unload_modules_service(struct service_t *service);

/**
 * Adds a connection to the given service.
 * Adding a connection implies the changing
 * of the polling (provider).
 *
 * @param service The service to be used.
 * @param connection The connection to be added.
 * @return The resulting error code.
 */
ERROR_CODE add_connection_service(struct service_t *service, struct connection_t *connection);

/**
 * Removes a connection from the given service.
 * Removing a connection implies the changing
 * of the polling (provider).
 *
 * @param service The service to be used.
 * @param connection The connection to be removed.
 * @return The resulting error code.
 */
ERROR_CODE remove_connection_service(struct service_t *service, struct connection_t *connection);

/**
 * Constructor of the connection.
 *
 * @param service_pointer The pointer to the connetcion to
 * be constructed.
 * @param socket_handle The socket handle to be associated
 * with the connection.
 * @return The resulting error code.
 */
ERROR_CODE create_connection(struct connection_t **connection_pointer, SOCKET_HANDLE socket_handle);

/**
 * Destructor of the connection.
 *
 * @param connection The connection to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE delete_connection(struct connection_t *connection);

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
 * @parm callback_parameters The parameters to be sent
 * back to the callback.
 * @return The resulting error code.
 */
ERROR_CODE write_connection(struct connection_t *connection, unsigned char *data, unsigned int size, connection_data_callback callback, void *callback_parameters);

/**
 * Opens the given connection, creating (and starting) all
 * the required structures.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be open.
 * @return The resulting error code.
 */
ERROR_CODE open_connection(struct connection_t *connection);

/**
 * Closes the given connection, destroying all of the
 * created structures.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be closed.
 * @return The resulting error code.
 */
ERROR_CODE close_connection(struct connection_t *connection);

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
ERROR_CODE register_write_connection(struct connection_t *connection);

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
ERROR_CODE unregister_write_connection(struct connection_t *connection);

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
ERROR_CODE alloc_connection(struct connection_t *connection, size_t size, void **data_pointer);
ERROR_CODE create_http_handler_service(struct service_t *service, struct http_handler_t **http_handler_pointer, unsigned char *name);
ERROR_CODE delete_http_handler_service(struct service_t *service, struct http_handler_t *http_handler);
ERROR_CODE add_http_handler_service(struct service_t *service, struct http_handler_t *http_handler);
ERROR_CODE remove_http_handler_service(struct service_t *service, struct http_handler_t *http_handler);
ERROR_CODE get_http_handler_service(struct service_t *service, struct http_handler_t **http_handler_pointer, unsigned char *name);
ERROR_CODE _default_options_service(struct service_t *service, struct hash_map_t *arguments);
ERROR_CODE _file_options_service(struct service_t *service, struct hash_map_t *arguments);
ERROR_CODE _comand_line_options_service(struct service_t *service, struct hash_map_t *arguments);
