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

#include "stream_http2_test.h"

#ifdef VIRIATUM_HTTP2

/**
 * The record that the handler of the tests writes into, it is a
 * single one as only one connection is driven at a time.
 */
static struct http2_record_t _record;

/**
 * The handler that the tests install as the one a new message is
 * served by, it records the callbacks rather than responding.
 */
static struct http_handler_t _handler;

/**
 * Flag controlling if the handler of the tests answers the message
 * it has just received, the tests of the response set it so that
 * the writing operations of the connection are exercised.
 */
static char _responder = FALSE;

/**
 * The payload that the handler of the tests answers with.
 */
#define HTTP2_TEST_MESSAGE "ok"

static ERROR_CODE _begin_http2_test(struct http_request_t *http_request) {
    _record.begin++;
    RAISE_NO_ERROR;
}

static ERROR_CODE _url_http2_test(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    _record.url++;
    if(data_size >= sizeof(_record.path)) { data_size = sizeof(_record.path) - 1; }
    memcpy(_record.path, data, data_size);
    _record.path[data_size] = '\0';
    RAISE_NO_ERROR;
}

static ERROR_CODE _field_http2_test(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    _record.field++;
    if(data_size >= sizeof(_record.name)) { data_size = sizeof(_record.name) - 1; }
    memcpy(_record.name, data, data_size);
    _record.name[data_size] = '\0';
    RAISE_NO_ERROR;
}

static ERROR_CODE _value_http2_test(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    _record.value++;
    if(data_size >= sizeof(_record.header)) { data_size = sizeof(_record.header) - 1; }
    memcpy(_record.header, data, data_size);
    _record.header[data_size] = '\0';
    RAISE_NO_ERROR;
}

static ERROR_CODE _headers_http2_test(struct http_request_t *http_request) {
    _record.headers++;
    RAISE_NO_ERROR;
}

static ERROR_CODE _body_http2_test(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    _record.body++;
    if(data_size >= sizeof(_record.payload)) { data_size = sizeof(_record.payload) - 1; }
    memcpy(_record.payload, data, data_size);
    _record.payload[data_size] = '\0';
    RAISE_NO_ERROR;
}

static ERROR_CODE _complete_http2_test(struct http_request_t *http_request) {
    /* allocates space for the buffers of the response and for the
    position that the building of it reaches */
    char *headers;
    unsigned char *message;
    size_t count;

    /* retrieves the connection out of the message and out of it the
    substrates that carry the writing operations */
    struct connection_t *connection = (struct connection_t *) http_request->parameters;
    struct io_connection_t *io_connection;
    struct http_connection_t *http_connection;

    _record.complete++;
    if(_responder == FALSE) { RAISE_NO_ERROR; }

    io_connection = (struct io_connection_t *) connection->lower;
    http_connection = (struct http_connection_t *) io_connection->lower;

    /* builds the response through the operations of the connection,
    which is what a handler of the tree does */
    headers = (char *) MALLOC(VIRIATUM_HTTP_SIZE);
    count = http_connection->write_status(
        connection,
        headers,
        VIRIATUM_HTTP_SIZE,
        http_request->version,
        200,
        "OK",
        KEEP_ALIVE
    );
    count = http_connection->write_field(
        connection,
        headers,
        VIRIATUM_HTTP_SIZE,
        count,
        CONTENT_LENGTH_H,
        "2"
    );
    count = http_connection->write_end(connection, headers, VIRIATUM_HTTP_SIZE, count, FALSE);
    http_connection->write_flush(connection, (unsigned char *) headers, count, NULL, NULL);

    /* writes the payload of the response, it is the fragment that
    closes the message */
    message = (unsigned char *) MALLOC(sizeof(HTTP2_TEST_MESSAGE) - 1);
    memcpy(message, HTTP2_TEST_MESSAGE, sizeof(HTTP2_TEST_MESSAGE) - 1);
    http_connection->write_chunk(
        connection,
        message,
        sizeof(HTTP2_TEST_MESSAGE) - 1,
        TRUE,
        NULL,
        NULL
    );

    RAISE_NO_ERROR;
}

static ERROR_CODE _set_http2_test(struct http_connection_t *http_connection) {
    /* installs the callbacks that record the pipeline, they are set
    on the settings that belong to the stream being served */
    http_connection->http_settings->on_message_begin = _begin_http2_test;
    http_connection->http_settings->on_url = _url_http2_test;
    http_connection->http_settings->on_header_field = _field_http2_test;
    http_connection->http_settings->on_header_value = _value_http2_test;
    http_connection->http_settings->on_headers_complete = _headers_http2_test;
    http_connection->http_settings->on_body = _body_http2_test;
    http_connection->http_settings->on_message_complete = _complete_http2_test;
    RAISE_NO_ERROR;
}

static ERROR_CODE _unset_http2_test(struct http_connection_t *http_connection) {
    RAISE_NO_ERROR;
}

/**
 * Installs the handler that records the pipeline as the one a new
 * message of the provided connection is served by, forgetting
 * whatever the test that ran before this one has produced.
 *
 * @param context The test context to install the handler on.
 */
static void _install_http2_test(struct test_context_t *context) {
    _handler.name = (unsigned char *) "record";
    _handler.resolve_index = FALSE;
    _handler.set = _set_http2_test;
    _handler.unset = _unset_http2_test;
    _handler.reset = NULL;
    context->http_connection->base_handler = &_handler;

    memset(&_record, 0, sizeof(_record));
}

/**
 * Builds the chain of a connection together with the session that
 * drives the protocol over it, installing the handler that the
 * tests observe as the one a new message is served by.
 *
 * @param context_pointer The pointer to the test context that has
 * been built.
 * @param http2_connection_pointer The pointer to the session that
 * has been built over it.
 */
static void _create_http2_test(struct test_context_t **context_pointer, struct http2_connection_t **http2_connection_pointer) {
    struct test_context_t *context;

    create_test_context(&context);
    create_test_connection(context);
    _install_http2_test(context);

    create_http2_connection(http2_connection_pointer, context->http_connection);
    *context_pointer = context;
}

/**
 * Releases the chain of a connection together with the session
 * that was driving the protocol over it.
 *
 * @param context The test context to be released.
 * @param http2_connection The session to be released.
 */
static void _delete_http2_test(struct test_context_t *context, struct http2_connection_t *http2_connection) {
    delete_http2_connection(http2_connection);
    delete_test_connection(context);
    delete_test_context(context);
}

/**
 * Writes a header block carrying the pseudo headers of a request
 * into the provided buffer, using an encoder of its own so that
 * the decoding of the session is exercised end to end.
 *
 * @param buffer The buffer to write the block into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param path The path of the request being built.
 * @return The size in bytes of the block that has been written.
 */
static size_t _request_http2_test(unsigned char *buffer, size_t buffer_size, const char *path) {
    struct hpack_table_t *encoder;
    struct hpack_header_t hpack_header;
    size_t offset = 0;

    create_hpack_table(&encoder);

    hpack_header.name = (unsigned char *) ":method";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) "GET";
    hpack_header.value_size = 3;
    encode_hpack(encoder, buffer, buffer_size, &offset, &hpack_header, FALSE);

    hpack_header.name = (unsigned char *) ":scheme";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) "http";
    hpack_header.value_size = 4;
    encode_hpack(encoder, buffer, buffer_size, &offset, &hpack_header, FALSE);

    hpack_header.name = (unsigned char *) ":path";
    hpack_header.name_size = 5;
    hpack_header.value = (unsigned char *) path;
    hpack_header.value_size = strlen(path);
    encode_hpack(encoder, buffer, buffer_size, &offset, &hpack_header, FALSE);

    hpack_header.name = (unsigned char *) ":authority";
    hpack_header.name_size = 10;
    hpack_header.value = (unsigned char *) "localhost";
    hpack_header.value_size = 9;
    encode_hpack(encoder, buffer, buffer_size, &offset, &hpack_header, FALSE);

    hpack_header.name = (unsigned char *) "accept";
    hpack_header.name_size = 6;
    hpack_header.value = (unsigned char *) "text/html";
    hpack_header.value_size = 9;
    encode_hpack(encoder, buffer, buffer_size, &offset, &hpack_header, FALSE);

    delete_hpack_table(encoder);

    return offset;
}

const char *test_http2_connection(void) {
    /* allocates space for the chain of the connection and for the
    session that drives the protocol over it */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;

    /* gathers the number of allocations that are outstanding before
    anything is built, so that the release of everything may be
    verified against it once the session is gone */
    size_t allocated = ALLOCATIONS;

    /* builds the connection together with the session, the session
    starts at the values that the specification defines */
    _create_http2_test(&context, &http2_connection);
    V_ASSERT_EQ_U(http2_connection->count, 0);
    V_ASSERT_EQ_U(http2_connection->last_stream_id, 0);
    V_ASSERT_EQ_U(http2_connection->send_window, HTTP2_DEFAULT_WINDOW_SIZE);
    V_ASSERT_EQ_U(http2_connection->receive_window, HTTP2_DEFAULT_WINDOW_SIZE);
    V_ASSERT(http2_connection->preface == FALSE);
    V_ASSERT(http2_connection->goaway == FALSE);
    V_ASSERT_NULL(http2_connection->block);

    /* the session is reachable from the connection, which is how the
    reading of it finds the session */
    V_ASSERT_EQ_P(context->http_connection->http2_connection, http2_connection);

    /* a stream that is still open when the session is released is
    closed together with it, nothing is leaked */
    open_stream_http2_connection(http2_connection, 1, &http2_stream);
    V_ASSERT_EQ_U(http2_connection->count, 1);

    _delete_http2_test(context, http2_connection);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the session existed at all */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_streams(void) {
    /* allocates space for the chain of the connection, for the
    session and for the streams being opened */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_stream_t *other;
    size_t index;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* an even identifier is reserved for the pushes of the server
    and so a peer is not allowed to open one */
    error = open_stream_http2_connection(http2_connection, 2, &http2_stream);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* the first stream a client opens carries the identifier one and
    starts with the windows of the settings of both ends */
    error = open_stream_http2_connection(http2_connection, 1, &http2_stream);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_stream->stream_id, 1);
    V_ASSERT_EQ_U(http2_stream->state, HTTP2_STATE_OPEN);
    V_ASSERT_EQ_U(http2_stream->send_window, HTTP2_DEFAULT_WINDOW_SIZE);
    V_ASSERT_EQ_U(http2_stream->receive_window, HTTP2_DEFAULT_WINDOW_SIZE);
    V_ASSERT_EQ_U(http2_stream->request->version, HTTP20);
    V_ASSERT_EQ_U(http2_stream->request->stream_id, 1);

    /* the identifiers of the streams a peer opens are required to be
    strictly increasing, the same one is never reopened */
    error = open_stream_http2_connection(http2_connection, 1, &other);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a stream that has already been passed is refused too, it
    refers to one that has been closed */
    error = open_stream_http2_connection(http2_connection, 3, &other);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    error = open_stream_http2_connection(http2_connection, 3, &other);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* both of the streams are found by the identifier they carry and
    an unknown one is reported as absent */
    V_ASSERT_NOT_NULL(find_stream_http2_connection(http2_connection, 1));
    V_ASSERT_NOT_NULL(find_stream_http2_connection(http2_connection, 3));
    V_ASSERT_NULL(find_stream_http2_connection(http2_connection, 5));

    /* closing the first of them keeps the table compact, the one
    that was last takes over the slot it has freed */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    close_stream_http2_connection(http2_connection, http2_stream);
    V_ASSERT_EQ_U(http2_connection->count, 1);
    V_ASSERT_NULL(find_stream_http2_connection(http2_connection, 1));
    V_ASSERT_NOT_NULL(find_stream_http2_connection(http2_connection, 3));

    /* the peer is only allowed to hold as many streams open as this
    end has announced, the excess is refused */
    http2_connection->settings.max_concurrent_streams = 3;
    for(index = 0; index < 2; index++) {
        error = open_stream_http2_connection(http2_connection, 5 + (unsigned int) index * 2, &other);
        V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    }
    error = open_stream_http2_connection(http2_connection, 11, &other);
    V_ASSERT_EQ_U(error, HTTP2_REFUSED_STREAM);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_priority(void) {
    /* allocates space for the chain of the connection, for the
    session and for the streams that make up the tree */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *first;
    struct http2_stream_t *second;
    struct http2_stream_t *third;
    struct http2_priority_t http2_priority;
    struct http2_frame_t http2_frame;
    unsigned char payload[256];
    size_t offset;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* opens three streams, every one of them hanging from the root
    of the tree, which is where a stream starts its life */
    open_stream_http2_connection(http2_connection, 1, &first);
    open_stream_http2_connection(http2_connection, 3, &second);
    open_stream_http2_connection(http2_connection, 5, &third);
    V_ASSERT_EQ_U(first->priority.dependency, 0);
    V_ASSERT_EQ_U(first->priority.weight, HTTP2_DEFAULT_WEIGHT);

    /* places the second stream below the first, which is the plain
    form of a dependency and the one a browser uses the most */
    http2_priority.dependency = 1;
    http2_priority.weight = 32;
    http2_priority.exclusive = FALSE;
    error = prioritise_stream_http2_connection(http2_connection, second, &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(second->priority.dependency, 1);
    V_ASSERT_EQ_U(second->priority.weight, 32);

    /* an exclusive dependency takes over the siblings, so the third
    stream takes the place the second held below the first and the
    second comes to hang from the third */
    http2_priority.dependency = 1;
    http2_priority.weight = 64;
    http2_priority.exclusive = TRUE;
    error = prioritise_stream_http2_connection(http2_connection, third, &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(third->priority.dependency, 1);
    V_ASSERT_EQ_U(second->priority.dependency, 5);

    /* a dependency on a stream that sits below moves that one out of
    the way first, so that the tree never closes a cycle */
    http2_priority.dependency = 5;
    http2_priority.weight = 16;
    http2_priority.exclusive = FALSE;
    error = prioritise_stream_http2_connection(http2_connection, first, &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(first->priority.dependency, 5);
    V_ASSERT_EQ_U(third->priority.dependency, 0);

    /* a stream is never allowed to depend on itself, the tree would
    carry a cycle that no walk of it would ever leave */
    http2_priority.dependency = 1;
    error = prioritise_stream_http2_connection(http2_connection, first, &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a block of headers that carries the priority places the stream
    it opens right away, so that the first frame of it is enough for
    the peer to describe where it belongs */
    payload[0] = 0x80;
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x03;
    payload[4] = 31;
    offset = _request_http2_test(&payload[HTTP2_PRIORITY_SIZE], sizeof(payload) - HTTP2_PRIORITY_SIZE, "/placed");

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM | HTTP2_FLAG_PRIORITY;
    http2_frame.stream_id = 7;
    http2_frame.length = HTTP2_PRIORITY_SIZE + offset;
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the weight travels deducted of a unit and the most significant
    bit of the dependency is the exclusive flag */
    second = find_stream_http2_connection(http2_connection, 7);
    V_ASSERT_NOT_NULL(second);
    V_ASSERT_EQ_U(second->priority.dependency, 3);
    V_ASSERT_EQ_U(second->priority.weight, 32);
    V_ASSERT(second->priority.exclusive == TRUE);

    /* a block that announces the priority and carries less than the
    five bytes of it is malformed */
    http2_frame.stream_id = 9;
    http2_frame.length = HTTP2_PRIORITY_SIZE - 1;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* the streams that hang from one that closes come to hang from
    the one it hung from, so the tree keeps its shape */
    third = find_stream_http2_connection(http2_connection, 5);
    close_stream_http2_connection(http2_connection, third);
    first = find_stream_http2_connection(http2_connection, 1);
    second = find_stream_http2_connection(http2_connection, 3);
    V_ASSERT_NOT_NULL(first);
    V_ASSERT_NOT_NULL(second);
    V_ASSERT_EQ_U(first->priority.dependency, 0);
    V_ASSERT_EQ_U(second->priority.dependency, 0);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_push(void) {
    /* allocates space for the chain of the connection, for the
    session and for the frame that the promise produces */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_stream_t *promised;
    struct http2_frame_t http2_frame;
    struct hpack_table_t *decoder;
    struct hpack_collector_t collector;
    struct data_t *data;
    unsigned char block[256];
    size_t queued;
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    the promising may be verified to release every buffer it takes */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);
    _responder = TRUE;

    /* hands a complete request over, the promise is made on the
    stream that has asked for the resource referring to it */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/index.html");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.complete, 1);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    queued = context->connection->write_queue->size;

    error = push_stream_http2_connection(http2_connection, http2_stream, "/style.css");
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the stream that the promise reserves carries an even numbered
    identifier, the odd ones belong to the peer, and it hangs from
    the stream that has made the promise */
    V_ASSERT_EQ_U(http2_connection->push_stream_id, 2);
    promised = find_stream_http2_connection(http2_connection, 2);
    V_ASSERT_NOT_NULL(promised);
    V_ASSERT_EQ_U(promised->state, HTTP2_STATE_RESERVED_LOCAL);
    V_ASSERT_EQ_U(promised->priority.dependency, 1);
    V_ASSERT_EQ_U(http2_connection->count, 2);

    /* the stream that has asked is the current one again, the
    handler of it is still writing the response of the request */
    V_ASSERT_EQ_P(context->http_connection->request, http2_stream->request);

    /* the frame of the promise travels on the stream that has asked
    and carries the identifier of the one it reserves */
    get_value_linked_list(context->connection->write_queue, queued, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    error = decode_frame_http2(data->data, data->size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_PUSH_PROMISE);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);
    V_ASSERT(http2_frame.flags & HTTP2_FLAG_END_HEADERS);
    V_ASSERT_EQ_U(decode_number_http2(http2_frame.payload), 2);

    /* the block of the promise describes the request of the resource
    that the peer receives without ever having asked for it */
    create_hpack_table(&decoder);
    collector.count = 0;
    error = decode_hpack(
        decoder,
        &http2_frame.payload[4],
        http2_frame.length - 4,
        collect_hpack_test,
        (void *) &collector
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 4);
    V_ASSERT_EQ_S(collector.names[0], ":method");
    V_ASSERT_EQ_S(collector.values[0], "GET");
    V_ASSERT_EQ_S(collector.names[1], ":scheme");
    V_ASSERT_EQ_S(collector.values[1], "http");
    V_ASSERT_EQ_S(collector.names[2], ":path");
    V_ASSERT_EQ_S(collector.values[2], "/style.css");
    V_ASSERT_EQ_S(collector.names[3], ":authority");
    V_ASSERT_EQ_S(collector.values[3], "localhost");
    delete_hpack_table(decoder);

    /* the handler has been driven for the promised resource in the
    very same shape a request of the peer would have produced */
    V_ASSERT_EQ_U(_record.begin, 2);
    V_ASSERT_EQ_U(_record.url, 2);
    V_ASSERT_EQ_S(_record.path, "/style.css");
    V_ASSERT_EQ_U(_record.headers, 2);
    V_ASSERT_EQ_U(_record.complete, 2);

    /* the response of the promised resource has gone out on the
    stream that the promise has reserved */
    V_ASSERT(promised->complete == TRUE);

    /* a peer that has turned the pushing off gets nothing at all, no
    stream is reserved and no frame is written */
    http2_connection->remote.enable_push = FALSE;
    queued = context->connection->write_queue->size;
    error = push_stream_http2_connection(http2_connection, http2_stream, "/script.js");
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->push_stream_id, 2);
    V_ASSERT_EQ_U(http2_connection->count, 2);
    V_ASSERT_EQ_U(context->connection->write_queue->size, queued);

    /* a peer that holds as many streams open as it allows gets
    nothing either, a promise opens one of them */
    http2_connection->remote.enable_push = TRUE;
    http2_connection->remote.max_concurrent_streams = 2;
    error = push_stream_http2_connection(http2_connection, http2_stream, "/script.js");
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->count, 2);
    V_ASSERT_EQ_U(context->connection->write_queue->size, queued);

    _responder = FALSE;
    _delete_http2_test(context, http2_connection);

    /* every one of the buffers that the promise has taken is gone
    together with the connection that carried them */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_window(void) {
    /* allocates space for the chain of the connection, for the
    session and for the stream the windows apply to */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);
    open_stream_http2_connection(http2_connection, 1, &http2_stream);

    /* an increment of nothing at all carries no meaning and is an
    error, both on the connection and on a stream */
    error = update_window_http2_connection(http2_connection, 0, 0);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);
    error = update_window_http2_connection(http2_connection, 1, 0);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* an increment that applies to the connection widens the window
    that bounds every one of the streams together */
    error = update_window_http2_connection(http2_connection, 0, 1024);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->send_window, HTTP2_DEFAULT_WINDOW_SIZE + 1024);
    V_ASSERT_EQ_U(http2_stream->send_window, HTTP2_DEFAULT_WINDOW_SIZE);

    /* an increment that applies to a stream widens only that one */
    error = update_window_http2_connection(http2_connection, 1, 2048);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_stream->send_window, HTTP2_DEFAULT_WINDOW_SIZE + 2048);

    /* an increment for a stream that has never been opened refers to
    an idle one, which carries no window at all to be widened */
    error = update_window_http2_connection(http2_connection, 99, 1024);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* an increment for a stream that has already been closed is
    discarded, the peer is allowed to send one that crosses it */
    open_stream_http2_connection(http2_connection, 3, &http2_stream);
    close_stream_http2_connection(http2_connection, http2_stream);
    error = update_window_http2_connection(http2_connection, 3, 1024);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* takes the stream that is still open again, the closing of
    another one may have moved it inside the table */
    http2_stream = find_stream_http2_connection(http2_connection, 1);

    /* an increment that takes a window beyond the largest value the
    protocol represents is refused, on both of the levels */
    error = update_window_http2_connection(http2_connection, 1, (unsigned int) HTTP2_MAX_WINDOW_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_FLOW_CONTROL_ERROR);
    error = update_window_http2_connection(http2_connection, 0, (unsigned int) HTTP2_MAX_WINDOW_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_FLOW_CONTROL_ERROR);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_settings(void) {
    /* allocates space for the chain of the connection, for the
    session and for the payload of the settings */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    unsigned char payload[16];
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);
    open_stream_http2_connection(http2_connection, 1, &http2_stream);
    V_ASSERT_EQ_U(http2_stream->send_window, HTTP2_DEFAULT_WINDOW_SIZE);

    /* a change of the initial window is carried over to the streams
    that are already open, which is the part most commonly missed */
    payload[0] = 0x00;
    payload[1] = HTTP2_SETTING_INITIAL_WINDOW_SIZE;
    encode_number_http2(&payload[2], HTTP2_DEFAULT_WINDOW_SIZE + 4096);
    error = apply_settings_http2_connection(http2_connection, payload, HTTP2_SETTING_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->remote.initial_window_size, HTTP2_DEFAULT_WINDOW_SIZE + 4096);
    V_ASSERT_EQ_U(http2_stream->send_window, HTTP2_DEFAULT_WINDOW_SIZE + 4096);

    /* a window that shrinks is carried over just the same, a stream
    is allowed to end up with a window below zero */
    encode_number_http2(&payload[2], 0);
    error = apply_settings_http2_connection(http2_connection, payload, HTTP2_SETTING_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_stream->send_window, 0);

    /* a change that would take the window of a stream beyond the
    largest value the protocol represents is a flow control error,
    the sum of the two is never computed as such */
    http2_stream->send_window = HTTP2_MAX_WINDOW_SIZE;
    encode_number_http2(&payload[2], (unsigned int) HTTP2_MAX_WINDOW_SIZE);
    error = apply_settings_http2_connection(http2_connection, payload, HTTP2_SETTING_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_FLOW_CONTROL_ERROR);

    /* a window above the largest one that the protocol represents is
    refused before it ever reaches a stream */
    encode_number_http2(&payload[2], (unsigned int) HTTP2_MAX_WINDOW_SIZE + 1);
    error = apply_settings_http2_connection(http2_connection, payload, HTTP2_SETTING_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_FLOW_CONTROL_ERROR);

    /* a table smaller than the one of this end shrinks the encoder,
    it is never allowed to index beyond what the peer holds */
    payload[1] = HTTP2_SETTING_HEADER_TABLE_SIZE;
    encode_number_http2(&payload[2], 512);
    error = apply_settings_http2_connection(http2_connection, payload, HTTP2_SETTING_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->encoder->max_size, 512);

    /* a payload that the frame layer refuses is refused here too */
    payload[1] = HTTP2_SETTING_MAX_FRAME_SIZE;
    encode_number_http2(&payload[2], 1);
    error = apply_settings_http2_connection(http2_connection, payload, HTTP2_SETTING_SIZE);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_frames(void) {
    /* allocates space for the chain of the connection, for the
    session and for the frames being handled */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char payload[16];
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* the settings of the peer are applied and acknowledged, the
    acknowledgement of the ones of this end carries nothing */
    payload[0] = 0x00;
    payload[1] = HTTP2_SETTING_MAX_CONCURRENT_STREAMS;
    encode_number_http2(&payload[2], 8);
    http2_frame.type = HTTP2_SETTINGS;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = HTTP2_SETTING_SIZE;
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->remote.max_concurrent_streams, 8);

    http2_frame.flags = HTTP2_FLAG_ACK;
    http2_frame.length = 0;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* a ping of the peer is answered and the acknowledgement of the
    one of this end is not */
    http2_frame.type = HTTP2_PING;
    http2_frame.flags = 0x00;
    http2_frame.length = HTTP2_PING_SIZE;
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_frame.flags = HTTP2_FLAG_ACK;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the goaway of the peer closes the connection, no new stream is
    accepted from that point on */
    http2_frame.type = HTTP2_GOAWAY;
    http2_frame.flags = 0x00;
    http2_frame.length = 8;
    encode_number_http2(payload, 0);
    encode_number_http2(&payload[4], HTTP2_NO_ERROR);
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT(http2_connection->goaway == TRUE);

    /* the update of a window of the connection widens it */
    http2_connection->goaway = FALSE;
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.length = 4;
    encode_number_http2(payload, 1024);
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->send_window, HTTP2_DEFAULT_WINDOW_SIZE + 1024);

    /* the priority of a stream is recorded, one that depends on
    itself would make the tree carry a cycle and is refused */
    open_stream_http2_connection(http2_connection, 1, &http2_stream);
    http2_frame.type = HTTP2_PRIORITY;
    http2_frame.stream_id = 1;
    http2_frame.length = HTTP2_PRIORITY_SIZE;
    encode_number_http2(payload, 0);
    payload[4] = 31;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_stream->priority.weight, 32);

    encode_number_http2(payload, 1);
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* the reset of a stream closes it, one that has never been open
    refers to an idle stream and is refused */
    http2_frame.type = HTTP2_RST_STREAM;
    http2_frame.length = 4;
    encode_number_http2(payload, HTTP2_CANCEL);
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->count, 0);

    http2_frame.stream_id = 99;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a promise is never received by a server, only a client is the
    one that receives one */
    http2_frame.type = HTTP2_PUSH_PROMISE;
    http2_frame.stream_id = 1;
    http2_frame.length = 0;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a frame of an unknown type is ignored so that the protocol may
    be extended without breaking this end */
    http2_frame.type = 0xfe;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_headers(void) {
    /* allocates space for the chain of the connection, for the
    session and for the block of the request */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* hands a complete request over as a single frame, the block
    carries the four pseudo headers and one regular field */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/index.html");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the handler has been driven through the very same sequence
    that an HTTP/1.1 message produces */
    V_ASSERT_EQ_U(_record.begin, 1);
    V_ASSERT_EQ_U(_record.url, 1);
    V_ASSERT_EQ_S(_record.path, "/index.html");
    V_ASSERT_EQ_U(_record.headers, 1);
    V_ASSERT_EQ_U(_record.complete, 1);

    /* the authority reaches the handler as the host header and the
    regular field reaches it as it stands */
    V_ASSERT_EQ_U(_record.field, 2);
    V_ASSERT_EQ_S(_record.name, "accept");
    V_ASSERT_EQ_S(_record.header, "text/html");

    /* the message of the stream carries the pseudo headers that the
    block has delivered */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->request->method, HTTP_GET);
    V_ASSERT_EQ_S(http2_stream->request->scheme, HTTP_SCHEME);
    V_ASSERT_EQ_S((char *) http2_stream->request->path, "/index.html");
    V_ASSERT_EQ_S((char *) http2_stream->request->authority, "localhost");
    V_ASSERT(http2_stream->end_stream == TRUE);
    V_ASSERT(http2_stream->headers_complete == TRUE);
    V_ASSERT_EQ_U(http2_stream->state, HTTP2_STATE_HALF_CLOSED_REMOTE);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

/**
 * Writes a header block carrying the provided sequence of fields
 * into the buffer, the sequence is a flat one of names and values
 * that ends at an unset name.
 *
 * @param buffer The buffer to write the block into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param fields The names and the values of the fields, one after
 * the other and ending at an unset value.
 * @return The size in bytes of the block that has been written.
 */
static size_t _fields_http2_test(unsigned char *buffer, size_t buffer_size, const char **fields) {
    struct hpack_table_t *encoder;
    struct hpack_header_t hpack_header;
    size_t offset = 0;
    size_t index;

    create_hpack_table(&encoder);

    for(index = 0; fields[index] != NULL; index += 2) {
        hpack_header.name = (unsigned char *) fields[index];
        hpack_header.name_size = strlen(fields[index]);
        hpack_header.value = (unsigned char *) fields[index + 1];
        hpack_header.value_size = strlen(fields[index + 1]);
        encode_hpack(encoder, buffer, buffer_size, &offset, &hpack_header, FALSE);
    }

    delete_hpack_table(encoder);

    return offset;
}

/**
 * Hands the provided sequence of fields to a session of its own as
 * a complete request, a block that is refused leaves the decoder
 * out of step and so it is never reused.
 *
 * @param fields The names and the values of the fields, one after
 * the other and ending at an unset value.
 * @return The resulting error code.
 */
static ERROR_CODE _deliver_http2_test(const char **fields) {
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    ERROR_CODE return_value;

    _create_http2_test(&context, &http2_connection);

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _fields_http2_test(block, sizeof(block), fields);
    http2_frame.payload = block;
    return_value = handle_frame_http2_connection(http2_connection, &http2_frame);

    _delete_http2_test(context, http2_connection);

    return return_value;
}

const char *test_http2_connection_fields(void) {
    /* allocates space for the chain of the connection, for the
    session and for the block of the request */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    size_t index;
    ERROR_CODE error;

    /* the blocks that a peer is not allowed to send, every one of
    them carrying a single violation of the rules of a request */
    static const char *invalid[][12] = {
        {":method", "BREW", ":scheme", "http", ":path", "/", NULL},
        {":method", "GET", ":scheme", "ftp", ":path", "/", NULL},
        {":method", "GET", ":scheme", "http", ":path", "", NULL},
        {":method", "GET", ":method", "POST", ":scheme", "http", ":path", "/", NULL},
        {":method", "GET", ":scheme", "http", ":scheme", "http", ":path", "/", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", ":path", "/", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", ":authority", "a", ":authority", "b", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", ":status", "200", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "accept", "*/*", ":authority", "a", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "Accept", "*/*", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "connection", "close", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "keep-alive", "1", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "upgrade", "h2c", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "transfer-encoding", "chunked", NULL},
        {":method", "GET", ":scheme", "http", ":path", "/", "te", "gzip", NULL}
    };

    /* the block that leaves out the path, it is the only one of the
    malformed ones that survives the decoding of the block */
    static const char *partial[] = {
        ":method", "GET", ":scheme", "http", NULL
    };

    /* the fields that a peer is allowed to send, the secure scheme
    and the only value the trailer announcement carries */
    static const char *valid[] = {
        ":method", "POST", ":scheme", "https", ":path", "/secure", "te", "trailers", NULL
    };

    /* every one of the malformed blocks is refused, the decoding of
    it is aborted half way and so the table of the connection is out
    of step with the one of the peer from that point on */
    for(index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        error = _deliver_http2_test(invalid[index]);
        V_ASSERT_EQ_U(error, HTTP2_COMPRESSION_ERROR);
    }

    /* a block that leaves out one of the required pseudo headers is
    verified once the complete set of them has been gathered, which
    happens after the decoding of it has run to the end */
    error = _deliver_http2_test(partial);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* the block that carries the secure scheme and the announcement
    of the trailers is served as any other one is */
    _create_http2_test(&context, &http2_connection);

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _fields_http2_test(block, sizeof(block), valid);
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.complete, 1);

    /* the scheme is kept as one of the static strings, so nothing at
    all has to be released together with the message */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->request->method, HTTP_POST);
    V_ASSERT_EQ_S(http2_stream->request->scheme, HTTPS_SCHEME);
    V_ASSERT_EQ_S((char *) http2_stream->request->path, "/secure");

    /* the message carries no authority at all, so nothing has been
    handed over as the host header of it */
    V_ASSERT_EQ_U(_record.field, 1);
    V_ASSERT_EQ_S(_record.name, "te");

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

/**
 * The handler that the dispatching one switches the connection
 * onto, it stands in for the target of a message and counts the
 * releases of it so that a test verifies that it is the one that
 * gets to release what it has taken.
 */
static struct http_handler_t _target;

/**
 * The number of times the handler of the target has been unset,
 * the release of a message is what drives it.
 */
static size_t _released = 0;

static ERROR_CODE _set_target_http2_test(struct http_connection_t *http_connection) {
    RAISE_NO_ERROR;
}

static ERROR_CODE _unset_target_http2_test(struct http_connection_t *http_connection) {
    _released++;
    RAISE_NO_ERROR;
}

/**
 * Stands in for the setting of a handler that dispatches, it
 * switches the connection onto the target as the url of a message
 * reaches it, which is what the one of the tree does.
 *
 * @param http_connection The connection being served.
 * @return The resulting error code.
 */
static ERROR_CODE _url_dispatch_http2_test(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    struct connection_t *connection = (struct connection_t *) http_request->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* switches the connection onto the target, the very same way the
    handler that dispatches does once it resolves one */
    http_connection->http_handler->unset(http_connection);
    _target.set(http_connection);
    http_connection->http_handler = &_target;

    return _url_http2_test(http_request, data, data_size);
}

static ERROR_CODE _set_dispatch_http2_test(struct http_connection_t *http_connection) {
    _set_http2_test(http_connection);
    http_connection->http_settings->on_url = _url_dispatch_http2_test;
    RAISE_NO_ERROR;
}

const char *test_http2_connection_dispatch(void) {
    /* allocates space for the chain of the connection, for the
    session and for the block of the request */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* installs the handler that stands in for the target and makes
    the one of the tests switch onto it as a dispatching one does */
    _target.name = (unsigned char *) "target";
    _target.resolve_index = FALSE;
    _target.set = _set_target_http2_test;
    _target.unset = _unset_target_http2_test;
    _target.reset = NULL;
    _released = 0;
    _handler.set = _set_dispatch_http2_test;

    /* hands a complete request over, the handler that serves it is
    the target rather than the one the connection started with */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/dispatched");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_S(_record.path, "/dispatched");

    /* the stream carries the handler that is actually serving it, so
    that the release of the message reaches the one that took it */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_P(http2_stream->http_handler, &_target);

    /* closing the stream releases the message through the handler of
    the target, the one that never served it is left alone */
    close_stream_http2_connection(http2_connection, http2_stream);
    V_ASSERT_EQ_U(_released, 1);

    _handler.set = _set_http2_test;
    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_continuation(void) {
    /* allocates space for the chain of the connection, for the
    session and for the block that is going to be split */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    size_t block_size;
    size_t split;
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    the assembling of a block over several frames may be verified to
    release every one of the buffers it takes */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);
    block_size = _request_http2_test(block, sizeof(block), "/split");
    split = block_size / 2;

    /* the first half opens the block, the sequence stays open as the
    frame does not close the headers */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 1;
    http2_frame.length = split;
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->continuation, 1);
    V_ASSERT_EQ_U(_record.headers, 0);

    /* no other frame is allowed to come between the frames of a
    block, not even one that belongs to another stream */
    http2_frame.type = HTTP2_PING;
    http2_frame.stream_id = 0;
    http2_frame.length = HTTP2_PING_SIZE;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a continuation on a stream other than the one that opened the
    block is refused just the same */
    http2_frame.type = HTTP2_CONTINUATION;
    http2_frame.stream_id = 3;
    http2_frame.length = 0;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* the second half closes the block, at which point the handler
    is driven through the complete request */
    http2_frame.type = HTTP2_CONTINUATION;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = block_size - split;
    http2_frame.payload = &block[split];
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_connection->continuation, 0);
    V_ASSERT_EQ_U(_record.headers, 1);
    V_ASSERT_EQ_S(_record.path, "/split");

    /* a continuation that does not follow a block is not expected at
    all and is refused */
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    _delete_http2_test(context, http2_connection);

    /* the buffer that assembled the block is gone together with the
    session, nothing at all is left behind */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_trailers(void) {
    /* allocates space for the chain of the connection, for the
    session and for the blocks that the streams carry */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    struct hpack_table_t *encoder;
    struct hpack_header_t hpack_header;
    unsigned char block[256];
    unsigned char payload[8];
    size_t offset;
    size_t index;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* opens a stream with a message that carries a payload, so that
    the section of the trailers is allowed to follow it */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/trailers");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.headers, 1);

    /* the section of the trailers is a second block on the very same
    stream and it is required to close it */
    create_hpack_table(&encoder);
    offset = 0;
    hpack_header.name = (unsigned char *) "x-checksum";
    hpack_header.name_size = 10;
    hpack_header.value = (unsigned char *) "1234";
    hpack_header.value_size = 4;
    encode_hpack(encoder, block, sizeof(block), &offset, &hpack_header, FALSE);
    delete_hpack_table(encoder);

    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.length = offset;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT(http2_stream->request->trailers == TRUE);
    V_ASSERT_EQ_S(_record.name, "x-checksum");
    V_ASSERT_EQ_S(_record.header, "1234");
    V_ASSERT_EQ_U(_record.complete, 1);

    _delete_http2_test(context, http2_connection);

    /* a peer that opens more streams than this end has announced is
    refused the excess rather than taken down */
    _create_http2_test(&context, &http2_connection);
    http2_connection->settings.max_concurrent_streams = 1;

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.payload = block;

    for(index = 0; index < 2; index++) {
        http2_frame.stream_id = (unsigned int) (index * 2 + 1);
        http2_frame.length = _request_http2_test(block, sizeof(block), "/many");
        error = handle_frame_http2_connection(http2_connection, &http2_frame);
        V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    }

    V_ASSERT_EQ_U(http2_connection->count, 1);
    V_ASSERT_NULL(find_stream_http2_connection(http2_connection, 3));

    /* the stream that has been refused is told so through a frame of
    its own, the connection carries on serving the other one */
    V_ASSERT(context->connection->write_queue->size > 0);
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    /* an increment that overflows the window of a stream takes only
    that stream down, the connection is left alone */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    http2_stream->send_window = HTTP2_MAX_WINDOW_SIZE;
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 1;
    http2_frame.length = 4;
    encode_number_http2(payload, (unsigned int) HTTP2_MAX_WINDOW_SIZE);
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_closed(void) {
    /* allocates space for the chain of the connection, for the
    session and for the buffers that are written on it */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    char *headers;
    unsigned char *message;
    size_t count;
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    a write that is discarded may be verified to release its buffer */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);

    /* opens a stream, answers nothing at all and closes it, the
    handler of a message is free to answer it later than that */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/gone");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the message of the stream stays the current one, the writing
    operations resolve the stream out of it */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    context->http_connection->request = http2_stream->request;

    /* a response that closes the message together with the block of
    the headers carries the flag that marks the end of the stream */
    headers = (char *) MALLOC(VIRIATUM_HTTP_SIZE);
    count = write_status_http2(
        context->connection,
        headers,
        VIRIATUM_HTTP_SIZE,
        HTTP20,
        204,
        "No Content",
        KEEP_ALIVE
    );
    count = write_end_http2(context->connection, headers, VIRIATUM_HTTP_SIZE, count, TRUE);
    V_ASSERT(http2_stream->complete == TRUE);
    error = write_flush_http2(context->connection, (unsigned char *) headers, count, NULL, NULL);
    V_ASSERT_EQ_U(error, 0);

    /* closes the stream and then writes for it, both of the writes
    are discarded and the buffers of them released */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    close_stream_http2_connection(http2_connection, http2_stream);
    V_ASSERT_NULL(context->http_connection->request);

    headers = (char *) MALLOC(VIRIATUM_HTTP_SIZE);
    error = write_flush_http2(context->connection, (unsigned char *) headers, 16, NULL, NULL);
    V_ASSERT_EQ_U(error, 0);

    message = (unsigned char *) MALLOC(16);
    error = write_chunk_http2(context->connection, message, 16, TRUE, NULL, NULL);
    V_ASSERT_EQ_U(error, 0);

    _delete_http2_test(context, http2_connection);

    /* the buffers of the writes that have been discarded are gone
    together with the ones the response has taken */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_data(void) {
    /* allocates space for the chain of the connection, for the
    session and for the frames being handled */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    unsigned char payload[8];
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* opens a stream through a block that does not close it, so that
    the payload of the request may follow */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/upload");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.complete, 0);

    /* the payload reaches the handler and is accounted against both
    of the windows, which are then widened again */
    http2_frame.type = HTTP2_DATA;
    http2_frame.flags = 0x00;
    http2_frame.length = 4;
    memcpy(payload, "body", 4);
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.body, 1);
    V_ASSERT_EQ_S(_record.payload, "body");
    V_ASSERT_EQ_U(http2_connection->receive_window, HTTP2_DEFAULT_WINDOW_SIZE);

    /* the frame that closes the stream completes the message */
    http2_frame.flags = HTTP2_FLAG_END_STREAM;
    http2_frame.length = 0;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.complete, 1);

    /* a payload that follows the closing of the stream by the peer
    is refused, the stream is no longer receiving */
    http2_frame.flags = 0x00;
    http2_frame.length = 4;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_STREAM_CLOSED);

    /* a payload larger than the window of the connection is a flow
    control error of the connection as a whole */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    http2_stream->end_stream = FALSE;
    http2_connection->receive_window = 2;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_FLOW_CONTROL_ERROR);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_errors(void) {
    /* allocates space for the chain of the connection, for the
    session and for the block being refused */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_frame_t http2_frame;
    struct hpack_table_t *encoder;
    struct hpack_header_t hpack_header;
    unsigned char block[256];
    size_t offset;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* a block whose compression is broken corrupts the table of the
    connection and so it never is a stream error */
    block[0] = 0xff;
    block[1] = 0xff;
    block[2] = 0xff;
    block[3] = 0xff;
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = 4;
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_COMPRESSION_ERROR);

    _delete_http2_test(context, http2_connection);

    /* a request that carries no path at all is not a complete one,
    the pseudo headers of a request are all required */
    _create_http2_test(&context, &http2_connection);
    create_hpack_table(&encoder);
    offset = 0;
    hpack_header.name = (unsigned char *) ":method";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) "GET";
    hpack_header.value_size = 3;
    encode_hpack(encoder, block, sizeof(block), &offset, &hpack_header, FALSE);
    delete_hpack_table(encoder);

    http2_frame.length = offset;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    _delete_http2_test(context, http2_connection);

    /* a block carrying a connection specific field is refused, they
    carry no meaning at all under this protocol */
    _create_http2_test(&context, &http2_connection);
    create_hpack_table(&encoder);
    offset = _request_http2_test(block, sizeof(block), "/");
    hpack_header.name = (unsigned char *) "connection";
    hpack_header.name_size = 10;
    hpack_header.value = (unsigned char *) "keep-alive";
    hpack_header.value_size = 10;
    encode_hpack(encoder, block, sizeof(block), &offset, &hpack_header, FALSE);
    delete_hpack_table(encoder);

    http2_frame.length = offset;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_COMPRESSION_ERROR);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_read(void) {
    /* allocates space for the chain of the connection, for the
    session and for the stream of bytes being fed to it */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    unsigned char stream[512];
    unsigned char block[256];
    size_t block_size;
    size_t offset = 0;
    size_t index;

    /* builds the connection and lets the upgrade take it over,
    which is what the recognition of the protocol does */
    create_test_context(&context);
    create_test_connection(context);

    _handler.name = (unsigned char *) "record";
    _handler.resolve_index = FALSE;
    _handler.set = _set_http2_test;
    _handler.unset = _unset_http2_test;
    _handler.reset = NULL;
    context->http_connection->base_handler = &_handler;
    memset(&_record, 0, sizeof(_record));

    upgrade_handler_stream_http2(context->io_connection);
    http2_connection = context->http_connection->http2_connection;
    V_ASSERT_NOT_NULL(http2_connection);
    V_ASSERT_EQ_P(context->io_connection->on_data, data_handler_stream_http2);

    /* a preface that arrives split over two reads is only consumed
    once the complete one is available */
    data_handler_stream_http2(context->io_connection, (unsigned char *) HTTP2_PREFACE, 10);
    V_ASSERT(http2_connection->preface == FALSE);
    data_handler_stream_http2(
        context->io_connection,
        (unsigned char *) &HTTP2_PREFACE[10],
        HTTP2_PREFACE_SIZE - 10
    );
    V_ASSERT(http2_connection->preface == TRUE);

    /* the buffer of the connection is compacted once the preface has
    been consumed, nothing at all is left pending */
    V_ASSERT_EQ_U(context->http_connection->read_offset, 0);
    V_ASSERT_EQ_U(context->http_connection->buffer_offset, 0);

    /* builds a stream of bytes carrying an empty settings frame
    followed by a complete request */
    encode_frame_http2(&stream[offset], sizeof(stream) - offset, 0, HTTP2_SETTINGS, 0x00, 0);
    offset += HTTP2_HEADER_SIZE;

    block_size = _request_http2_test(block, sizeof(block), "/read");
    encode_frame_http2(
        &stream[offset],
        sizeof(stream) - offset,
        block_size,
        HTTP2_HEADERS,
        HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM,
        1
    );
    offset += HTTP2_HEADER_SIZE;
    memcpy(&stream[offset], block, block_size);
    offset += block_size;

    /* feeds the stream one byte at a time, which exercises both the
    frame that is not yet complete and the compaction of the buffer */
    for(index = 0; index < offset; index++) {
        data_handler_stream_http2(context->io_connection, &stream[index], 1);
    }

    /* the request has reached the handler through the very same
    sequence a message of HTTP/1.1 produces */
    V_ASSERT_EQ_U(_record.begin, 1);
    V_ASSERT_EQ_U(_record.url, 1);
    V_ASSERT_EQ_S(_record.path, "/read");
    V_ASSERT_EQ_U(_record.headers, 1);
    V_ASSERT_EQ_U(_record.complete, 1);
    V_ASSERT_EQ_U(http2_connection->count, 1);

    /* nothing is left pending in the buffer once the last of the
    frames has been handled */
    V_ASSERT_EQ_U(context->http_connection->buffer_offset, 0);

    /* a frame that violates the protocol takes the connection down,
    the peer is told the reason first and the closing only happens
    once that frame has actually reached it */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    encode_frame_http2(stream, sizeof(stream), 0, HTTP2_HEADERS, HTTP2_FLAG_END_HEADERS, 0);
    data_handler_stream_http2(context->io_connection, stream, HTTP2_HEADER_SIZE);
    V_ASSERT(http2_connection->goaway == TRUE);
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    V_ASSERT(context->connection->write_queue->size > 0);

    /* the completion of the write of that frame is what takes the
    connection down, the peer learns the reason first */
    flush_test_connection(context, NULL, 0);
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);

    delete_http2_connection(http2_connection);
    delete_test_connection(context);
    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_preface(void) {
    /* allocates space for the chain of the connection and for the
    preface that is not going to match */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    unsigned char preface[HTTP2_PREFACE_SIZE];

    create_test_context(&context);
    create_test_connection(context);
    upgrade_handler_stream_http2(context->io_connection);
    http2_connection = context->http_connection->http2_connection;

    /* a preface that does not match belongs to another protocol
    altogether, so the connection is taken down */
    memcpy(preface, HTTP2_PREFACE, HTTP2_PREFACE_SIZE);
    preface[0] = 'X';
    data_handler_stream_http2(context->io_connection, preface, HTTP2_PREFACE_SIZE);
    V_ASSERT(http2_connection->preface == FALSE);
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);

    delete_http2_connection(http2_connection);
    delete_test_connection(context);
    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

/**
 * Gathers the frames that the connection of the provided context
 * has queued, so that a test verifies what actually went out.
 *
 * @param context The test context holding the connection.
 * @param buffer The buffer to gather the frames into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @return The number of bytes that have been gathered.
 */
static size_t _written_http2_test(struct test_context_t *context, unsigned char *buffer, size_t buffer_size) {
    struct data_t *data;
    size_t index;
    size_t offset = 0;

    for(index = 0; index < context->connection->write_queue->size; index++) {
        get_value_linked_list(context->connection->write_queue, index, (void **) &data);
        if(data == NULL) { break; }
        if(offset + data->size > buffer_size) { break; }
        memcpy(&buffer[offset], data->data, data->size);
        offset += data->size;
    }

    return offset;
}

const char *test_http2_connection_detect(void) {
    /* allocates space for the chain of the connection and for the
    bytes that tell the two versions of the protocol apart */
    struct test_context_t *context;
    unsigned char preface[HTTP2_PREFACE_SIZE];
    size_t split = HTTP2_PREFACE_SIZE / 2;
    ERROR_CODE error;

    /* a connection that starts with a message of HTTP/1.1 is never
    taken over, the decision is taken once and only once */
    create_test_context(&context);
    create_test_connection(context);
    _install_http2_test(context);
    V_ASSERT(context->http_connection->detect == TRUE);

    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) "GET / HT",
        8
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(context->http_connection->detect == FALSE);
    V_ASSERT_NULL(context->http_connection->http2_connection);

    delete_test_connection(context);
    delete_test_context(context);

    /* a connection that starts with the preface is taken over by a
    session, the bytes of it arriving over as many reads as the peer
    happens to have used */
    create_test_context(&context);
    create_test_connection(context);
    _install_http2_test(context);
    memcpy(preface, HTTP2_PREFACE, HTTP2_PREFACE_SIZE);

    error = data_handler_stream_http(context->io_connection, preface, split);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(context->http_connection->detect == TRUE);
    V_ASSERT_NULL(context->http_connection->http2_connection);

    error = data_handler_stream_http(
        context->io_connection,
        &preface[split],
        HTTP2_PREFACE_SIZE - split
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(context->http_connection->detect == FALSE);
    V_ASSERT_NOT_NULL(context->http_connection->http2_connection);
    V_ASSERT(context->http_connection->http2_connection->preface == TRUE);

    /* the settings of this end have gone out as soon as the session
    has taken the connection over, which is what the peer waits for */
    V_ASSERT(context->connection->write_queue->size > 0);

    /* the session is released together with the connection that was
    being driven by it, nothing else holds it */
    delete_test_connection(context);
    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_response(void) {
    /* allocates space for the chain of the connection, for the
    session and for the frames that the response produces */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    struct hpack_table_t *decoder;
    struct hpack_collector_t collector;
    unsigned char block[256];
    unsigned char written[1024];
    size_t size;
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    the complete cycle of a response may be verified to release
    every one of the buffers it takes */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);
    _responder = TRUE;

    /* hands a complete request over, the handler answers it through
    the operations that the connection carries */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/answer");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.complete, 1);

    /* gathers what has been queued, the settings of this end are not
    among it as the preface has not been consumed in this test */
    size = _written_http2_test(context, written, sizeof(written));
    V_ASSERT(size > HTTP2_HEADER_SIZE);

    /* the first frame of the response carries the block of the
    headers and it does not close the stream by itself */
    error = decode_frame_http2(written, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_HEADERS);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);
    V_ASSERT(http2_frame.flags & HTTP2_FLAG_END_HEADERS);
    V_ASSERT(!(http2_frame.flags & HTTP2_FLAG_END_STREAM));

    /* the block decodes into the status and the fields that the
    handler has written, in the lower case the protocol requires */
    create_hpack_table(&decoder);
    collector.count = 0;
    error = decode_hpack(
        decoder,
        http2_frame.payload,
        http2_frame.length,
        collect_hpack_test,
        (void *) &collector
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_S(collector.names[0], ":status");
    V_ASSERT_EQ_S(collector.values[0], "200");
    V_ASSERT_EQ_S(collector.names[2], "content-length");
    V_ASSERT_EQ_S(collector.values[2], "2");
    delete_hpack_table(decoder);

    /* the frame that follows the block carries the payload and it is
    the one that closes the stream */
    error = decode_frame_http2(
        &written[HTTP2_HEADER_SIZE + http2_frame.length],
        size - HTTP2_HEADER_SIZE - http2_frame.length,
        &http2_frame
    );
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);
    V_ASSERT_EQ_U(http2_frame.length, 2);
    V_ASSERT(http2_frame.flags & HTTP2_FLAG_END_STREAM);
    V_ASSERT_MEM(http2_frame.payload, "ok", 2);

    /* the stream is still open as the completion of the write is the
    thing that closes it and no io loop runs in a test */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT(http2_stream->complete == TRUE);

    _responder = FALSE;
    _delete_http2_test(context, http2_connection);

    /* every one of the buffers that the response has taken is gone
    together with the connection that carried them */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_complete(void) {
    /* allocates space for the chain of the connection, for the
    session and for the streams that are answered */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    unsigned char written[1024];
    size_t size;
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    the completion of a write may be verified to release both the
    buffers and the structures that carry a stream through it */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);
    _responder = TRUE;

    /* hands two complete requests over, the handler answers both of
    them and the responses are queued on the connection */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.payload = block;

    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/first");
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_frame.stream_id = 3;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/second");
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* every one of the writes carries a structure that restores the
    stream once it completes, the block of the headers and the
    payload of the response for each of the streams */
    V_ASSERT_EQ_U(http2_connection->count, 2);
    V_ASSERT_EQ_U(http2_connection->callbacks->size, 4);

    /* completes the writes, a stream only closes once the payload
    that closes the message of it has actually left this end */
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > HTTP2_HEADER_SIZE);
    V_ASSERT_EQ_U(http2_connection->count, 0);
    V_ASSERT_EQ_U(http2_connection->callbacks->size, 0);
    V_ASSERT_NULL(find_stream_http2_connection(http2_connection, 1));
    V_ASSERT_NULL(find_stream_http2_connection(http2_connection, 3));

    /* the connection is left pointing at no message at all, the ones
    it was pointing at have gone together with the streams */
    V_ASSERT_NULL(context->http_connection->request);

    /* the closing of the streams does not take the connection down,
    a session serves one message after the other over it */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _responder = FALSE;
    _delete_http2_test(context, http2_connection);

    /* both the buffers of the responses and the structures that were
    carrying the streams through the writes are gone */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_error(void) {
    /* allocates space for the chain of the connection, for the
    session and for the frames that the error produces */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_frame_t http2_frame;
    struct hpack_table_t *decoder;
    struct hpack_collector_t collector;
    unsigned char block[256];
    unsigned char written[1024];
    size_t size;
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    the writing of an error may be verified to release the buffers
    that it takes, the payload of it is copied */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);

    /* opens a stream through a complete request, the error is the
    response of the message that it carries */
    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/missing");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* writes an error for the message, it reaches the peer as the
    frames of this protocol rather than as the text of HTTP/1.1 */
    error = write_http_error(
        context->connection,
        NULL,
        0,
        HTTP20,
        404,
        "Not Found",
        NULL,
        KEEP_ALIVE,
        NULL,
        NULL
    );
    V_ASSERT_EQ_U(error, 0);

    /* the first of the frames carries the block of the headers and
    it does not close the stream by itself */
    size = _written_http2_test(context, written, sizeof(written));
    error = decode_frame_http2(written, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_HEADERS);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);
    V_ASSERT(http2_frame.flags & HTTP2_FLAG_END_HEADERS);

    /* the block carries the status of the error together with the
    fields that describe the payload of it */
    create_hpack_table(&decoder);
    collector.count = 0;
    error = decode_hpack(
        decoder,
        http2_frame.payload,
        http2_frame.length,
        collect_hpack_test,
        (void *) &collector
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 4);
    V_ASSERT_EQ_S(collector.names[0], ":status");
    V_ASSERT_EQ_S(collector.values[0], "404");
    V_ASSERT_EQ_S(collector.names[1], "server");
    V_ASSERT_EQ_S(collector.names[2], "content-length");
    V_ASSERT_EQ_S(collector.names[3], "cache-control");
    delete_hpack_table(decoder);

    /* the payload follows the block and it is the frame that closes
    the stream, the text of it describes the error */
    error = decode_frame_http2(
        &written[HTTP2_HEADER_SIZE + http2_frame.length],
        size - HTTP2_HEADER_SIZE - http2_frame.length,
        &http2_frame
    );
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);
    V_ASSERT(http2_frame.flags & HTTP2_FLAG_END_STREAM);
    V_ASSERT_MEM(http2_frame.payload, "404 - Not Found - ", 18);

    _delete_http2_test(context, http2_connection);

    /* an error that carries a realm announces the authentication the
    peer is expected to provide as one more field of the block */
    _create_http2_test(&context, &http2_connection);

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/private");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    error = write_http_error_a(
        context->connection,
        NULL,
        0,
        HTTP20,
        401,
        "Unauthorized",
        NULL,
        "viriatum",
        KEEP_ALIVE,
        NULL,
        NULL
    );
    V_ASSERT_EQ_U(error, 0);

    size = _written_http2_test(context, written, sizeof(written));
    error = decode_frame_http2(written, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    create_hpack_table(&decoder);
    collector.count = 0;
    error = decode_hpack(
        decoder,
        http2_frame.payload,
        http2_frame.length,
        collect_hpack_test,
        (void *) &collector
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 5);
    V_ASSERT_EQ_S(collector.values[0], "401");
    V_ASSERT_EQ_S(collector.names[4], "www-authenticate");
    V_ASSERT_EQ_S(collector.values[4], "Basic realm=\"viriatum\"");
    delete_hpack_table(decoder);

    _delete_http2_test(context, http2_connection);

    /* an installation that asks for the errors to be built from a
    template and carries no such file falls back on the text of it,
    which reaches the peer through the very same frames */
    _create_http2_test(&context, &http2_connection);
    context->options->use_template = 1;
    SPRINTF(
        (char *) context->options->resources_path,
        VIRIATUM_MAX_PATH_SIZE, "%s",
        "/missing"
    );

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/template");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    error = write_http_error(
        context->connection,
        NULL,
        0,
        HTTP20,
        404,
        "Not Found",
        NULL,
        KEEP_ALIVE,
        NULL,
        NULL
    );
    V_ASSERT_EQ_U(error, 0);

    size = _written_http2_test(context, written, sizeof(written));
    error = decode_frame_http2(written, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_HEADERS);

    error = decode_frame_http2(
        &written[HTTP2_HEADER_SIZE + http2_frame.length],
        size - HTTP2_HEADER_SIZE - http2_frame.length,
        &http2_frame
    );
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT(http2_frame.flags & HTTP2_FLAG_END_STREAM);
    V_ASSERT_MEM(http2_frame.payload, "404 - Not Found - ", 18);

    _delete_http2_test(context, http2_connection);

    /* the buffer of the headers and the copy of the payload are both
    gone together with the connection that carried them */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_flow(void) {
    /* allocates space for the chain of the connection, for the
    session and for the stream the payload is held back on */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    unsigned char payload[8];
    size_t queued;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);
    _responder = TRUE;

    /* closes the window of the connection so that the payload of the
    response has nowhere to go and has to be held back */
    http2_connection->send_window = 0;

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/held");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the block of the headers is not accounted against the window,
    so it goes out, the payload of the response does not */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->pending->size, 1);
    V_ASSERT(http2_stream->complete == FALSE);
    queued = context->connection->write_queue->size;

    /* widening the window of the connection lets the payload that has
    been held back through */
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = 4;
    encode_number_http2(payload, 1024);
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->pending->size, 0);
    V_ASSERT(http2_stream->complete == TRUE);
    V_ASSERT(context->connection->write_queue->size > queued);

    _delete_http2_test(context, http2_connection);

    /* a change of the initial window widens the window of a stream
    just the same, so the payload it is holding back also has to go
    out as soon as the settings that carry it are applied */
    _create_http2_test(&context, &http2_connection);
    http2_connection->send_window = 0;

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/settings");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->pending->size, 1);
    queued = context->connection->write_queue->size;

    /* the window of the connection is widened by hand, the settings
    only ever carry the one of the streams */
    http2_connection->send_window = 1024;
    payload[0] = 0x00;
    payload[1] = HTTP2_SETTING_INITIAL_WINDOW_SIZE;
    encode_number_http2(&payload[2], HTTP2_DEFAULT_WINDOW_SIZE + 1024);

    http2_frame.type = HTTP2_SETTINGS;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = HTTP2_SETTING_SIZE;
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->pending->size, 0);
    V_ASSERT(http2_stream->complete == TRUE);
    V_ASSERT(context->connection->write_queue->size > queued);

    _responder = FALSE;
    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_split(void) {
    /* allocates space for the chain of the connection, for the
    session and for the stream the payload is split on */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    unsigned char block[256];
    unsigned char payload[8];
    ERROR_CODE error;

    /* gathers the number of allocations that are outstanding so that
    the splitting may be verified to release every buffer it copies */
    size_t allocated = ALLOCATIONS;

    _create_http2_test(&context, &http2_connection);
    _responder = TRUE;

    /* leaves room for a single byte of payload, so that the response
    of two bytes has to be split over two frames */
    http2_connection->send_window = 1;

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/split-body");
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* only one of the two bytes has gone out, so the fragment is
    still being held back and the message is not complete */
    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->pending->size, 1);
    V_ASSERT(http2_stream->complete == FALSE);
    V_ASSERT_EQ_U(http2_connection->send_window, 0);

    /* widening the window lets the remainder of the payload through,
    which is the part that closes the message */
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = 4;
    encode_number_http2(payload, 16);
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT_EQ_U(http2_stream->pending->size, 0);
    V_ASSERT(http2_stream->complete == TRUE);

    _responder = FALSE;
    _delete_http2_test(context, http2_connection);

    /* the buffers of both of the parts are gone, the one of the
    handler and the copies that the splitting has taken */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_schedule(void) {
    /* allocates space for the chain of the connection, for the
    session and for the streams that hold the payload back */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *first;
    struct http2_stream_t *second;
    struct http2_stream_t *third;
    struct http2_frame_t http2_frame;
    struct http2_priority_t http2_priority;
    struct data_t *data;
    unsigned char block[256];
    unsigned char payload[8];
    size_t queued;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);
    _responder = TRUE;

    /* closes the window of the connection so that every one of the
    streams ends up holding the payload of its response back */
    http2_connection->send_window = 0;

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS | HTTP2_FLAG_END_STREAM;
    http2_frame.payload = block;

    http2_frame.stream_id = 1;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/first");
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_frame.stream_id = 3;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/second");
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_frame.stream_id = 5;
    http2_frame.length = _request_http2_test(block, sizeof(block), "/third");
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    first = find_stream_http2_connection(http2_connection, 1);
    second = find_stream_http2_connection(http2_connection, 3);
    third = find_stream_http2_connection(http2_connection, 5);
    V_ASSERT_EQ_U(first->pending->size, 1);
    V_ASSERT_EQ_U(second->pending->size, 1);
    V_ASSERT_EQ_U(third->pending->size, 1);

    /* places the second stream below the first and leaves the third
    at the root carrying the heavier of the weights */
    http2_priority.dependency = 1;
    http2_priority.weight = HTTP2_DEFAULT_WEIGHT;
    http2_priority.exclusive = FALSE;
    prioritise_stream_http2_connection(http2_connection, second, &http2_priority);

    http2_priority.dependency = 0;
    http2_priority.weight = 64;
    http2_priority.exclusive = FALSE;
    prioritise_stream_http2_connection(http2_connection, third, &http2_priority);

    /* widens the window of the connection by exactly one payload, so
    that only one of the streams is able to write */
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = 4;
    http2_frame.payload = payload;

    queued = context->connection->write_queue->size;
    encode_number_http2(payload, 2);
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* among the streams that are able to write the heavier one goes
    first, so the third is the one that has written */
    first = find_stream_http2_connection(http2_connection, 1);
    second = find_stream_http2_connection(http2_connection, 3);
    third = find_stream_http2_connection(http2_connection, 5);
    V_ASSERT_EQ_U(third->pending->size, 0);
    V_ASSERT_EQ_U(first->pending->size, 1);
    V_ASSERT_EQ_U(second->pending->size, 1);

    get_value_linked_list(context->connection->write_queue, queued, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    error = decode_frame_http2(data->data, data->size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT_EQ_U(http2_frame.stream_id, 5);
    V_ASSERT_EQ_U(http2_frame.length, 2);

    /* the room that follows goes to the first stream, the second one
    hangs from it and so it waits for it to be done */
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = 4;
    http2_frame.payload = payload;

    queued = context->connection->write_queue->size;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    first = find_stream_http2_connection(http2_connection, 1);
    second = find_stream_http2_connection(http2_connection, 3);
    V_ASSERT_EQ_U(first->pending->size, 0);
    V_ASSERT_EQ_U(second->pending->size, 1);

    get_value_linked_list(context->connection->write_queue, queued, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    error = decode_frame_http2(data->data, data->size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);

    /* with nothing above it holding anything back the stream that
    sits below finally writes the payload it was holding */
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 0;
    http2_frame.length = 4;
    http2_frame.payload = payload;

    queued = context->connection->write_queue->size;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    second = find_stream_http2_connection(http2_connection, 3);
    V_ASSERT_EQ_U(second->pending->size, 0);

    get_value_linked_list(context->connection->write_queue, queued, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    error = decode_frame_http2(data->data, data->size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT_EQ_U(http2_frame.stream_id, 3);

    _responder = FALSE;
    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_connection_length(void) {
    /* allocates space for the chain of the connection, for the
    session and for the block that announces the payload */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http2_frame_t http2_frame;
    struct hpack_table_t *encoder;
    struct hpack_header_t hpack_header;
    unsigned char block[256];
    unsigned char payload[8];
    size_t offset;
    ERROR_CODE error;

    _create_http2_test(&context, &http2_connection);

    /* builds a request that announces a payload of four bytes and
    then delivers exactly that many */
    create_hpack_table(&encoder);
    offset = _request_http2_test(block, sizeof(block), "/upload");
    hpack_header.name = (unsigned char *) "content-length";
    hpack_header.name_size = 14;
    hpack_header.value = (unsigned char *) "4";
    hpack_header.value_size = 1;
    encode_hpack(encoder, block, sizeof(block), &offset, &hpack_header, FALSE);
    delete_hpack_table(encoder);

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = offset;
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_stream = find_stream_http2_connection(http2_connection, 1);
    V_ASSERT_NOT_NULL(http2_stream);
    V_ASSERT(http2_stream->announced == TRUE);
    V_ASSERT_EQ_U(http2_stream->content_length, 4);

    /* delivers the announced amount, the message closes without any
    complaint about the size of it */
    http2_frame.type = HTTP2_DATA;
    http2_frame.flags = HTTP2_FLAG_END_STREAM;
    http2_frame.length = 4;
    memcpy(payload, "body", 4);
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(_record.complete, 1);

    _delete_http2_test(context, http2_connection);

    /* a message that delivers less than it announces is refused, the
    size is verified when the stream closes */
    _create_http2_test(&context, &http2_connection);
    create_hpack_table(&encoder);
    offset = _request_http2_test(block, sizeof(block), "/short");
    hpack_header.name = (unsigned char *) "content-length";
    hpack_header.name_size = 14;
    hpack_header.value = (unsigned char *) "10";
    hpack_header.value_size = 2;
    encode_hpack(encoder, block, sizeof(block), &offset, &hpack_header, FALSE);
    delete_hpack_table(encoder);

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = offset;
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    http2_frame.type = HTTP2_DATA;
    http2_frame.flags = HTTP2_FLAG_END_STREAM;
    http2_frame.length = 4;
    http2_frame.payload = payload;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    _delete_http2_test(context, http2_connection);

    /* a size that is not a number at all is refused as soon as the
    field carrying it is decoded */
    _create_http2_test(&context, &http2_connection);
    create_hpack_table(&encoder);
    offset = _request_http2_test(block, sizeof(block), "/bad");
    hpack_header.name = (unsigned char *) "content-length";
    hpack_header.name_size = 14;
    hpack_header.value = (unsigned char *) "abc";
    hpack_header.value_size = 3;
    encode_hpack(encoder, block, sizeof(block), &offset, &hpack_header, FALSE);
    delete_hpack_table(encoder);

    http2_frame.type = HTTP2_HEADERS;
    http2_frame.flags = HTTP2_FLAG_END_HEADERS;
    http2_frame.stream_id = 1;
    http2_frame.length = offset;
    http2_frame.payload = block;
    error = handle_frame_http2_connection(http2_connection, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_COMPRESSION_ERROR);

    _delete_http2_test(context, http2_connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_alpn(void) {
#ifdef VIRIATUM_ALPN
    /* allocates space for the list of the protocols that a peer
    announces and for the selection that comes out of it */
    unsigned char list[32];
    const unsigned char *selected;
    unsigned char selected_size;
    size_t offset;
    int result;

    /* a peer that announces the two protocols in the order that a
    browser uses gets the most recent of them */
    offset = 0;
    list[offset] = sizeof(HTTP2_ALPN) - 1;
    memcpy(&list[offset + 1], HTTP2_ALPN, sizeof(HTTP2_ALPN) - 1);
    offset += sizeof(HTTP2_ALPN);
    list[offset] = sizeof(HTTP11_ALPN) - 1;
    memcpy(&list[offset + 1], HTTP11_ALPN, sizeof(HTTP11_ALPN) - 1);
    offset += sizeof(HTTP11_ALPN);

    result = alpn_handler_service(NULL, &selected, &selected_size, list, (unsigned int) offset, NULL);
    V_ASSERT_EQ_I(result, SSL_TLSEXT_ERR_OK);
    V_ASSERT_EQ_U(selected_size, sizeof(HTTP2_ALPN) - 1);
    V_ASSERT_MEM(selected, HTTP2_ALPN, sizeof(HTTP2_ALPN) - 1);

    /* the order of the peer is the one that is honoured, so a peer
    that puts the older protocol first gets that one */
    offset = 0;
    list[offset] = sizeof(HTTP11_ALPN) - 1;
    memcpy(&list[offset + 1], HTTP11_ALPN, sizeof(HTTP11_ALPN) - 1);
    offset += sizeof(HTTP11_ALPN);
    list[offset] = sizeof(HTTP2_ALPN) - 1;
    memcpy(&list[offset + 1], HTTP2_ALPN, sizeof(HTTP2_ALPN) - 1);
    offset += sizeof(HTTP2_ALPN);

    result = alpn_handler_service(NULL, &selected, &selected_size, list, (unsigned int) offset, NULL);
    V_ASSERT_EQ_I(result, SSL_TLSEXT_ERR_OK);
    V_ASSERT_EQ_U(selected_size, sizeof(HTTP11_ALPN) - 1);
    V_ASSERT_MEM(selected, HTTP11_ALPN, sizeof(HTTP11_ALPN) - 1);

    /* a name that this end does not speak is walked past, the one
    that follows it is the one selected */
    offset = 0;
    list[offset] = 4;
    memcpy(&list[offset + 1], "spdy", 4);
    offset += 5;
    list[offset] = sizeof(HTTP2_ALPN) - 1;
    memcpy(&list[offset + 1], HTTP2_ALPN, sizeof(HTTP2_ALPN) - 1);
    offset += sizeof(HTTP2_ALPN);

    result = alpn_handler_service(NULL, &selected, &selected_size, list, (unsigned int) offset, NULL);
    V_ASSERT_EQ_I(result, SSL_TLSEXT_ERR_OK);
    V_ASSERT_MEM(selected, HTTP2_ALPN, sizeof(HTTP2_ALPN) - 1);

    /* a peer that announces nothing this end speaks negotiates
    nothing at all, the connection falls back on its own */
    offset = 0;
    list[offset] = 4;
    memcpy(&list[offset + 1], "spdy", 4);
    offset += 5;

    result = alpn_handler_service(NULL, &selected, &selected_size, list, (unsigned int) offset, NULL);
    V_ASSERT_EQ_I(result, SSL_TLSEXT_ERR_NOACK);

    /* an empty list carries no name at all to be selected */
    result = alpn_handler_service(NULL, &selected, &selected_size, list, 0, NULL);
    V_ASSERT_EQ_I(result, SSL_TLSEXT_ERR_NOACK);

    /* a length that goes past the end of the list is malformed, the
    walk stops instead of reading past the buffer */
    list[0] = 32;
    list[1] = 'h';
    result = alpn_handler_service(NULL, &selected, &selected_size, list, 2, NULL);
    V_ASSERT_EQ_I(result, SSL_TLSEXT_ERR_NOACK);
#endif

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

#endif
