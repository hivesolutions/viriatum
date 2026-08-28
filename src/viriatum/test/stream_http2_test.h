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

#include "../stream/stream_http2.h"
#include "test_support.h"

/**
 * Holds the sequence of callbacks that the session has driven
 * so that a test verifies that a stream reaches the handler in
 * the very same shape an HTTP/1.1 message would.
 */
typedef struct http2_record_t {
    /**
     * The number of times each one of the callbacks of the
     * pipeline has been driven.
     */
    size_t begin;
    size_t url;
    size_t field;
    size_t value;
    size_t headers;
    size_t body;
    size_t complete;

    /**
     * The url of the last message that has begun, gathered so
     * that the pseudo header may be compared against it.
     */
    char path[VIRIATUM_MAX_URL_SIZE];

    /**
     * The name and the value of the last field that has been
     * handed over, terminated so that they compare as strings.
     */
    char name[VIRIATUM_MAX_HEADER_SIZE];
    char header[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * The payload of the last body fragment that has been handed
     * over, terminated so that it compares as a string.
     */
    char payload[VIRIATUM_MAX_HEADER_SIZE];
} http2_record_t;

/**
 * Tests the life-cycle of the session, verifying that it starts
 * at the values the specification defines and that it releases
 * the streams that are still open.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection(void);

/**
 * Tests the opening, the lookup and the closing of the streams
 * of a connection, including the identifiers that a peer is not
 * allowed to use and the bound on the open ones.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_streams(void);

/**
 * Tests the flow control windows of both the connection and the
 * streams, including the increment that takes one of them beyond
 * the largest value the protocol represents.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_window(void);

/**
 * Tests the application of the settings of the peer, in
 * particular that a change of the initial window is carried over
 * to the streams that are already open.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_settings(void);

/**
 * Tests the handling of the frames that drive a connection
 * rather than a stream, the settings, the ping, the goaway and
 * the update of a window.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_frames(void);

/**
 * Tests that a header block reaches the handler as the very same
 * sequence of callbacks that an HTTP/1.1 message produces.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_headers(void);

/**
 * Tests a header block that is spread over a sequence of
 * continuation frames, including the frame that is not allowed
 * to come between them.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_continuation(void);

/**
 * Tests the payload of a stream, both the accounting of it
 * against the windows and the delivery of it to the handler.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_data(void);

/**
 * Tests the violations of the protocol that take the complete
 * connection down rather than a single stream.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_errors(void);

/**
 * Tests the reading of a connection at the level of the bytes,
 * covering the preface, a frame that arrives split over several
 * reads and the compaction of the buffer.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_read(void);

/**
 * Tests that a preface that does not match takes the connection
 * down before a single frame is handled.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_preface(void);
