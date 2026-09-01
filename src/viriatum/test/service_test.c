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
    V_ASSERT(service_options->default_virtual_host == NULL);
    V_ASSERT(service_options->index_count == 0);

    /* deletes the service options */
    delete_service_options(service_options);

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

/**
 * The path of the configuration file that the test writes so that
 * the reading of a named one may be observed, it is taken out of
 * the file system once the test is over.
 */
#define SERVICE_CONFIG_TEST_PATH "./viriatum_service_config.ini"

/**
 * The path that stands in for a configuration file that is not
 * around, no file is ever written under it.
 */
#define SERVICE_CONFIG_TEST_MISSING "./viriatum_service_config_missing.ini"

/**
 * The contents that the configuration file carries, a general
 * section naming a port that no default would ever produce.
 */
#define SERVICE_CONFIG_TEST_CONTENTS "[general]\nport = 19499\n"

/**
 * That very same port, kept apart so that the options the loading
 * produced may be verified against it.
 */
#define SERVICE_CONFIG_TEST_PORT 19499

/**
 * Runs the loading of the options that a configuration file carries
 * over a service of its own, so that each of the scenarios starts
 * from a service that carries nothing at all and the configuration
 * of the previous one is never left behind.
 *
 * @param arguments The map of the arguments of the command line.
 * @param port The pointer to the port the options ended up with.
 * @param configured The pointer to the flag that says whether a
 * configuration map was produced at all.
 * @return The resulting error code.
 */
static ERROR_CODE _config_options_service_test(
    struct hash_map_t *arguments,
    unsigned short *port,
    char *configured
) {
    ERROR_CODE error;
    struct service_t *service;

    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    _default_options_service(service, arguments);
    error = _file_options_service(service, arguments);

    /* gathers the state that is going to be verified before the
    service, and with it the configuration, is released */
    *port = service->options->port;
    *configured = service->configuration != NULL;
    delete_service(service);

    return error;
}

const char *test_config_options_service(void) {
    /* allocates space for the error code of the loading, for the
    state that is gathered out of each of the scenarios and for the
    two arguments that decide which file is read */
    ERROR_CODE error;
    unsigned short port;
    char configured;
    struct hash_map_t *arguments;
    struct argument_t config;
    struct argument_t no_config;

    /* writes the configuration file that is going to be named, the
    directory of the process is what the path is resolved against */
    write_file(
        (char *) SERVICE_CONFIG_TEST_PATH,
        (unsigned char *) SERVICE_CONFIG_TEST_CONTENTS,
        sizeof(SERVICE_CONFIG_TEST_CONTENTS) - 1
    );
    create_hash_map(&arguments, 0);

    /* the named configuration file is the only one that is read and
    the options of the service are taken out of it */
    config.type = VALUE_ARGUMENT;
    SPRINTF(config.key, sizeof(config.key), "%s", "config");
    SPRINTF(config.value, sizeof(config.value), "%s", SERVICE_CONFIG_TEST_PATH);
    set_value_string_hash_map(arguments, (unsigned char *) "config", (void *) &config);

    error = _config_options_service_test(arguments, &port, &configured);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(configured == TRUE);
    V_ASSERT_EQ_U(port, SERVICE_CONFIG_TEST_PORT);

    /* a named configuration file that is not around fails the loading
    outright, instead of falling back on one never asked for */
    SPRINTF(config.value, sizeof(config.value), "%s", SERVICE_CONFIG_TEST_MISSING);

    error = _config_options_service_test(arguments, &port, &configured);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT(configured == FALSE);

    /* the flag on its own names no file at all and so there is
    nothing that could be read out of it */
    config.type = SINGLE_ARGUMENT;

    error = _config_options_service_test(arguments, &port, &configured);
    V_ASSERT(IS_ERROR_CODE(error));

    /* the two of the flags ask for opposite things, one for a file to
    be read and the other for none, so neither may win quietly */
    no_config.type = SINGLE_ARGUMENT;
    SPRINTF(no_config.key, sizeof(no_config.key), "%s", "no-config");
    set_value_string_hash_map(arguments, (unsigned char *) "no-config", (void *) &no_config);

    error = _config_options_service_test(arguments, &port, &configured);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_S(
        (char *) get_last_error_message_safe(),
        "conflicting configuration: --config names a file, --no-config asks for none"
    );

    /* the discovery is skipped altogether when asked for, no
    configuration map being produced and the port remaining the one
    that the defaults carry */
    set_value_string_hash_map(arguments, (unsigned char *) "config", NULL);

    error = _config_options_service_test(arguments, &port, &configured);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(configured == FALSE);
    V_ASSERT_EQ_U(port, VIRIATUM_DEFAULT_PORT);

    /* deletes the map of the arguments and takes the configuration
    file that was written out of the file system */
    delete_hash_map(arguments);
    remove(SERVICE_CONFIG_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_target_options_service(void) {
    /* allocates space for the error code of the normalisation and
    for the three parts that a target is taken apart into */
    ERROR_CODE error;
    unsigned char module[VIRIATUM_MAX_PATH_SIZE];
    unsigned char attribute[VIRIATUM_MAX_PATH_SIZE];
    unsigned char path[VIRIATUM_MAX_PATH_SIZE];

    /* allocates space for a target longer than the buffers it would
    be taken apart into, used in the verification of the rejection */
    char long_target[VIRIATUM_MAX_PATH_SIZE + 2];

    /* the dotted spelling carries the attribute as the last of its
    segments and the module as everything that comes before it */
    error = _target_options_service("budy.App", module, attribute, path);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) module, "budy");
    V_ASSERT_EQ_S((char *) attribute, "App");
    V_ASSERT_EQ_S((char *) path, ".");

    /* the colon spelling, the one that both gunicorn and uvicorn
    take, divides the module from the attribute at the colon */
    error = _target_options_service("budy:app", module, attribute, path);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) module, "budy");
    V_ASSERT_EQ_S((char *) attribute, "app");
    V_ASSERT_EQ_S((char *) path, ".");

    /* a module of more than one segment keeps every one of them, the
    colon and not the dot being what divides the two parts */
    error = _target_options_service("budy.web.main:app", module, attribute, path);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) module, "budy.web.main");
    V_ASSERT_EQ_S((char *) attribute, "app");

    /* the file spelling names the file the module lives in, the
    directory of it being what has to be put on the import path */
    error = _target_options_service("./app.py:app", module, attribute, path);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) module, "app");
    V_ASSERT_EQ_S((char *) attribute, "app");
    V_ASSERT_EQ_S((char *) path, "./");

    /* a file named with no directory at all is looked for under the
    working directory, the same one a bare module is looked for in */
    error = _target_options_service("app.py:app", module, attribute, path);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) module, "app");
    V_ASSERT_EQ_S((char *) path, ".");

    /* a directory written with the separator of windows is taken
    apart the very same way as one written with the other */
    error = _target_options_service("srv\\app.py:app", module, attribute, path);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) module, "app");
    V_ASSERT_EQ_S((char *) path, "srv\\");

    /* a target that names no attribute at all leaves nothing to be
    loaded out of the module and so is refused */
    error = _target_options_service("budy", module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));
    error = _target_options_service("./app.py", module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));
    error = _target_options_service("budy:", module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a target that names no module is refused the same way, the
    attribute on its own naming nothing that may be imported */
    error = _target_options_service(":app", module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));

    /* neither an unset nor an empty target names anything at all */
    error = _target_options_service(NULL, module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));
    error = _target_options_service("", module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a target longer than the buffers is refused instead of being
    truncated into a module that was never named */
    memset(long_target, 'a', sizeof(long_target) - 1);
    long_target[sizeof(long_target) - 1] = '\0';
    error = _target_options_service(long_target, module, attribute, path);
    V_ASSERT(IS_ERROR_CODE(error));

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
const char *test_mode_options_service(void) {
    /* allocates space for the service, for the map of the arguments
    that the command line produces and for the ones of them through
    which what is served is selected */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;
    struct argument_t file;
    struct argument_t handler;
    struct argument_t application;

    /* creates the service together with the empty map of the
    arguments, the options of it start at the default values */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    create_hash_map(&arguments, 0);

    /* the file flag on its own selects the handler of the static
    files and serves them out of the working directory */
    file.type = SINGLE_ARGUMENT;
    SPRINTF(file.key, sizeof(file.key), "%s", "file");
    set_value_string_hash_map(arguments, (unsigned char *) "file", (void *) &file);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->handler_name, "file");
    V_ASSERT_EQ_S((char *) service->options->www_root, ".");

    /* the value that the flag carries names the root the files are
    served from, taking the place of the working directory */
    file.type = VALUE_ARGUMENT;
    SPRINTF(file.value, sizeof(file.value), "%s", "/srv/www");

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->www_root, "/srv/www");

    /* a handler flag asking for the very same handler is no conflict
    at all, the two of them agreeing on what is going to serve */
    handler.type = VALUE_ARGUMENT;
    SPRINTF(handler.key, sizeof(handler.key), "%s", "handler");
    SPRINTF(handler.value, sizeof(handler.value), "%s", "file");
    set_value_string_hash_map(arguments, (unsigned char *) "handler", (void *) &handler);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->handler_name, "file");

    /* a handler flag asking for another handler is in conflict with
    the mode flag and is rejected naming both of them, so that
    neither of the two wins over the other quietly */
    SPRINTF(handler.value, sizeof(handler.value), "%s", "proxy");

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_S(
        (char *) get_last_error_message_safe(),
        "conflicting handler: --file selects \"file\", --handler asks for \"proxy\""
    );

    /* the two flags are taken out of the map so that the modes which
    follow are verified on their own */
    set_value_string_hash_map(arguments, (unsigned char *) "file", NULL);
    set_value_string_hash_map(arguments, (unsigned char *) "handler", NULL);

    /* the asgi flag selects the handler that drives an application
    through the loop of events and records the target it named */
    application.type = VALUE_ARGUMENT;
    SPRINTF(application.key, sizeof(application.key), "%s", "asgi");
    SPRINTF(application.value, sizeof(application.value), "%s", "budy:app");
    set_value_string_hash_map(arguments, (unsigned char *) "asgi", (void *) &application);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT_EQ_S((char *) service->options->handler_name, "asgi");
    V_ASSERT_EQ_S((char *) service->options->target_module, "budy");
    V_ASSERT_EQ_S((char *) service->options->target_attribute, "app");
    V_ASSERT_EQ_S((char *) service->options->target_path, ".");

    /* a build that carries no python support reports the missing
    support instead of failing deep inside the resolution */
#ifdef VIRIATUM_PYTHON
    V_ASSERT(!IS_ERROR_CODE(error));
#else
    V_ASSERT(IS_ERROR_CODE(error));
#endif

    /* a target that names no attribute is refused before the support
    of the build is ever looked at */
    SPRINTF(application.value, sizeof(application.value), "%s", "budy");
    error = _comand_line_options_service(service, arguments);
    V_ASSERT(IS_ERROR_CODE(error));

    /* the flag on its own carries no application at all and so is
    refused naming the flag that was given */
    application.type = SINGLE_ARGUMENT;
    error = _comand_line_options_service(service, arguments);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_S(
        (char *) get_last_error_message_safe(),
        "--asgi requires an application target"
    );

    /* the wsgi flag selects the synchronous handler instead, the key
    of the argument being what tells the two of them apart */
    set_value_string_hash_map(arguments, (unsigned char *) "asgi", NULL);
    application.type = VALUE_ARGUMENT;
    SPRINTF(application.key, sizeof(application.key), "%s", "wsgi");
    SPRINTF(application.value, sizeof(application.value), "%s", "budy.App");
    set_value_string_hash_map(arguments, (unsigned char *) "wsgi", (void *) &application);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT_EQ_S((char *) service->options->handler_name, "python");
    V_ASSERT_EQ_S((char *) service->options->target_module, "budy");
    V_ASSERT_EQ_S((char *) service->options->target_attribute, "App");

    /* deletes the map of the arguments and the service, the options
    of it are released together with it */
    delete_hash_map(arguments);
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_bind_options_service(void) {
    /* allocates space for the service, for the map of the arguments
    that the command line produces and for the ones of them through
    which the place the service listens at is named */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;
    struct argument_t bind;
    struct argument_t port;
    struct argument_t host;

    /* creates the service together with the empty map of the
    arguments and loads the defaults, which are what the flags below
    are meant to be taking the place of */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    create_hash_map(&arguments, 0);
    _default_options_service(service, arguments);

    /* the value that names both the interface and the port sets the
    two of them at once, dividing at the colon between them */
    bind.type = VALUE_ARGUMENT;
    SPRINTF(bind.key, sizeof(bind.key), "%s", "bind");
    SPRINTF(bind.value, sizeof(bind.value), "%s", "127.0.0.1:8080");
    set_value_string_hash_map(arguments, (unsigned char *) "bind", (void *) &bind);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->address, "127.0.0.1");
    V_ASSERT_EQ_U(service->options->port, 8080);

    /* the value that names no interface stands for every one of them,
    the address being left at the one the defaults carry */
    SPRINTF(bind.value, sizeof(bind.value), "%s", ":8081");

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->address, VIRIATUM_DEFAULT_HOST);
    V_ASSERT_EQ_U(service->options->port, 8081);

    /* an ip6 address written between brackets is divided at the last
    of its colons and not at one of the ones it carries itself */
    SPRINTF(bind.value, sizeof(bind.value), "%s", "[::1]:8082");

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->address, "[::1]");
    V_ASSERT_EQ_U(service->options->port, 8082);

    /* a value carrying no colon at all names no port and so there is
    nothing that could be bound out of it */
    SPRINTF(bind.value, sizeof(bind.value), "%s", "127.0.0.1");

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(IS_ERROR_CODE(error));

    /* the flag on its own names neither of the two */
    bind.type = SINGLE_ARGUMENT;

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_S(
        (char *) get_last_error_message_safe(),
        "--bind requires a host and a port"
    );

    /* the short forms name the same things the long ones do, and are
    read after the value that names the two of them together, so that
    the narrower of the flags is the one that decides */
    set_value_string_hash_map(arguments, (unsigned char *) "bind", NULL);
    port.type = VALUE_ARGUMENT;
    SPRINTF(port.key, sizeof(port.key), "%s", "p");
    SPRINTF(port.value, sizeof(port.value), "%s", "8083");
    set_value_string_hash_map(arguments, (unsigned char *) "p", (void *) &port);
    host.type = VALUE_ARGUMENT;
    SPRINTF(host.key, sizeof(host.key), "%s", "h");
    SPRINTF(host.value, sizeof(host.value), "%s", "127.0.0.2");
    set_value_string_hash_map(arguments, (unsigned char *) "h", (void *) &host);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_S((char *) service->options->address, "127.0.0.2");
    V_ASSERT_EQ_U(service->options->port, 8083);

    /* the long forms take the place of the short ones whenever both
    of them are given, they are the ones looked for first */
    port.type = VALUE_ARGUMENT;
    SPRINTF(port.key, sizeof(port.key), "%s", "port");
    SPRINTF(port.value, sizeof(port.value), "%s", "8084");
    set_value_string_hash_map(arguments, (unsigned char *) "port", (void *) &port);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->port, 8084);

    /* deletes the map of the arguments and the service, the options
    of it are released together with it */
    delete_hash_map(arguments);
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_static_options_service(void) {
    /* allocates space for the service, for the map of the arguments
    that the command line produces and for the ones of them that
    qualify the serving of the static files */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;
    struct argument_t index;
    struct argument_t listing;
    struct argument_t no_listing;
    struct argument_t spa;
    struct argument_t cors;

    /* creates the service together with the empty map of the
    arguments and loads the defaults into it */
    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    create_hash_map(&arguments, 0);
    _default_options_service(service, arguments);

    /* the listing of a directory is produced by default and neither
    the routing of an application nor the cross origin fields are */
    V_ASSERT_EQ_U(service->options->listing, 1);
    V_ASSERT_EQ_U(service->options->spa, 0);
    V_ASSERT_EQ_U(service->options->cors, 0);
    V_ASSERT_EQ_U(service->options->index_count, 0);

    /* the value of the index flag is divided around the space, so that
    more than one name may be given the way the configuration does */
    index.type = VALUE_ARGUMENT;
    SPRINTF(index.key, sizeof(index.key), "%s", "index");
    SPRINTF(index.value, sizeof(index.value), "%s", "app.html index.html");
    set_value_string_hash_map(arguments, (unsigned char *) "index", (void *) &index);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->index_count, 2);
    V_ASSERT_EQ_S((char *) service->options->index[0], "app.html");
    V_ASSERT_EQ_S((char *) service->options->index[1], "index.html");

    /* the negative form of the listing flag turns the producing of one
    off and the positive one turns it back on */
    no_listing.type = SINGLE_ARGUMENT;
    SPRINTF(no_listing.key, sizeof(no_listing.key), "%s", "no-listing");
    set_value_string_hash_map(arguments, (unsigned char *) "no-listing", (void *) &no_listing);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->listing, 0);

    /* the negative form is read last and so it is the one that decides
    whenever both of them are given */
    listing.type = SINGLE_ARGUMENT;
    SPRINTF(listing.key, sizeof(listing.key), "%s", "listing");
    set_value_string_hash_map(arguments, (unsigned char *) "listing", (void *) &listing);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->listing, 0);

    /* on its own the positive form turns the producing of the listing
    back on, which is the value the defaults already carry */
    set_value_string_hash_map(arguments, (unsigned char *) "no-listing", NULL);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->listing, 1);

    /* both of the remaining flags are turned on by their presence
    alone, neither of them carrying a value of its own */
    spa.type = SINGLE_ARGUMENT;
    SPRINTF(spa.key, sizeof(spa.key), "%s", "spa");
    set_value_string_hash_map(arguments, (unsigned char *) "spa", (void *) &spa);
    cors.type = SINGLE_ARGUMENT;
    SPRINTF(cors.key, sizeof(cors.key), "%s", "cors");
    set_value_string_hash_map(arguments, (unsigned char *) "cors", (void *) &cors);

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->spa, 1);
    V_ASSERT_EQ_U(service->options->cors, 1);

    /* the routing of an application is decided by an index file, so one
    is named whenever nothing else has named any, the flag would
    otherwise have nothing at all to fall back on */
    set_value_string_hash_map(arguments, (unsigned char *) "index", NULL);
    service->options->index_count = 0;

    error = _comand_line_options_service(service, arguments);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(service->options->index_count, 1);
    V_ASSERT_EQ_S((char *) service->options->index[0], VIRIATUM_DEFAULT_INDEX_FILE);

    /* deletes the map of the arguments and the service, the options
    of it are released together with it */
    delete_hash_map(arguments);
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_ephemeral_service(void) {
    /* allocates space for the error codes of the lifecycle calls, for
    the service and for the port that was bound */
    ERROR_CODE error;
    struct service_t *service;
    struct hash_map_t *arguments;
    unsigned short port;

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

    /* asks for an ephemeral port, which is the one the kernel picks
    at the binding instead of one that was named */
    service->options->port = 0;
    service->options->load_modules = 0;
    service->options->workers = 0;
    service->options->ip6 = 0;
    calculate_options_service(service);
    calculate_locations_service(service);

    /* opens the service, which binds the socket and reads the port
    that was given back off it */
    error = open_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));
    port = service->options->port;

    /* closes the service before anything is verified, so that the
    socket is never left behind by a failing assertion */
    error = close_service(service);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* the port that was bound is the one the options now carry, a
    port of zero would leave nothing at all able to name it */
    V_ASSERT(port != 0);

    /* the string of the port agrees with it, that is the one the ip6
    binding and the responses of the service are going to read */
    V_ASSERT_EQ_U((unsigned short) atoi((char *) service->options->_port), port);

    /* deletes the service releasing every internal structure */
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
