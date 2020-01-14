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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "viriatum.h"

#define HELP_STRING "\
usage: viriatum [--version] [--port[=<port>] [--host[=<hostname>]] [-ip6]\n\
                [handler[=<name>]] [workers[=<count>]] [--local] [--ssl]\n\
                [--daemon] [--test] [--speed] [--help]\n\
\n\
The most commonly used viriatum commands are:\n\
   --version  Prints the current version\n\
   --port     Sets the tcp port to be used as primary\n\
   --host     Sets the tcp host to bind\n\
   --ip6      Runs the service with support for ipv6\n\
   --handler  Name of the handler to be used as default\n\
   --workers  Defines the ammount of worker to be used\n\
   --local    Runs the service in local mode no internet support\n\
   --ssl      Listens to the sockets using ssl encryption\n\
   --daemon   Runs the service as daemon (background)\n\
   --test     Runs a series of test for viriatum\n\
   --speed    Runs a series of speed relates tests for viriatum\n\
   --help     Prints this (help) message\n\
\n\
See 'viriatum --help[=<command>]' for more information on a specific command.\n"

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
    output the information to the standar output */
    print_options_service(service);

    /* raises no error to the caller method, normal
    exit operation (should provide no problem) */
    RAISE_NO_ERROR;
}

ERROR_CODE destroy_service() {
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

ERROR_CODE run_service() {
    /* allocates the return value to be used to gather
    the error result from the service calls */
    ERROR_CODE return_value;

    /* allocates the socket data and then initializes
    the socket infrastructure (global structures) with it */
    SOCKET_DATA socket_data;
    SOCKET_INITIALIZE(&socket_data);

    /* starts the service, this call should be able to bootstrap
    all the required structures and initialize the main loop, this
    should block the control flow fduring the run of the service */
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
    finished, the retrives the return value from it */
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

ERROR_CODE ran_service() {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* in case the service status is open */
    if(service->status == STATUS_CLOSED) {
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

void register_signals() {
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

ERROR_CODE print_information() {
    /* retrieves the viriatum version and description */
    unsigned char *version = version_viriatum();
    unsigned char *description = description_viriatum();

    /* prints a message */
    V_PRINT_F(
        "%s %s (%s, %s) [%s %s %d bit (%s)] %s\n",
        description,
        version,
        VIRIATUM_COMPILATION_DATE,
        VIRIATUM_COMPILATION_TIME,
        VIRIATUM_COMPILER,
        VIRIATUM_COMPILER_VERSION_STRING,
        (int) VIRIATUM_PLATFORM_CPU_BITS,
        VIRIATUM_PLATFORM_CPU,
        VIRIATUM_PLATFORM_STRING
    );

    /* prints a message */
    V_PRINT_F("%s\n", VIRIATUM_COPYRIGHT);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE help() { V_PRINT(HELP_STRING); RAISE_NO_ERROR; }
ERROR_CODE version() {
    V_PRINT_F(
        "%s - %s (%s, %s)\n",
        VIRIATUM_NAME,
        VIRIATUM_VERSION,
        VIRIATUM_COMPILATION_DATE,
        VIRIATUM_COMPILATION_TIME
    ); RAISE_NO_ERROR;
}
ERROR_CODE test() { return run_simple_tests(); }
ERROR_CODE speed() { return run_speed_tests(); }

#ifdef VIRIATUM_PLATFORM_WIN32
void daemonize() { }
void daemonclean() { }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
void daemonize() {
    /* allocates space for the various dameon
    related variables */
    PID_TYPE pid;
    PID_TYPE sid;
    FILE *pid_file;
    char pid_string[1024];
    size_t pid_string_length;

    /* allocates space for the file descriptors
    to be used to redirect the default stream */
    int log_file;

    /* forks off the parent process, this
    is the main trick in the process */
    pid = fork();

    /* checks if the pid is invalid in case
    it's exits the parent process in error */
    if(pid < 0) { exit(EXIT_FAILURE); }
    /* checks if the pid of the parent
    process is good in case it's can exit
    the parent process */
    if(pid > 0) { exit(EXIT_SUCCESS); }

    /* changes the file mode mask */
    umask(0);

    /* create a new sid for the child process and then
    verifies if it has been successfull */
    sid = setsid();
    if(sid < 0) { exit(EXIT_FAILURE); }

    /* changes the current working directory to the
    base of the file system */
    if(chdir("/") < 0) { exit(EXIT_FAILURE); }

    /* retrieves the pid of the current process this
    must be called because the current pid value is invalid */
    pid = GET_PID();

    /* opens the pid file and writes the pid stirng into it
    this will allow external programs to make sure viriatum
    is correctly running */
    FOPEN(&pid_file, VIRIATUM_PID_PATH, "wb");
    SPRINTF(pid_string, 1024, "%d\n", pid);
    pid_string_length = strlen(pid_string);
    fwrite(pid_string, sizeof(char), pid_string_length, pid_file);
    fclose(pid_file);

    /* opens the log file and redirects the standard output stream
    into it so that every log message is sent there */
    log_file = open(VIRIATUM_LOG_PATH, O_CREAT | O_WRONLY | O_APPEND, 0640);
    dup2(log_file, STDOUT_FILENO);
    close(log_file);

    /* opens the error file and redirects the standard error stream
    into it so that every error message is sent there */
    log_file = open(VIRIATUM_LOG_E_PATH, O_CREAT | O_WRONLY | O_APPEND, 0640);
    dup2(log_file, STDERR_FILENO);
    close(log_file);

    /* closes the various pending streams from the
    daemon process (not going to output them) */
    close(STDIN_FILENO);
}
void daemonclean() {
    /* removes the viriatum pid path, so that the daemon
    watching tool are notified that the process is no
    longer running in the current environment */
    remove(VIRIATUM_PID_PATH);
}
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
void localize() { local = 1; }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
void localize() { }
#endif

int execute_arguments(char *program_name, struct hash_map_t *arguments) {
    /* allocates space for the possible argument
    to be executed from the arguments map */
    void *value;

    /* allocates the value to be used to verify the
    exitence of error from the function */
    ERROR_CODE return_value;

    /* sets space for the flag that will control if
    the service should be run or not, this is used
    for certain situations (mostyle test) where the
    service is not meant to be run */
    char run_service = TRUE;

    /* tries to retrieve the help argument from the arguments
    map in case the value exists prints the help value and then
    exits the current system */
    get_value_string_hash_map(arguments, (unsigned char *) "help", &value);
    if(value != NULL) { return help(); }

    /* tries to retrieve the version argument from the arguments
    map in case the value exists prints the version value and then
    exits the current system */
    get_value_string_hash_map(arguments, (unsigned char *) "version", &value);
    if(value != NULL) { return version(); }

    /* retrieves the test argument value from the arguments map
    and in case it's set starts the test process runing a series
    of test functions in sequence */
    get_value_string_hash_map(arguments, (unsigned char *) "test", &value);
    if(value != NULL) { return test(); }

    /* retrieves the speed argument value from the arguments map
    and in case it's set starts the speed measuring and disables
    the runnig of the service */
    get_value_string_hash_map(arguments, (unsigned char *) "speed", &value);
    if(value != NULL) { return speed(); }

    /* tries to retrieve the daemon argument from the
    arguments map in case the value is set daemonizes
    the current process so that it remains in background
    and returns to the caller process immediately, otherwise
    prints the viriatum information into the standard
    output "file", the label should be standard */
    get_value_string_hash_map(arguments, (unsigned char *) "daemon", &value);
    if(value != NULL) { daemonize(); }
    else { print_information(); }

    /* tries to retrieve the local argument from the arguments
    map in case the value exists localizes the current service
    so that any file read is read from the current directory */
    get_value_string_hash_map(arguments, (unsigned char *) "local", &value);
    if(value != NULL) { localize(); }

    /* in cas the flag that control if the service must be run is
    unset the control flow must be returned immediately (avoids
    running service) */
    if(run_service == FALSE) { RAISE_NO_ERROR; }

    /* runs the service, with the given arguments, this call
    should block the program control flow until an event
    stop the running of the main loop */
    return_value = run_service_s(program_name, arguments);

    /* tests the error code for error in case it exists
    prints a message indicating the problem that occurred */
    if(IS_ERROR_CODE(return_value)) {
        V_ERROR_F("Problem running service (%s)\n", (char *) GET_ERROR());
        RAISE_AGAIN(return_value);
    }

    /* returns the normal result value as no problems has
    occured during the execution of the command */
    RAISE_NO_ERROR;
}

void cleanup(struct hash_map_t *arguments) {
    /* allocates space for the possible argument
    to be executed from the arguments map */
    void *value;

    /* prints a debug message */
    V_DEBUG("Cleaning the process information\n");

    /* in case no arguments map is provided must return
    immediately nothing left to be processed */
    if(arguments == NULL) { return; }

    /* tries to retrieve the daemon argument from the
    arguments map in case the value is set daemonclean
    the current process so that no structures remaining
    from the daemon process are left */
    get_value_string_hash_map(arguments, (unsigned char *) "daemon", &value);
    if(value != NULL) { daemonclean(); }
}

#ifndef VIRIATUM_PLATFORM_IPHONE
int main(int argc, char *argv[]) {
    /* allocates the space for the "final" result code
    that is going to be returned as part of the normal
    command execution, a positive or negative values
    should indicate an error, a zero value indicates that
    a normal execution has just finished */
    ERROR_CODE return_value;

    /* allocates space for the name of the program
    (process) to be executed */
    char *program_name;

    /* allocates the map that will contain the various
    processed arguments, indexed by name */
    struct hash_map_t *arguments;

    /* prints a debug message */
    V_DEBUG_F("Receiving %d argument(s)\n", argc);

    /* in case the number of arguments is less than one
    (exception case) returns in error */
    if(argc < 1) { cleanup(NULL); RAISE_ERROR_S(1); }

    /* retrieves the first argument value as the name
    of the process (program) to be executed */
    program_name = argv[0];

    /* processes the various arguments into a map and then
    executes the corresponding (initial) actions */
    process_arguments(argc, argv, &arguments);
    return_value = execute_arguments(program_name, arguments);

    /* cleans the current process information so that
    no remaining structure or resource is left in an
    invalid or erroneous state */
    cleanup(arguments);

    /* deletes the processed arguments and then cleans up
    the pool based memory allocation system releasing all
    of its memory before the exit (no leaks) */
    delete_arguments(arguments);
    cleanup_palloc();

    /* prints a debug message about the ending of the sytem
    for the execution of the service and then returns the
    normal return code (success status) to the caller process */
    V_DEBUG_F(
        "Finishing process [%ld pending]\n",
        (long int) ALLOCATIONS
    );
    RAISE_AGAIN(return_value);
}
#endif
