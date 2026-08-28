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

#include "../http/http.h"
#include "../system/system.h"
#include "stream_http.h"

/**
 * The number of slots of the table that holds the streams of a
 * connection, it is the number of streams that a peer is allowed
 * to keep open at the same time.
 * The table is kept compact, the slots below the count are the
 * ones in use, so that a lookup only ever walks the streams that
 * are actually open rather than the complete table.
 */
#define HTTP2_STREAM_SLOTS HTTP2_MAX_CONCURRENT

/**
 * The largest header block that this end assembles out of a
 * sequence of continuation frames, a peer is otherwise able to
 * make a single block grow without any bound at all.
 */
#define HTTP2_MAX_BLOCK 65536

/**
 * The size of the buffer used to build the frames that this end
 * writes, it holds the largest header block that a response is
 * expected to produce plus the header of the frame.
 */
#define HTTP2_WRITE_SIZE (VIRIATUM_HTTP_SIZE + HTTP2_HEADER_SIZE)

/**
 * Enumeration defining the states that a stream goes through,
 * as described by the section 5.1 of RFC 9113.
 */
typedef enum http2_stream_state_e {
    HTTP2_STATE_IDLE = 1,
    HTTP2_STATE_RESERVED_LOCAL,
    HTTP2_STATE_RESERVED_REMOTE,
    HTTP2_STATE_OPEN,
    HTTP2_STATE_HALF_CLOSED_LOCAL,
    HTTP2_STATE_HALF_CLOSED_REMOTE,
    HTTP2_STATE_CLOSED
} http2_stream_state;

/**
 * Structure describing a single stream of a connection, it owns
 * both the message that the handlers observe and the settings
 * that carry the callbacks of the handler serving it, as several
 * streams are live at the same time and each one of them may be
 * served by a different handler.
 */
typedef struct http2_stream_t {
    /**
     * The identifier of the stream, the value zero marks a slot
     * of the table that is not in use.
     */
    unsigned int stream_id;

    /**
     * The state of the stream according to the state machine of
     * the specification.
     */
    enum http2_stream_state_e state;

    /**
     * The amount of payload that this end is still allowed to
     * send on the stream before the peer widens the window.
     */
    long send_window;

    /**
     * The amount of payload that this end is still willing to
     * receive on the stream before it widens the window.
     */
    long receive_window;

    /**
     * The priority of the stream inside the tree, both the stream
     * it depends on and the weight it carries in it.
     */
    struct http2_priority_t priority;

    /**
     * The message that the handlers observe for this stream, it
     * is the protocol agnostic structure of the pipeline.
     */
    struct http_request_t *request;

    /**
     * The settings carrying the callbacks of the handler that is
     * serving this stream, they are swapped into the connection
     * while the stream is being driven.
     */
    struct http_settings_t *http_settings;

    /**
     * The handler that is serving this stream, it is set when the
     * headers arrive and unset when the response completes.
     */
    struct http_handler_t *http_handler;

    /**
     * Flag controlling if the peer has already closed its side of
     * the stream, meaning that no more payload is coming.
     */
    char end_stream;

    /**
     * Flag controlling if the header block of the stream has been
     * completely received and handed to the handler.
     */
    char headers_complete;
} http2_stream;

/**
 * Structure describing the session of a connection that speaks
 * HTTP/2, it hangs from the HTTP connection so that the handlers
 * keep reaching the very same structures they always have.
 */
typedef struct http2_connection_t {
    /**
     * The HTTP connection that owns this session, it is the one
     * the handlers interact with.
     */
    struct http_connection_t *http_connection;

    /**
     * The settings that this end has announced to the peer, they
     * bound what the peer is allowed to send.
     */
    struct http2_settings_t settings;

    /**
     * The settings that the peer has announced, they bound what
     * this end is allowed to send.
     */
    struct http2_settings_t remote;

    /**
     * The dynamic table used to decode the header blocks that the
     * peer sends, it follows the encoder of the peer.
     */
    struct hpack_table_t *decoder;

    /**
     * The dynamic table used to encode the header blocks that this
     * end sends, it follows the decoder of the peer.
     */
    struct hpack_table_t *encoder;

    /**
     * The settings that the connection carried before the session
     * took over, restored once the session is released.
     */
    struct http_settings_t *base_settings;

    /**
     * The table holding the streams of the connection, the slots
     * below the count are the ones in use and the table is kept
     * compact by moving the last one into a slot that is freed.
     * A reference to a stream is therefore only valid until the
     * next stream of the connection is closed.
     */
    struct http2_stream_t streams[HTTP2_STREAM_SLOTS];

    /**
     * The number of streams that the peer currently has open, it
     * is bounded by the setting that has been announced.
     */
    size_t count;

    /**
     * The identifier of the last stream that the peer has opened,
     * a new one is required to be strictly above it.
     */
    unsigned int last_stream_id;

    /**
     * The identifier of the last stream that a push has reserved,
     * the server uses the even numbered identifiers.
     */
    unsigned int push_stream_id;

    /**
     * The amount of payload that this end is still allowed to send
     * on the connection as a whole.
     */
    long send_window;

    /**
     * The amount of payload that this end is still willing to
     * receive on the connection as a whole.
     */
    long receive_window;

    /**
     * Flag controlling if the preface of the connection has been
     * consumed, no frame is accepted before it.
     */
    char preface;

    /**
     * Flag controlling if this end has already told the peer that
     * the connection is going away.
     */
    char goaway;

    /**
     * The stream whose header block is being assembled out of a
     * sequence of continuation frames, the value zero means that
     * no such sequence is open.
     */
    unsigned int continuation;

    /**
     * The flags of the headers frame that opened the sequence of
     * continuation frames currently being assembled.
     */
    unsigned char continuation_flags;

    /**
     * The buffer that assembles a header block spread over a
     * sequence of continuation frames.
     */
    unsigned char *block;

    /**
     * The size of the header block assembled so far.
     */
    size_t block_size;
} http2_connection;

/**
 * Constructor of the HTTP/2 session, it takes over an HTTP
 * connection that has been recognised as speaking the protocol.
 *
 * @param http2_connection_pointer The pointer to the session to
 * be constructed.
 * @param http_connection The HTTP connection to be taken over.
 * @return The resulting error code.
 */
ERROR_CODE create_http2_connection(struct http2_connection_t **http2_connection_pointer, struct http_connection_t *http_connection);

/**
 * Destructor of the HTTP/2 session, it releases every one of the
 * streams that are still open and restores the connection to the
 * state it had before the session took over.
 *
 * @param http2_connection The session to be destroyed.
 * @return The resulting error code.
 */
ERROR_CODE delete_http2_connection(struct http2_connection_t *http2_connection);

/**
 * Retrieves the stream carrying the provided identifier, the
 * table is walked by probing from the slot the identifier hashes
 * into, so an absent stream costs a short walk.
 *
 * @param http2_connection The session holding the streams.
 * @param stream_id The identifier of the stream to be found.
 * @return The stream found or an unset value when the connection
 * holds no stream with that identifier.
 */
struct http2_stream_t *find_stream_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id);

/**
 * Opens a new stream in the provided session, the identifier is
 * required to be one that the peer is allowed to open and the
 * number of open streams is required to be inside the announced
 * bound.
 *
 * @param http2_connection The session to open the stream in.
 * @param stream_id The identifier of the stream to be opened.
 * @param http2_stream_pointer The pointer to the stream that has
 * been opened.
 * @return The resulting error code.
 */
ERROR_CODE open_stream_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id, struct http2_stream_t **http2_stream_pointer);

/**
 * Closes the provided stream, releasing the message and the
 * settings that it owns and unsetting the handler that has been
 * serving it.
 *
 * @param http2_connection The session holding the stream.
 * @param http2_stream The stream to be closed.
 * @return The resulting error code.
 */
ERROR_CODE close_stream_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream);

/**
 * Applies a window increment over both the connection and the
 * streams, an increment that takes a window beyond the largest
 * value the protocol represents is refused.
 *
 * @param http2_connection The session to be updated.
 * @param stream_id The stream the increment applies to, the value
 * zero refers to the connection itself.
 * @param increment The amount the window grows by.
 * @return The resulting error code.
 */
ERROR_CODE update_window_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id, unsigned int increment);

/**
 * Applies the settings that the peer has announced, the change of
 * the initial window is carried over to the streams that are
 * already open, which is what the specification requires.
 *
 * @param http2_connection The session to be updated.
 * @param data The payload of the settings frame.
 * @param data_size The size in bytes of the payload.
 * @return The resulting error code.
 */
ERROR_CODE apply_settings_http2_connection(struct http2_connection_t *http2_connection, const unsigned char *data, size_t data_size);

/**
 * Handles a single frame of the connection, this is the operation
 * that the reading of the connection drives once a complete frame
 * has been gathered.
 *
 * @param http2_connection The session handling the frame.
 * @param http2_frame The frame to be handled.
 * @return The resulting error code.
 */
ERROR_CODE handle_frame_http2_connection(struct http2_connection_t *http2_connection, struct http2_frame_t *http2_frame);

/**
 * Writes a frame that carries no payload beyond a single value,
 * used for the reset of a stream and the update of a window.
 *
 * @param http2_connection The session writing the frame.
 * @param type The type of the frame to be written.
 * @param stream_id The stream the frame belongs to.
 * @param value The value carried by the payload.
 * @return The resulting error code.
 */
ERROR_CODE write_value_http2_connection(struct http2_connection_t *http2_connection, unsigned char type, unsigned int stream_id, unsigned int value);

/**
 * Tells the peer that the connection is going away, carrying both
 * the last stream that has been handled and the reason.
 *
 * @param http2_connection The session closing the connection.
 * @param error The reason the connection is being closed.
 * @return The resulting error code.
 */
ERROR_CODE write_goaway_http2_connection(struct http2_connection_t *http2_connection, unsigned int error);

/**
 * Processes the data that has arrived on a connection that speaks
 * HTTP/2, this is the callback that the io layer drives.
 *
 * @param io_connection The io connection the data arrived on.
 * @param buffer The buffer holding the data that arrived.
 * @param buffer_size The size in bytes of the data.
 * @return The resulting error code.
 */
ERROR_CODE data_handler_stream_http2(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size);

/**
 * Takes over a connection that has been recognised as speaking
 * HTTP/2, replacing the callback of the io layer and announcing
 * the settings of this end.
 *
 * @param io_connection The io connection to be taken over.
 * @return The resulting error code.
 */
ERROR_CODE upgrade_handler_stream_http2(struct io_connection_t *io_connection);
