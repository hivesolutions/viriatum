/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "service.h"

static unsigned long long connection_id = 0;

void create_service(struct service_t **service_pointer, unsigned char *name, unsigned char *program_name) {
    /* retrieves the service size */
    size_t service_size = sizeof(struct service_t);

    /* allocates space for the service */
    struct service_t *service = (struct service_t *) MALLOC(service_size);

    /* sets the service attributes (default) values */
    service->name = name;
    service->program_name = program_name;
    service->status = STATUS_CLOSED;
    service->configuration = NULL;
    service->service_socket_handle = 0;
    service->service_socket6_handle = 0;
    service->http_handler = NULL;
    service->register_signals = NULL;
    service->get_uptime = _get_uptime_service;
    service->get_mime_type = _get_mime_type_service;
    service->create_http_handler = create_http_handler_service;
    service->delete_http_handler = delete_http_handler_service;
    service->add_http_handler = add_http_handler_service;
    service->remove_http_handler = remove_http_handler_service;
    service->get_http_handler = get_http_handler_service;
    service->locations.count = 0;

#ifdef VIRIATUM_SSL
    service->ssl_handle = NULL;
    service->ssl_context = NULL;
#endif

    /* creates the service options that are going to be
    used to store the various options loaded from the
    various sources (eg: command line, config file, etc.)*/
    create_service_options(&service->options);

    /* creates the various structures that are going to
    be used for the internal working of the service */
    create_polling(&service->polling);
    create_linked_list(&service->connections_list);
    create_linked_list(&service->modules_list);
    create_hash_map(&service->http_handlers_map, 0);

    /* sets the service in the service pointer */
    *service_pointer = service;
}

void delete_service(struct service_t *service) {
    /* in case the service socket handle is defined and set
    must close the it using the safe approach (gracefully) */
    if(service->service_socket_handle) {
        SOCKET_CLOSE(service->service_socket_handle);
    }

    /* in case the service configuration is defined and set
    must close the it using the safe approach (gracefully) */
    if(service->configuration) {
        delete_configuration(service->configuration, 1);
    }

    /* deletes the various internal structures associated
    with the service avoiding any memory leak */
    delete_hash_map(service->http_handlers_map);
    delete_linked_list(service->modules_list);
    delete_linked_list(service->connections_list);
    delete_polling(service->polling);
    delete_service_options(service->options);

    /* releases the service */
    FREE(service);
}

void create_service_options(struct service_options_t **service_options_pointer) {
    /* retrieves the service options size */
    size_t service_options_size = sizeof(struct service_options_t);

    /* allocates space for the service options */
    struct service_options_t *service_options = (struct service_options_t *) MALLOC(service_options_size);

    /* sets the service options attributes (default) values */
    service_options->port = 0;
    service_options->address = NULL;
    service_options->ip6 = 0;
    service_options->address6 = NULL;
    service_options->ssl = 0;
    service_options->ssl_csr = NULL;
    service_options->ssl_key = NULL;
    service_options->handler_name = NULL;
    service_options->local = 0;
    service_options->default_index = 0;
    service_options->use_template = 0;
    service_options->default_virtual_host = NULL;
    service_options->index_count = 0;

    /* resets the memry buffer on the index sequence structure so
    that no index is considered before configuration */
    memset(service_options->index, 0, sizeof(service_options->index));

    /* creates the hash map for both the mime types
    and the virtual hosts information */
    create_hash_map(&service_options->mime_types, 64);
    create_hash_map(&service_options->virtual_hosts, 0);

    /* sets the service options in the service options pointer */
    *service_options_pointer = service_options;
}

void delete_service_options(struct service_options_t *service_options) {
    /* deletes the internal structures used for the
    service (configuration) options */
    delete_hash_map(service_options->mime_types);
    delete_hash_map(service_options->virtual_hosts);

    /* releases the service options */
    FREE(service_options);
}

void create_data(struct data_t **data_pointer) {
    /* retrieves the data size */
    size_t data_size = sizeof(struct data_t);

    /* allocates space for the data */
    struct data_t *data = (struct data_t *) MALLOC(data_size);

    /* sets the data attributes (default) values */
    data->data = NULL;
    data->data_base = NULL;
    data->size = 0;
    data->size_base = 0;
    data->callback = NULL;
    data->callback_parameters = NULL;

    /* sets the data in the data pointer */
    *data_pointer = data;
}

void delete_data(struct data_t *data) {
    /* releases the data buffer in case the proper
    release flag is set (release required) */
    if(data->release) { FREE(data->data_base); }

    /* releases the data */
    FREE(data);
}

void create_polling(struct polling_t **polling_pointer) {
    /* retrieves the polling size */
    size_t polling_size = sizeof(struct polling_t);

    /* allocates space for the polling */
    struct polling_t *polling = (struct polling_t *) MALLOC(polling_size);

    /* sets the polling attributes (default) values */
    polling->service = NULL;
    polling->open = NULL;
    polling->close = NULL;
    polling->register_connection = NULL;
    polling->unregister_connection = NULL;
    polling->register_read = NULL;
    polling->unregister_read = NULL;
    polling->register_write = NULL;
    polling->unregister_write = NULL;
    polling->add_outstanding = NULL;
    polling->poll = NULL;
    polling->call = NULL;
    polling->lower = NULL;

    /* sets the data in the polling pointer */
    *polling_pointer = polling;
}

void delete_polling(struct polling_t *polling) {
    /* releases the polling */
    FREE(polling);
}

void delete_configuration(struct sort_map_t *configuration, int is_top) {
    /* allocates space for the pointer to the element and
    for the option to be retrieved */
    struct hash_map_element_t *element;
    void *option;

    /* allocates space for the iterator for the configuration */
    struct iterator_t *configuration_iterator;

    /* creates an iterator for the configuration sort map */
    create_element_iterator_sort_map(configuration, &configuration_iterator);

    /* iterates continuously */
    while(TRUE) {
        /* retrieves the next value from the configuration iterator */
        get_next_iterator(configuration_iterator, (void **) &element);

        /* in case the current module is null (end of iterator)
        must break the loop immediately */
        if(element == NULL) { break; }

        /* retrievs the sort map value for the key pointer */
        get_value_sort_map(configuration, element->key, element->key_string, (void **) &option);

        /* in case the current iteration is of type top must delete the
        inner configuration because this is "just" a section, otherwise
        releases the memory space occupied by the value */
        if(is_top) { delete_configuration((struct sort_map_t *) option, 0); }
        else { FREE(option); }
    }

    /* deletes the iterator for the configuration sort map */
    delete_iterator_sort_map(configuration, configuration_iterator);

    /* deletes the sort map that holds the configuration */
    delete_sort_map(configuration);
}

ERROR_CODE load_specifications(struct service_t *service) {
    /* sets the basic specification values from the global
    wide contsnt values */
    service->version = (unsigned char *) VIRIATUM_VERSION;
    service->platform = (unsigned char *) VIRIATUM_PLATFORM_COMPLETE;
    service->flags = (unsigned char *) VIRIATUM_FLAGS;
    service->compiler = (unsigned char *) VIRIATUM_COMPILER;
    service->compiler_version = (unsigned char *) VIRIATUM_COMPILER_VERSION_STRING;
    service->compilation_date = (unsigned char *) VIRIATUM_COMPILATION_DATE;
    service->compilation_time = (unsigned char *) VIRIATUM_COMPILATION_TIME;

    /* "compiles" the various specification values into the
    the description value present in the service */
    SPRINTF(
        (char *) service->description,
        sizeof(service->description),
        "%s/%s (%s) (%s)",
        service->name,
        service->version,
        service->platform,
        service->flags
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE load_options_service(struct service_t *service, struct hash_map_t *arguments) {
    /* loads the options from the various sources, note
    that the loading order is important because the last
    loading takes priority over the previous ones */
    _default_options_service(service, arguments);
    _file_options_service(service, arguments);
    _comand_line_options_service(service, arguments);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE calculate_options_service(struct service_t *service) {
    /* converts the service port value into a string value
    using a base ten encoding and then populates the correspoding
    string value to use prefetched values */
    SPRINTF((char *) service->options->_port, 128, "%d", service->options->port);
    string_populate(&service->options->_port_string, service->options->_port, 0, 1);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE calculate_locations_service(struct service_t *service) {
    struct iterator_t *iterator;
    struct sort_map_t *location;
    struct hash_map_element_t *element;

    struct location_t *_location;

    unsigned char *name;
    unsigned char *path;
    unsigned char *handler;
    int is_equal;

    struct sort_map_t *configuration = service->configuration;
    create_element_iterator_sort_map(configuration, &iterator);

    while(TRUE) {
        get_next_iterator(iterator, (void **) &element);
        if(element == NULL) { break; }

        is_equal = memcmp(element->key_string, "location:", sizeof("location:") - 1);
        if(is_equal != 0) { continue; }

        get_value_string_sort_map(configuration, element->key_string, (void **) &location);
        if(location == NULL) { continue; }

        get_value_string_sort_map(location, (unsigned char *) "path", (void **) &path);
        if(path == NULL) { continue; }

        get_value_string_sort_map(location, (unsigned char *) "handler", (void **) &handler);
        if(handler == NULL) { continue; }

        name = &element->key_string[sizeof("location:") - 1];

        _location = &service->locations.values[service->locations.count];
        _location->name = name;
        _location->path = path;
        _location->handler = handler;
        _location->configuration = location;
        service->locations.count++;
    }

    delete_iterator_sort_map(configuration, iterator);

    /* raises no error */
    RAISE_NO_ERROR;
}

#ifdef VIRIATUM_PREFORK
ERROR_CODE create_workers(struct service_t *service) {
    /* creates the variables to hold the current pid value and
    the variable to hold the number of fork occurring */
    unsigned int fork_count = 0;
    PID_TYPE pid = 0;

    /* retrives the number of worker to be created from the options
    of the provided service if this value is invalid returns immeditely */
    struct service_options_t *service_options = service->options;
    unsigned char worker_count = service_options->workers;
    if(worker_count == 0) { RAISE_NO_ERROR; }

    /* iterates continuously for the forking of the
    current process (worker creation) */
    while(TRUE) {
        /* in case the number of forks is the same
        as the worker count, breaks the loop no more
        forking remaining */
        if(fork_count == worker_count) { break; }

        /* forks off the parent process, this
        shoud create the child (worker) process */
        pid = fork();

        /* checks if the pid is invalid in case
        it's exits the parent process in error */
        if(pid < 0) { exit(EXIT_FAILURE); }

        /* in case the current pid is zero this is
        the child process must break to avoid recursive
        forking of the worker processes */
        if(pid == 0) { break; }

        /* sets the pid of the worker process as the
        current pid (save for latter kill) */
        service->worker_pids[fork_count] = pid;

        /* increments the fork count variable (one more
        iteration ran) */
        fork_count++;
    }

    /* checks if the current process is a worker (zero based
    pid value) or the master process and sets the service
    process type accortin to this value */
    if(pid == 0) { service->process_type = WORKER_PROCESS; }
    else { service->process_type = MASTER_PROCESS; }

    /* checks if the current process is a worker (zero based
    pid value) or the master process and sets the process title
    according to this value */
    if(pid == 0) { SET_PROC_NAME("viriatum/w"); }
    else { SET_PROC_NAME("viriatum/m"); }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE join_workers(struct service_t *service) {
    /* creates the variables to hold the current pid value and
    the variable to hold the number of joins occurring */
    unsigned int join_count = 0;
    PID_TYPE pid = 0;

    /* allocates space for the variable that will hold the
    status of the process aftwer the wait call */
    int status;

    /* retrives the number of worker to be "destroyed" from the options
    of the provided service if this value is invalid returns immeditely */
    struct service_options_t *service_options = service->options;
    unsigned char worker_count = service_options->workers;
    if(worker_count == 0) { RAISE_NO_ERROR; }

    /* in case the current process type for the service is not master
    no need to join (and kill) the workers, it's not the responsible */
    if(service->process_type != MASTER_PROCESS) { RAISE_NO_ERROR; }

    /* iterates continuously for the joining of the
    current process (worker creation) */
    while(TRUE) {
        /* in case the number of joins is the same
        as the worker count, breaks the loop no more
        joining remaining */
        if(join_count == worker_count) { break; }

        /* retrieves the pid of the worker to be "killed"
        and then joined */
        pid = service->worker_pids[join_count];
        kill(pid, SIGINT);

        /* wiats for the process to exit it's existence
        this may hang the current process in case there's
        a problem in the child process */
        waitpid(pid, &status, WUNTRACED);

        /* increments the join count variable (one more
        iteration ran) */
        join_count++;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}
#else
ERROR_CODE create_workers(struct service_t *service) { RAISE_NO_ERROR; }
ERROR_CODE join_workers(struct service_t *service) { RAISE_NO_ERROR; }
#endif

ERROR_CODE start_service(struct service_t *service) {
#ifdef VIRIATUM_IP6
    /* allocates the socket address structure for the ip6
    connection elements */
#ifdef VIRIATUM_PLATFORM_WIN32
    SOCKET_ADDRESS_INTERNET6 *socket6_address;
#endif
    SOCKET_ADDRESS_INTERNET6 _socket6_address;

#ifdef VIRIATUM_PLATFORM_UNIX
    /* allocates space for the temporary variable to be used
    to sttore the address to be "treated" (trimmed) */
    unsigned char address6[64];
    unsigned char *_address6;
#endif

    /* allocates space for the service socket handle and for
    the associated connection structure, these values must be
    initialized in order to avoid warnings */
    SOCKET_HANDLE service_socket6_handle = 0;
    struct connection_t *service6_connection = NULL;
#endif

#ifdef VIRIATUM_SSL
    /* alocates space for the various configuration file paths
    to be loaded for the start of the service, this is a local
    temporary variable to work with configuration paths */
    char config_path[VIRIATUM_MAX_PATH_SIZE];
    char *_config_path;
#endif

    /* allocates the socket address structure */
    SOCKET_ADDRESS_INTERNET socket_address;

    /* allocates space for the service socket handle and for
    the associated connection structure */
    SOCKET_HANDLE service_socket_handle;
    struct connection_t *service_connection;

    /* allocates the socket result that will be used
    to test the socket related call for error */
    SOCKET_ERROR_CODE socket_result;




    /* allocates the various misc connections references */
    struct connection_t *tracker_connection;
    struct connection_t *torrent_connection;




    /* allocates the polling (provider) */
    struct polling_t *polling;

    /* allocates the process */
    PROCESS_TYPE process;

    /* allocates the memory information */
    MEMORY_INFORMATION_TYPE memory_information;

    /* allocates the memory ussage */
    size_t memory_usage;

    /* allocates the option value and sets it to one (valid)
    not that a larger value is also created */
    SOCKET_OPTION option_value = 1;
    SOCKET_OPTION_LARGE option_value_l = 1;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* unpacks the service options from the service structure */
    struct service_options_t *service_options = service->options;

    /* sets the service status as open, this should mark the
    service loop as iterable (breaks on stop) */
    service->status = STATUS_OPEN;

    /* saves the current time so that it's possible to calculate
    the uptime for the current service */
    service->start_time = (unsigned long long) time(NULL);

    /* registers the various "local" handlers
    in the service, for later usage */
    register_handler_dispatch(service);
    register_handler_default(service);
    register_handler_file(service);
    register_handler_proxy(service);

    /* loads (all) the currently available modules, this operation may
    affect a series of internal structures including signal handlers */
    load_modules_service(service);

    /* registers the signal handler for the service, this must
    be done at this stage so that no module creates problems and
    overrides the default signal handlers */
    CALL_F(service->register_signals);

    /* sets the current http handler according to the current options
    in the service, the http handler must be loaded in the handlers map */
    get_value_string_hash_map(
        service->http_handlers_map,
        service_options->handler_name,
        (void **) &service->http_handler
    );

    /* sets the socket address attributes in the socket address
    structure so that the connection is properly defined */
    socket_address.sin_family = SOCKET_INTERNET_TYPE;
    socket_address.sin_addr.s_addr = inet_addr((char *) service_options->address);
    socket_address.sin_port = htons(service_options->port);

    /* creates the service socket for the given types */
    service->service_socket_handle = SOCKET_CREATE(
        SOCKET_INTERNET_TYPE,
        SOCKET_PACKET_TYPE,
        SOCKET_PROTOCOL_TCP
    );

    /* in case there was an error creating the service socket */
    if(SOCKET_TEST_ERROR(service->service_socket_handle)) {
        SOCKET_ERROR_CODE creating_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_ERROR_F("Problem creating socket: %d\n", creating_error_code);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem creating socket"
        );
    }

    /* in case viriatum is set to non blocking, changes the current
    socket behavior to non blocking mode then sets the socket to the
    non push mode in case it's required by configuration this implies
    also checking for the no (tcp) wait variable */
    if(VIRIATUM_NON_BLOCKING) { SOCKET_SET_NON_BLOCKING(service->service_socket_handle, flags); }
    if(VIRIATUM_NO_WAIT) { SOCKET_SET_NO_WAIT(service->service_socket_handle, option_value); }
    if(VIRIATUM_NO_PUSH) { SOCKET_SET_NO_PUSH(service->service_socket_handle, option_value); }

    /* sets the socket reuse address option in the socket, this should
    be done by first setting the option value to the original set value */
    option_value = 1;
    socket_result = SOCKET_SET_OPTIONS(
        service->service_socket_handle,
        SOCKET_OPTIONS_LEVEL_SOCKET,
        SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET,
        option_value
    );

    /* in case there was an error settings the socket option
    must handle it prompting the error information */
    if(SOCKET_TEST_ERROR(socket_result)) {
        SOCKET_ERROR_CODE option_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_ERROR_F("Problem setting socket option: %d\n", option_error_code);
        SOCKET_CLOSE(service->service_socket_handle);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem setting socket option"
        );
    }

#ifdef VIRIATUM_IP6
    /* in case the ip6 connection flag is set the ip6 connection
    must be correctly initialized (structure initialized) */
    if(service_options->ip6) {
        /* resets the ip6 socket address into a zero occupied buffer
        for the range of the socket address */
        memset(&_socket6_address, 0, sizeof(_socket6_address));

#ifdef VIRIATUM_PLATFORM_WIN32
        /* populates the socket ip6 address with the values provided
        from the configuration of the service */
        _socket6_address.ai_family = SOCKET_INTERNET6_TYPE;
        _socket6_address.ai_socktype = SOCKET_PACKET_TYPE;
        _socket6_address.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
        socket_result = getaddrinfo(
            service_options->address6,
            service_options->_port,
            &_socket6_address,
            &socket6_address
        );

        /* in case there was an error retrieving the address information
        must be correctly displayed */
        if(SOCKET_TEST_ERROR(socket_result)) {
            SOCKET_ERROR_CODE error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem getting ip6 address information: %d\n", error_code);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem getting ip6 address information"
            );
        }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
        /* copies the ip6 address to a temporary buffer and then
        uses the buffer to trim the string remogin the first and
        last chracters from it (ip6 normal representation) */
        STRCPY((char *) address6, 64, (char *) service_options->address6);
        _address6 = trim(address6);

        /* populates the socket ip6 address with the values provided
        from the configuration of the service */
        _socket6_address.sin6_family = SOCKET_INTERNET6_TYPE;
        _socket6_address.sin6_addr = in6addr_any;
        _socket6_address.sin6_port = htons(service_options->port);
        socket_result = inet_pton(
            SOCKET_INTERNET6_TYPE,
            (const char *) _address6,
            (void *) &_socket6_address.sin6_addr
        );

        /* in case there was an error retrieving the address information
        must be correctly displayed */
        if(SOCKET_EX_TEST_ERROR(socket_result)) {
            SOCKET_ERROR_CODE error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem getting ip6 address information: %d\n", error_code);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem getting ip6 address information"
            );
        }
#endif

        /* creates the service socket for the given types */
        service->service_socket6_handle = SOCKET_CREATE(
            SOCKET_INTERNET6_TYPE,
            SOCKET_PACKET_TYPE,
            SOCKET_PROTOCOL_TCP
        );

        /* in case there was an error creating the service socket */
        if(SOCKET_TEST_ERROR(service->service_socket6_handle)) {
            SOCKET_ERROR_CODE creating_error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem creating ip6 socket: %d\n", creating_error_code);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem creating ip6 socket"
            );
        }

        /* in case viriatum is set to non blocking, changes the current
        socket behavior to non blocking mode then sets the socket to the
        non push mode in case it's required by configuration this implies
        also checking for the no (tcp) wait variable */
        if(VIRIATUM_NON_BLOCKING) { SOCKET_SET_NON_BLOCKING(service->service_socket6_handle, flags); }
        if(VIRIATUM_NO_WAIT) { SOCKET_SET_NO_WAIT(service->service_socket6_handle, option_value); }
        if(VIRIATUM_NO_PUSH) { SOCKET_SET_NO_PUSH(service->service_socket6_handle, option_value); }

        /* sets the socket ip6 only address option in the socket, this should
        be done by first setting the option value to the original set value */
        option_value_l = 1;
        socket_result = SOCKET_SET_OPTIONS(
            service->service_socket6_handle,
            SOCKET_OPTIONS_IP6_SOCKET,
            SOCKET_OPTIONS_IP6_ONLY_SOCKET,
            option_value_l
        );

        /* in case there was an error settings the socket option
        must handle it prompting the error information */
        if(SOCKET_TEST_ERROR(socket_result)) {
#ifdef VIRIATUM_PLATFORM_WIN32
            SOCKET_GET_ERROR_CODE(socket_result);
#else
            SOCKET_ERROR_CODE option_error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem setting ip6 socket option: %d\n", option_error_code);
            SOCKET_CLOSE(service->service_socket6_handle);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem setting ip6 socket option"
            );
#endif
        }

        /* sets the socket reuse address option in the socket, this should
        be done by first setting the option value to the original set value */
        option_value = 1;
        socket_result = SOCKET_SET_OPTIONS(
            service->service_socket6_handle,
            SOCKET_OPTIONS_LEVEL_SOCKET,
            SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET,
            option_value
        );

        /* in case there was an error settings the socket option
        must handle it prompting the error information */
        if(SOCKET_TEST_ERROR(socket_result)) {
            SOCKET_ERROR_CODE option_error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem setting ip6 socket option: %d\n", option_error_code);
            SOCKET_CLOSE(service->service_socket6_handle);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem setting ip6 socket option"
            );
        }
    }
#endif

#ifdef VIRIATUM_SSL
    /* in case the ssl encryption flag is set the ssl sub system
    should be loaded */
    if(service_options->ssl) {
        /* loads the ssl error strings and starts the ssl library, this
        should be enought to get the ssl sub system running */
        SSL_load_error_strings();
        SSL_library_init();

        /* creates the new ssl context and updates the context with the
        correct certificate file and (private) key file */
        service->ssl_context = SSL_CTX_new(TLSv1_server_method());
        SSL_CTX_set_options(service->ssl_context, SSL_OP_SINGLE_DH_USE);

        /* resolves the configuration file from the ssl certificate defaulting to
        the predefined "server" certificate file */
        _config_path = VIRIATUM_RESOLVE_PATH(
            (char *) service_options->ssl_csr,
            "cert/server.crt",
            config_path
        );

        /* loads the certificate file into the ssl context so that the
        context is correctly validated */
        socket_result = SSL_CTX_use_certificate_file(service->ssl_context, _config_path, SSL_FILETYPE_PEM);

        /* tests if there was an error loading the certificate file and
        if it did closes the socket and returns in error (major problem) */
        if(SOCKET_EX_TEST_ERROR(socket_result)) {
            SOCKET_CLOSE(service->service_socket_handle);
            V_ERROR("Problem loading the ssl certificate file (*.crt)\n");
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem loading ssl certificate"
            );
        }

        /* resolves the configuration file from the ssl key defaulting to
        the predefined "server" key file */
        _config_path = VIRIATUM_RESOLVE_PATH(
            (char *) service_options->ssl_key,
            "cert/server.key",
            config_path
        );

        /* loads the private key file into the ssl context so that the
        context is correctly validated */
        socket_result = SSL_CTX_use_PrivateKey_file(service->ssl_context, _config_path, SSL_FILETYPE_PEM);

        /* tests if there was an error loading private key file and
        if it did closes the socket and returns in error (major problem) */
        if(SOCKET_EX_TEST_ERROR(socket_result)) {
            SOCKET_CLOSE(service->service_socket_handle);
            V_ERROR("Problem loading the ssl key file (*.key)\n");
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem loading ssl key"
            );
        }
    }
#endif

    /* binds the service socket */
    socket_result = SOCKET_BIND(service->service_socket_handle, socket_address);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(socket_result)) {
        /* retrieves the binding error code and
        prints a description about it in log */
        SOCKET_ERROR_CODE binding_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_ERROR_F("Problem binding socket: %d\n", binding_error_code);

        /* closes the service socket and raises an error indicating
        the problem binding the socket */
        SOCKET_CLOSE(service->service_socket_handle);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem binding socket"
        );
    }

    /* listens for a service socket change */
    socket_result = SOCKET_LISTEN(service->service_socket_handle);

    /* in case there was an error listening the socket */
    if(SOCKET_TEST_ERROR(socket_result)) {
        /* retrieves the listening error code and
        prints a description about it in log */
        SOCKET_ERROR_CODE binding_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_ERROR_F("Problem listening socket: %d\n", binding_error_code);

        /* closes the service socket and raises an error indicating
        the problem listening the socket */
        SOCKET_CLOSE(service->service_socket_handle);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem listening socket"
        );
    }

#ifdef VIRIATUM_IP6
    /* in case the ip6 connection flag is set the ip6 connection
    must be bound to the current context (bind operation) */
    if(service_options->ip6) {
#ifdef VIRIATUM_PLATFORM_WIN32
        /* binds the ip6 service socket */
        socket_result = SOCKET_BIND_EX(
            service->service_socket6_handle,
            socket6_address->ai_addr,
            socket6_address->ai_addrlen
        );
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
        /* binds the ip6 service socket */
        socket_result = SOCKET_BIND_EX(
            service->service_socket6_handle,
            _socket6_address,
            sizeof(_socket6_address)
        );
#endif

        /* in case there was an error binding the socket */
        if(SOCKET_TEST_ERROR(socket_result)) {
            /* retrieves the listening error code and
            prints a description about it in log */
            SOCKET_ERROR_CODE binding_error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem binding ip6 socket: %d\n", binding_error_code);

            /* closes the service socket and raises an error indicating
            the problem listening the socket */
            SOCKET_CLOSE(service->service_socket_handle);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem binding socket"
            );
        }

        /* listens for a service socket change */
        socket_result = SOCKET_LISTEN(service->service_socket6_handle);

        /* in case there was an error listening the socket */
        if(SOCKET_TEST_ERROR(socket_result)) {
            /* retrieves the listening error code and
            prints a description about it in log */
            SOCKET_ERROR_CODE binding_error_code = SOCKET_GET_ERROR_CODE(socket_result);
            V_ERROR_F("Problem listening ip6 socket: %d\n", binding_error_code);

            /* closes the service socket and raises an error indicating
            the problem listening the socket */
            SOCKET_CLOSE(service->service_socket_handle);
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem listening socket"
            );
        }
    }
#endif

    /* retrieves the polling service currently in use
    for the service */
    polling = service->polling;

    /* retrieves the required elements (eg: the handle
    to the (normal) socket) */
    service_socket_handle = service->service_socket_handle;

#ifdef VIRIATUM_IP6
    /* in case the ip6 connection flag is set the ip6 connection
    must be retrieved into the temporary variable */
    if(service_options->ip6) {
        /* retrieves the ip6 required elements (eg: the handle
        to the ip6 socket) */
        service_socket6_handle = service->service_socket6_handle;
    }
#endif

    /* in case the current os is compatible with the forking of process
    creates the worker processes to handle more connections at a time,
    this operation creates a much more flexible and scalable solution */
    create_workers(service);

    /* sets the service in the polling */
    polling->service = service;

#ifdef VIRIATUM_EPOLL
    /* sets the various handler values to the polling process
    they will be called for the various read, write operations */
    polling->open = open_polling_epoll;
    polling->close = close_polling_epoll;
    polling->register_connection = register_connection_polling_epoll;
    polling->unregister_connection = unregister_connection_polling_epoll;
    polling->register_read = register_read_polling_epoll;
    polling->unregister_read = unregister_read_polling_epoll;
    polling->register_write = register_write_polling_epoll;
    polling->unregister_write = unregister_write_polling_epoll;
    polling->add_outstanding = add_outstanding_polling_epoll;
    polling->poll = poll_polling_epoll;
    polling->call = call_polling_epoll;
#else
    /* sets the various handler values to the polling process
    they will be called for the various read, write operations */
    polling->open = open_polling_select;
    polling->close = close_polling_select;
    polling->register_connection = register_connection_polling_select;
    polling->unregister_connection = unregister_connection_polling_select;
    polling->register_read = register_read_polling_select;
    polling->unregister_read = unregister_read_polling_select;
    polling->register_write = register_write_polling_select;
    polling->unregister_write = unregister_write_polling_select;
    polling->add_outstanding = add_outstanding_polling_select;
    polling->poll = poll_polling_select;
    polling->call = call_polling_select;
#endif

    /* opens the polling (provider) */
    polling->open(polling);

    /* creates the (service) connection and then resolves it so
    that all of is "resolved" elements are correctly populated */
    create_connection(&service_connection, service_socket_handle);
    resolve_connection(service_connection, *(SOCKET_ADDRESS *) &socket_address);

#ifdef VIRIATUM_IP6
    /* in case the ip6 connection flag is set the ip6 connection
    must be correctly created */
    if(service_options->ip6) {
        /* creates the (service) connection for the ip6 based
        connection (for upper compatibility) */
        create_connection(&service6_connection, service_socket6_handle);
        resolve_connection(service6_connection, *(SOCKET_ADDRESS *) &_socket6_address);
    }
#endif

#ifdef VIRIATUM_SSL
    /* in case the ssl encryption flag is set the ssl sub system
    should be set in the service connection */
    if(service_options->ssl) {
        /* updates the ssl context and handle in the service connection
        so that it's possible to access the ssl connection */
        service_connection->ssl_context = service->ssl_context;
        service_connection->ssl_handle = service->ssl_handle;

#ifdef VIRIATUM_IP6
        /* in case the ip6 connection flag is set the ssl sub system
        should be set in the service ip6 connection */
        if(service_options->ip6) {
            /* updates the ssl context and handle in the service connection
            so that it's possible to access the ssl connection */
            service6_connection->ssl_context = service->ssl_context;
            service6_connection->ssl_handle = service->ssl_handle;
        }
#endif
    }
#endif

    /* sets the service select service as the service in the service connection */
    service_connection->service = service;

    /* sets the base hanlding functions in the service connection */
    service_connection->open_connection = open_connection;
    service_connection->close_connection = close_connection;
    service_connection->write_connection = write_connection;
    service_connection->register_read = register_read_connection;
    service_connection->unregister_read = unregister_read_connection;
    service_connection->register_write = register_write_connection;
    service_connection->unregister_write = unregister_write_connection;
    service_connection->invalidate_read = invalidate_read_connection;
    service_connection->invalidate_write = invalidate_write_connection;
    service_connection->add_outstanding = add_outstanding_connection;

    /* sets the fucntion to be called uppon read on the service
    connection (it should be the accept handler stream io, default) */
    service_connection->on_read = accept_handler_stream_io;

    /* opens the (service) connection, this operation should trigger
    the normal event handling operations and then add this connection
    to the list of connections that are part of the service */
    service_connection->open_connection(service_connection);

#ifdef VIRIATUM_IP6
    /* in case the ip6 connection flag is set the ip6 connection
    must be correctly "opened" */
    if(service_options->ip6) {
        /* sets the service select service as the service in the service connection */
        service6_connection->service = service;

        /* sets the base hanlding functions in the service connection */
        service6_connection->open_connection = open_connection;
        service6_connection->close_connection = close_connection;
        service6_connection->write_connection = write_connection;
        service6_connection->register_read = register_read_connection;
        service6_connection->unregister_read = unregister_read_connection;
        service6_connection->register_write = register_write_connection;
        service6_connection->unregister_write = unregister_write_connection;
        service6_connection->invalidate_read = invalidate_read_connection;
        service6_connection->invalidate_write = invalidate_write_connection;
        service6_connection->add_outstanding = add_outstanding_connection;

        /* sets the fucntion to be called uppon read on the service
        connection (it should be the accept handler stream io, default) */
        service6_connection->on_read = accept_handler_stream_io;

        /* opens the (service) connection */
        service6_connection->open_connection(service6_connection);
    }
#endif


    /*
    _create_tracker_connection(
        &tracker_connection,
        service,
        "http://hole1.hive:9090/ptorrent/announce.php",
        "C:/Users/joamag/Downloads/scudum.iso.torrent"
    );
*/

    /*
    _create_torrent_connection(
        &torrent_connection,
        service,
        "localhost",
        32967
    );*/


    /* iterates continuously, while the service is open (this
    is the main loop triggering all the actions) */
    while(service->status == STATUS_OPEN) {
        /* retrieves the (current) process, to be used
        to retrieves some memory information, and then closes it*/
        process = GET_PROCESS();
        GET_MEMORY_INFORMATION(process, memory_information);
        memory_usage = GET_MEMORY_USAGE(memory_information);
        CLOSE_PROCESS(process);

        /* prints a debug message about the current memory
        usage, usefull for extreme debugging */
        V_DEBUG_F(
            "Memory status: [%ld objects] [%ld KBytes]\n",
            (long int) ALLOCATIONS,
            (long int) memory_usage / 1024
        );

        /* polls the connections using the polling (provider)
        and calls the callbacks for the connection (events)
        using the polling (provider) */
        polling->poll(polling);
        polling->call(polling);
    }

    /* closes (all) the service connections and then
    closes the polling (provider of data) */
    close_connections_service(service);
    polling->close(polling);

    /* unloads the modules for the service */
    unload_modules_service(service);

    /* unregisters the various "local" handlers
    from the service, for structure destruction */
    unregister_handler_proxy(service);
    unregister_handler_file(service);
    unregister_handler_default(service);
    unregister_handler_dispatch(service);

#ifdef VIRIATUM_SSL
    /* in case the ssl encryption flag is set the ssl sub system
    should be unloaded (to avoid memmory leaks) */
    if(service_options->ssl) {
        /* releases the ssl error string and runs the final event process
        clenaup structure */
        ERR_free_strings();
        EVP_cleanup();
    }
#endif

    /* in case the current os supports the forking of the processes
    must join them back together waiting for their finish before exiting
    the main (coordinator) process */
    join_workers(service);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_service(struct service_t *service) {
    /* sets the service status as closed, this should
    trigger the unloading of the service in the next
    poll iteration to be executed */
    service->status = STATUS_CLOSED;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_connections_service(struct service_t *service) {
    /* allocates space for the current connection */
    struct connection_t *current_connection;

    /* retrieves the service connections list */
    struct linked_list_t *connections_list = service->connections_list;

    /* prints a debug message */
    V_DEBUG("Closing the service connections\n");

    /* iterates continuously in order to iterate over all the
    connection and delete them */
    while(TRUE) {
        /* pops a value from the connections list in order to be
        able to close and delete it, in case the retrieved value
        is invalid this is the end of iteration (must tbreak loop) */
        pop_value_linked_list(
            connections_list,
            (void **) &current_connection,
            TRUE
        );
        if(current_connection == NULL) { break; }

        /* closes the current connection (gracefully), the connection
        remains in memory (no delete) as the delete operation should
        be handled by the polling mechanism in the close operation */
        current_connection->close_connection(current_connection);
    }

#ifdef VIRIATUM_SSL
    /* closes the ssl context for the current serice, this should disable
    all the metadata access to the ssl */
    if(service->ssl_context != NULL) {
        SSL_CTX_free(service->ssl_context);
        service->ssl_context = NULL;
    }
#endif

    /* prints a debug message */
    V_DEBUG("Finished closing the service connections\n");

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE load_modules_service(struct service_t *service) {
    /* allocates the error code */
    ERROR_CODE error_code;

    /* allocates space for the pointer to be used in the
    prefix comparision for the module name */
    char *pointer;

    /* allocates space for the linked list for the entries
    and for the iterator to iterate "around" them */
    struct linked_list_t *entries;
    struct iterator_t *entries_iterator;

    /* allocates space for an entry of the directory */
    struct file_t *entry;

    /* allocates space for the path to be used to load the module */
    unsigned char module_path[VIRIATUM_MAX_PATH_SIZE];

    /* prints a debug message */
    V_DEBUG_F("Loading modules (%s)\n", VIRIATUM_MODULES_PATH);

    /* creates the linked list for the entries and populates
    it with the entries from the modules path */
    create_linked_list(&entries);
    list_directory_file(VIRIATUM_MODULES_PATH, entries);

    /* creates the iterator for the entries */
    create_iterator_linked_list(entries, &entries_iterator);

    /* iterates continuously over all the module entries */
    while(TRUE) {
        /* retrieves the next value from the iterator */
        get_next_iterator(entries_iterator, (void **) &entry);

        /* in case the current module is null (end of iterator)
        must break the current loop */
        if(entry == NULL) {  break; }

        /* in case the entry name does not ends with the shared object extension
        it must not be a module to be loaded, skips the current iteration */
        if(ends_with_string(entry->name, (unsigned char *) VIRIATUM_SHARED_OBJECT_EXTENSION) == 0) {
            continue;
        }

        /* creates the complete module path for the loading of it (this
        should be the path to the module file) */
        SPRINTF(
            (char *) module_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s/%s",
            VIRIATUM_MODULES_PATH,
            entry->name
        );

        /* tries to find the module prefix in the current entry name
        in case it's not found continues the loop immediately no library
        loading is required (not the correct format) */
        pointer = strstr((char *) entry->name, "viriatum_");
        if(pointer != (char *) entry->name && pointer != (char *) entry->name + 3) { continue; }

        /* loads the module, retrieving a possible error code in case
        there was a problem in the middle of the module load */
        error_code = load_module(service, module_path);

        /* tests the error code for error, and in case there's an
        error prints a warning message */
        if(IS_ERROR_CODE(error_code)) {
            V_WARNING_F("Problem loading module (%s)\n", (char *) GET_ERROR());
        }
    }

    /* deletes the iterator used for the entries */
    delete_iterator_linked_list(entries, entries_iterator);

    /* deletes both the entries internal structures
    and the entries linked list */
    delete_directory_entries_file(entries);
    delete_linked_list(entries);

    /* re-creates the new modules label for the service
    this should be able to update the names as a buffer */
    create_modules_label(service);

    /* prints a debug message */
    V_DEBUG("Finished loading modules\n");

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unload_modules_service(struct service_t *service) {
    /* allocates the current module */
    struct module_t *current_module;

    /* allocates space for the modules list iterator */
    struct iterator_t *modules_list_iterator;

    /* creates the iterator for the linked list */
    create_iterator_linked_list(service->modules_list, &modules_list_iterator);

    /* iterates continuously */
    while(TRUE) {
        /* retrieves the next value from the iterator and
        in case the current module is null (end of iterator)
        must break the current loop */
        get_next_iterator(modules_list_iterator, (void **) &current_module);
        if(current_module == NULL) { break; }

        /* unloads the current module in iteration */
        unload_module(service, current_module);
    }

    /* deletes the iterator linked list */
    delete_iterator_linked_list(service->modules_list, modules_list_iterator);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_modules_label(struct service_t *service) {
    /* allocates space for the various structures to
    be used for the creation of the label */
    size_t name_size;
    struct module_t *current_module;
    struct iterator_t *modules_list_iterator;
    unsigned char is_first = TRUE;
    unsigned char *buffer = service->modules;

    /* creates the iterator for the linked list */
    create_iterator_linked_list(service->modules_list, &modules_list_iterator);

    /* iterates continuously over the list of modules
    to create the modules label */
    while(TRUE) {
        /* retrieves the next value from the iterator and
        in case the current module is null (end of iterator)
        must break the current loop */
        get_next_iterator(modules_list_iterator, (void **) &current_module);
        if(current_module == NULL) { break; }

        /* in case this is the first iteration no extra space
        copy occurs, otherwise the space is copied to the buffer */
        if(is_first) {
            is_first = FALSE;
        } else {
            memcpy(buffer, " ", 1);
            buffer += 1;
        }

        /* retrieves the size of the current module name and
        then copies the name of it into the buffer */
        name_size = strlen((char *) current_module->name_s);
        memcpy(buffer, current_module->name_s, name_size);
        buffer += name_size;
    }

    /* closes the buffer with the final end of string charater
    so that the string becomes null closed */
    *buffer = '\0';

    /* deletes the iterator linked list */
    delete_iterator_linked_list(service->modules_list, modules_list_iterator);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE add_connection_service(struct service_t *service, struct connection_t *connection) {
    /* retrieves the polling (provider) */
    struct polling_t *polling = service->polling;

    /* adds the connection to the connections list and then
    registes the connection in the polling (provider) */
    append_value_linked_list(service->connections_list, connection);
    polling->register_connection(polling, connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE remove_connection_service(struct service_t *service, struct connection_t *connection) {
    /* retrieves the polling (provider) */
    struct polling_t *polling = service->polling;

    /* unregisters the connection from the polling (provider)
    and then removes the connection from the connections list
    note that the connection will be removed at the loop end */
    polling->unregister_connection(polling, connection, TRUE);
    remove_value_linked_list(service->connections_list, connection, TRUE);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_connection(struct connection_t **connection_pointer, SOCKET_HANDLE socket_handle) {
    /* retrieves the connection size */
    size_t connection_size = sizeof(struct connection_t);

    /* allocates space for the connection */
    struct connection_t *connection =\
        (struct connection_t *) MALLOC(connection_size);

    /* sets the connection attributes (default) values */
    connection->id = connection_id + 1;
    connection->status = STATUS_CLOSED;
    connection->creation = time(NULL);
    connection->sent = 0;
    connection->received = 0;
    connection->family = UNDEFINED_FAMILY;
    connection->protocol = UNDEFINED_PROTOCOL;
    connection->socket_handle = socket_handle;
    connection->port = 0;
    connection->service = NULL;
    connection->read_registered = TRUE;
    connection->write_registered = FALSE;
    connection->read_valid = FALSE;
    connection->write_valid = FALSE;
    connection->is_outstanding = FALSE;
    connection->open_connection = NULL;
    connection->close_connection = NULL;
    connection->write_connection = NULL;
    connection->register_read = NULL;
    connection->unregister_read = NULL;
    connection->register_write = NULL;
    connection->unregister_write = NULL;
    connection->invalidate_read = NULL;
    connection->invalidate_write = NULL;
    connection->add_outstanding = NULL;
    connection->alloc_data = alloc_connection;
    connection->on_read = NULL;
    connection->on_write = NULL;
    connection->on_error = NULL;
    connection->on_handshake = NULL;
    connection->on_open = NULL;
    connection->on_close = NULL;
    connection->parameters = NULL;
    connection->lower = NULL;

#ifdef VIRIATUM_SSL
    connection->ssl_handle = NULL;
    connection->ssl_context = NULL;
#endif

    /* creates both the read and the write queue linked lists that
    will be used as buffers for the data based operations */
    create_linked_list(&connection->read_queue);
    create_linked_list(&connection->write_queue);

    /* sets the first character of the host string to end of string
    so that the default value for it is an empty string, avoilding
    possible buffer overflows latter one */
    connection->host[0] = '\0';

    /* increments the current identifier for the connection so that
    the next connection to be created already has a new identifier */
    connection_id++;

    /* sets the connection in the connection pointer */
    *connection_pointer = connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_connection(struct connection_t *connection) {
    /* allocates the temporary data pointe that will be used
    in the iteration for the releasing of the pending data */
    struct data_t *data;

    /* iterates continuously to release all the pending
    memory to be released because the connection was dropped */
    while(TRUE) {
        /* pops a value (data) from the linked list (write queue)
        in case the data is invalid (list is empty) must break
        the current loop, this is the end of iteration */
        pop_value_linked_list(connection->write_queue, (void **) &data, TRUE);
        if(data == NULL) { break; }

        /* prints a debug message */
        V_DEBUG("Deleting data (cleanup structures)\n");

        /* deletes the data, releasing any pending buffer
        that is defined in it, to avoid any memory leak */
        delete_data(data);
    }

    /* in case the connection parameters are defined their
    memory must be relased (assumes a contiguous allocation) */
    if(connection->parameters) { FREE(connection->parameters); }

    /* deletes both the read and the write queue linked lists to
    avoid any more memory leak for these buffered lists */
    delete_linked_list(connection->read_queue);
    delete_linked_list(connection->write_queue);

    /* invalidates the addresses for both the read and the write
    queue this simplifies the structure of the connection */
    connection->read_queue = NULL;
    connection->write_queue = NULL;

    /* releases the connection */
    FREE(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE resolve_connection(struct connection_t *connection, SOCKET_ADDRESS socket_address) {
    /* allocates the temporary variable used to store
    the family of connection that is going to be accepted */
    SOCKET_FAMILY family;

    /* allocates the reference to the host value that is
    going to be returned uppon host resolution (ip address) */
    unsigned char *host;

    /* retrieves the reference to the family of the socket
    in order to be able to check if it's ipv6 or ipv4 and then
    switched over it to correctly populate the resolved attributes */
    family = ((SOCKET_ADDRESS_BASE *) &socket_address)->sa_family;
    switch(family) {
        /* in case the family of the connection that has been created
        and established is internet (ipv4) then the host may be created
        using the simple string conversion */
        case SOCKET_INTERNET_TYPE:
            connection->family = IP_V4_FAMILY;
            host = (unsigned char *) inet_ntoa(
                ((SOCKET_ADDRESS_INTERNET *) &socket_address)->sin_addr
            );
            memcpy(connection->host, host, strlen((char *) host) + 1);
            connection->port = ntohs(
                ((SOCKET_ADDRESS_INTERNET *) &socket_address)->sin_port
            );

            break;

#ifdef VIRIATUM_IP6
        case SOCKET_INTERNET6_TYPE:
            connection->family = IP_V6_FAMILY;
#ifdef VIRIATUM_PLATFORM_UNIX
			inet_ntop(
				family,
				&(((SOCKET_ADDRESS_INTERNET6 *) socket_address)->sin6_addr),
				(char *) connection->host,
				sizeof(connection->host)
			)
#endif
			connection->port = ntohs(
                ((SOCKET_ADDRESS_INTERNET *) &socket_address)->sin_port
            );
            break;
#endif

        default:
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE write_connection(
    struct connection_t *connection,
    unsigned char *data,
    unsigned int size,
    connection_data_callback
    callback,
    void *callback_parameters
) {
    return write_connection_c(
        connection,
        data,
        size,
        callback,
        callback_parameters,
        TRUE
    );
}

ERROR_CODE write_connection_c(
    struct connection_t *connection,
    unsigned char *data,
    unsigned int size,
    connection_data_callback callback,
    void *callback_parameters,
    char release
) {
    /* allocates the data structure that is going to be used as
    the deplyment chunk for the connection */
    struct data_t *_data;

    /* creates the data structure that will be used to store metada
    information on the chunk of data that is going to be written */
    create_data(&_data);

    /* sets the data contents in the data structure, configuring the
    data write operation accordingly */
    _data->data = data;
    _data->data_base = data;
    _data->size = size;
    _data->size_base = size;
    _data->release = release;
    _data->callback = callback;
    _data->callback_parameters = callback_parameters;

    /* adds the file buffer to the write queue and then
    registers the connection for write in the next loop */
    append_value_linked_list(connection->write_queue, (void *) _data);
    connection->register_write(connection);

    /* prints a debug message about the data that is going
    to be sent through the socket */
    V_DEBUG_F(
        "Queued %ld bytes to be sent using socket: %d\n",
        (long int) size,
        connection->socket_handle
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_connection(struct connection_t *connection) {
    /* in case the connection is (already) open, must return
    immediately with no error (duplicated operation) */
    if(connection->status == STATUS_OPEN) { RAISE_NO_ERROR; }

    /* prints a debug message */
    V_DEBUG_F("Opening connection: %d\n", connection->socket_handle);

    /* sets the connection status as open */
    connection->status = STATUS_OPEN;

    /* adds the connection to the service select */
    add_connection_service(connection->service, connection);

    /* in case the on open handler is defined */
    if(connection->on_open != NULL) {
        /* prints a debug message about the handler to be called
        and then calls the on open handler with the connection */
        V_DEBUG("Calling on open handler\n");
        connection->on_open(connection);
        V_DEBUG("Finished calling on open handler\n");
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_connection(struct connection_t *connection) {
    /* in case the connection is (already) closed, must return
    immediately with no error (duplicated operation) */
    if(connection->status == STATUS_CLOSED) { RAISE_NO_ERROR; }

    /* prints a debug message */
    V_DEBUG_F("Closing connection: %d\n", connection->socket_handle);

    /* in case the on close handler is defined */
    if(connection->on_close != NULL) {
        /* prints a debug message about the handler to be called
        and then calls the on close handler with the connection */
        V_DEBUG("Calling on close handler\n");
        connection->on_close(connection);
        V_DEBUG("Finished calling on close handler\n");
    }

    /* removes the connection from the service select */
    remove_connection_service(connection->service, connection);

    /* unsets the base hanlding functions from the connection */
    connection->open_connection = NULL;
    connection->close_connection = NULL;
    connection->register_read = NULL;
    connection->unregister_read = NULL;
    connection->register_write = NULL;
    connection->unregister_write = NULL;
    connection->invalidate_read = NULL;
    connection->invalidate_write = NULL;
    connection->add_outstanding = NULL;

#ifdef VIRIATUM_SSL
    /* in case there is a valid ssl connection set must release
    it correcly to avoid any memory leak */
    if(connection->ssl_handle) {
        /* shutsdown the ssl handle a releases it memory, unsetting
        the reference from the connection (avoids leaks) */
        SSL_shutdown(connection->ssl_handle);
        SSL_free(connection->ssl_handle);
        connection->ssl_handle = NULL;
    }
#endif

    /* closes the socket associated with the connection and then
    updates the connection status to closed in order to notify any
    logic acessing the connection about its state */
    SOCKET_CLOSE(connection->socket_handle);
    connection->status = STATUS_CLOSED;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_read_connection(struct connection_t *connection) {
    /* retrieves the (connection) service and uses it to
    retrieve the associated polling provider */
    struct service_t *service = connection->service;
    struct polling_t *polling = service->polling;

    /* registers the connection for read in the current
    polling mechanism and then sets the flag */
    polling->register_read(polling, connection);
    connection->read_registered = TRUE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_read_connection(struct connection_t *connection) {
    /* retrieves the (connection) service and uses it to
    retrieve the associated polling provider */
    struct service_t *service = connection->service;
    struct polling_t *polling = service->polling;

    /* unregisters the connection from read in the current
    polling mechanism and then unsets the flag */
    polling->unregister_read(polling, connection);
    connection->read_registered = FALSE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_connection(struct connection_t *connection) {
    /* retrieves the (connection) service and uses it to
    retrieve the associated polling provider */
    struct service_t *service = connection->service;
    struct polling_t *polling = service->polling;

    /* registers the connection for write in the current
    polling mechanism and then sets the flag */
    polling->register_write(polling, connection);
    connection->write_registered = TRUE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_connection(struct connection_t *connection) {
    /* retrieves the (connection) service and uses it to
    retrieve the associated polling provider */
    struct service_t *service = connection->service;
    struct polling_t *polling = service->polling;

    /* unregisters the connection from write in the current
    polling mechanism and then unsets the flag */
    polling->unregister_write(polling, connection);
    connection->write_registered = FALSE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE invalidate_read_connection(struct connection_t *connection) {
    connection->read_valid = FALSE;
    RAISE_NO_ERROR;
}

ERROR_CODE invalidate_write_connection(struct connection_t *connection) {
    connection->write_valid = FALSE;
    RAISE_NO_ERROR;
}

ERROR_CODE add_outstanding_connection(struct connection_t *connection) {
    struct service_t *service = connection->service;
    struct polling_t *polling = service->polling;
    polling->add_outstanding(polling, connection);
    RAISE_NO_ERROR;
}

ERROR_CODE alloc_connection(struct connection_t *connection, size_t size, void **data_pointer) {
    /* allocates a data chunk with the required size */
    *data_pointer = MALLOC(size);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_http_handler_service(struct service_t *service, struct http_handler_t **http_handler_pointer, unsigned char *name) {
    /* creates the http handler */
    create_http_handler(http_handler_pointer, name);

    /* sets a service reference in the http handler
    (this may be used latter for option reference) */
    (*http_handler_pointer)->service = service;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http_handler_service(struct service_t *service, struct http_handler_t *http_handler) {
    /* deletes the http handler */
    delete_http_handler(http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE add_http_handler_service(struct service_t *service, struct http_handler_t *http_handler) {
    /* sets the http handler in the http handers map for the handler name */
    set_value_string_hash_map(service->http_handlers_map, http_handler->name, (void *) http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE remove_http_handler_service(struct service_t *service, struct http_handler_t *http_handler) {
    /* unsets the http handler from the http handers map */
    set_value_string_hash_map(service->http_handlers_map, http_handler->name, NULL);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE get_http_handler_service(struct service_t *service, struct http_handler_t **http_handler_pointer, unsigned char *name) {
    /* tries to retrieve the http handler for the given name from the
    http handlers map to return it as the appropriate handler */
    get_value_string_hash_map(service->http_handlers_map, name, (void **) http_handler_pointer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _default_options_service(struct service_t *service, struct hash_map_t *arguments) {
    /* unpacks the service options from the service */
    struct service_options_t *service_options = service->options;

    /* sets the varius default service options */
    service_options->port = VIRIATUM_DEFAULT_PORT;
    service_options->address = (unsigned char *) VIRIATUM_DEFAULT_HOST;
    service_options->address6 = (unsigned char *) VIRIATUM_DEFAULT_HOST6;
    service_options->handler_name = (unsigned char *) VIRIATUM_DEFAULT_HANDLER;
    service_options->default_index = VIRIATUM_DEFAULT_INDEX;
    service_options->use_template = VIRIATUM_DEFAULT_USE_TEMPLATE;

    /* sets the default and staticaly defined mime type values
    for the evaluation of the basic file extensions */
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".323", "text/h323");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".*", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".acx", "application/internet-property-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ai", "application/postscript");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".aif", "audio/x-aiff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".aifc", "audio/x-aiff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".aiff", "audio/x-aiff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".asf", "video/x-ms-asf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".asr", "video/x-ms-asf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".asx", "video/x-ms-asf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".au", "audio/basic");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".avi", "video/x-msvideo");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".axs", "application/olescript");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".bas", "text/plain");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".bcpio", "application/x-bcpio");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".bin", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".bmp", "image/bmp");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".c", "text/plain");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cat", "application/vnd.ms-pkiseccat");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cbx", "application/colony-bundle");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ccx", "application/colony-container");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cdf", "application/x-cdf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cdf", "application/x-netcdf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cer", "application/x-x509-ca-cert");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".class", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".clp", "application/x-msclip");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cmx", "image/x-cmx");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cod", "image/cis-cod");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cpio", "application/x-cpio");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".cpx", "application/colony-plugin");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".crd", "application/x-mscardfile");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".crl", "application/pkix-crl");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".crt", "application/x-x509-ca-cert");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".csh", "application/x-csh");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".css", "text/css");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dcr", "application/x-director");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".der", "application/x-x509-ca-cert");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dir", "application/x-director");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dll", "application/x-msdownload");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dms", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".doc", "application/msword");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dot", "application/msword");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dvi", "application/x-dvi");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".dxr", "application/x-director");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".eps", "application/postscript");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".etx", "text/x-setext");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".evy", "application/envoy");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".exe", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".fif", "application/fractals");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".flr", "x-world/x-vrml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".gif", "image/gif");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".gtar", "application/x-gtar");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".gz", "application/x-gzip");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".h", "text/plain");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".hdf", "application/x-hdf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".hlp", "application/winhlp");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".hqx", "application/mac-binhex40");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".hta", "application/hta");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".htc", "text/x-component");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".htm", "text/html");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".html", "text/html");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".htt", "text/webviewhtml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ico", "image/x-icon");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ief", "image/ief");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".iii", "application/x-iphone");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ins", "application/x-internet-signup");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".isp", "application/x-internet-signup");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".jfif", "image/pipeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".jpe", "image/jpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".jpeg", "image/jpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".jpg", "image/jpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".js", "application/x-javascript");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".json", "application/json");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".latex", "application/x-latex");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".lha", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".lsf", "video/x-la-asf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".lsx", "video/x-la-asf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".lzh", "application/octet-stream");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".m13", "application/x-msmediaview");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".m14", "application/x-msmediaview");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".m3u", "audio/x-mpegurl");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".man", "application/x-troff-man");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mdb", "application/x-msaccess");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".me", "application/x-troff-me");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mht", "message/rfc822");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mhtml", "message/rfc822");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mid", "audio/mid");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mny", "application/x-msmoney");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mov", "video/quicktime");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".movie", "video/x-sgi-movie");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mp2", "video/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mp3", "audio/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mpa", "video/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mpe", "video/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mpeg", "video/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mpg", "video/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mpp", "application/vnd.ms-project");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mpv2", "video/mpeg");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ms", "application/x-troff-ms");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".msg", "application/vnd.ms-outlook");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".mvb", "application/x-msmediaview");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".nc", "application/x-netcdf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".nws", "message/rfc822");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".oda", "application/oda");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".odf", "font/opentype");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p10", "application/pkcs10");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p12", "application/x-pkcs12");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p7b", "application/x-pkcs7-certificates");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p7c", "application/x-pkcs7-mime");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p7m", "application/x-pkcs7-mime");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p7r", "application/x-pkcs7-certreqresp");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".p7s", "application/x-pkcs7-signature");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pbm", "image/x-portable-bitmap");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pdf", "application/pdf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pfx", "application/x-pkcs12");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pgm", "image/x-portable-graymap");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pko", "application/ynd.ms-pkipko");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pma", "application/x-perfmon");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pmc", "application/x-perfmon");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pml", "application/x-perfmon");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pmr", "application/x-perfmon");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pmw", "application/x-perfmon");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".png", "image/png");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pnm", "image/x-portable-anymap");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pot", "application/vnd.ms-powerpoint");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ppm", "image/x-portable-pixmap");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pps", "application/vnd.ms-powerpoint");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ppt", "application/vnd.ms-powerpoint");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".prf", "application/pics-rules");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ps", "application/postscript");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".pub", "application/x-mspublisher");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".qt", "video/quicktime");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ra", "audio/x-pn-realaudio");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ram", "audio/x-pn-realaudio");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ras", "image/x-cmu-raster");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".rgb", "image/x-rgb");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".rmi", "audio/mid");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".roff", "application/x-troff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".rtf", "application/rtf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".rtx", "text/richtext");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".scd", "application/x-msschedule");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".sct", "text/scriptlet");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".setpay", "application/set-payment-initiation");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".setreg", "application/set-registration-initiation");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".sh", "application/x-sh");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".shar", "application/x-shar");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".sit", "application/x-stuffit");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".snd", "audio/basic");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".spc", "application/x-pkcs7-certificates");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".spl", "application/futuresplash");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".src", "application/x-wais-source");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".sst", "application/vnd.ms-pkicertstore");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".stl", "application/vnd.ms-pkistl");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".stm", "text/html");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".sv4cpio", "application/x-sv4cpio");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".sv4crc", "application/x-sv4crc");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".svg", "image/svg+xml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".swf", "application/x-shockwave-flash");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".t", "application/x-troff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tar", "application/x-tar");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tcl", "application/x-tcl");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tex", "application/x-tex");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".texi", "application/x-texinfo");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".texinfo", "application/x-texinfo");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tgz", "application/x-compressed");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tif", "image/tiff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tiff", "image/tiff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tr", "application/x-troff");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".trm", "application/x-msterminal");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".tsv", "text/tab-separated-values");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ttf", "application/x-font-ttf");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".txt", "text/plain");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".uls", "text/iuls");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".ustar", "application/x-ustar");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".vcf", "text/x-vcard");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".vrml", "x-world/x-vrml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wav", "audio/x-wav");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wcm", "application/vnd.ms-works");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wdb", "application/vnd.ms-works");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wks", "application/vnd.ms-works");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wmf", "application/x-msmetafile");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wps", "application/vnd.ms-works");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wri", "application/x-mswrite");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wrl", "x-world/x-vrml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".wrz", "x-world/x-vrml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xaf", "x-world/x-vrml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xbm", "image/x-xbitmap");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xla", "application/vnd.ms-excel");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xlc", "application/vnd.ms-excel");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xlm", "application/vnd.ms-excel");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xls", "application/vnd.ms-excel");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xlt", "application/vnd.ms-excel");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xlw", "application/vnd.ms-excel");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xml", "application/xml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xof", "x-world/x-vrml");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xpm", "image/x-xpixmap");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".xwd", "image/x-xwindowdump");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".z", "application/x-compress");
    set_value_string_hash_map(service_options->mime_types, (unsigned char *) ".zip", "application/zip");

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _file_options_service(struct service_t *service, struct hash_map_t *arguments) {
    /* allocates the value reference to be used
    during the arguments retrieval */
    void *value;

    /* allocates space for the path to the proper configuration
    file (the ini base file) */
    char config_path[VIRIATUM_MAX_PATH_SIZE];

    /* allocates space for both the general configuration sort
    map and the "concrete" general configuration map */
    struct sort_map_t *configuration;
    struct sort_map_t *general;

    /* unpacks the service options from the service */
    struct service_options_t *service_options = service->options;

    /* creates the configuration file path using the defined viriatum
    path to the configuration directory and then loads it as an ini file,
    this should retrieve the configuration as a set of maps */
    SPRINTF(config_path, VIRIATUM_MAX_PATH_SIZE, "%s/viriatum.ini", VIRIATUM_CONFIG_PATH);
    process_ini_file(config_path, &configuration);
    service->configuration = configuration;

    /* tries to retrieve the general section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(configuration, (unsigned char *) "general", (void **) &general);
    if(general == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the port argument from the arguments map and
    in case the (port) value is set, casts the port value into integer
    and sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "port", &value);
    if(value != NULL) { service_options->port = (unsigned short) atoi(value); }

    /* tries to retrieve the host argument from the arguments map and
    in case the (host) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "host", &value);
    if(value != NULL) { service_options->address = (unsigned char *) value; }

    /* tries to retrieve the ip6 argument from the arguments map and
    in case the (ip6) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "ip6", &value);
    if(value != NULL) { service_options->ip6 = (unsigned char) atob(value); }

    /* tries to retrieve the ip6 host argument from the arguments map and
    in case the (host) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "ip6_host", &value);
    if(value != NULL) { service_options->address6 = (unsigned char *) value; }

    /* tries to retrieve the ssl argument from the arguments map and
    in case the (ssl) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "ssl", &value);
    if(value != NULL) { service_options->ssl = (unsigned char) atob(value); }

    /* tries to retrieve the ssl certificate from the arguments map and
    in case the (ssl csr) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "ssl_csr", &value);
    if(value != NULL) { service_options->ssl_csr = (unsigned char *) value;  }

    /* tries to retrieve the ssl (private) key from the arguments map and
    in case the (ssl key) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "ssl_key", &value);
    if(value != NULL) { service_options->ssl_key = (unsigned char *) value;  }

    /* tries to retrieve the handler argument from the arguments map and
    in case the (handler) value is set, sets it in the service options */
    get_value_string_sort_map(general, (unsigned char *) "handler", &value);
    if(value != NULL) { service_options->handler_name = (unsigned char *) value; }

    /* tries to retrieve the local argument from the arguments map, then
    in case the (local) value is set, sets the service as local  */
    get_value_string_sort_map(general, (unsigned char *) "local", &value);
    if(value != NULL) { service_options->local = (unsigned char) atob(value); }

    /* tries to retrieve the workers argument from the arguments map, then
    sets the workers (count) value for the service */
    get_value_string_sort_map(general, (unsigned char *) "workers", &value);
    if(value != NULL) { service_options->workers = (unsigned char) atoi(value); }

    /* tries to retrieve the use template argument from the arguments map, then
    sets the use template (boolean) value for the service */
    get_value_string_sort_map(general, (unsigned char *) "use_template", &value);
    if(value != NULL) { service_options->use_template = (unsigned char) atob(value); }

    /* tries to retrieve the index (file) argument from the arguments map, then
    sets the split value arround the space character in the index value */
    get_value_string_sort_map(general, (unsigned char *) "index", &value);
    if(value != NULL) { service_options->index_count = split(value, (char *) service_options->index, 128, ' '); }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _comand_line_options_service(struct service_t *service, struct hash_map_t *arguments) {
    /* allocates the value reference to be used
    during the arguments retrieval */
    void *value;

    /* unpacks the service options from the service */
    struct service_options_t *service_options = service->options;

    /* tries to retrieve the port argument from the arguments map, then
    in case the (port) value is set, casts the port value into
    integer and set it in the service options */
    get_value_string_hash_map(arguments, (unsigned char *) "port", &value);
    if(value != NULL) { service_options->port = (unsigned short) atoi(((struct argument_t *) value)->value); }

    /* tries to retrieve the host argument from the arguments map, then
    in case the (host) value is set, sets the address value in
    the service options */
    get_value_string_hash_map(arguments, (unsigned char *) "host", &value);
    if(value != NULL) { service_options->address = (unsigned char *) ((struct argument_t *) value)->value; }

    /* tries to retrieve the handler argument from the arguments map, then
    in case the (handler) value is set, sets the handler name value
    in the service options */
    get_value_string_hash_map(arguments, (unsigned char *) "handler", &value);
    if(value != NULL) { service_options->handler_name = (unsigned char *) ((struct argument_t *) value)->value; }

    /* tries to retrieve the local argument from the arguments map, then
    in case the (local) value is set, sets the service as local  */
    get_value_string_hash_map(arguments, (unsigned char *) "local", &value);
    if(value != NULL) { service_options->local = 1; }

    /* tries to retrieve the workers argument from the arguments map, then
    sets the workers (count) value for the service */
    get_value_string_hash_map(arguments, (unsigned char *) "workers", &value);
    if(value != NULL) { service_options->workers = (unsigned char) atoi(((struct argument_t *) value)->value); }

    /* raises no error */
    RAISE_NO_ERROR;
}

const char *_get_uptime_service(struct service_t *service, size_t count) {
    /* calculates the delata time (in seconds) and uses it to format
    the delta using the provied count value (number of parts), the
    uptime string is store in the service constant and then returned */
    unsigned long long delta = (unsigned long long) time(NULL) - service->start_time;
    format_delta(service->_uptime, sizeof(service->_uptime), delta, count);
    return service->_uptime;
}

const char *_get_mime_type_service(struct service_t *service, char *extension) {
    /* allocates space for the buffer reference that will hold the mime type
    then unpacks the service options from the service and uses it to access
    the mime types hash map and retrieve the mime type string from the provided
    extension string value and returns it to the caller function */
    char *mime_type;
    struct service_options_t *service_options = service->options;
    get_value_string_hash_map(
        service_options->mime_types,
        (unsigned char *) extension,
        (void **) &mime_type
    );
    return mime_type;
}
