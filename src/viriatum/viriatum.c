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

#include "viriatum.h"

/* starts the memory structures */
START_MEMORY;

unsigned char local = 0;
static struct service_t *service = NULL;

ERROR_CODE init_service(char *program_name, struct hash_map_t *arguments) {
    /* allocates the return value to be used to gather
    the error result from the service calls */
    ERROR_CODE return_value;

    /* creates the service and loads the options
    taking into account the arguments */
    create_service(
        &service,
        (unsigned char *) VIRIATUM_NAME,
        (unsigned char *) program_name
    );
    return_value = load_specifications(service);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    return_value = load_options_service(service, arguments);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    return_value = calculate_options_service(service);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* updates the registers signals handler so that the service
    may be able to register the handlers at the proper timing */
    service->register_signals = register_signals;

    /* calculates the locations structure for the service based
    on the currently loaded configuration, this a complex operation */
    calculate_locations_service(service);

    /* runs the printing operation on the service, this should
    output the information to the standard output */
    print_options_service(service);
    debug_options_service(service);

    /* raises no error to the caller method, normal
    exit operation (should provide no problem) */
    RAISE_NO_ERROR;
}

ERROR_CODE destroy_service(void) {
    /* prints a debug message about the initial stage
    of the service structures destruction */
    V_DEBUG("Destroying the service structures\n");

    /* deletes the service, disallowing any further
    access to the service instance, and then sets its
    reference back to the original (unset sate) */
    delete_service(service);
    service = NULL;

    /* prints a debug message about the final stage
    of the service structures destruction */
    V_DEBUG("Finished destroying the service structures\n");

    /* raises no error to the caller method, normal
    exit operation (should provide no problem) */
    RAISE_NO_ERROR;
}

ERROR_CODE run_service(void) {
    /* allocates the return value to be used to gather
    the error result from the service calls */
    ERROR_CODE return_value;

    /* allocates the socket data and then initializes
    the socket infrastructure (global structures) with it */
    SOCKET_DATA socket_data;
    SOCKET_INITIALIZE(&socket_data);

    /* starts the service, this call should be able to bootstrap
    all the required structures and initialize the main loop, this
    should block the control flow during the run of the service */
    return_value = start_service(service);

    /* tests the error code value for error and in case there's
    one runs the appropriate measures */
    if(IS_ERROR_CODE(return_value)) {
        /* runs the socket finish so that the proper cleanup
        operations are performed and then re-raises the error*/
        SOCKET_FINISH();
        RAISE_AGAIN(return_value);
    }

    /* runs the socket finish releasing any pending memory information
    regarding the socket infra-structure */
    SOCKET_FINISH();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE run_service_s(char *program_name, struct hash_map_t *arguments) {
    /* allocates space for the error value that will be used
    to check for an error in the call */
    ERROR_CODE return_value;

    /* initializes the service creating the structures and starting
    the values for the configuration of it */
    return_value = init_service(program_name, arguments);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* run the service, blocking the call until the service is
    finished, the retrieves the return value from it */
    return_value = run_service();
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* destroys the service eliminating any structures that have
    been created in the service life-time */
    return_value = destroy_service();
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* raises no error as the execution of the service went normally
    and no problems have been issued */
    RAISE_NO_ERROR;
}

ERROR_CODE check_service_s(char *program_name, struct hash_map_t *arguments, char print) {
    /* allocates space for the error value that will be used
    to check for an error in the call */
    ERROR_CODE return_value;

    /* initializes the service the very same way a run of it would, so
    that whatever would fail a start fails this instead, and then tears
    it back down again without ever reaching for a socket */
    return_value = init_service(program_name, arguments);
    if(IS_ERROR_CODE(return_value)) {
        destroy_service();
        RAISE_AGAIN(return_value);
    }

    /* writes the configuration that the merging of the three layers
    produced, whenever the writing of it is what was asked for */
    if(print == TRUE) { print_config_service(service); }

    /* destroys the service eliminating any structures that have
    been created while it was being initialized */
    return_value = destroy_service();
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* raises no error as everything the service is made of was loaded
    and validated without a single problem */
    RAISE_NO_ERROR;
}

ERROR_CODE handlers_service_s(char *program_name, struct hash_map_t *arguments) {
    /* allocates space for the error value that will be used
    to check for an error in the call */
    ERROR_CODE return_value;

    /* initializes the service so that the modules path it carries is
    the resolved one, the handlers of the modules are only reachable
    once that path is known */
    return_value = init_service(program_name, arguments);
    if(IS_ERROR_CODE(return_value)) {
        destroy_service();
        RAISE_AGAIN(return_value);
    }

    /* writes the handlers that this build carries, the ones of the
    service itself together with the ones of the modules */
    list_handlers_service(service);

    /* destroys the service eliminating any structures that have
    been created while it was being initialized */
    return_value = destroy_service();
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE ran_service(void) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* in case there's no service set or the service has already been
    closed there's nothing to be stopped (graceful return) */
    if(service == NULL || service->status == STATUS_CLOSED) {
        /* prints a debug message */
        V_DEBUG("No service to be stopped\n");
    } else {
        /* prints a debug message */
        V_DEBUG("Stopping service\n");

        /* stops the service, this call should make the
        required changes in the service structure so that
        it's stopped as soon as possible */
        return_value = stop_service(service);

        /* tests the error code for error */
        if(IS_ERROR_CODE(return_value)) {
            /* runs the socket finish so that the proper cleanup
            operations are performed and then re-raises the error*/
            SOCKET_FINISH();
            RAISE_AGAIN(return_value);
        }

        /* prints a debug message */
        V_DEBUG("Finished stopping service\n");
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE pointer_service(struct service_t **service_pointer) {
    *service_pointer = service;
    RAISE_NO_ERROR;
}

void kill_handler(int signal_number) {
    /* defaults the signal handler (only one handling) */
    signal(signal_number, SIG_DFL);

    /* runs the "ran" service */
    ran_service();
}

void ignore_handler(int signal_number) {
}

void register_signals(void) {
    /* registers the kill handler for the various signals
    associated with the "destroy" operation */
    signal(SIGHUP, kill_handler);
    signal(SIGINT, kill_handler);
    signal(SIGQUIT, kill_handler);
    signal(SIGTERM, kill_handler);

    /* registers the ignore action in the signal indicating
    a broken pipe (unexpected close of socket) */
    signal(SIGPIPE, SIG_IGN);
}
