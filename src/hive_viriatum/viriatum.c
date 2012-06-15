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

#include "viriatum.h"

#define HELP_STRING "\
usage: viriatum [--version] [--port[=<port>] [--host[=<hostname>]]\n\
                [--daemon] [--help]\n\
\n\
The most commonly used git commands are:\n\
   --version  Prints the current version\n\
   --port     Sets the tcp port to be used as primary\n\
   --host     Sets the tcp host to bind\n\
   --host     Runs the service as daemon (background)\n\
   --host     Prints this (help) message\n\
\n\
See 'viriatum --help[=<command>]' for more information on a specific command.\n"

/* starts the memory structures */
START_MEMORY;

unsigned char local = 0;
static struct service_t *service;

ERROR_CODE run_service(char *program_name, struct hash_map_t *arguments) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates the socket data */
    SOCKET_DATA socket_data;

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socket_data);

    /* creates the service and loads the options
    taking into account the arguments */
    create_service(&service, (unsigned char *) VIRIATUM_NAME, (unsigned char *) program_name);
    load_options_service(service, arguments);
    calculate_options_service(service);

    /* starts the service */
    return_value = start_service(service);

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
         /* runs the socket finish */
        SOCKET_FINISH();

        /* raises the error again */
        RAISE_AGAIN(return_value);
    }

    /* deletes the service */
    delete_service(service);

    /* runs the socket finish */
    SOCKET_FINISH();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE ran_service() {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* in case the service status is open */
    if(service->status == STATUS_CLOSED) {
        /* prints a warning message */
        V_DEBUG("No service to be stopped\n");
    } else {
        /* prints a warning message */
        V_DEBUG("Stopping service\n");

        /* stops the service */
        return_value = stop_service(service);

        /* tests the error code for error */
        if(IS_ERROR_CODE(return_value)) {
            /* runs the socket finish */
            SOCKET_FINISH();

            /* raises the error again */
            RAISE_AGAIN(return_value);
        }

        /* prints a warning message */
        V_DEBUG("Finished stopping service\n");
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

void kill_handler(int signal_number) {
    /* defaults the signal handler (only one handling) */
    signal(signal_number, SIG_DFL);

    /* runs the "ran" service */
    ran_service();
}

ERROR_CODE register_signals() {
    /* registers the kill handler for the various signals
    associated with the "destroy" operation */
    signal(SIGHUP, kill_handler);
    signal(SIGINT, kill_handler);
    signal(SIGQUIT, kill_handler);
    signal(SIGTERM, kill_handler);

	/* registers the ignore action in the signal indicating
    a broken pipe (unexpected close of socket) */
    signal(SIGPIPE, SIG_IGN);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE print_information() {
    /* retrieves the viriatum version and description */
    unsigned char *version = version_viriatum();
    unsigned char *description = description_viriatum();

    /* prints a message */
    V_PRINT_F("%s %s (%s, %s) [%s %s %d bit (%s)] on %s\n", description, version, VIRIATUM_COMPILATION_DATE, VIRIATUM_COMPILATION_TIME, VIRIATUM_COMPILER, VIRIATUM_COMPILER_VERSION_STRING, (int) VIRIATUM_PLATFORM_CPU_BITS, VIRIATUM_PLATFORM_CPU, VIRIATUM_PLATFORM_STRING);

    /* prints a message */
    V_PRINT_F("%s\n", VIRIATUM_COPYRIGHT);

    /* raises no error */
    RAISE_NO_ERROR;
}

void help() { V_PRINT(HELP_STRING); }
void version() { V_PRINT_F("%s - %s (%s, %s)\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_COMPILATION_DATE, VIRIATUM_COMPILATION_TIME); }

#ifdef VIRIATUM_PLATFORM_WIN32
void daemonize() { }
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
	of the output stream and the log file */
	int log_file;
	int out_file;

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

	/* opens the log file and redirect the standard output stream
	into it so that every log message is sent there */
	log_file = open("/viriatum.log", O_CREAT | O_WRONLY | O_APPEND, 0666);
	out_file = dup(STDOUT_FILENO);
	dup2(log_file, STDOUT_FILENO);
	close(log_file);

	printf("TOBIAS\n");
	fflush(stdout);

    /* closes the various pending streams from the
    daemon process (not going to output them) */
    /*close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);*/
}
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
void localize() { local = 1; }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
void localize() { }
#endif

void execute_arguments(struct hash_map_t *arguments) {
    /* allocates space for the possible argument
    to be executed from the arguments map */
    void *value;

    /* tries to retrieve the help argument from the arguments
    map in case the value exists prints the help value and then
    exits the current system */
    get_value_string_hash_map(arguments, (unsigned char *) "help", &value);
    if(value != NULL) { help(); exit(0); }

    /* tries to retrieve the version argument from the arguments
    map in case the value exists prints the version value and then
    exits the current system */
    get_value_string_hash_map(arguments, (unsigned char *) "version", &value);
    if(value != NULL) { version(); exit(0); }

    /* tries to retrieve the daemon argument from the
    arguments map in case the value is set daemonizes
    the current process so that it remains in background
    and returns to the caller process immediately, otherwise
    prints the viriatum information into the standard
    output "file", the label should be standard */
    get_value_string_hash_map(arguments, (unsigned char *) "daemon", &value);
    if(value != NULL) { daemonize(); }
    else { print_information(); }

	/* registers the various kill signal handlers so that
	in case such event happens it's possible to correctly
	"destroy" the various internal structures and processes */
	register_signals();

    /* tries to retrieve the local argument from the arguments
    map in case the value exists localizes the current service
    so that any file read is read from the current directory */
    get_value_string_hash_map(arguments, (unsigned char *) "local", &value);
    if(value != NULL) { localize(); }
}

void cleanup() {
    /* prints a debug message */
    V_DEBUG("Cleaning the process information\n");

    /* removes the viriatum pid path, so that the daemon
    watching tool are notified that the process is no
    longer running in the current environment */
    remove(VIRIATUM_PID_PATH);
}

#ifndef VIRIATUM_PLATFORM_IPHONE
int main(int argc, char *argv[]) {
    /* allocates space for the name of the program
    (process) to be executed */
    char *program_name;

    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates the map that will contain the various
    processed arguments, indexed by name */
    struct hash_map_t *arguments;

    /* prints a debug message */
    V_DEBUG_F("Receiving %d argument(s)\n", argc);

    /* in case the number of arguments is less than one
    (exception case) returns in error */
    if(argc < 1) { cleanup(); return -1; }

    /* retrieves the first argument value as the name
    of the process (program) to be executed */
    program_name = argv[0];

    /* processes the various arguments into a map and then
    executes the corresponding (initial) actions */
    process_arguments(argc, argv, &arguments);
    execute_arguments(arguments);

    /* runs the service, with the given arguments */
    return_value = run_service(program_name, arguments);

    /* deletes the processed arguments */
    delete_arguments(arguments);

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message */
        V_ERROR_F("Problem running service (%s)\n", (char *) GET_ERROR());
    }

    /* cleans the current process information so that
    no remaining structure or resource is left in an
    invalid or erroneous state */
    cleanup();

    /* prints a debug message */
    V_DEBUG("Finishing process\n");

    /* returns zero (valid) */
    return 0;
}
#endif
