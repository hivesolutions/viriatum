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

#pragma once

#include "../system/service.h"

/**
 * Holds the minimal service infrastructure needed
 * by handler tests: a connection wired to a service
 * with initialized service options (including the
 * pre-resolved paths).
 */
typedef struct test_context_t {

    /**
     * The service options containing the pre-resolved
     * paths for contents, resources and modules.
     */
    struct service_options_t *options;

    /**
     * The service instance that owns the options
     * and is referenced by the connection.
     */
    struct service_t *service;

    /**
     * The connection that is wired to the service
     * and can be assigned to http_parser->parameters.
     */
    struct connection_t *connection;

} test_context_t;

/**
 * Creates a minimal test context with a connection,
 * service and service options chain properly wired
 * together. The service options are populated with
 * default paths so that handler callbacks can safely
 * dereference connection->service->options.
 *
 * @param context_pointer The pointer to receive
 * the created test context.
 */
void create_test_context(struct test_context_t **context_pointer);

/**
 * Destroys a test context and frees all associated
 * resources including the connection, service and
 * service options.
 *
 * @param context The test context to destroy.
 */
void delete_test_context(struct test_context_t *context);
