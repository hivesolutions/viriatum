/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "service_test.h"

const char *test_delete_service(void) {
    /* deletes an unset service reference, this should be
    a no operation instead of an invalid memory access */
    delete_service(NULL);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_create_service_options(void) {
    /* allocates space for the service options structure
    to be used in the test */
    struct service_options_t *service_options;

    /* creates the service options and verifies that every
    default value is properly initialized, note that the
    workers value is relevant as it's used as the upper
    bound of the worker pids buffer */
    create_service_options(&service_options);

    V_ASSERT(service_options->port == 0);
    V_ASSERT(service_options->address == NULL);
    V_ASSERT(service_options->ip6 == 0);
    V_ASSERT(service_options->address6 == NULL);
    V_ASSERT(service_options->ssl == 0);
    V_ASSERT(service_options->ssl_csr == NULL);
    V_ASSERT(service_options->ssl_key == NULL);
    V_ASSERT(service_options->handler_name == NULL);
    V_ASSERT(service_options->local == 0);
    V_ASSERT(service_options->workers == 0);
    V_ASSERT(service_options->default_index == 0);
    V_ASSERT(service_options->www_root[0] == '\0');
    V_ASSERT(service_options->use_template == 0);
    V_ASSERT(service_options->access_log == 1);
    V_ASSERT(service_options->error_log == 1);
    V_ASSERT(service_options->default_virtual_host == NULL);
    V_ASSERT(service_options->index_count == 0);

    /* deletes the service options */
    delete_service_options(service_options);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_base_path_service(void) {
    /* allocates space for the path of the directory that the binary
    sits in and for the flag that tells a directory apart */
    char *base_path;
    unsigned int is_directory = 0;

    /* resolves the directory of the binary, which every tree that is
    unpacked rather than installed is found from */
    base_path = get_base_path();
    V_ASSERT_NOT_NULL(base_path);

    /* the resolving happens once and is kept, a second call gives
    back the very same buffer rather than doing it again */
    V_ASSERT_EQ_P(get_base_path(), base_path);

    /* the platforms that are able to ask for the name of their own
    executable always resolve it, the ones that are not are left with
    an empty path and nothing more may be said about it */
#if defined(VIRIATUM_PLATFORM_WIN32) || defined(VIRIATUM_PLATFORM_MACOSX) || defined(VIRIATUM_PLATFORM_LINUX)
    V_ASSERT_M(base_path[0] != '\0', "the base path was not resolved");
#endif
    if(base_path[0] == '\0') { return NULL; }

    /* what it names is a directory, the one holding the binary, and
    it never carries the separator that closed it */
    is_directory_file(base_path, &is_directory);
    V_ASSERT_M(is_directory, "the base path is not a directory");
    V_ASSERT(base_path[strlen(base_path) - 1] != VIRIATUM_PATH_SEPARATOR_C);

    /* the directory is named in absolute terms, the name the process
    was launched through being resolved before anything is taken from
    it, otherwise the tree would move with the process */
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(base_path[0] == VIRIATUM_PATH_SEPARATOR_C || base_path[1] == ':');
#else
    V_ASSERT(base_path[0] == VIRIATUM_PATH_SEPARATOR_C);
#endif

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_bundled_path_service(void) {
    /* allocates space for the path that is resolved, for the name
    that is built to the length that is under test and for the one
    that stands for a tree that was installed rather than unpacked */
    unsigned char path[VIRIATUM_MAX_PATH_SIZE];
    char name[VIRIATUM_MAX_PATH_SIZE];
    size_t name_length;
    const char *fallback = "viriatum-installed-tree";

    /* a tree beside the binary only exists once the directory of it
    has been resolved, a platform that is unable to ask for it keeps
    the tree it was built with and prefers nothing over it */
    if(get_base_path()[0] == '\0') {
        _bundled_path_service(path, "..", fallback);
        V_ASSERT_EQ_S((char *) path, fallback);
        return NULL;
    }

    /* a name that is really a directory beside the binary is the one
    that is kept, the parent of it being one on every platform */
    _bundled_path_service(path, "..", fallback);
    V_ASSERT_M(
        strcmp((char *) path, fallback) != 0,
        "a directory beside the binary was not preferred"
    );
    V_ASSERT_NOT_NULL(strstr((char *) path, ".."));
    V_ASSERT(strstr((char *) path, get_base_path()) == (char *) path);

    /* a name that nothing sits under falls back to the path that the
    binary was built with, which is what an installed tree uses */
    _bundled_path_service(path, "viriatum-not-a-directory", fallback);
    V_ASSERT_EQ_S((char *) path, fallback);

    /* a name that is a file rather than a directory is no more a tree
    than a missing one is, so it falls back just the same */
    _bundled_path_service(path, VIRIATUM_NAME, fallback);
    V_ASSERT_EQ_S((char *) path, fallback);

    /* the longest name that still fits beside the base path is built
    the way any other one is, the path of it filling the buffer that
    holds it right up to the end of it */
    name_length = VIRIATUM_MAX_PATH_SIZE - strlen(get_base_path()) - 2;
    V_ASSERT(name_length > 0 && name_length < VIRIATUM_MAX_PATH_SIZE);
    memset(name, 'v', name_length);
    name[name_length] = '\0';
    _bundled_path_service(path, name, fallback);
    V_ASSERT_EQ_S((char *) path, fallback);

    /* one character more than that no longer fits, the path of it is
    never built so that nothing is written past the end of the buffer
    that was handed over to hold it */
    name[name_length] = 'v';
    name[name_length + 1] = '\0';
    _bundled_path_service(path, name, fallback);
    V_ASSERT_EQ_S((char *) path, fallback);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_calculate_options_service(void) {
    /* allocates space for the error code returned by the options
    calculation, for the service that carries them and for the map
    of arguments that the loading of the defaults takes */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;

    /* allocates space for the directory that stands in for a web
    root and for the absolute form that it is resolved into */
    char current_directory[VIRIATUM_MAX_PATH_SIZE];
    char expected_path[VIRIATUM_MAX_PATH_SIZE];

    /* retrieves the working directory of the process, the only one
    that is known to exist on every machine the tests run on */
    if(CURRENT_DIRECTORY(current_directory, VIRIATUM_MAX_PATH_SIZE) == NULL) {
        return "problem retrieving the current directory";
    }

    /* creates the service and the empty arguments map that the
    loading of the default options takes */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    create_hash_map(&arguments, 0);
    V_ASSERT(service->options->www_root[0] == '\0');

    /* the web root stands for the override that the configuration and
    the command line carry, the loading of the defaults never sets it
    or the tree beside the binary would never be preferred */
    _default_options_service(service, arguments);
    V_ASSERT_M(
        service->options->www_root[0] == '\0',
        "the loading of the default options set a web root"
    );

    /* with no web root set both the contents and the resources come
    from the same tree, the one beside the binary when it was unpacked
    and the one it was built with when it was installed */
    error = calculate_options_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S(
        (char *) service->options->contents_path,
        (char *) service->options->resources_path
    );
    V_ASSERT(service->options->modules_path[0] != '\0');

    /* a web root that was set overrides both of them, the modules
    being resolved on their own and so never following it */
    SPRINTF(
        (char *) service->options->www_root,
        VIRIATUM_MAX_PATH_SIZE, "%s", current_directory
    );
    error = calculate_options_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));

    STRCPY(expected_path, VIRIATUM_MAX_PATH_SIZE, current_directory);
    absolute_path_file(expected_path, TRUE);
    V_ASSERT_EQ_S((char *) service->options->contents_path, expected_path);
    V_ASSERT_EQ_S((char *) service->options->resources_path, expected_path);
    V_ASSERT(strcmp((char *) service->options->modules_path, expected_path) != 0);

    /* deletes the arguments map and the service releasing every
    internal structure that has been created */
    delete_hash_map(arguments);
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_calculate_locations_service(void) {
    /* allocates space for the error code returned by the
    locations calculation and for the service structure */
    ERROR_CODE error;
    struct service_t *service;

    /* allocates space for the configuration that is built by hand,
    for the section that is too short to name a location and for the
    one that does name one */
    struct sort_map_t *configuration;
    struct sort_map_t *general;
    struct sort_map_t *location;

    /* creates the service, note that a newly created service
    has no configuration loaded into it */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    V_ASSERT(service->configuration == NULL);

    /* calculates the locations for a service with no configuration,
    this should be tolerated instead of iterating a null sort map */
    error = calculate_locations_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(service->locations.count == 0);

    /* builds a configuration carrying a section whose name is shorter
    than the prefix of a location, which is the shape of the general
    one that every configuration file of the project opens with */
    create_sort_map(&configuration, 0);
    create_sort_map(&general, 0);
    create_sort_map(&location, 0);
    set_value_string_sort_map(location, (unsigned char *) "path", (void *) "/");
    set_value_string_sort_map(location, (unsigned char *) "handler", (void *) "file");
    set_value_string_sort_map(configuration, (unsigned char *) "general", (void *) general);
    set_value_string_sort_map(configuration, (unsigned char *) "location:/", (void *) location);
    service->configuration = configuration;

    /* the section that is too short to carry the prefix is skipped
    without the comparison reading past the end of the name of it,
    and only the one that does carry it becomes a location */
    error = calculate_locations_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->locations.count, 1);
    V_ASSERT_EQ_S((char *) service->locations.values[0].name, "/");
    V_ASSERT_EQ_S((char *) service->locations.values[0].path, "/");
    V_ASSERT_EQ_S((char *) service->locations.values[0].handler, "file");

    /* releases the configuration that has been built by hand, the
    service never took ownership of the maps inside of it */
    service->configuration = NULL;
    delete_sort_map(location);
    delete_sort_map(general);
    delete_sort_map(configuration);

    /* deletes the service releasing every internal structure */
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_open_close_service(void) {
    /* allocates space for the error codes returned by the various
    lifecycle calls and for the service and arguments structures */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;

    /* allocates space for the handler that is looked up so that the
    registrations of the opening may be told from the closing */
    struct http_handler_t *http_handler;

    /* creates the service and loads both the specifications and the
    default options into it, no configuration file is involved */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    load_specifications(service);
    create_hash_map(&arguments, 0);
    _default_options_service(service, arguments);
    delete_hash_map(arguments);

    /* sets the options so that an unlikely port is bound and neither
    the modules nor the worker processes are involved */
    service->options->port = VIRIATUM_TEST_PORT;
    service->options->load_modules = 0;
    service->options->workers = 0;
    service->options->ip6 = 0;
    calculate_options_service(service);
    calculate_locations_service(service);

    /* opens the service, this should bind the socket and leave the
    service in the open state without entering any loop */
    error = open_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(service->status == STATUS_OPEN);

    /* the opening registers the handlers that the service carries of
    its own, no module is loaded by this test */
    service->get_http_handler(service, &http_handler, (unsigned char *) "dispatch");
    V_ASSERT_NOT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "default");
    V_ASSERT_NOT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "file");
    V_ASSERT_NOT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "proxy");
    V_ASSERT_NOT_NULL(http_handler);

    /* runs a single iteration of the loop with a non blocking timeout,
    without it the epoll based provider would wait indefinitely as no
    connection is ever established during the test */
    service->polling->timeout = 0;
    error = poll_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    error = call_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* stops the service and verifies that the status has been
    changed, which is what breaks the main loop */
    error = stop_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(service->status == STATUS_CLOSED);

    /* closes the service releasing every structure created during
    the opening and then deletes the service itself */
    error = close_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* the closing unregisters every one of them again, which is what
    releases the structures the registration had created */
    service->get_http_handler(service, &http_handler, (unsigned char *) "dispatch");
    V_ASSERT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "default");
    V_ASSERT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "file");
    V_ASSERT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "proxy");
    V_ASSERT_NULL(http_handler);

    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_open_service_busy(void) {
    /* allocates space for the handler that is looked up so that a
    failed opening may be verified to have left nothing behind */
    struct http_handler_t *http_handler;

    /* allocates space for the error code and for the service that is
    going to fail the opening operation */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;

    /* creates the service and loads both the specifications and the
    default options into it */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    load_specifications(service);
    create_hash_map(&arguments, 0);
    _default_options_service(service, arguments);
    delete_hash_map(arguments);

    /* points the service at an address that is assigned to no interface
    so that the binding of the socket is guaranteed to fail */
    service->options->port = VIRIATUM_TEST_PORT;
    service->options->address = (unsigned char *) VIRIATUM_TEST_ADDRESS;
    service->options->load_modules = 0;
    service->options->workers = 0;
    service->options->ip6 = 0;
    calculate_options_service(service);
    calculate_locations_service(service);

    /* verifies that the opening failed and that the service has been
    left in the closed state with no socket handle set, otherwise the
    deletion of it would close a descriptor it no longer owns */
    error = open_service(service);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT(service->status == STATUS_CLOSED);
    V_ASSERT(service->service_socket_handle == 0);

    /* the handlers that the opening had already registered are
    unregistered again, the closing of a service is what balances
    them and it is never reached by one that failed to open */
    service->get_http_handler(service, &http_handler, (unsigned char *) "dispatch");
    V_ASSERT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "default");
    V_ASSERT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "file");
    V_ASSERT_NULL(http_handler);
    service->get_http_handler(service, &http_handler, (unsigned char *) "proxy");
    V_ASSERT_NULL(http_handler);

    /* deletes the service releasing every internal structure */
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_options_service(void) {
    /* allocates space for the error codes returned by both the
    options loading and the locations calculation, together with
    the service and the (empty) arguments map */
    ERROR_CODE error;
    ERROR_CODE locations_error;
    struct service_t *service;
    struct hash_map_t *arguments;

    /* allocates space for the observed service state, gathered
    before the working directory is restored */
    struct sort_map_t *configuration;
    size_t locations_count;

    /* allocates space for the working directory of the process
    so that it may be restored at the end of the test */
    char current_directory[VIRIATUM_MAX_PATH_SIZE];

    /* saves the current working directory and then changes into the
    root directory, where no relative configuration file is reachable */
    if(CURRENT_DIRECTORY(current_directory, VIRIATUM_MAX_PATH_SIZE) == NULL) {
        return "problem retrieving the current directory";
    }
    if(CHANGE_DIRECTORY(VIRIATUM_PATH_SEPARATOR) != 0) {
        return "problem changing to the root directory";
    }

    /* creates the service and the empty arguments map to be used
    in the loading of the file based options */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    create_hash_map(&arguments, 0);

    /* runs both consumers of the configuration map while no relative
    configuration file is reachable, the results are gathered so that
    the working directory may be restored before any assertion */
    error = _file_options_service(service, arguments);
    locations_error = calculate_locations_service(service);
    configuration = service->configuration;
    locations_count = service->locations.count;

    /* restores the working directory that was saved at the start of
    the test, keeping the remaining tests unaffected */
    if(CHANGE_DIRECTORY(current_directory) != 0) {
        return "problem restoring the current directory";
    }

    /* verifies that the absence of a configuration file is handled
    gracefully by both of the configuration map consumers */
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(!IS_ERROR_CODE(locations_error));

    /* in case no configuration file was reachable at all the
    configuration must be unset and no location may have been
    calculated, otherwise a system wide file has been loaded */
    if(configuration == NULL) { V_ASSERT(locations_count == 0); }

    /* deletes the arguments map and the service releasing every
    internal structure that has been created */
    delete_hash_map(arguments);
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_ran_service(void) {
    /* allocates space for the error code returned by the
    service stopping operation */
    ERROR_CODE error;

    /* stops the service while no service has been initialized,
    this should be a graceful no operation instead of an invalid
    access to the unset (null) service reference */
    error = ran_service();
    V_ASSERT(!IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_arguments_options_service(void) {
    /* allocates space for the service, for the map of the arguments
    that the command line produces and for one of them */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;
    struct argument_t argument;

    /* creates the service together with the empty map of the
    arguments, the options of it start at the default values */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    create_hash_map(&arguments, 0);

    /* the cleartext form of the most recent version of the protocol
    is served by default, no argument is required for it */
    V_ASSERT_EQ_U(service->options->http2, VIRIATUM_DEFAULT_HTTP2);

    /* an empty map leaves every one of the options at the value it
    already carries, nothing at all is overridden */
    error = _comand_line_options_service(service, arguments);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(service->options->http2, VIRIATUM_DEFAULT_HTTP2);

    /* the presence of the argument is what turns the serving of the
    cleartext form off, the value of it carries no meaning */
    argument.type = VALUE_ARGUMENT;
    SPRINTF(argument.key, sizeof(argument.key), "%s", "no-http2");
    argument.value[0] = '\0';
    set_value_string_hash_map(arguments, (unsigned char *) "no-http2", (void *) &argument);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(service->options->http2, 0);

    /* deletes the map of the arguments and the service, the options
    of it are released together with it */
    delete_hash_map(arguments);
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
const char *test_polling_service(void) {
    /* allocates space for the service whose specifications are going
    to be loaded and for the name of the mechanism it reports */
    struct service_t *service;
    const char *flags = VIRIATUM_FLAGS;

    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    load_specifications(service);

    /* the name is always one of the three, a service that reported
    nothing would leave the status page with an empty row where the
    mechanism in use is meant to be read */
    V_ASSERT_NOT_NULL(service->polling_name);
    V_ASSERT(
        strcmp((char *) service->polling_name, "epoll") == 0 ||
        strcmp((char *) service->polling_name, "kqueue") == 0 ||
        strcmp((char *) service->polling_name, "select") == 0
    );

    /* the name has to agree with the mechanism the build reached for,
    a page that named one while the service waited through another
    would be worse than one that named none at all */
#if defined(VIRIATUM_EPOLL)
    V_ASSERT_EQ_S((char *) service->polling_name, "epoll");
#elif defined(VIRIATUM_KQUEUE)
    V_ASSERT_EQ_S((char *) service->polling_name, "kqueue");
#else
    V_ASSERT_EQ_S((char *) service->polling_name, "select");
#endif

    /* the mechanism that scales with the connections shows up in the
    banner as well, and the one that is left to fall back on never
    does, so that the two of them are told apart at a glance */
#if defined(VIRIATUM_EPOLL)
    V_ASSERT_NOT_NULL(strstr(flags, "epoll"));
    V_ASSERT_NULL(strstr(flags, "kqueue"));
#elif defined(VIRIATUM_KQUEUE)
    V_ASSERT_NOT_NULL(strstr(flags, "kqueue"));
    V_ASSERT_NULL(strstr(flags, "epoll"));
#else
    V_ASSERT_NULL(strstr(flags, "epoll"));
    V_ASSERT_NULL(strstr(flags, "kqueue"));
#endif

    /* the description that a response carries is built out of the
    flags, so the mechanism travels with every one of them */
    V_ASSERT_NOT_NULL(strstr((char *) service->description, (char *) service->flags));

    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_flags_service(void) {
    /* allocates space for the chain of the connection and for the
    string of the flags that the banner of the startup carries */
    struct test_context_t *context;
    const char *flags = VIRIATUM_FLAGS;

    /* the safety of the threading is always one of the two values
    that the string carries, so it is never empty */
    V_ASSERT_NOT_NULL(flags);

    /* the transport only shows up in the banner when the build has
    actually been able to reach the library of it */
#ifdef VIRIATUM_SSL
    V_ASSERT_NOT_NULL(strstr(flags, "ssl"));
#else
    V_ASSERT_NULL(strstr(flags, "ssl"));
#endif

    /* the most recent version of the protocol shows up in the very
    same way, it is left out for the footprint of a smaller target */
#ifdef VIRIATUM_HTTP2
    V_ASSERT_NOT_NULL(strstr(flags, "http2"));
#else
    V_ASSERT_NULL(strstr(flags, "http2"));
#endif

    /* a connection only ever looks at the bytes that open it when
    the build carries the version they may belong to */
    create_test_context(&context);
    create_test_connection(context);
#ifdef VIRIATUM_HTTP2
    V_ASSERT(context->http_connection->detect == TRUE);
#else
    V_ASSERT(context->http_connection->detect == FALSE);
#endif
    delete_test_connection(context);

    /* the setting of the service decides it just the same, a service
    that turns it off never looks at those bytes either */
    context->options->http2 = 0;
    create_test_connection(context);
    V_ASSERT(context->http_connection->detect == FALSE);
    delete_test_connection(context);

    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
