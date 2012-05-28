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
   add        Add file contents to the index\n\
   bisect     Find by binary search the change that introduced a bug\n\
   branch     List, create, or delete branches\n\
   checkout   Checkout a branch or paths to the working tree\n\
   clone      Clone a repository into a new directory\n\
\n\
See 'viriatum --help[=<command>]' for more information on a specific command.\n"

/* starts the memory structures */
START_MEMORY;

unsigned char local = 0;
static struct Service_t *service;

ERROR_CODE runService(struct HashMap_t *arguments) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* allocates the socket data */
    SOCKET_DATA socketData;

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the service and loads the options
    taking into account the arguments */
    createService(&service, (unsigned char *) VIRIATUM_NAME);
    loadOptionsService(service, arguments);

    /* starts the service */
    returnValue = startService(service);

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
         /* runs the socket finish */
        SOCKET_FINISH();

        /* raises the error again */
        RAISE_AGAIN(returnValue);
    }

    /* deletes the service */
    deleteService(service);

    /* runs the socket finish */
    SOCKET_FINISH();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE ranService() {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* in case the service status is open */
    if(service->status == STATUS_CLOSED) {
        /* prints a warning message */
        V_DEBUG("No service to be stopped\n");
    } else {
        /* prints a warning message */
        V_DEBUG("Stopping service\n");

        /* stops the service */
        returnValue = stopService(service);

        /* tests the error code for error */
        if(IS_ERROR_CODE(returnValue)) {
            /* runs the socket finish */
            SOCKET_FINISH();

            /* raises the error again */
            RAISE_AGAIN(returnValue);
        }

        /* prints a warning message */
        V_DEBUG("Finished stopping service\n");
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

void killHandler(int signalNumber) {
    /* defaults the signal handler (only one handling) */
    signal(signalNumber, SIG_DFL);

    /* runs the "ran" service */
    ranService();
}

ERROR_CODE printInformation() {
    /* retrieves the viriatum version and description */
    unsigned char *version = versionViriatum();
    unsigned char *description = descriptionViriatum();

    /* registers the kill handler for the various signals
    associated with the "destroy" operation */
    signal(SIGHUP, killHandler);
    signal(SIGINT, killHandler);
    signal(SIGQUIT, killHandler);
    signal(SIGTERM, killHandler);

    /* registers the ignore action in the signal indicating
    a broken pipe (unexpected close of socket) */
    signal(SIGPIPE, SIG_IGN);

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
    FILE *pidFile;
    char pidString[1024];
    size_t pidStringLength;

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
    FOPEN(&pidFile, VIRIATUM_PID_PATH, "wb");
    SPRINTF(pidString, 1024, "%d\n", pid);
    pidStringLength = strlen(pidString);
    fwrite(pidString, sizeof(char), pidStringLength, pidFile);
    fclose(pidFile);

    /* closes the various pending streams from the
    daemon process (not going to output them) */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
void localize() { local = 1; }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
void localize() { }
#endif

void executeArguments(struct HashMap_t *arguments) {
    /* allocates space for the possible argument
    to be executed from the arguments map */
    void *value;

    /* tries to retrieve the help argument from the arguments
    map in case the value exists prints the help value and then
    exits the current system */
    getValueStringHashMap(arguments, (unsigned char *) "help", &value);
    if(value != NULL) { help(); exit(0); }

    /* tries to retrieve the version argument from the arguments
    map in case the value exists prints the version value and then
    exits the current system */
    getValueStringHashMap(arguments, (unsigned char *) "version", &value);
    if(value != NULL) { version(); exit(0); }

    /* tries to retrieve the daemon argument from the
    arguments map in case the value is set daemonizes
    the current process so that it remains in background
    and returns to the caller process immediately, otherwise
    prints the viriatum information into the standard
    output "file", the label should be standard */
    getValueStringHashMap(arguments, (unsigned char *) "daemon", &value);
    if(value != NULL) { daemonize(); }
    else { printInformation(); }

    /* tries to retrieve the local argument from the arguments
    map in case the value exists localizes the current service
    so that any file read is read from the current directory */
    getValueStringHashMap(arguments, (unsigned char *) "local", &value);
    if(value != NULL) { localize(); }
}

#ifndef VIRIATUM_PLATFORM_IPHONE
int main(int argc, char *argv[]) {
    /* allocates the return value */
    ERROR_CODE returnValue;

    /* allocates the map that will contain the various
    processed arguments, indexed by name */
    struct HashMap_t *arguments;

    /* prints a debug message */
    V_DEBUG_F("Receiving %d argument(s)\n", argc);

    /* processes the various arguments into a map and then
    executes the corresponding (initial) actions */
    processArguments(argc, argv, &arguments);
    executeArguments(arguments);

    /* runs the service, with the given arguments */
    returnValue = runService(arguments);

    /* deletes the processed arguments */
    deleteArguments(arguments);

    /* tests the error code for error */
    if(IS_ERROR_CODE(returnValue)) {
        /* prints an error message */
        V_ERROR_F("Problem running service (%s)\n", (char *) GET_ERROR());
    }

    /* removes the viriatum pid path, so that the daemon
    watching tool are notified that the process is no
    longer running in the current environment */
    remove(VIRIATUM_PID_PATH);

    /* prints a debug message */
    V_DEBUG("Finishing process\n");

    /* returns zero (valid) */
    return 0;
}
#endif
