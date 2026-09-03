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
#include "../stream/stream_http2.h"
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
/**
 * Tests the serving of a complete file, verifying that the response
 * of it carries the status, the fields that describe the resource
 * and the payload that the file holds.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_response(void);

/**
 * Tests the serving of a part of a file, the one that the range of
 * the request asks for rather than the complete resource.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_range(void);

/**
 * Tests the serving of a resource that does not exist, including
 * the closing of a connection that is not meant to be kept alive.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_missing(void);

/**
 * Tests the page of an error built out of the template of it,
 * including one that is rewritten between two requests served
 * by the very same service.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_missing_template(void);

/**
 * Tests the serving of a resource that goes away after the response
 * of it has been decided upon, the connection is handed back rather
 * than left locked with the handler of it still in place.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_gone(void);

/**
 * Tests the promising of the resources that the location of a
 * request lists, each one of them reserving a stream of its own.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_push(void);
/**
 * Tests the serving of a directory that is asked for without the
 * trailing slash, the peer is sent to the very same place with it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_directory(void);

/**
 * Tests the listing of a directory, the page that is built out of
 * the template of it naming every entry and the count of them.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_listing(void);

/**
 * Tests a directory whose contents change between two requests,
 * an entry that appears and one that goes away being listed as
 * they stand at the time of each request.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_listing_changed(void);

/**
 * Tests the template of the listing rewritten between two requests
 * served by the very same service, the second page being built out
 * of the template as it now stands.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_listing_template(void);

/**
 * Tests the resolution of the path of a resource, both against the
 * contents of the service and against the base path of a location.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_path(void);

/**
 * Tests that the matching of a location carries the settings of it
 * into the context, the resources to be promised among them.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_location(void);

/**
 * Tests the setting, the resetting and the unsetting of the
 * handler, the operations that build and release the context that
 * travels with a message.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_handler(void);

/**
 * Tests the creation of the cache of the files that
 * the handler keeps open and its default state.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache(void);

/**
 * Tests the acquiring of a file out of the cache, both
 * the first time and once it is already being held.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_acquire(void);

/**
 * Tests the acquiring of a file that is not there, which
 * raises instead of handing back an empty entry.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_missing(void);

/**
 * Tests that a file written over in place is described by
 * the size it now has and never by the one it used to have.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_changed(void);

/**
 * Tests that two files falling on the same entry of the
 * cache are each described by their own contents.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_collision(void);

/**
 * Tests the clearing of the cache, which closes every file
 * being held and leaves it working afterwards.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_clear(void);

/**
 * Tests the handing out of a descriptor for a file of the
 * cache, one apart from the descriptor it is holding.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_open(void);

/**
 * Tests that a path longer than an entry is able to carry
 * is refused instead of being copied past the end of it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_long(void);

/**
 * Tests that an entry which has gone past the time it is
 * trusted for is renewed when the file has not moved on.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_expired(void);

/**
 * Tests that a file put in the place of the one being held
 * is opened in its place once the entry is looked at again.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_replaced(void);

/**
 * Tests that a file replaced by another of the very same length
 * is served as the one that is now under the path, the length of
 * it being unable to tell the two of them apart.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_rewritten(void);

/**
 * Tests that an entry left holding a descriptor which no
 * longer reaches anything answers with an error.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_cache_stale(void);
