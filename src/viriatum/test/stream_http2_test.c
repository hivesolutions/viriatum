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
    _record.complete++;
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

    /* installs the handler that records the pipeline as the one a
    new message is served by */
    _handler.name = (unsigned char *) "record";
    _handler.resolve_index = FALSE;
    _handler.set = _set_http2_test;
    _handler.unset = _unset_http2_test;
    _handler.reset = NULL;
    context->http_connection->base_handler = &_handler;

    /* resets the record so that a test never observes what the one
    that ran before it has produced */
    memset(&_record, 0, sizeof(_record));

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

    /* an increment for a stream that is no longer open is discarded,
    the peer is allowed to send one that crosses the closing */
    error = update_window_http2_connection(http2_connection, 99, 1024);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

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

    /* a frame that violates the protocol takes the connection down
    and the peer is told the reason before it happens */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    encode_frame_http2(stream, sizeof(stream), 0, HTTP2_HEADERS, HTTP2_FLAG_END_HEADERS, 0);
    data_handler_stream_http2(context->io_connection, stream, HTTP2_HEADER_SIZE);
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);
    V_ASSERT(http2_connection->goaway == TRUE);

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
