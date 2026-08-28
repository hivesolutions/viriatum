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

#include "../handlers/handler_file.h"
#include "../http/http_parser.h"
#include "test_support.h"

/**
 * Tests the handler file context creation
 * and default value initialization.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_context(void);

/**
 * Holds the complete chain of structures required by
 * the callbacks of the file handler, it is built by the
 * setup of the fixture and destroyed by its teardown so
 * that nothing is leaked when a test fails midway.
 */
typedef struct handler_file_fixture_t {

    /**
     * The minimal connection, service and options chain
     * assigned to the parameters of the request.
     */
    struct test_context_t *test_context;

    /**
     * The HTTP request wired to the handler file context.
     */
    struct http_request_t *http_request;

    /**
     * The context of the file handler under test.
     */
    struct handler_file_context_t *handler_file_context;

} handler_file_fixture;

/**
 * Creates the fixture with the complete chain of structures
 * the callbacks of the file handler require, already wired
 * together and ready for a request to be parsed.
 *
 * @return The created fixture as an opaque context.
 */
void *setup_handler_file_test(void);

/**
 * Destroys the fixture created by the setup, including every
 * one of the structures that it carries.
 *
 * @param context The fixture to be destroyed.
 */
void cleanup_handler_file_test(void *context);

/**
 * Tests the handler file url callback including
 * query string stripping and path traversal rejection.
 *
 * @param context The fixture carrying the parser and the
 * context of the handler under test.
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_url(void *context);

/**
 * Tests the handler file header field callback
 * for correct recognition of known HTTP headers.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_header_field(void);

/**
 * Tests the handler file header value callback
 * for correct storage of header values in context.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_header_value(void);
