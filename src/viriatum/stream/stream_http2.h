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

/* forward references (avoids loop) */
struct connection_t;
struct data_t;
struct io_connection_t;
struct http_connection_t;

/**
 * Callback of the completion of the write of a fragment of a
 * response, it is declared here as the inclusion of the headers
 * of the stream layer loops back into this one.
 */
typedef ERROR_CODE (*connection_data_callback_h2)(struct connection_t *, struct data_t *, void *);

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

    /**
     * Flag controlling if the response of the stream has already
     * been written in full, at which point the stream is closed as
     * soon as the handler is done with it.
     */
    char complete;

    /**
     * The size of the payload that the message has announced, only
     * meaningful when it has actually announced one.
     */
    size_t content_length;

    /**
     * Flag controlling if the message has announced the size of its
     * payload, which is then verified against the one received.
     */
    char announced;

    /**
     * The size of the payload that has been received so far, it is
     * compared against the announced one when the stream closes.
     */
    size_t received;

    /**
     * The fragments of the payload of the response that the window
     * of the stream or of the connection has not allowed to be sent
     * yet, they are flushed as the peer widens either of them.
     */
    struct linked_list_t *pending;
} http2_stream;

/**
 * Structure describing a fragment of the payload of a response
 * that is waiting for the window to widen, it owns the buffer of
 * the fragment and the callback that the handler is waiting on.
 */
typedef struct http2_pending_t {
    /**
     * The buffer of the fragment, it is owned by this structure
     * until it reaches the connection.
     */
    unsigned char *data;

    /**
     * The size in bytes of the fragment that is still pending.
     */
    size_t size;

    /**
     * The position of the fragment that has already been sent, the
     * window may have allowed only a part of it through.
     */
    size_t offset;

    /**
     * Flag controlling if the fragment closes the message.
     */
    char last;

    /**
     * The callback of the handler, driven once the complete
     * fragment has reached the connection.
     */
    connection_data_callback_h2 callback;

    /**
     * The value handed to the callback of the handler.
     */
    void *parameters;
} http2_pending;

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
     * The structures that carry a stream through the completion of
     * a write and that have not been driven yet, they are released
     * together with the session so that a connection that is torn
     * down with writes still queued leaves nothing behind.
     */
    struct linked_list_t *callbacks;

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
 * Places the provided stream at the position of the tree that the
 * given priority describes, moving the parent out of the way when
 * the dependency would otherwise close a cycle and taking over the
 * siblings when the dependency is an exclusive one.
 *
 * @param http2_connection The session holding the stream.
 * @param http2_stream The stream to be placed.
 * @param http2_priority The priority describing the position.
 * @return The resulting error code.
 */
ERROR_CODE prioritise_stream_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream, struct http2_priority_t *http2_priority);

/**
 * Promises a resource to the peer on the provided stream, which
 * reserves a stream of its own for the response and hands the
 * request of it to the handler as though the peer had asked.
 * A peer that has turned the pushing off gets nothing at all.
 *
 * @param http2_connection The session making the promise.
 * @param http2_stream The stream the promise is made on.
 * @param path The path of the resource being promised.
 * @return The resulting error code.
 */
ERROR_CODE push_stream_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream, const char *path);

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
 * Writes the buffer that a response has been built in through the
 * connection under HTTP/2, restoring the stream it belongs to once
 * the write of it completes.
 *
 * @param connection The connection the response belongs to.
 * @param data The buffer of the response.
 * @param size The size in bytes of the buffer.
 * @param callback The callback of the completion of the write.
 * @param parameters The value handed to the callback.
 * @return The resulting error code.
 */
ERROR_CODE write_flush_http2(struct connection_t *connection, unsigned char *data, size_t size, connection_data_callback_h2 callback, void *parameters);

/**
 * Opens a response in the provided buffer under HTTP/2, the block
 * of the headers is opened after the space that the header of the
 * frame is going to take.
 *
 * @param connection The connection the response belongs to.
 * @param buffer The buffer the response is built in.
 * @param size The size in bytes of the provided buffer.
 * @param version The version of the protocol in use.
 * @param status_code The status code of the response.
 * @param status_message The message that describes the status.
 * @param keep_alive The mode the connection is left in.
 * @return The number of bytes the buffer holds.
 */
size_t write_status_http2(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive);

/**
 * Appends a single header field to a response being built under
 * HTTP/2, the name is lowered as the protocol requires and the
 * fields that carry no meaning in it are dropped.
 *
 * @param connection The connection the response belongs to.
 * @param buffer The buffer the response is built in.
 * @param size The size in bytes of the provided buffer.
 * @param offset The position the field is written at.
 * @param name The name of the field.
 * @param value The value of the field.
 * @return The number of bytes the buffer holds.
 */
size_t write_field_http2(struct connection_t *connection, char *buffer, size_t size, size_t offset, const char *name, const char *value);

/**
 * Closes the header section of a response being built under
 * HTTP/2, writing the header of the frame into the space that has
 * been reserved for it.
 *
 * @param connection The connection the response belongs to.
 * @param buffer The buffer the response is built in.
 * @param size The size in bytes of the provided buffer.
 * @param offset The position the section is closed at.
 * @param last The value one when no payload follows.
 * @return The number of bytes the buffer holds.
 */
size_t write_end_http2(struct connection_t *connection, char *buffer, size_t size, size_t offset, char last);

/**
 * Writes a fragment of the payload of a response under HTTP/2,
 * framing it and holding back whatever the windows do not allow
 * through yet.
 *
 * @param connection The connection the response belongs to.
 * @param data The fragment to be written.
 * @param size The size in bytes of the fragment.
 * @param last The value one when the fragment is the last one.
 * @param callback The callback of the completion of the write.
 * @param parameters The value handed to the callback.
 * @return The resulting error code.
 */
ERROR_CODE write_chunk_http2(struct connection_t *connection, unsigned char *data, size_t size, char last, connection_data_callback_h2 callback, void *parameters);

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
