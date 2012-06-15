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
    service->http_handler = NULL;
    service->create_http_handler = create_http_handler_service;
    service->delete_http_handler = delete_http_handler_service;
    service->add_http_handler = add_http_handler_service;
    service->remove_http_handler = remove_http_handler_service;
    service->get_http_handler = get_http_handler_service;

    /* creates the service options */
    create_service_options(&service->options);

    /* creates the polling (provider) */
    create_polling(&service->polling);

    /* creates the connections list */
    create_linked_list(&service->connections_list);

    /* creates the http modules list */
    create_linked_list(&service->modules_list);

    /* creates the http handlers map */
    create_hash_map(&service->http_handlers_map, 0);

    /* sets the service in the service pointer */
    *service_pointer = service;
}

void delete_service(struct service_t *service) {
    /* in case the service socket handle is defined */
    if(service->service_socket_handle) {
        /* closes the service socket (secure closing) */
        SOCKET_CLOSE(service->service_socket_handle);
    }

    /* in case the service configuration is defined */
    if(service->configuration) {
        /* deletes the service configuration */
        delete_configuration(service->configuration, 1);
    }

    /* deletes the http handlers map */
    delete_hash_map(service->http_handlers_map);

    /* deletes the http modules list */
    delete_linked_list(service->modules_list);

    /* deletes the connections list */
    delete_linked_list(service->connections_list);

    /* deletes the polling (provider) */
    delete_polling(service->polling);

    /* deletes the service options */
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
    service_options->handler_name = NULL;
    service_options->local = 0;
    service_options->default_index = 0;
    service_options->use_template = 0;
    service_options->default_virtual_host = NULL;

    /* creates the hash map for the virtual hosts */
    create_hash_map(&service_options->virtual_hosts, 0);

    /* sets the service options in the service options pointer */
    *service_options_pointer = service_options;
}

void delete_service_options(struct service_options_t *service_options) {
    /* deletes the hash map for the virtual hosts */
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
    data->callback = NULL;
    data->callback_parameters = NULL;

    /* sets the data in the data pointer */
    *data_pointer = data;
}

void delete_data(struct data_t *data) {
    /* releases the data */
    FREE(data->data_base);

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
    polling->register_write = NULL;
    polling->unregister_write = NULL;
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

void delete_configuration(struct hash_map_t *configuration, int is_top) {
    /* allocates space for the pointer to the element and
    for the option to be retrieved */
    struct hash_map_element_t *element;
    void *option;

    /* allocates space for the iterator for the configuration */
    struct iterator_t *configuration_iterator;

    /* creates an iterator for the ´configuration hash map */
    create_element_iterator_hash_map(configuration, &configuration_iterator);

    /* iterates continuously */
    while(1) {
        /* retrieves the next value from the configuration iterator */
        get_next_iterator(configuration_iterator, (void **) &element);

        /* in case the current module is null (end of iterator) */
        if(element == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrievs the hash map value for the key pointer */
        get_value_hash_map(configuration, element->key, element->key_string, (void **) &option);

        /* in case the current iteration is of type top must delete the
        inner configuration because this is "just" a section, otherwise
        releases the memory space occupied by the value */
        if(is_top) { delete_configuration((struct hash_map_t *) option, 0); }
        else { FREE(option); }
    }

    /* deletes the iterator for the configuration hash map */
    delete_iterator_hash_map(configuration, configuration_iterator);

    /* deletes the hash map that holds the configuration */
    delete_hash_map(configuration);
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







#ifdef VIRIATUM_PLATFORM_LINUX
#ifdef VIRIATUM_PLATFORM_ANDROID
#define SET_PROC_NAME(name)
#else
#ifdef PR_SET_NAME
#define SET_PROC_NAME(name) prctl(PR_SET_NAME, name)
#endif
#endif
#endif

#ifdef VIRIATUM_PLATFORM_BSD
#define SET_PROC_NAME(name) setproctitle(name)
#endif

#ifndef SET_PROC_NAME
#define SET_PROC_NAME(name)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
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
    while(1) {
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
    while(1) {
        /* in case the number of joins is the same
        as the worker count, breaks the loop no more
        joining remaining */
        if(join_count == worker_count) { break; }

		/* retrieves the pid of the worker to be "killed"
		and then joined */
		pid = service->worker_pids[join_count];
		kill(pid, SIGINT);

		printf("joining %d", pid);

		/* wiats for the process to exit it's existence
		this may hang the current process in case there's
		a problem in the child process */
		waitpid(pid, &status, WUNTRACED);

        /* increments the join count variable (one more
        iteration ran) */
        join_count++;
    }
}
#endif









ERROR_CODE _create_client_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
    /* allocates the socket handle */
    SOCKET_HANDLE socket_handle;
    SOCKET_ADDRESS_INTERNET serv_addr;
    SOCKET_HOSTENT *server;
    SOCKET_ERROR_CODE error;

    /* allocates the (client) connection, this is the
    structure representing the connection */
    struct connection_t *connection;

    /* allocates the option value and sets it to one (valid) */
    SOCKET_OPTION option_value = 1;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    socket_handle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    if(SOCKET_TEST_ERROR(socket_handle)) { fprintf(stderr, "ERROR opening socket"); }

    server = SOCKET_GET_HOST_BY_NAME(hostname);

    if(server == NULL) { fprintf(stderr, "ERROR, no such host\n"); }

    memset(&serv_addr, 0, sizeof(SOCKET_ADDRESS_INTERNET));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    error = SOCKET_CONNECT_SIZE(socket_handle, serv_addr, sizeof(SOCKET_ADDRESS_INTERNET));
    if(SOCKET_TEST_ERROR(error)) { fprintf(stderr, "ERROR connecting host\n"); }

    /* in case viriatum is set to non blocking, changes the current
    socket behavior to non blocking mode then sets the socket to the
    non push mode in case it's required by configuration this implies
    also checking for the no (tcp) wait variable */
    if(VIRIATUM_NON_BLOCKING) { SOCKET_SET_NON_BLOCKING(socket_handle, flags); }
    if(VIRIATUM_NO_WAIT) { SOCKET_SET_NO_WAIT(socket_handle, option_value); }
    if(VIRIATUM_NO_PUSH) { SOCKET_SET_NO_PUSH(socket_handle, option_value); }

    /* creates the (client) connection */
    create_connection(&connection, socket_handle);

    /* sets the socket address in the (client) connection
    this is going to be very usefull for later connection
    identification (address, port, etc.) */
    /*connection->socket_address = (SOCKET_ADDRESS) serv_addr;*/

    /* sets the service select service as the service in the (client)  connection */
    connection->service = service;

    /* sets the base hanlding functions in the client connection */
    connection->open_connection = open_connection;
    connection->close_connection = close_connection;
    connection->write_connection = write_connection;
    connection->register_write = register_write_connection;
    connection->unregister_write = unregister_write_connection;

    /* sets the various stream io connection callbacks
    in the client connection */
    connection->on_read = read_handler_stream_io;
    connection->on_write = write_handler_stream_io;
    connection->on_error = error_handler_stream_io;
    connection->on_open = open_handler_stream_io;
    connection->on_close = close_handler_stream_io;

    /* updats the connection pointer with the refernce
    to the connection structure */
    *connection_pointer = connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _create_tracker_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
    /* allocates space for the connection reference */
    struct connection_t *connection;

    /* alocates dynamic space for the parameters to the
    http stream (http client) this structure will be able
    to guide the stream of http client */
    struct http_client_parameters_t *parameters = (struct http_client_parameters_t *) MALLOC(sizeof(struct http_client_parameters_t));

    /* populates the parameters structure with the
    required values for the http client request */
    parameters->url = "/ptorrent/announce.php";

    /* creates a general client conneciton structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(connection_pointer, service, hostname, port);
    connection = *connection_pointer;

    /* sets the http client protocol as the protocol to be
    "respected" for this client connection, this should
    be able to set the apropriate handlers in io then sets
    the parameters structure in the connection so that the
    lower layers "know" what to do */
    connection->protocol = HTTP_CLIENT_PROTOCOL;
    connection->parameters = (void *) parameters;

    /* opens the connection */
    connection->open_connection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _create_torrent_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
    /* allocates space for the connection reference */
    struct connection_t *connection;

    /* creates a general client conneciton structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(connection_pointer, service, hostname, port);
    connection = *connection_pointer;

    /* sets the torrent protocol as the protocol to be
    "respected" for this client connection, this should
    be able to set the apropriate handlers in io */
    connection->protocol = TORRENT_PROTOCOL;

    /* opens the connection */
    connection->open_connection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}




















ERROR_CODE start_service(struct service_t *service) {
    /* allocates the socket address structure */
    SOCKET_ADDRESS_INTERNET socket_address;

    /* allocates the socket result */
    SOCKET_ERROR_CODE socket_result;

    /* allocates the service socket handle */
    SOCKET_HANDLE service_socket_handle;

    /* allocates the service connection */
    struct connection_t *service_connection;




    /* allocates the misc connections references */
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

    /* allocates the option value and sets it to one (valid) */
    SOCKET_OPTION option_value = 1;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* unpacks the service options from the service structure */
    struct service_options_t *service_options = service->options;

    /* registers the various "local" handlers
    in the service, for later usage */
    register_handler_dispatch(service);
    register_handler_default(service);
    register_handler_file(service);

    /* loads (all) the currently available modules */
    load_modules_service(service);

    /* sets the current http handler accoring to the current options
    in the service, the http handler must be loaded in the handlers map */
    get_value_string_hash_map(service->http_handlers_map, service_options->handler_name, (void **) &service->http_handler);

    /* sets the socket address attributes */
    socket_address.sin_family = SOCKET_INTERNET_TYPE;
    socket_address.sin_addr.s_addr = inet_addr((char *) service_options->address);
    socket_address.sin_port = htons(service_options->port);

    /* creates the service socket for the given types */
    service->service_socket_handle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    /* in case there was an error creating the service socket */
    if(SOCKET_TEST_ERROR(service->service_socket_handle)) {
        /* retrieves the creating error code */
        SOCKET_ERROR_CODE creating_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints the error */
        V_ERROR_F("Problem creating socket: %d\n", creating_error_code);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem creating socket");
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

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(socket_result)) {
        /* retrieves the option error code */
        SOCKET_ERROR_CODE option_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints the error */
        V_ERROR_F("Problem setting socket option: %d\n", option_error_code);

        /* closes the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem setting socket option");
    }

    /* binds the service socket */
    socket_result = SOCKET_BIND(service->service_socket_handle, socket_address);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(socket_result)) {
        /* retrieves the binding error code */
        SOCKET_ERROR_CODE binding_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints the error */
        V_ERROR_F("Problem binding socket: %d\n", binding_error_code);

        /* closes the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem binding socket");
    }

    /* listens for a service socket change */
    socket_result = SOCKET_LISTEN(service->service_socket_handle);

    /* in case there was an error listening the socket */
    if(SOCKET_TEST_ERROR(socket_result)) {
        /* retrieves the listening error code */
        SOCKET_ERROR_CODE binding_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints the error */
        V_ERROR_F("Problem listening socket: %d\n", binding_error_code);

        /* closes the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem listening socket");
    }

    /* sets the service status as open */
    service->status = STATUS_OPEN;

    /* retrieves the service (required) elements */
    polling = service->polling;
    service_socket_handle = service->service_socket_handle;

    /* sets the service in the polling */
    polling->service = service;

    /* sets the various handler values to the polling process
    they will be called for the various read, write operations */
    polling->open = open_polling_select;
    polling->close = close_polling_select;
    polling->register_connection = register_connection_polling_select;
    polling->unregister_connection = unregister_connection_polling_select;
    polling->register_write = register_write_polling_select;
    polling->unregister_write = unregister_write_polling_select;
    polling->poll = poll_polling_select;
    polling->call = call_polling_select;

    /* opens the polling (provider) */
    polling->open(polling);

    /* creates the (service) connection */
    create_connection(&service_connection, service_socket_handle);

    /* sets the service select service as the service in the service connection */
    service_connection->service = service;

    /* sets the base hanlding functions in the service connection */
    service_connection->open_connection = open_connection;
    service_connection->close_connection = close_connection;
    service_connection->write_connection = write_connection;
    service_connection->register_write = register_write_connection;
    service_connection->unregister_write = unregister_write_connection;

    /* sets the fucntion to be called uppon read on the service
    connection (it should be the accept handler stream io, default) */
    service_connection->on_read = accept_handler_stream_io;

    /* opens the (service) connection */
    service_connection->open_connection(service_connection);

#ifdef VIRIATUM_PLATFORM_UNIX
    /* in case the current os is compatible with the forking of process
    creates the worker processes to handle more connections at a time,
    this operation creates a much more flexible and scalable solution */
    create_workers(service);
#endif



/*    _create_tracker_connection(&tracker_connection, service, "localhost", 9090);
    _create_torrent_connection(&torrent_connection, service, "localhost", 32967);*/














    /* iterates continuously, while the service is open (this
    is the main loop triggering all the actions) */
    while(service->status == STATUS_OPEN) {
        /* retrieves the (current) process, to be used
        to retrieves some memory information, and then closes it*/
        process = GET_PROCESS();
        GET_MEMORY_INFORMATION(process, memory_information);
        memory_usage = GET_MEMORY_USAGE(memory_information);
        CLOSE_PROCESS(process);

        /* prints a debug message */
        V_DEBUG_F("Memory status: [%ld objects] [%ld KBytes]\n", (long int) ALLOCATIONS, (long int) memory_usage / 1024);

        /* polls the connections using the polling (provider) */
        polling->poll(polling);

        /* calls the callbacks for the connection (events)
        using the polling (provider) */
        polling->call(polling);
    }

    /* closes (all) the service connections */
    close_connections_service(service);

    /* closes the polling (provider) */
    polling->close(polling);

    /* unloads the modules for the service */
    unload_modules_service(service);

    /* unregisters the various "local" handlers
    from the service, for structure destruction */
    unregister_handler_file(service);
    unregister_handler_default(service);
    unregister_handler_dispatch(service);

#ifdef VIRIATUM_PLATFORM_UNIX
    join_workers(service);
#endif

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_service(struct service_t *service) {
    /* sets the service status as closed */
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

    /* iterates continuously */
    while(1) {
        /* pops a value from the connections list (and deletes the node) */
        pop_value_linked_list(connections_list, (void **) &current_connection, 1);

        /* in case the current connection is null (connections list is empty) */
        if(current_connection == NULL) {
            /* breaks the loop */
            break;
        }

        /* closes the current connection */
        current_connection->close_connection(current_connection);

        /* deletes the current connection */
        delete_connection(current_connection);
    }

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
    while(1) {
        /* retrieves the next value from the iterator */
        get_next_iterator(entries_iterator, (void **) &entry);

        /* in case the current module is null (end of iterator) */
        if(entry == NULL) {
            /* breaks the loop */
            break;
        }

        /* in case the entry name does not ends with the shared object extension
        it must not be a module to be loaded */
        if(ends_with_string(entry->name, (unsigned char *) VIRIATUM_SHARED_OBJECT_EXTENSION) == 0) {
            /* continue with the loop */
            continue;
        }

        /* creates the complete module path for the loading of it */
        SPRINTF((char *) module_path, VIRIATUM_MAX_PATH_SIZE, "%s/%s", VIRIATUM_MODULES_PATH, entry->name);

        /* tries to find the module prefix in the current entry name
        in case it's not found continues the loop immediately no library
        loading is required (not the correct format) */
        pointer = strstr((char *) entry->name, "viriatum_");
        if(pointer != (char *) entry->name && pointer != (char *) entry->name + 3) { continue; }

        /* loads the module, retrieving a possible error code */
        error_code = load_module(service, module_path);

        /* tests the error code for error */
        if(IS_ERROR_CODE(error_code)) {
            /* prints a warning message */
            V_WARNING_F("Problem loading module (%s)\n", (char *) GET_ERROR());

            /* raises again the error */
            RAISE_AGAIN(error_code);
        }
    }

    /* deletes the iterator used for the entries */
    delete_iterator_linked_list(entries, entries_iterator);

    /* deletes both the entries internal structures
    and the entries linked list */
    delete_directory_entries_file(entries);
    delete_linked_list(entries);

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
    while(1) {
        /* retrieves the next value from the iterator */
        get_next_iterator(modules_list_iterator, (void **) &current_module);

        /* in case the current module is null (end of iterator) */
        if(current_module == NULL) {
            /* breaks the loop */
            break;
        }

        /* unloads the current module */
        unload_module(service, current_module);
    }

    /* deletes the iterator linked list */
    delete_iterator_linked_list(service->modules_list, modules_list_iterator);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE add_connection_service(struct service_t *service, struct connection_t *connection) {
    /* retrieves the polling (provider) */
    struct polling_t *polling = service->polling;

    /* adds the connection to the connections list */
    append_value_linked_list(service->connections_list, connection);

    /* registes the connection in the polling (provider) */
    polling->register_connection(polling, connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE remove_connection_service(struct service_t *service, struct connection_t *connection) {
    /* retrieves the polling (provider) */
    struct polling_t *polling = service->polling;

    /* unregisters the connection from the polling (provider) */
    polling->unregister_connection(polling, connection);

    /* removes the connection from the connections list */
    remove_value_linked_list(service->connections_list, connection, 1);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_connection(struct connection_t **connection_pointer, SOCKET_HANDLE socket_handle) {
    /* retrieves the connection size */
    size_t connection_size = sizeof(struct connection_t);

    /* allocates space for the connection */
    struct connection_t *connection = (struct connection_t *) MALLOC(connection_size);

    /* sets the connection attributes (default) values */
    connection->status = STATUS_CLOSED;
    connection->protocol = UNDEFINED_PROTOCOL;
    connection->socket_handle = socket_handle;
    connection->service = NULL;
    connection->write_registered = 0;
    connection->open_connection = NULL;
    connection->close_connection = NULL;
    connection->write_connection = NULL;
    connection->register_write = NULL;
    connection->unregister_write = NULL;
    connection->alloc_data = alloc_connection;
    connection->on_read = NULL;
    connection->on_write = NULL;
    connection->on_error = NULL;
    connection->on_open = NULL;
    connection->on_close = NULL;
    connection->parameters = NULL;
    connection->lower = NULL;

    /* creates the read queue linked list */
    create_linked_list(&connection->read_queue);

    /* creates the write queue linked list */
    create_linked_list(&connection->write_queue);

    /* sets the connection in the connection pointer */
    *connection_pointer = connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_connection(struct connection_t *connection) {
    /* allocates the data */
    struct data_t *data;

    /* iterates continuously */
    while(1) {
        /* pops a value (data) from the linked list (write queue) */
        pop_value_linked_list(connection->write_queue, (void **) &data, 1);

        /* in case the data is invalid (list is empty) */
        if(data == NULL) {
            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG("Deleting data (cleanup structures)\n");

        /* deletes the data */
        delete_data(data);
    }

    /* in case the connection parameters are defined their
    memory must be relased (assumes a contiguous allocation) */
    if(connection->parameters) { FREE(connection->parameters); }

    /* deletes the read queue linked list */
    delete_linked_list(connection->read_queue);

    /* deletes the write queue linked list */
    delete_linked_list(connection->write_queue);

    /* releases the connection */
    FREE(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE write_connection(struct connection_t *connection, unsigned char *data, unsigned int size, connection_data_callback callback, void *callback_parameters) {
    /* allocates the data */
    struct data_t *_data;

    /* creates the data */
    create_data(&_data);

    /* sets the data contents */
    _data->data = data;
    _data->data_base = data;
    _data->size = size;
    _data->callback = callback;
    _data->callback_parameters = callback_parameters;

    /* adds the file buffer to the write queue */
    append_value_linked_list(connection->write_queue, (void *) _data);

    /* registers the connection for write */
    connection->register_write(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_connection(struct connection_t *connection) {
    /* in case the connection is (already) open */
    if(connection->status == STATUS_OPEN) {
        /* raises no error */
        RAISE_NO_ERROR;
    }

    /* prints a debug message */
    V_DEBUG_F("Opening connection: %d\n", connection->socket_handle);

    /* sets the connection status as open */
    connection->status = STATUS_OPEN;

    /* adds the connection to the service select */
    add_connection_service(connection->service, connection);

    /* in case the on open handler is defined */
    if(connection->on_open != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on open handler\n");

        /* calls the on open handler */
        connection->on_open(connection);

        /* prints a debug message */
        V_DEBUG("Finished calling on open handler\n");
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_connection(struct connection_t *connection) {
    /* in case the connection is (already) closed */
    if(connection->status == STATUS_CLOSED) {
        /* raises no error */
        RAISE_NO_ERROR;
    }

    /* prints a debug message */
    V_DEBUG_F("Closing connection: %d\n", connection->socket_handle);

    /* in case the on close handler is defined */
    if(connection->on_close != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on close handler\n");

        /* calls the on close handler */
        connection->on_close(connection);

        /* prints a debug message */
        V_DEBUG("Finished calling on close handler\n");
    }

    /* removes the connection from the service select */
    remove_connection_service(connection->service, connection);

    /* unsets the base hanlding functions from the connection */
    connection->open_connection = NULL;
    connection->close_connection = NULL;
    connection->register_write = NULL;
    connection->unregister_write = NULL;

    /* closes the socket */
    SOCKET_CLOSE(connection->socket_handle);

    /* sets the connection status as closed */
    connection->status = STATUS_CLOSED;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_connection(struct connection_t *connection) {
    /* retrieves the (connection) service */
    struct service_t *service = connection->service;

    /* retrieves the polling (provider) */
    struct polling_t *polling = service->polling;

    /* registers the connection for write */
    polling->register_write(polling, connection);

    /* sets the connection as write registered */
    connection->write_registered = 1;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_connection(struct connection_t *connection) {
    /* retrieves the (connection) service */
    struct service_t *service = connection->service;

    /* retrieves the polling (provider) */
    struct polling_t *polling = service->polling;

    /* unregisters the connection for write */
    polling->unregister_write(polling, connection);

    /* sets the connection as not write registered */
    connection->write_registered = 0;

    /* raises no error */
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
    service_options->handler_name = (unsigned char *) VIRIATUM_DEFAULT_HANDLER;
    service_options->default_index = VIRIATUM_DEFAULT_INDEX;
    service_options->use_template = VIRIATUM_DEFAULT_USE_TEMPLATE;

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

    /* allocates space for both the general configuration hash
    map and the "concrete" general configuration map */
    struct hash_map_t *configuration;
    struct hash_map_t *general;

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
    get_value_string_hash_map(configuration, (unsigned char *) "general", (void **) &general);
    if(general == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the port argument from the arguments map and
    in case the (port) value is set, casts the port value into integer
    and sets it in the service options */
    get_value_string_hash_map(general, (unsigned char *) "port", &value);
    if(value != NULL) { service_options->port = (unsigned short) atoi(value); }

    /* tries to retrieve the host argument from the arguments map and
    in case the (host) value is set, sets it in the service options */
    get_value_string_hash_map(general, (unsigned char *) "host", &value);
    if(value != NULL) { service_options->address = (unsigned char *) value; }

    /* tries to retrieve the handler argument from the arguments map and
    in case the (handler) value is set, sets it in the service options */
    get_value_string_hash_map(general, (unsigned char *) "handler", &value);
    if(value != NULL) { service_options->handler_name = (unsigned char *) value; }

    /* tries to retrieve the local argument from the arguments map, then
    in case the (local) value is set, sets the service as local  */
    get_value_string_hash_map(general, (unsigned char *) "local", &value);
    if(value != NULL) { service_options->local = (unsigned char) atoi(value); }

    /* tries to retrieve the workers argument from the arguments map, then
    sets the workers (count) value for the service */
    get_value_string_hash_map(general, (unsigned char *) "workers", &value);
    if(value != NULL) { service_options->workers = (unsigned char) atoi(value); }

    /* tries to retrieve the use template argument from the arguments map, then
    sets the use template (boolean) value for the service */
    get_value_string_hash_map(general, (unsigned char *) "use_template", &value);
    if(value != NULL) { service_options->use_template = (unsigned char) atoi(value); }

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
