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
#include "hpack_test.h"
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
 * Tests the tree of the priorities, including the exclusive
 * dependency that takes over the siblings, the one that would
 * close a cycle and the inheritance of the closing of a stream.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_priority(void);

/**
 * Tests the promising of a resource, including the frame that
 * carries it, the stream it reserves and the settings of the peer
 * that keep the promise from being made at all.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_push(void);

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
 * Tests the rules that the fields of a header block are bound by,
 * both the pseudo headers of a request and the regular ones that
 * carry no meaning at all under this protocol.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_fields(void);

/**
 * Tests that a stream follows the handler that a dispatching one
 * switches the connection onto, so that the release of a message
 * reaches the handler that actually served it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_dispatch(void);

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
 * Tests the section of the trailers that follows the payload of a
 * message, together with the streams that are refused because the
 * peer holds as many of them open as this end has announced.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_trailers(void);

/**
 * Tests the writing of a response for a stream that is no longer
 * open, the buffers of it are released rather than queued.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_closed(void);

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

/**
 * Tests the telling apart of the two versions of the protocol at
 * the very start of a connection, both the one that carries a
 * message of the older one and the one that carries the preface.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_detect(void);

/**
 * Tests the writing of a complete response, verifying that the
 * frames it produces carry the status, the fields and the payload
 * that the handler has written.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_response(void);

/**
 * Tests the completion of the writes of a response, verifying that
 * a stream only closes once the payload that closes the message of
 * it has actually left this end.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_complete(void);

/**
 * Tests the writing of an error over a session, verifying that it
 * reaches the peer as the frames of this protocol rather than as
 * the text of the older version of it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_error(void);

/**
 * Tests that the payload of a response is held back while the
 * window does not allow it through and goes out as soon as the
 * peer widens it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_flow(void);

/**
 * Tests that a payload larger than the window is split over
 * several frames, the parts of it being copied out of the buffer
 * that the handler owns.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_split(void);

/**
 * Tests the order in which the streams that are holding payload
 * back write it, the ones that sit above going first and the
 * heavier of the siblings going before the lighter ones.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_schedule(void);

/**
 * Tests the size that a message announces for its payload, both
 * the one that matches what arrives and the ones that are refused.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_connection_length(void);

/**
 * Tests the selection of the protocol out of the list that a peer
 * announces through the transport, including the order that is
 * honoured and the list that is malformed.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_alpn(void);
