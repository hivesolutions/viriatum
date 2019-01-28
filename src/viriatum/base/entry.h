/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../system/system.h"

/**
 * Initializes the service structure populating it's values
 * and loading the arguments and configuration from the command
 * line and configuration files.
 *
 * @param program_name The name (path) to the current program
 * (process) in execution for the service context.
 * @param parameters The map containing the various parameters
 * to be used while running the service.
 */
ERROR_CODE init_service(char *program_name, struct hash_map_t *arguments);

/**
 * Destroys the service structure, so that the running of it is
 * not possible anymore.
 * If one wants to run a service after this call the init service
 * function must be called first.
 */
ERROR_CODE destroy_service();

/**
 * Start the process of running a service by looping for connection
 * changes for the context of execution.
 * Note that the modules are only loaded after this call.
 * To unblock this call the ran service function should be called.
 */
ERROR_CODE run_service();

/**
 * Starts the process of running the service, registering the
 * appropriate handlers and creating all the service structures.
 * This call blocks until the service is correctly stopped
 * from a diferent flow control.
 * At the end of the execution the global service object is then
 * destroyed an no other execution is possible from it.
 *
 * This function is considered safe because the service is forced
 * to be initialized before the run/start of it.
 *
 * @param program_name The name (path) to the current program
 * (process) in execution for the service context.
 * @param parameters The map containing the various parameters
 * to be used while running the service.
 */
ERROR_CODE run_service_s(char *program_name, struct hash_map_t *parameters);

/**
 * Stops the process of a running the service.
 * This call unblock a previous run service call.
 */
ERROR_CODE ran_service();

/**
 * Retrieves the pointer to the global service instance
 * so that it may be used to recover information from it
 *
 * @param service_pointer The pointer to the service structure
 * to be set with the current service instance.
 */
ERROR_CODE pointer_service(struct service_t **service_pointer);

/**
 * Handler callback for the kill signal.
 * This callback stops the current service instance.
 *
 * @param signal_number The signal number (context) for
 * which this event is being generated.
 */
void kill_handler(int signal_number);

/**
 * Handler callback for the ignore signal.
 * This is just a placeholder callback.
 *
 * @param signal_number The signal number (context) for
 * which this event is being generated.
 */
void ignore_handler(int signal_number);

/**
 * Registers the various signal handlers for the default
 * events, this must be done so that the proper action
 * occur for such events.
 */
void register_signals();
