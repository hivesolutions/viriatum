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
     * and can be assigned to http_request->parameters.
     */
    struct connection_t *connection;

    /**
     * The io connection layer, only built by the tests
     * that require the complete chain of a connection.
     */
    struct io_connection_t *io_connection;

    /**
     * The HTTP connection layer, only built by the tests
     * that require the complete chain of a connection.
     */
    struct http_connection_t *http_connection;

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

/**
 * Stands in for the closing of a connection that holds no socket,
 * only the request itself is recorded.
 *
 * @param connection The connection being closed.
 * @return The resulting error code.
 */
ERROR_CODE close_test_connection(struct connection_t *connection);

/**
 * Forgets the closings that have been requested so far, a test
 * calls it before driving a connection.
 */
void reset_closed_test_connection(void);

/**
 * Retrieves the number of closings that have been requested on
 * the connections of the tests.
 *
 * @return The number of closings that have been requested.
 */
size_t get_closed_test_connection(void);

/**
 * Stands in for the registration of the writing of a connection
 * that is not attached to a polling, the data that has been
 * queued stays in the queue for the test to observe.
 *
 * @param connection The connection being registered.
 * @return The resulting error code.
 */
ERROR_CODE register_write_test_connection(struct connection_t *connection);

/**
 * Completes the writes that are queued on the connection of the
 * provided context, driving the callbacks of them in the very same
 * sequence that the io layer of a connection would.
 * A test uses it to reach the operations that only ever run once a
 * write has actually left this end.
 *
 * @param context The test context holding the connection.
 * @param buffer The buffer to gather what has been written into,
 * it may be left unset when the contents are of no interest.
 * @param buffer_size The size in bytes of the provided buffer.
 * @return The number of the bytes that have been gathered.
 */
size_t flush_test_connection(struct test_context_t *context, unsigned char *buffer, size_t buffer_size);

/**
 * Builds the io and the HTTP connection layers on top of
 * the connection of the provided context, the handler that a
 * message is served by is left unset so that a test installs
 * the one it wants to observe.
 * This is only required by the tests that drive a complete
 * connection rather than a single callback.
 *
 * @param context The test context to be completed.
 */
void create_test_connection(struct test_context_t *context);

/**
 * Destroys the io and the HTTP connection layers that have
 * been built on top of the connection of the context.
 *
 * @param context The test context to be reduced.
 */
void delete_test_connection(struct test_context_t *context);
