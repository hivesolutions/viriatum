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

#define HELP_STRING "\
usage: viriatum [--file[=<path>]] [--asgi=<target>] [--wsgi=<target>]\n\
                [--bind=<host>:<port>] [-p=<port>] [-h=<host>] [--ip6]\n\
                [--config=<path>] [--no-config] [--handler=<name>]\n\
                [--index=<file>] [--listing] [--no-listing] [--spa] [--cors]\n\
                [--workers=<count>] [--wwwroot=<path>] [--local] [--ssl]\n\
                [--no-http2] [--template] [--no-template] [--access-log]\n\
                [--no-access-log] [-v] [-q]\n\
                [--dev] [--daemon] [--print-config] [--check]\n\
                [--list-handlers] [--version] [--info] [--test] [--speed]\n\
                [--help]\n\
\n\
Every flag that carries a value takes it after an equals sign, there is no\n\
positional argument and no sub command.\n\
\n\
What is going to be served:\n\
   --file      Serves the files of a directory, the working one by default\n\
   --asgi      Serves the application of a target through a loop of events\n\
   --wsgi      Serves the application of a target synchronously\n\
   --handler   Name of the handler to be used as default\n\
   --wwwroot   Sets the root directory from which static files are served\n\
\n\
Both --asgi and --wsgi need a build carrying python support, and name the\n\
application as 'module:attribute', 'module.attribute' or 'file.py:attribute'.\n\
\n\
Where it is going to listen:\n\
   --bind      Sets the host and the port together, ':8080' standing for\n\
                 every interface and a port of '0' for one the system picks\n\
   --port, -p  Sets the tcp port to be used as primary\n\
   --host, -h  Sets the tcp host to bind\n\
   --ip6       Runs the service with support for ipv6\n\
   --ssl       Listens to the sockets using ssl encryption\n\
   --no-http2  Serves only the previous version of the http protocol\n\
\n\
Which configuration file is read:\n\
   --config     Reads the named configuration file and no other one\n\
   --no-config  Reads none of them, the defaults and these flags decide\n\
\n\
With neither of them the first of the following that is around is read:\n\
   %s" VIRIATUM_PATH_SEPARATOR "viriatum.ini\n\
   ." VIRIATUM_PATH_SEPARATOR "viriatum.ini\n\
   ." VIRIATUM_PATH_SEPARATOR "src" VIRIATUM_PATH_SEPARATOR "viriatum" VIRIATUM_PATH_SEPARATOR "resources" VIRIATUM_PATH_SEPARATOR "config" VIRIATUM_PATH_SEPARATOR "viriatum" VIRIATUM_PATH_SEPARATOR "viriatum.ini\n\
\n\
How the static files are served:\n\
   --index       Names the file that answers for a directory\n\
   --listing     Produces the listing of a directory carrying no index\n\
   --no-listing  Answers such a directory with an error instead\n\
   --spa         Serves the index file for a path that resolves to nothing\n\
   --cors        Puts the permissive cross origin fields on a response\n\
\n\
What is written and how it runs:\n\
   -v               Writes the messages of the debugging as well\n\
   -q               Writes only the messages that report a problem\n\
   --template       Sends the error pages through the template engine\n\
   --no-template    Sends the plain ones instead\n\
   --access-log     Writes a line for each one of the requests\n\
   --no-access-log  Writes none of those lines\n\
   --dev            Turns the friendly shape of the serving on together\n\
   --workers        Defines the amount of worker to be used\n\
   --local          Runs the service in local mode no internet support\n\
   --daemon         Runs the service as daemon (background)\n\
\n\
What this build would do:\n\
   --print-config   Writes the configuration a run would use and exits\n\
   --check          Validates everything a run is made of and exits\n\
   --list-handlers  Writes the handlers that this build carries\n\
   --version        Prints the current version\n\
   --info           Prints the service information\n\
   --help           Prints this (help) message\n\
\n\
The tests may be selected and reported using:\n\
   --test           Runs a series of test for viriatum\n\
   --speed          Runs a series of speed relates tests for viriatum\n\
   --test-list      Lists the selected tests instead of running them\n\
   --test-filter    Runs only the tests whose name matches the pattern\n\
   --test-tags      Runs only the tests carrying one of the tags\n\
   --test-format    Format of the report (text, tap, junit or markdown)\n\
   --test-output    Path of the file the report is written to\n\
\n\
Examples:\n\
   viriatum --file=. --bind=:8080\n\
   viriatum --file=./dist --spa --cors --dev\n\
   viriatum --wsgi=budy:app --no-config -p=8080\n\
   viriatum --config=/etc/viriatum/viriatum.ini --check\n"

ERROR_CODE print_information(void) {
    /* retrieves the viriatum version and description */
    unsigned char *version = version_viriatum();
    unsigned char *description = description_viriatum();

    /* prints the banner message including the version, compiler
    information, platform and the active compilation flags */
    V_PRINT_F(
        "%s %s (%s, %s) [%s %s %d bit (%s)] %s [%s]\n",
        description,
        version,
        VIRIATUM_COMPILATION_DATE,
        VIRIATUM_COMPILATION_TIME,
        VIRIATUM_COMPILER,
        VIRIATUM_COMPILER_VERSION_STRING,
        (int) VIRIATUM_PLATFORM_CPU_BITS,
        VIRIATUM_PLATFORM_CPU,
        VIRIATUM_PLATFORM_STRING,
        VIRIATUM_FLAGS
    );

    /* prints a message on the copyright of the system */
    V_PRINT_F("%s\n", VIRIATUM_COPYRIGHT);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE help(void) {
    /* writes the help of the command, the path of the system wide
    configuration file is only known at runtime on some of the
    platforms and so it travels as a value of its own */
    V_PRINT_F(HELP_STRING, VIRIATUM_CONFIG_PATH);
    RAISE_NO_ERROR;
}
ERROR_CODE version(void) {
    V_PRINT_F(
        "%s - %s (%s, %s)\n",
        VIRIATUM_NAME,
        VIRIATUM_VERSION,
        VIRIATUM_COMPILATION_DATE,
        VIRIATUM_COMPILATION_TIME
    );
    RAISE_NO_ERROR;
}
ERROR_CODE info(void) { return print_information(); }

void load_test_options(struct test_options_t *options, struct hash_map_t *arguments) {
    /* allocates the value reference to be used
    during the arguments retrieval */
    void *value;

    /* starts the options with the default values, every one of
    the tests is run and no report file is produced */
    create_test_options(options);

    /* retrieves the pattern that the name of a test must match
    for it to be selected for the run */
    get_value_string_hash_map(arguments, (unsigned char *) "test-filter", &value);
    if(value != NULL && ((struct argument_t *) value)->type == VALUE_ARGUMENT) {
        options->filter = ((struct argument_t *) value)->value;
    }

    /* retrieves the tags that a test must carry at least one of
    for it to be selected for the run */
    get_value_string_hash_map(arguments, (unsigned char *) "test-tags", &value);
    if(value != NULL && ((struct argument_t *) value)->type == VALUE_ARGUMENT) {
        options->tags = ((struct argument_t *) value)->value;
    }

    /* retrieves the format under which the results of the run
    are going to be reported */
    get_value_string_hash_map(arguments, (unsigned char *) "test-format", &value);
    if(value != NULL && ((struct argument_t *) value)->type == VALUE_ARGUMENT) {
        options->format = ((struct argument_t *) value)->value;
    }

    /* retrieves the path of the file to which the report is to
    be written, the standard output is used when it is not set */
    get_value_string_hash_map(arguments, (unsigned char *) "test-output", &value);
    if(value != NULL && ((struct argument_t *) value)->type == VALUE_ARGUMENT) {
        options->path = ((struct argument_t *) value)->value;
    }

    /* verifies if the tests are meant to be listed instead of
    run, no value is expected for such an argument */
    get_value_string_hash_map(arguments, (unsigned char *) "test-list", &value);
    if(value != NULL) { options->list = TRUE; }

    /* a machine readable report written to the standard output
    turns the echoing of the progress off, otherwise the stream
    handed to the consumer of the report would be polluted */
    if(options->format != NULL && options->path == NULL) { options->echo = FALSE; }
}

ERROR_CODE test(struct hash_map_t *arguments) {
    struct test_options_t options;
    load_test_options(&options, arguments);
    if(options.echo == TRUE && options.list == FALSE) { print_information(); }
    return run_simple_tests(&options);
}
ERROR_CODE speed(struct hash_map_t *arguments) {
    struct test_options_t options;
    load_test_options(&options, arguments);
    if(options.echo == TRUE && options.list == FALSE) { print_information(); }
    return run_speed_tests(&options);
}

#ifdef VIRIATUM_PLATFORM_WIN32
void daemonize(void) {}
void daemonclean(void) {}
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
void daemonize(void) {
    /* allocates space for the various daemon
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
    verifies if it has been successful */
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
void daemonclean(void) {
    /* removes the viriatum pid path, so that the daemon
    watching tool are notified that the process is no
    longer running in the current environment */
    remove(VIRIATUM_PID_PATH);
}
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
void localize(void) { local = 1; }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
void localize(void) {}
#endif

int execute_arguments(char *program_name, struct hash_map_t *arguments) {
    /* allocates space for the possible argument
    to be executed from the arguments map */
    void *value;

    /* allocates the value to be used to verify the
    existence of error from the function */
    ERROR_CODE return_value;

    /* sets space for the flag that will control if
    the service should be run or not, this is used
    for certain situations (mostly test) where the
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
    and in case it's set starts the test process running a series
    of test functions in sequence */
    get_value_string_hash_map(arguments, (unsigned char *) "test", &value);
    if(value != NULL) { return test(arguments); }

    /* retrieves the speed argument value from the arguments map
    and in case it's set starts the speed measuring and disables
    the running of the service */
    get_value_string_hash_map(arguments, (unsigned char *) "speed", &value);
    if(value != NULL) { return speed(arguments); }

    /* tries to retrieve the list handlers argument from the arguments
    map and in case it's set writes the handlers this build carries
    and returns, so that the flag that names one is discoverable */
    get_value_string_hash_map(arguments, (unsigned char *) "list-handlers", &value);
    if(value != NULL) { return handlers_service_s(program_name, arguments); }

    /* tries to retrieve the print config argument from the arguments
    map and in case it's set writes the configuration that the merging
    of the three layers produced and returns, so that it is possible to
    see what a run would actually do without starting one */
    get_value_string_hash_map(arguments, (unsigned char *) "print-config", &value);
    if(value != NULL) { return check_service_s(program_name, arguments, TRUE); }

    /* tries to retrieve the check argument from the arguments map and
    in case it's set loads and validates everything a run is made of
    and returns the status of that, without ever serving anything */
    get_value_string_hash_map(arguments, (unsigned char *) "check", &value);
    if(value != NULL) { return check_service_s(program_name, arguments, FALSE); }

    /* tries to retrieve the daemon argument from the
    arguments map in case the value is set "daemonizes"
    the current process so that it remains in background
    and returns to the caller process immediately, otherwise
    prints the viriatum information into the standard
    output "file", the label should be standard */
    get_value_string_hash_map(arguments, (unsigned char *) "daemon", &value);
    if(value != NULL) {
        daemonize();
    } else {
        print_information();
    }

    /* tries to retrieve the local argument from the arguments
    map in case the value exists localizes the current service
    so that any file read is read from the current directory */
    get_value_string_hash_map(arguments, (unsigned char *) "local", &value);
    if(value != NULL) { localize(); }

    /* tries to retrieve the info argument from the arguments map in
    case the value exists prints the information of the service and
    then returns, the same way the other commands that only report
    something about the service do */
    get_value_string_hash_map(arguments, (unsigned char *) "info", &value);
    if(value != NULL) { return info(); }

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
    occurred during the execution of the command */
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

    /* sets stdout and stderr to line-buffered mode so that output
    is flushed after each newline, this ensures log messages are
    visible immediately even when output is piped (e.g. Docker),
    on Windows MSVC does not support _IOLBF (treats it as _IOFBF)
    and rejects size 0, so we use unbuffered mode instead */
#ifdef VIRIATUM_PLATFORM_WIN32
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#else
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);
#endif

    /* prints a debug message */
    V_DEBUG("Running in DEBUG mode\n");
    V_DEBUG_F("Receiving %d argument(s)\n", argc);

    /* in case the number of arguments is less than one
    (exception case) returns in error */
    if(argc < 1) {
        cleanup(NULL);
        RAISE_ERROR_S(1);
    }

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

    /* prints a debug message about the ending of the system
    for the execution of the service and then returns the
    normal return code (success status) to the caller process */
    V_DEBUG_F(
        "Finishing process [%ld pending]\n",
        (long int) ALLOCATIONS
    );
    RAISE_AGAIN(return_value);
}
#endif
