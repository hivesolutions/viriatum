/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../stream/stream.h"
#include "../module/module.h"
#include "../polling/polling.h"

/* forward references (avoids loop) */
struct data_t;
struct polling_t;
struct location_t;
struct connection_t;
struct virtual_host_t;
struct http_handler_t;

#ifdef VIRIATUM_SSL
#define CONNECTION_SEND(connection, buffer, length, flags) connection->ssl_handle == NULL ? \
    SOCKET_SEND(connection->socket_handle, buffer, length, flags) :\
    SSL_write(connection->ssl_handle, buffer, length)
#define CONNECTION_RECEIVE(connection, buffer, length, flags) connection->ssl_handle == NULL ? \
    SOCKET_RECEIVE(connection->socket_handle, buffer, length, flags) :\
    SSL_read(connection->ssl_handle, buffer, length)
#define CONNECTION_GET_ERROR_CODE(connection, error_code) connection->ssl_handle == NULL ? \
    SOCKET_GET_ERROR_CODE(error_code) :\
    SSL_get_error(connection->ssl_handle, error_code)
#else
#define CONNECTION_SEND(connection, buffer, length, flags) \
    SOCKET_SEND(connection->socket_handle, buffer, length, flags)
#define CONNECTION_RECEIVE(connection, buffer, length, flags) \
    SOCKET_RECEIVE(connection->socket_handle, buffer, length, flags)
#define CONNECTION_GET_ERROR_CODE(connection, error_code) SOCKET_GET_ERROR_CODE(error_code)
#endif

/**
 * Enumeration defining the various types
 * of service processes that may exist.
 */
typedef enum process_type_e {
    MASTER_PROCESS = 1,
    WORKER_PROCESS
} process_type;

/**
 * The function used to retrieve a string in english representing
 * the uptime of the service refered.
 */
typedef const char *(*service_uptime) (struct service_t *, size_t);

/**
 * The function used to resolve a provided file extension (dot
 * prefixed) into the appropriate mime type.
 */
typedef const char *(*service_mime) (struct service_t *, char *);

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
 * The function used to unregister state in the polling
 * using the given connection as reference. An extra
 * parameter is provided so that one can remove the
 * connection from the polling mechanism.
 */
typedef ERROR_CODE (*polling_connection_unregister) (struct polling_t *, struct connection_t *, unsigned char);

/**
 * Structure describing a location to be
 * used by the service in the resolution
 * process.
 */
typedef struct location_t {
    /**
     * The name as a single (soft) description
     * for the location, this is not required
     * to be set or even unique.
     */
    unsigned char *name;

    /**
     * The path describing the location, this
     * may be a regular expression, a wildcard
     * or even something else.
     */
    unsigned char *path;

    /**
     * The name of the handler associated with
     * this location, this is going to be used
     * by the dispatched for correct relocation.
     */
    unsigned char *handler;

    /**
     * The hash map containg the configuration
     * of the location, this value should contain
     * the complete set of attributes parsed.
     */
    struct sort_map_t *configuration;
} location;

typedef struct locations_t {
    struct location_t values[1024];
    size_t count;
} locations;

typedef struct virtual_host_t {
    /**
     * The descriptive name of the viratual
     * host reference.
     * For textual representation.
     */
    unsigned char *name;
} virtual_host;

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
    polling_connection_unregister unregister_connection;

    /**
     * Function used to the start "listening"
     * to the read events in the connection.
     */
    polling_connection_update register_read;

    /**
     * Function used to the stop "listening"
     * to the read events in the connection.
     */
    polling_connection_update unregister_read;

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
     * Reference to the function that is going to
     * set the provided connection as outstanding
     * mening that in the next event loop it will
     * be verified for read operations.
     */
    polling_connection_update add_outstanding;

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
     * The version number as a string
     * for the current service structure.
     */
    unsigned char *version;

    /**
     * The platform as a string in which
     * the current service is running.
     */
    unsigned char *platform;

    /**
     * The abstract compilation flags for
     * the current service.
     */
    unsigned char *flags;

    /**
     * The description (string) of the compiler
     * used to compile the service structures.
     */
    unsigned char *compiler;

    /**
     * The version (string) of the compiler
     * used to compile the service structures.
     */
    unsigned char *compiler_version;

    /**
     * The date (as a string) for the compilation
     * of the service internal structures.
     */
    unsigned char *compilation_date;

    /**
     * The time (as a string) for the compilation
     * of the service internal structures.
     */
    unsigned char *compilation_time;

    /**
     * The complete set of flags used by the compiler
     * on the compilation process.
     */
    unsigned char *compilation_flags;

    /**
     * The string that described the currently loaded
     * modules, this string is created from the modules
     * structure list decribed "inside" the current
     * service structure.
     */
    unsigned char modules[4096];

    /**
     * The string describing the current service
     * and its configuration, usefull for a quick
     * visual description of the service.
     */
    unsigned char description[1024];

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
     * The start the (as seconds since epoch)
     * for the current service.
     * This value may be sued to calculate the
     * service uptime.
     */
    unsigned long long start_time;

    /**
     * The type of process that hold the
     * reference to this service.
     */
    enum process_type_e process_type;

    /**
     * The set of locations currently loaded
     * into the service.
     */
    struct locations_t locations;

    /**
     * The buffer containing the various pid values
     * for the worker processes in case they exists.
     * This buffer is used to kill the workers on
     * service closing.
     */
    PID_TYPE worker_pids[VIRIATUM_MAX_WORKERS];

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
    struct sort_map_t *configuration;

    /**
     * The socket handle to the service
     * connection, used to accept connections.
     * This is the base socket handle for
     * the service for the ipv4 connection.
     */
    SOCKET_HANDLE service_socket_handle;

    /**
     * The socket handle to the service
     * connection, used to accept connections.
     * This is the "extra" socket handle for
     * the service for the ipv6 connection.
     */
    SOCKET_HANDLE service_socket6_handle;

    /**
     * The HTTP handler currently in use.
     * Only one HTTP (parser) handler can
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
     * The map of HTTP handlers available
     * for the service.
     * The map associates the name of the handler
     * with the handler instance.
     */
    struct hash_map_t *http_handlers_map;

    /**
     * Buffer used as static reference to the
     * value to be returned in the retrieal of
     * the uptime function.
     */
    char _uptime[128];

    base_void register_signals;
    service_uptime get_uptime;
    service_mime get_mime_type;
    service_http_handler_access create_http_handler;
    service_http_handler_update delete_http_handler;
    service_http_handler_update add_http_handler;
    service_http_handler_update remove_http_handler;
    service_http_handler_access get_http_handler;

#ifdef VIRIATUM_SSL
    /**
     * The handle to the ssl socket reference that is encapsulating
     * the encrypted access to the socket.
     */
    SSL *ssl_handle;

    /**
     * The ssl configuration context that is currenly being
     * used in the ssl socket handle.
     * This object is used in the construction of the handle.
     */
    SSL_CTX *ssl_context;
#endif
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
     * Flag that controls if the service is meant
     * to be run "inside" an ipv6 protocol connection.
     */
    unsigned char ip6;

    /**
     * The "default" address to bind the service
     * associated with these options.
     * This is the address to be used in the ipv6
     * based connection, in case it exists.
     */
    unsigned char *address6;

    /**
     * Flag that controls if the service is meant
     * to be run "inside" an encrypted ssl channel.
     */
    unsigned char ssl;

    /**
     * The path to the ssl certificate file to be used
     * in the loading of the secure connection for this
     * service.
     */
    unsigned char *ssl_csr;

    /**
     * The path to the ssl private key file to be used
     * in the loading of the secure connection for this
     * service.
     */
    unsigned char *ssl_key;

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
     * The string based buffer of file apths to the
     * index files to be used for root directory listing.
     * This value should be used as the default way to
     * discover the file to be used for directory listing.
     */
    unsigned char index[32][128];

    /**
     * The number of index objects currently available and
     * parsed from the line value.
     */
    size_t index_count;

    /**
     * The map (dictionary) containg the relations between
     * the file extension (dot prefixed) and the string based
     * mime type according to IANA.
     */
    struct hash_map_t *mime_types;

    /**
     * The set of virtual hosts associated with the
     * current service.
     * The map key is the (virtual) host name to be
     * used, default host name is not contained in
     * the list for performance.
     */
    struct hash_map_t *virtual_hosts;
} service_options;

/**
 * Enumration definit the various types of socket
 * families available for the connections.
 */
typedef enum connection_family_e {
    UNDEFINED_FAMILY = 1,
    IP_V4_FAMILY,
    IP_V6_FAMILY
} connection_family;

/**
 * Arrays with the text based descriptions for the
 * various family names of the connection types.
 */
static const char *connection_family_strings[3] = {
    "undefined",
    "ipv4",
    "ipv6"
};

/**
 * Enumeration defining the various types
 * of possible protocols, this is an open
 * enumeration and registration is required.
 */
typedef enum connection_protocol_e {
    UNDEFINED_PROTOCOL = 1,
    UNKNOWN_PROTOCOL,
    CUSTOM_PROTOCOL,
    HTTP_PROTOCOL,
    HTTP_CLIENT_PROTOCOL,
    TORRENT_PROTOCOL,
} connection_protocol;

/**
 * Sequence containing the various string based
 * representation of the various protocol types.
 * This values may be used for enumeration to
 * string resolution.
 */
static const char *connection_protocol_strings[6] = {
    "undefined",
    "unknown",
    "custom",
    "http",
    "http client",
    "torrent"
};

/**
 * Structure defining a connection
 * conceptually, including the read and write
 * queue.
 * The references to the socket handle is also
 * maintained.
 */
typedef struct connection_t {
    /**
     * Identifier value for the current connection
     * this value should be as most unique as possible
     * so that the connection is correclty identified.
     */
    unsigned long long id;

    /**
     * The current status of the connection.
     * Used for connection control.
     */
    unsigned char status;

    /**
     * The timestamp value for the time of the
     * creation of the connection. Note that at
     * the time of connection creation the socket
     * may not have been opened.
     */
    unsigned long long creation;

    /**
     * The ammount of bytes that have been sent by
     * this connection. Please keep in mind that the
     * data type for this value is limited by the
     * current architecture and so overflows must be
     * taken into account (not reliable).
     */
    size_t sent;

    /**
     * The ammount of bytes that have been received by
     * this connection. Please keep in mind that the
     * data type for this value is limited by the
     * current architecture and so overflows must be
     * taken into account (not reliable).
     */
    size_t received;

    /**
     * The socket/connection family for the current connection
     * should be an actual value (eg: ipv4, ipv6, etc).
     * This value may be latter translated into a string value
     * using the typical enumeration to array conversion.
     */
    enum connection_family_e family;

    /**
     * The type of the connection that is used as a label
     * and guides the connection underlying workflow.
     */
    enum connection_protocol_e protocol;

    /**
     * The socket handle associated with
     * the connection.
     * Typically this represents a file
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
     * The null terminated string containing the
     * best host description for the connection,
     * this value may be an ipv4 or ipv6 or even
     * an dns based hostname resolution.
     */
    unsigned char host[64];

    /**
     * The tcp based port of the other end of the
     * connection, this value is not guaranteed to
     * be populated at any time.
     */
    size_t port;

    /**
     * Flag that is set in case the current connection
     * is secured using any kind of encryption mechanism.
     * Typically this flag is set if ssl is used.
     */
    char is_secure;

    /**
     * The reference to the service controlling
     * (managing) this connection (owner).
     */
    struct service_t *service;

    /**
     * Value that controls if the read operation
     * are currently being monitored in the polling
     * (provider).
     * This flag shall be used carefully as it may
     * create unwanted behavior.
     */
    unsigned char read_registered;

    /**
     * "Flag" controlling if the write operations
     * are currently being monitored in the polling
     * (provider).
     * This flag shall be used carefully as it may
     * create unwanted behavior.
     */
    unsigned char write_registered;

    /**
     * Boolean flag value that controls if the connection
     * is currently valid (ready) for reading this should
     * be set to false if a would block event is raised on
     * a read operation, and then set back to valid if the
     * the underlying socket is ready for reading.
     */
    unsigned char read_valid;

    /**
     * Boolean flag value that controls if the connection
     * is currently valid (ready) for writing this should
     * be set to false if a would block event is raised on
     * a write operation, and then set back to valid if the
     * the underlying socket is ready for writing.
     */
    unsigned char write_valid;

    /**
     * Flag that controls if the current connection has
     * outstanding operations pending to be performed.
     * In case this flag is set the connection should be
     * pooled in the next loop for read operations.
     */
    unsigned char is_outstanding;

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
     * reading detection in the polling (provider).
     */
    connection_update register_read;

    /**
     * Function to be used for uregistering
     * reading detection in the polling (provider).
     */
    connection_update unregister_read;

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
     * Pointer to the function that invalidates the
     * current function for reading meaning that if
     * this function is called the connection needs
     * to be reset (level up) again for reading at
     * the polling level.
     */
    connection_update invalidate_read;

    /**
     * Pointer to the function that invalidates the
     * current function for writing meaning that if
     * this function is called the connection needs
     * to be reset (level up) again for writing at
     * the polling level.
     */
    connection_update invalidate_write;

    /**
     * Adds the provided connection as outstanding
     * meaning that it will be verified for read
     * operations in the next event loop (this will
     * create a defered reading).
     */
    connection_update add_outstanding;

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
     * when the connection in an handshake state.
     */
    connection_callback on_handshake;

    /**
     * Callback function reference to be called
     * when the connection in an erroneous state.
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

#ifdef VIRIATUM_SSL
    /**
     * The handle to the ssl socket reference that is encapsulating
     * the encrypted access to the socket.
     */
    SSL *ssl_handle;

    /**
     * The ssl configuration context that is currenly being
     * used in the ssl socket handle.
     * This object is used in the construction of the handle.
     */
    SSL_CTX *ssl_context;
#endif
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
    STATUS_CLOSED,

    /**
     * Status where an object is in the
     * handshake state, this is the typical
     * status for the initial part of a
     * secure encrypted connection.
     */
    STATUS_HANDSHAKE
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
    size_t size_base;
    char release;
    connection_data_callback callback;
    void *callback_parameters;
} data_;

/**
 * The strings that describe the various status
 * that are possible to exist for the description
 * of various items (eg: connection, service, etc.).
 */
static const char *status_strings[3] = {
    "open",
    "closed",
    "handshake"
};

#ifdef VIRIATUM_SSL
/**
 * The buffer containing the various possible
 * error codes for an ssl read/write operation.
 */
static const char *ssl_error_codes[9] = {
    "SSL_ERROR_NONE",
    "SSL_ERROR_SSL",
    "SSL_ERROR_WANT_READ",
    "SSL_ERROR_WANT_WRITE",
    "SSL_ERROR_WANT_X509_LOOKUP",
    "SSL_ERROR_SYSCALL",
    "SSL_ERROR_ZERO_RETURN",
    "SSL_ERROR_WANT_CONNECT",
    "SSL_ERROR_WANT_ACCEPT"
};
#endif

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
void delete_configuration(struct sort_map_t *configuration, int is_top);

/**
 * Loads the global wide service specifications into the
 * internal data structures that describe it.
 *
 * @param service The service to be loaded with specifications.
 * @return The resulting error code.
 */
ERROR_CODE load_specifications(struct service_t *service);

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
 * @param service The service to have the options calculated.
 * during the options loading.
 * @return The resulting error code.
 */
ERROR_CODE calculate_options_service(struct service_t *service);

/**
 * Calculates the various locations, these values correspond to the
 * various calculated attributes in the options.
 *
 * @param service The service to have the locations calculated.
 * during the options loading.
 * @return The resulting error code.
 */
ERROR_CODE calculate_locations_service(struct service_t *service);

/**
 * Prints a series of information value about the service that
 * has just been passed, the information should be sent to the
 * defined debug output.
 *
 * @param service The service to have it's internal status debugged
 * to the default output.
 * @return The resulting error code.
 */
ERROR_CODE print_options_service(struct service_t *service);

/**
 * Creates a series of worker processes that are going to
 * be used to handle new connections.
 *
 * These processes inherit all the memory footprint from
 * the master process, because they were crated with fork.
 *
 * @param service The service to be use as reference for
 * the forking (creation) of the various worker processes.
 * @return The resulting error code.
 */
ERROR_CODE create_workers(struct service_t *service);

/**
 * Joins the various worker processes associated with the
 * service (waiting for their exit).
 *
 * This is a blocking call and as such should be used with
 * care to avoid any stalling.
 *
 * @param service The service to be use as reference for
 * the joining of the various worker processes.
 * @return The resulting error code.
 */
ERROR_CODE join_workers(struct service_t *service);

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
 * Creates the modules label, updating it with the
 * latest state for the loaded modules.
 *
 * @param service The service to be used when creating
 * and updating the modules label.
 * @return The resulting error code.
 */
ERROR_CODE create_modules_label(struct service_t *service);

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
 * Writes the given data to the connection, the write operation
 * will only be performed in the next loop iteration.
 * The data buffer will be release after the write.
 * A callback may be provided and if it is provided
 * it will be called after the data has been successfully
 * sent through the connection.
 *
 * @param connection The connection to be used to
 * send the data.
 * @param data The data to be send throught the connection.
 * @param size The size of the data to be sent.
 * @param callback The callback to be called after the
 * successful write of data.
 * @param callback_parameters The parameters to be sent
 * back to the callback.
 * @return The resulting error code.
 */
ERROR_CODE write_connection(
    struct connection_t *connection,
    unsigned char *data,
    unsigned int size,
    connection_data_callback
    callback,
    void *callback_parameters
);

/**
 * Resolves the various resolution dependent fields in the connection
 * using the provided socket address as the base for such resolution.
 *
 * This may be an expensive operationn (at least for fast execution)
 * and any execution of this function should be done with care.
 *
 * @param connection The connection to be used in the resolution operation
 * that is going to be performed.
 * @param socket_address The socket address structure that is going to be
 * used to guide the resolution operation.
 * @return The resulting error code.
 */
ERROR_CODE resolve_connection(struct connection_t *connection, SOCKET_ADDRESS *socket_address);

/**
 * Writes the given data to the connection, the write operation
 * will only be performed in the next loop iteration.
 * The data buffer will be release after the write.
 * A callback may be provided and if it is provided
 * it will be called after the data has been successfully
 * sent through the connection.
 *
 * The release or not of the data buffer may be controlled using
 * the proper flag in the arguments.
 *
 * @param connection The connection to be used to
 * send the data.
 * @param data The data to be send throught the connection.
 * @param size The size of the data to be sent.
 * @param callback The callback to be called after the
 * successful write of data.
 * @param callback_parameters The parameters to be sent
 * back to the callback.
 * @param release If the data buffer should be released after
 * the write operation has been completed.
 * @return The resulting error code.
 */
ERROR_CODE write_connection_c(
    struct connection_t *connection,
    unsigned char *data,
    unsigned int size,
    connection_data_callback callback,
    void *callback_parameters,
    char release
);

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
 * Registers the given connection for read events.
 * This registration is done according to the polling
 * provider specification.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be registered
 * for reading "events".
 * @return The resulting error code.
 */
ERROR_CODE register_read_connection(struct connection_t *connection);

/**
 * Unregisters the given connection for read events.
 * This unregistration is done according to the polling
 * provider specification.
 * This method should be always called indirectly.
 *
 * @param connection The connection to be unregistered
 * for reading "events".
 * @return The resulting error code.
 */
ERROR_CODE unregister_read_connection(struct connection_t *connection);

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
 * Invalidates the connection for read operation meaning
 * than any other read operation should be first validated
 * by the polling mechanism.
 *
 * @param connection The connection to be invalidated
 * for reading "events".
 * @return The resulting error code.
 */
ERROR_CODE invalidate_read_connection(struct connection_t *connection);

/**
 * Invalidates the connection for write operation meaning
 * than any other write operation should be first validated
 * by the polling mechanism.
 *
 * @param connection The connection to be invalidated
 * for writing "events".
 * @return The resulting error code.
 */
ERROR_CODE invalidate_write_connection(struct connection_t *connection);

/**
 * Adds the current connection as outstanding meaning that
 * outstanding read operations are pending and should be
 * performed at the begining of the next loop.
 *
 * @param connection The connection to be set as having
 * outstanding (read) opertions.
 * @return The resulting error code.
 */
ERROR_CODE add_outstanding_connection(struct connection_t *connection);

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
const char *_get_uptime_service(struct service_t *service, size_t count);
const char *_get_mime_type_service(struct service_t *service, char *extension);

static __inline const char *get_family_string(enum connection_family_e family) {
    return connection_family_strings[family - 1];
}

static __inline const char *get_protocol_string(enum connection_protocol_e protocol) {
    return connection_protocol_strings[protocol - 1];
}

static __inline const char *get_status_string(enum status_e status) {
    return status_strings[status - 1];
}

#ifdef VIRIATUM_SSL
__inline static const char *_get_ssl_error_code(size_t index) {
    return ssl_error_codes[index];
}
#endif
