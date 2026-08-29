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

#include "stream_http2.h"

/**
 * The bits that mark each one of the pseudo headers of a request
 * as already seen, the specification refuses a block that carries
 * any of them more than once.
 */
#define HTTP2_PSEUDO_METHOD 0x01
#define HTTP2_PSEUDO_SCHEME 0x02
#define HTTP2_PSEUDO_PATH 0x04
#define HTTP2_PSEUDO_AUTHORITY 0x08

/**
 * The set of pseudo headers that a request is required to carry,
 * the authority is the only one of them that is optional.
 */
#define HTTP2_PSEUDO_REQUIRED (HTTP2_PSEUDO_METHOD | HTTP2_PSEUDO_SCHEME | HTTP2_PSEUDO_PATH)

/**
 * Structure handed to the decoding of a header block so that
 * each one of the fields it produces reaches both the message of
 * the stream and the handler that is serving it.
 */
typedef struct http2_block_t {
    /**
     * The session the block belongs to.
     */
    struct http2_connection_t *http2_connection;

    /**
     * The stream the block belongs to.
     */
    struct http2_stream_t *http2_stream;

    /**
     * Flag controlling if a regular field has already been seen,
     * a pseudo header is not allowed to follow one.
     */
    char regular;

    /**
     * Flag controlling if the pseudo headers have already been
     * handed to the handler, which only happens once the complete
     * set of them has been gathered.
     */
    char flushed;

    /**
     * The set of pseudo headers that the block has carried so far,
     * used to refuse both a duplicated one and a missing one.
     */
    unsigned char pseudo;

    /**
     * Flag controlling if the block is a trailer section, which is
     * not allowed to carry a pseudo header at all.
     */
    char trailers;
} http2_block;

/**
 * Writes as much of the payload that the provided stream is
 * holding back as the windows allow through, declared ahead of
 * its definition as the handling of a frame drives it.
 *
 * @param http2_connection The session the stream belongs to.
 * @param http2_stream The stream to be flushed.
 * @return The resulting error code.
 */
static ERROR_CODE _flush_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream);

/**
 * Writes the payload that the streams of the connection are holding
 * back in the order that the tree of the priorities describes,
 * declared ahead of its definition as the handling of a frame drives
 * it just as the queueing of a fragment does.
 *
 * @param http2_connection The session to be flushed.
 * @return The resulting error code.
 */
static ERROR_CODE _schedule_http2_connection(struct http2_connection_t *http2_connection);

/**
 * Retrieves the method of a request out of the value of the
 * method pseudo header, the comparison walks the very same table
 * the HTTP/1.1 parser reports its own values from.
 *
 * @param data The buffer holding the name of the method.
 * @param data_size The size in bytes of the name.
 * @return The value of the method enumeration or zero in case the
 * name is not one of the known methods.
 */
static unsigned char _method_http2_connection(const unsigned char *data, size_t data_size) {
    /* allocates space for the iteration over the known methods */
    size_t index;
    const char *method;

    /* walks the table of the methods, the size is compared before
    the contents as it discards almost every candidate */
    for(index = 0; index < 24; index++) {
        method = http_method_strings[index];
        if(strlen(method) != data_size) { continue; }
        if(memcmp(method, data, data_size) != 0) { continue; }
        return (unsigned char) (index + 1);
    }

    /* returns the value that marks an unknown method, the caller is
    the one deciding what to do about it */
    return 0;
}

/**
 * Swaps the message, the settings and the handler of the provided
 * stream into the HTTP connection, so that a handler observes the
 * stream it is being driven for.
 *
 * @param http2_connection The session holding the stream.
 * @param http2_stream The stream to be made the current one.
 */
static void _activate_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream) {
    /* retrieves the HTTP connection, it is the structure that the
    handlers reach through the message they are handed */
    struct http_connection_t *http_connection = http2_connection->http_connection;

    /* points the connection at the structures of the stream, under
    HTTP/2 they change from one stream to the next */
    http_connection->request = http2_stream->request;
    http_connection->http_settings = http2_stream->http_settings;
    http_connection->http_handler = http2_stream->http_handler;
}

struct http2_stream_t *find_stream_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id) {
    /* allocates space for the iteration over the streams that the
    connection currently holds open */
    size_t index;

    /* walks only the slots that are in use, the table is kept
    compact so the ones above the count are never live */
    for(index = 0; index < http2_connection->count; index++) {
        if(http2_connection->streams[index].stream_id == stream_id) {
            return &http2_connection->streams[index];
        }
    }

    /* returns an unset value as the connection holds no stream
    carrying the provided identifier */
    return NULL;
}

ERROR_CODE open_stream_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id, struct http2_stream_t **http2_stream_pointer) {
    /* allocates space for the stream that is going to be taken out
    of the table of the connection */
    struct http2_stream_t *http2_stream;

    /* a stream opened by a client carries an odd identifier, an even
    one is reserved for the pushes of the server */
    if(stream_id % 2 == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

    /* the identifiers of the streams a peer opens are required to be
    strictly increasing, a lower one refers to a stream that has
    already been closed and is never reopened */
    if(stream_id <= http2_connection->last_stream_id) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

    /* refuses the stream when the peer already holds as many of them
    open as this end has announced, the excess is refused rather than
    served so that the memory of a connection stays bounded */
    if(http2_connection->count >= http2_connection->settings.max_concurrent_streams ||
       http2_connection->count >= HTTP2_STREAM_SLOTS) {
        RAISE_ERROR_S(HTTP2_REFUSED_STREAM);
    }

    /* takes the slot that follows the ones in use and sets the
    values the stream starts its life with */
    http2_stream = &http2_connection->streams[http2_connection->count];
    http2_stream->stream_id = stream_id;
    http2_stream->state = HTTP2_STATE_OPEN;
    http2_stream->send_window = (long) http2_connection->remote.initial_window_size;
    http2_stream->receive_window = (long) http2_connection->settings.initial_window_size;
    http2_stream->priority.dependency = 0;
    http2_stream->priority.weight = HTTP2_DEFAULT_WEIGHT;
    http2_stream->priority.exclusive = FALSE;
    http2_stream->http_handler = NULL;
    http2_stream->end_stream = FALSE;
    http2_stream->headers_complete = FALSE;
    http2_stream->complete = FALSE;
    http2_stream->content_length = 0;
    http2_stream->announced = FALSE;
    http2_stream->received = 0;

    /* creates both the message that the handlers observe and the
    settings that carry the callbacks of the handler, they are owned
    by the stream as several of them are live at the same time */
    create_http_request(&http2_stream->request);
    create_http_settings(&http2_stream->http_settings);
    create_linked_list(&http2_stream->pending);

    /* populates the part of the message that comes from the
    connection rather than from the header block */
    http2_stream->request->version = HTTP20;
    http2_stream->request->stream_id = stream_id;
    http2_stream->request->parameters = http2_connection->http_connection->io_connection->connection;
    http2_stream->request->flags = FLAG_KEEP_ALIVE;

    /* accounts the stream in both the number of open ones and the
    last identifier that the peer has used */
    http2_connection->count++;
    http2_connection->last_stream_id = stream_id;

    /* sets the stream in the stream pointer */
    *http2_stream_pointer = http2_stream;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_stream_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream) {
    /* allocates space for the slot of the stream being closed, for
    the one that holds the last of the open streams and for the
    fragments that are still being held back */
    struct http2_pending_t *http2_pending;
    size_t index;
    size_t slot = (size_t) (http2_stream - http2_connection->streams);
    size_t last = http2_connection->count - 1;

    /* unsets the handler that has been serving the stream, this is
    what releases the context that the handler owns */
    if(http2_stream->http_handler != NULL) {
        _activate_http2_connection(http2_connection, http2_stream);
        http2_stream->http_handler->unset(http2_connection->http_connection);
        http2_stream->http_handler = NULL;
    }

    /* releases the fragments of the payload that the stream is still
    holding back, they never reach the connection and so nothing
    else is going to release them */
    while(TRUE) {
        pop_value_linked_list(http2_stream->pending, (void **) &http2_pending, TRUE);
        if(http2_pending == NULL) { break; }
        FREE(http2_pending->data);
        FREE(http2_pending);
    }
    delete_linked_list(http2_stream->pending);

    /* the connection may still be pointing at the structures of this
    stream as the current ones, they are about to be released and so
    the references to them have to go first */
    if(http2_connection->http_connection->request == http2_stream->request) {
        http2_connection->http_connection->request = NULL;
        http2_connection->http_connection->http_settings = http2_connection->base_settings;
        http2_connection->http_connection->http_handler = NULL;
    }

    /* releases both the message and the settings, they are owned by
    the stream and nothing else references them */
    delete_http_request(http2_stream->request);
    delete_http_settings(http2_stream->http_settings);

    /* the streams that hung from this one come to hang from the one
    it hung from, so that the tree keeps its shape once the stream is
    gone, which is what the specification requires */
    for(index = 0; index < http2_connection->count; index++) {
        if(http2_connection->streams[index].priority.dependency != http2_stream->stream_id) { continue; }
        http2_connection->streams[index].priority.dependency = http2_stream->priority.dependency;
    }

    /* moves the last of the open streams into the slot that has just
    been freed, keeping the table compact */
    if(slot != last) { http2_connection->streams[slot] = http2_connection->streams[last]; }
    http2_connection->streams[last].stream_id = 0;
    http2_connection->count--;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_http2_connection(struct http2_connection_t **http2_connection_pointer, struct http_connection_t *http_connection) {
    /* retrieves the session size */
    size_t http2_connection_size = sizeof(struct http2_connection_t);

    /* allocates space for the session */
    struct http2_connection_t *http2_connection =
        (struct http2_connection_t *) MALLOC(http2_connection_size);

    /* sets the session attributes, the settings of both ends start
    at the values that the specification defines */
    http2_connection->http_connection = http_connection;
    create_settings_http2(&http2_connection->settings);
    create_settings_http2(&http2_connection->remote);
    http2_connection->count = 0;
    http2_connection->last_stream_id = 0;
    http2_connection->push_stream_id = 0;
    http2_connection->send_window = HTTP2_DEFAULT_WINDOW_SIZE;
    http2_connection->receive_window = HTTP2_DEFAULT_WINDOW_SIZE;
    http2_connection->preface = FALSE;
    http2_connection->goaway = FALSE;
    http2_connection->continuation = 0;
    http2_connection->continuation_flags = 0;
    http2_connection->block = NULL;
    http2_connection->block_size = 0;

    /* creates the list that holds the structures carrying a stream
    through the completion of a write */
    create_linked_list(&http2_connection->callbacks);

    /* creates the dynamic tables of both directions, each one of
    them follows the opposite one on the peer */
    create_hpack_table(&http2_connection->decoder);
    create_hpack_table(&http2_connection->encoder);

    /* keeps the settings that the connection carried before the
    session took over, they are restored on release */
    http2_connection->base_settings = http_connection->http_settings;

    /* sets the session in the HTTP connection so that the reading of
    the connection is able to reach it */
    http_connection->http2_connection = http2_connection;

    /* points the writing of a response at the operations that frame
    it, a handler builds a response the very same way either way */
    http_connection->write_status = write_status_http2;
    http_connection->write_field = write_field_http2;
    http_connection->write_end = write_end_http2;
    http_connection->write_chunk = write_chunk_http2;
    http_connection->write_flush = write_flush_http2;

    /* sets the session in the session pointer */
    *http2_connection_pointer = http2_connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http2_connection(struct http2_connection_t *http2_connection) {
    /* allocates space for the structures that were carrying a stream
    through the completion of a write */
    struct http2_callback_t *http2_callback;

    /* closes every one of the streams that are still open, the
    closing of one of them moves another into its slot and so the
    first of them is closed over and over */
    while(http2_connection->count > 0) {
        close_stream_http2_connection(http2_connection, &http2_connection->streams[0]);
    }

    /* restores the settings that the connection carried before the
    session took over, they are the ones it owns, and unsets the
    session itself so that the release of the connection does not
    reach for it a second time */
    http2_connection->http_connection->http_settings = http2_connection->base_settings;
    http2_connection->http_connection->request = NULL;
    http2_connection->http_connection->http_handler = NULL;
    http2_connection->http_connection->http2_connection = NULL;

    /* restores the writing operations of HTTP/1.1, the connection is
    no longer being driven by a session */
    http2_connection->http_connection->write_status = write_status_http;
    http2_connection->http_connection->write_field = write_field_http;
    http2_connection->http_connection->write_end = write_end_http;
    http2_connection->http_connection->write_chunk = write_chunk_http;
    http2_connection->http_connection->write_flush = write_flush_http;

    /* releases the structures that were carrying a stream through
    the completion of a write that is never going to happen, the
    connection releases the buffers but not these */
    while(TRUE) {
        pop_value_linked_list(http2_connection->callbacks, (void **) &http2_callback, TRUE);
        if(http2_callback == NULL) { break; }
        FREE(http2_callback);
    }
    delete_linked_list(http2_connection->callbacks);

    /* releases the buffer that assembles a header block spread over
    a sequence of continuation frames */
    if(http2_connection->block != NULL) { FREE(http2_connection->block); }

    /* deletes the dynamic tables of both of the directions */
    delete_hpack_table(http2_connection->decoder);
    delete_hpack_table(http2_connection->encoder);

    /* releases the session */
    FREE(http2_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Verifies whether the stream carrying the first identifier sits
 * below the one carrying the second, walking the dependencies up
 * from it towards the root of the tree.
 *
 * @param http2_connection The session holding the streams.
 * @param stream_id The stream the walk starts at.
 * @param ancestor_id The stream being looked for above it.
 * @return The value one when the second is above the first.
 */
static char _descends_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id, unsigned int ancestor_id) {
    /* allocates space for the stream being visited along the walk and
    for the bound that keeps a broken tree from looping forever */
    struct http2_stream_t *http2_stream;
    size_t index;

    for(index = 0; index < http2_connection->count; index++) {
        if(stream_id == 0) { return FALSE; }
        if(stream_id == ancestor_id) { return TRUE; }

        /* a stream that is no longer open is not part of the tree, so
        the walk ends at the root as far as this end is concerned */
        http2_stream = find_stream_http2_connection(http2_connection, stream_id);
        if(http2_stream == NULL) { return FALSE; }
        stream_id = http2_stream->priority.dependency;
    }

    return FALSE;
}

ERROR_CODE prioritise_stream_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream, struct http2_priority_t *http2_priority) {
    /* allocates space for the iteration over the streams that are
    going to be moved along with this one */
    struct http2_stream_t *parent;
    size_t index;

    /* a stream is never allowed to depend on itself, it would make the
    tree carry a cycle that no walk of it would ever leave */
    if(http2_priority->dependency == http2_stream->stream_id) {
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }

    /* in case the new parent sits below this stream the tree would
    close a cycle, so the parent takes the place this stream held
    before the move, which is what the specification requires */
    if(_descends_http2_connection(http2_connection, http2_priority->dependency, http2_stream->stream_id)) {
        parent = find_stream_http2_connection(http2_connection, http2_priority->dependency);
        if(parent != NULL) { parent->priority.dependency = http2_stream->priority.dependency; }
    }

    /* an exclusive dependency takes over the siblings, every stream
    that hung from the new parent comes to hang from this one */
    if(http2_priority->exclusive == TRUE) {
        for(index = 0; index < http2_connection->count; index++) {
            if(http2_connection->streams[index].stream_id == http2_stream->stream_id) { continue; }
            if(http2_connection->streams[index].priority.dependency != http2_priority->dependency) { continue; }
            http2_connection->streams[index].priority.dependency = http2_stream->stream_id;
        }
    }

    /* places the stream at the position that has been described */
    http2_stream->priority = *http2_priority;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE update_window_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id, unsigned int increment) {
    /* allocates space for the stream the increment applies to, it is
    unset when the increment applies to the connection */
    struct http2_stream_t *http2_stream;

    /* an increment of nothing at all carries no meaning and is an
    error, either of the connection or of the stream */
    if(increment == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

    /* an increment that applies to the connection as a whole widens
    the window that bounds every one of the streams together */
    if(stream_id == 0) {
        /* the room that is left is what the increment is compared
        against, the sum of the two would go beyond the type that
        carries a window on the platforms where it is of four bytes */
        if(http2_connection->send_window > (long) HTTP2_MAX_WINDOW_SIZE - (long) increment) {
            RAISE_ERROR_S(HTTP2_FLOW_CONTROL_ERROR);
        }
        http2_connection->send_window += (long) increment;
        RAISE_NO_ERROR;
    }

    /* an increment for a stream that has never been opened refers to
    an idle one, which carries no window at all to be widened */
    http2_stream = find_stream_http2_connection(http2_connection, stream_id);
    if(http2_stream == NULL) {
        if(stream_id > http2_connection->last_stream_id) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

        /* an increment for a stream that has already been closed is
        discarded, the peer is allowed to send one that crosses it */
        RAISE_NO_ERROR;
    }

    if(http2_stream->send_window > (long) HTTP2_MAX_WINDOW_SIZE - (long) increment) {
        RAISE_ERROR_S(HTTP2_FLOW_CONTROL_ERROR);
    }
    http2_stream->send_window += (long) increment;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE apply_settings_http2_connection(struct http2_connection_t *http2_connection, const unsigned char *data, size_t data_size) {
    /* allocates space for the iteration over the open streams and
    for the difference of the initial window */
    size_t index;
    long difference;
    size_t previous = http2_connection->remote.initial_window_size;
    ERROR_CODE return_value;

    /* applies the payload over the settings of the peer, an entry
    that is not recognised is ignored rather than refused */
    return_value = decode_settings_http2(data, data_size, &http2_connection->remote);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* a change of the initial window is carried over to the streams
    that are already open, which is the part of the negotiation that
    is most commonly left out of an implementation */
    difference = (long) http2_connection->remote.initial_window_size - (long) previous;
    if(difference != 0) {
        for(index = 0; index < http2_connection->count; index++) {
            /* a window that would go beyond the largest value the
            protocol represents is a flow control error, one that
            goes below zero is valid and stalls the stream */
            if(difference > 0 &&
               http2_connection->streams[index].send_window >
                   (long) HTTP2_MAX_WINDOW_SIZE - difference) {
                RAISE_ERROR_S(HTTP2_FLOW_CONTROL_ERROR);
            }
            http2_connection->streams[index].send_window += difference;
        }
    }

    /* resizes the table of the encoder so that it never indexes
    beyond what the decoder of the peer is able to hold */
    if(http2_connection->remote.header_table_size < HPACK_TABLE_SIZE) {
        return_value = resize_hpack_table(
            http2_connection->encoder,
            http2_connection->remote.header_table_size
        );
        if(IS_ERROR_CODE(return_value)) { RAISE_ERROR_S(HTTP2_COMPRESSION_ERROR); }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Writes the provided frame into the connection, the buffer is
 * handed over to the io layer which is the one releasing it once
 * the write completes.
 *
 * @param http2_connection The session writing the frame.
 * @param buffer The buffer holding the complete frame.
 * @param size The size in bytes of the frame.
 * @return The resulting error code.
 */
static ERROR_CODE _write_http2_connection(struct http2_connection_t *http2_connection, unsigned char *buffer, size_t size) {
    /* retrieves the connection out of the session, it is the one
    that owns the write queue */
    struct connection_t *connection =
        http2_connection->http_connection->io_connection->connection;

    /* hands the buffer over to the connection, the release of it is
    the responsibility of the io layer from this point on */
    write_connection(connection, buffer, (unsigned int) size, NULL, NULL);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE write_value_http2_connection(struct http2_connection_t *http2_connection, unsigned char type, unsigned int stream_id, unsigned int value) {
    /* allocates the buffer of the frame, it is released by the io
    layer once the write of it completes */
    unsigned char *buffer = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE + 4);
    size_t size;
    ERROR_CODE return_value;

    return_value = encode_value_http2(buffer, HTTP2_HEADER_SIZE + 4, type, stream_id, value, &size);
    if(IS_ERROR_CODE(return_value)) {
        FREE(buffer);
        RAISE_AGAIN(return_value);
    }

    /* raises again the result of the write of the frame */
    return_value = _write_http2_connection(http2_connection, buffer, size);
    RAISE_AGAIN(return_value);
}

/**
 * Closes the connection once the frame that tells the peer that it
 * is going away has actually reached it, closing it any earlier
 * would drop the frame together with the queue that holds it.
 *
 * @param connection The connection to be closed.
 * @param data The data that has been written.
 * @param parameters The value handed to the callback.
 * @return The resulting error code.
 */
static ERROR_CODE _close_http2_connection(struct connection_t *connection, struct data_t *data, void *parameters) {
    connection->close_connection(connection);
    RAISE_NO_ERROR;
}

ERROR_CODE write_goaway_http2_connection(struct http2_connection_t *http2_connection, unsigned int error) {
    /* allocates the buffer of the frame, it is released by the io
    layer once the write of it completes */
    unsigned char *buffer;
    size_t size;
    ERROR_CODE return_value;

    /* retrieves the connection, it is the one that both the frame
    and the closing that follows it travel through */
    struct connection_t *connection =
        http2_connection->http_connection->io_connection->connection;

    /* the connection is told to go away exactly once, a second
    frame would carry a last stream that has already been passed */
    if(http2_connection->goaway == TRUE) {
        connection->close_connection(connection);
        RAISE_NO_ERROR;
    }
    http2_connection->goaway = TRUE;

    buffer = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE + 8);
    return_value = encode_goaway_http2(
        buffer,
        HTTP2_HEADER_SIZE + 8,
        http2_connection->last_stream_id,
        error,
        &size
    );
    if(IS_ERROR_CODE(return_value)) {
        FREE(buffer);
        connection->close_connection(connection);
        RAISE_AGAIN(return_value);
    }

    /* hands the frame over together with the closing of the
    connection, which only happens once it has reached the peer */
    write_connection(
        connection,
        buffer,
        (unsigned int) size,
        _close_http2_connection,
        NULL
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Writes a settings frame carrying the values of this end, which
 * is what opens the connection, or the acknowledgement of the ones
 * that the peer has sent.
 *
 * @param http2_connection The session writing the frame.
 * @param ack The value one to write the acknowledgement and the
 * value zero to write the settings themselves.
 * @return The resulting error code.
 */
static ERROR_CODE _write_settings_http2_connection(struct http2_connection_t *http2_connection, char ack) {
    /* allocates the buffer of the frame, it is released by the io
    layer once the write of it completes */
    unsigned char *buffer;
    size_t size;
    ERROR_CODE return_value;

    /* the acknowledgement carries no payload at all, so only the
    header of the frame has to be written */
    if(ack == TRUE) {
        buffer = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE);
        return_value = encode_frame_http2(
            buffer,
            HTTP2_HEADER_SIZE,
            0,
            HTTP2_SETTINGS,
            HTTP2_FLAG_ACK,
            0
        );
        size = HTTP2_HEADER_SIZE;
    } else {
        buffer = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE + HTTP2_SETTING_SIZE * 3);
        return_value = encode_settings_http2(
            buffer,
            HTTP2_HEADER_SIZE + HTTP2_SETTING_SIZE * 3,
            &http2_connection->settings,
            &size
        );
    }

    if(IS_ERROR_CODE(return_value)) {
        FREE(buffer);
        RAISE_AGAIN(return_value);
    }

    /* raises again the result of the write of the frame */
    return_value = _write_http2_connection(http2_connection, buffer, size);
    RAISE_AGAIN(return_value);
}

/**
 * Writes the acknowledgement of a ping, it carries back the very
 * same payload that the peer has sent.
 *
 * @param http2_connection The session writing the frame.
 * @param data The payload of the ping being acknowledged.
 * @return The resulting error code.
 */
static ERROR_CODE _write_ping_http2_connection(struct http2_connection_t *http2_connection, const unsigned char *data) {
    /* allocates the buffer of the frame, it is released by the io
    layer once the write of it completes */
    unsigned char *buffer = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE + HTTP2_PING_SIZE);
    ERROR_CODE return_value;

    return_value = encode_frame_http2(
        buffer,
        HTTP2_HEADER_SIZE + HTTP2_PING_SIZE,
        HTTP2_PING_SIZE,
        HTTP2_PING,
        HTTP2_FLAG_ACK,
        0
    );
    if(IS_ERROR_CODE(return_value)) {
        FREE(buffer);
        RAISE_AGAIN(return_value);
    }

    memcpy(&buffer[HTTP2_HEADER_SIZE], data, HTTP2_PING_SIZE);

    /* raises again the result of the write of the frame */
    return_value = _write_http2_connection(
        http2_connection,
        buffer,
        HTTP2_HEADER_SIZE + HTTP2_PING_SIZE
    );
    RAISE_AGAIN(return_value);
}

/**
 * Hands the pseudo headers of a block to the handler, in the very
 * same shape that the request line and the host header of HTTP/1.1
 * would have reached it.
 * The specification puts no order at all among the pseudo headers,
 * so none of them may be handed over before the complete set has
 * been gathered, which is why this only runs once the first of the
 * regular fields arrives or the block ends.
 *
 * @param http2_block The block whose pseudo headers are complete.
 * @return The resulting error code.
 */
static ERROR_CODE _pseudo_http2_connection(struct http2_block_t *http2_block) {
    /* retrieves both the message and the settings of the stream, the
    latter carries the callbacks of the handler serving it */
    struct http_request_t *http_request = http2_block->http2_stream->request;
    struct http_settings_t *http_settings = http2_block->http2_stream->http_settings;

    /* the pseudo headers are handed over exactly once, no matter how
    many of the regular fields follow them */
    if(http2_block->flushed == TRUE) { RAISE_NO_ERROR; }
    http2_block->flushed = TRUE;

    /* a trailer section carries no pseudo header at all, so there's
    nothing of them to be handed over */
    if(http2_block->trailers == TRUE) { RAISE_NO_ERROR; }

    /* a request that leaves out any one of the required pseudo
    headers is not a complete one and is refused */
    if((http2_block->pseudo & HTTP2_PSEUDO_REQUIRED) != HTTP2_PSEUDO_REQUIRED) {
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }

    /* hands the path over as the url of the message, which is what an
    handler reads the resource being asked for from */
    if(http_settings->on_url) {
        http_settings->on_url(http_request, http_request->path, http_request->path_size);
    }

    /* hands the authority over as the host header, which is where an
    handler looks for it */
    if(http_request->authority[0] != '\0') {
        if(http_settings->on_header_field) {
            http_settings->on_header_field(http_request, (unsigned char *) HOST_H, strlen(HOST_H));
        }
        if(http_settings->on_header_value) {
            http_settings->on_header_value(
                http_request,
                http_request->authority,
                strlen((char *) http_request->authority)
            );
        }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Gathers a single field of a header block into the message of the
 * stream, the pseudo headers populate it directly and the regular
 * ones are handed to the handler as they would be under HTTP/1.1.
 *
 * @param parameters The block structure carrying both the session
 * and the stream the field belongs to.
 * @param hpack_header The field that has been decoded.
 * @return The resulting error code.
 */
static ERROR_CODE _header_http2_connection(void *parameters, struct hpack_header_t *hpack_header) {
    /* retrieves the block structure and out of it both the stream
    and the message that the field is going to populate */
    struct http2_block_t *http2_block = (struct http2_block_t *) parameters;
    struct http2_stream_t *http2_stream = http2_block->http2_stream;
    struct http_request_t *http_request = http2_stream->request;
    struct http_settings_t *http_settings = http2_stream->http_settings;
    size_t index;
    ERROR_CODE return_value;

    /* a field whose name starts with a colon is a pseudo header,
    they carry the parts of the request line of HTTP/1.1 */
    if(hpack_header->name_size > 0 && hpack_header->name[0] == ':') {
        /* the pseudo headers are required to come before any of the
        regular ones, a peer that mixes them is in error */
        if(http2_block->regular == TRUE) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

        /* a trailer section is never allowed to carry one of them,
        the request line has already been delivered by then */
        if(http2_block->trailers == TRUE) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

        if(hpack_header->name_size == 7 && memcmp(hpack_header->name, ":method", 7) == 0) {
            if(http2_block->pseudo & HTTP2_PSEUDO_METHOD) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            http2_block->pseudo |= HTTP2_PSEUDO_METHOD;
            http_request->method = _method_http2_connection(hpack_header->value, hpack_header->value_size);
            if(http_request->method == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            RAISE_NO_ERROR;
        }

        if(hpack_header->name_size == 7 && memcmp(hpack_header->name, ":scheme", 7) == 0) {
            if(http2_block->pseudo & HTTP2_PSEUDO_SCHEME) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            http2_block->pseudo |= HTTP2_PSEUDO_SCHEME;

            /* the scheme is kept as one of the two static strings so
            that nothing has to be released together with it */
            if(hpack_header->value_size == 5 && memcmp(hpack_header->value, HTTPS_SCHEME, 5) == 0) {
                http_request->scheme = HTTPS_SCHEME;
            } else if(hpack_header->value_size == 4 && memcmp(hpack_header->value, HTTP_SCHEME, 4) == 0) {
                http_request->scheme = HTTP_SCHEME;
            } else {
                RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
            }
            RAISE_NO_ERROR;
        }

        if(hpack_header->name_size == 5 && memcmp(hpack_header->name, ":path", 5) == 0) {
            if(http2_block->pseudo & HTTP2_PSEUDO_PATH) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            http2_block->pseudo |= HTTP2_PSEUDO_PATH;

            /* an empty path never refers to a resource, the origin
            form of a request always carries at least a slash */
            if(hpack_header->value_size == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

            /* gathers the path into the message, it only reaches the
            handler once the complete set of the pseudo headers has
            been gathered as they carry no order among them */
            append_path_http_request(http_request, hpack_header->value, hpack_header->value_size);
            RAISE_NO_ERROR;
        }

        if(hpack_header->name_size == 10 && memcmp(hpack_header->name, ":authority", 10) == 0) {
            if(http2_block->pseudo & HTTP2_PSEUDO_AUTHORITY) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            http2_block->pseudo |= HTTP2_PSEUDO_AUTHORITY;
            if(hpack_header->value_size >= VIRIATUM_MAX_PATH_SIZE) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            memcpy(http_request->authority, hpack_header->value, hpack_header->value_size);
            http_request->authority[hpack_header->value_size] = '\0';
            RAISE_NO_ERROR;
        }

        /* a pseudo header that is not one of the four of a request
        is not allowed, the status one belongs to a response */
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }

    /* the section of the pseudo headers closes here, so the complete
    set of them is handed to the handler before this field is */
    if(http2_block->regular == FALSE) {
        return_value = _pseudo_http2_connection(http2_block);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    }
    http2_block->regular = TRUE;

    /* the name of a field never carries an upper case letter under
    this protocol, a block that does is malformed */
    for(index = 0; index < hpack_header->name_size; index++) {
        if(hpack_header->name[index] >= 'A' && hpack_header->name[index] <= 'Z') {
            RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
        }
    }

    /* the connection specific fields carry no meaning under HTTP/2
    and their presence is an error of the peer */
    if(hpack_header->name_size == 10 && memcmp(hpack_header->name, "connection", 10) == 0) {
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }
    if(hpack_header->name_size == 10 && memcmp(hpack_header->name, "keep-alive", 10) == 0) {
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }
    if(hpack_header->name_size == 7 && memcmp(hpack_header->name, "upgrade", 7) == 0) {
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }
    if(hpack_header->name_size == 17 && memcmp(hpack_header->name, "transfer-encoding", 17) == 0) {
        RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
    }

    /* the only value that the transfer encoding field is allowed to
    carry under this protocol is the one that announces trailers */
    if(hpack_header->name_size == 2 && memcmp(hpack_header->name, "te", 2) == 0) {
        if(hpack_header->value_size != 8 || memcmp(hpack_header->value, "trailers", 8) != 0) {
            RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
        }
    }

    /* gathers the length of the payload that the message announces,
    so that it may be verified against the one that arrives */
    if(hpack_header->name_size == 14 && memcmp(hpack_header->name, "content-length", 14) == 0) {
        http2_block->http2_stream->content_length = 0;
        for(index = 0; index < hpack_header->value_size; index++) {
            if(hpack_header->value[index] < '0' || hpack_header->value[index] > '9') {
                RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
            }
            http2_block->http2_stream->content_length =
                http2_block->http2_stream->content_length * 10 +
                (size_t) (hpack_header->value[index] - '0');
        }
        http2_block->http2_stream->announced = TRUE;
    }

    /* hands the field to the handler in the very same shape that the
    HTTP/1.1 parser would have handed it */
    if(http_settings->on_header_field) {
        http_settings->on_header_field(http_request, hpack_header->name, hpack_header->name_size);
    }
    if(http_settings->on_header_value) {
        http_settings->on_header_value(http_request, hpack_header->value, hpack_header->value_size);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Handles the header block that has been assembled for a stream,
 * decoding it and driving the handler through the very same
 * sequence of callbacks that the HTTP/1.1 parser produces.
 *
 * @param http2_connection The session holding the stream.
 * @param http2_stream The stream the block belongs to.
 * @param data The assembled header block.
 * @param data_size The size in bytes of the block.
 * @return The resulting error code.
 */
static ERROR_CODE _block_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream, const unsigned char *data, size_t data_size) {
    /* allocates the structure that carries the stream through the
    decoding of the block */
    struct http2_block_t http2_block;
    struct http_handler_t *http_handler;
    ERROR_CODE return_value;

    http2_block.http2_connection = http2_connection;
    http2_block.http2_stream = http2_stream;
    http2_block.regular = FALSE;
    http2_block.flushed = FALSE;
    http2_block.pseudo = 0;
    http2_block.trailers = http2_stream->headers_complete;

    /* takes the base handler of the connection and sets it on the
    stream, every stream is served from the very same entry point */
    if(http2_stream->http_handler == NULL) {
        http_handler = http2_connection->http_connection->base_handler;
        http2_stream->http_handler = http_handler;
        _activate_http2_connection(http2_connection, http2_stream);
        http_handler->set(http2_connection->http_connection);
        http2_stream->http_handler = http2_connection->http_connection->http_handler;
    }

    /* makes the stream the current one so that the callbacks reach
    the structures that belong to it */
    _activate_http2_connection(http2_connection, http2_stream);

    /* tells the handler that a new message begins, this is the very
    same sequence the HTTP/1.1 parser produces, note that the handler
    of the stream is taken again once the block is over as a handler
    that dispatches only resolves the target as the url reaches it */
    if(http2_stream->http_settings->on_message_begin) {
        http2_stream->http_settings->on_message_begin(http2_stream->request);
    }

    /* decodes the block, a failure of it corrupts the dynamic table
    of the connection and so it is never a stream error */
    return_value = decode_hpack(
        http2_connection->decoder,
        data,
        data_size,
        _header_http2_connection,
        (void *) &http2_block
    );
    if(IS_ERROR_CODE(return_value)) { RAISE_ERROR_S(HTTP2_COMPRESSION_ERROR); }

    /* a block that carries no regular field at all has not handed the
    pseudo headers over yet, so it happens here */
    return_value = _pseudo_http2_connection(&http2_block);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    http2_stream->headers_complete = TRUE;
    if(http2_stream->http_settings->on_headers_complete) {
        http2_stream->http_settings->on_headers_complete(http2_stream->request);
    }

    /* takes the handler that is serving the stream again, one that
    dispatches switches the connection onto the target as the url
    reaches it and the stream has to follow it, otherwise the release
    of the message would reach the handler that never served it */
    http2_stream->http_handler = http2_connection->http_connection->http_handler;

    /* a block that closes the stream carries the complete message,
    so the handler is told that it is complete right away */
    if(http2_stream->end_stream == TRUE) {
        if(http2_stream->http_settings->on_message_complete) {
            http2_stream->http_settings->on_message_complete(http2_stream->request);
        }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE push_stream_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream, const char *path) {
    /* allocates space for the stream that the promise reserves, for
    the frame that carries it and for the field being encoded */
    struct http2_stream_t *promised;
    struct hpack_header_t hpack_header;
    unsigned char *buffer;
    size_t offset = HTTP2_HEADER_SIZE + 4;
    size_t size;
    unsigned int stream_id;
    ERROR_CODE return_value;

    /* a peer that has turned the pushing off gets nothing at all, the
    setting is the only say it has on the matter */
    if(http2_connection->remote.enable_push == FALSE) { RAISE_NO_ERROR; }

    /* a promise opens a stream of this end, so it is bound by the
    limit that the peer has announced rather than by the one that
    bounds the streams the peer itself opens */
    if(http2_connection->count >= http2_connection->remote.max_concurrent_streams ||
       http2_connection->count >= HTTP2_STREAM_SLOTS) {
        RAISE_NO_ERROR;
    }

    /* reserves the next of the identifiers of this end, a server only
    ever opens the even numbered ones */
    http2_connection->push_stream_id += 2;
    stream_id = http2_connection->push_stream_id;
    if(stream_id > (unsigned int) HTTP2_MAX_STREAM_ID) { RAISE_NO_ERROR; }

    /* builds the block of the request that is being promised, the
    pseudo headers of it describe the resource the peer is going to
    receive without having asked for it */
    buffer = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE + 4 + VIRIATUM_HTTP_SIZE);
    size = HTTP2_HEADER_SIZE + 4 + VIRIATUM_HTTP_SIZE;

    hpack_header.name = (unsigned char *) ":method";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) "GET";
    hpack_header.value_size = 3;
    encode_hpack(http2_connection->encoder, buffer, size, &offset, &hpack_header, FALSE);

    hpack_header.name = (unsigned char *) ":scheme";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) http2_stream->request->scheme;
    hpack_header.value_size = strlen(http2_stream->request->scheme);
    encode_hpack(http2_connection->encoder, buffer, size, &offset, &hpack_header, FALSE);

    hpack_header.name = (unsigned char *) ":path";
    hpack_header.name_size = 5;
    hpack_header.value = (unsigned char *) path;
    hpack_header.value_size = strlen(path);
    encode_hpack(http2_connection->encoder, buffer, size, &offset, &hpack_header, FALSE);

    if(http2_stream->request->authority[0] != '\0') {
        hpack_header.name = (unsigned char *) ":authority";
        hpack_header.name_size = 10;
        hpack_header.value = http2_stream->request->authority;
        hpack_header.value_size = strlen((char *) http2_stream->request->authority);
        encode_hpack(http2_connection->encoder, buffer, size, &offset, &hpack_header, FALSE);
    }

    /* writes the header of the frame together with the identifier of
    the stream that the promise reserves, the promise itself travels
    on the stream that has asked for the resource that refers to it */
    return_value = encode_frame_http2(
        buffer,
        size,
        offset - HTTP2_HEADER_SIZE,
        HTTP2_PUSH_PROMISE,
        HTTP2_FLAG_END_HEADERS,
        http2_stream->stream_id
    );
    if(IS_ERROR_CODE(return_value)) {
        FREE(buffer);
        RAISE_AGAIN(return_value);
    }
    encode_number_http2(&buffer[HTTP2_HEADER_SIZE], stream_id);

    return_value = _write_http2_connection(http2_connection, buffer, offset);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* opens the stream that the promise has reserved, it is the one
    the response of the resource is going to be written on */
    promised = &http2_connection->streams[http2_connection->count];
    promised->stream_id = stream_id;
    promised->state = HTTP2_STATE_RESERVED_LOCAL;
    promised->send_window = (long) http2_connection->remote.initial_window_size;
    promised->receive_window = (long) http2_connection->settings.initial_window_size;
    promised->priority.dependency = http2_stream->stream_id;
    promised->priority.weight = HTTP2_DEFAULT_WEIGHT;
    promised->priority.exclusive = FALSE;
    promised->http_handler = NULL;
    promised->end_stream = TRUE;
    promised->headers_complete = TRUE;
    promised->complete = FALSE;
    promised->content_length = 0;
    promised->announced = FALSE;
    promised->received = 0;

    create_http_request(&promised->request);
    create_http_settings(&promised->http_settings);
    create_linked_list(&promised->pending);

    promised->request->version = HTTP20;
    promised->request->stream_id = stream_id;
    promised->request->parameters = http2_connection->http_connection->io_connection->connection;
    promised->request->flags = FLAG_KEEP_ALIVE;
    promised->request->method = HTTP_GET;
    promised->request->scheme = http2_stream->request->scheme;
    append_path_http_request(promised->request, (const unsigned char *) path, strlen(path));
    memcpy(promised->request->authority, http2_stream->request->authority, VIRIATUM_MAX_PATH_SIZE);

    http2_connection->count++;

    /* hands the promised request to the handler, which answers it as
    though the peer had asked for it in the first place */
    promised->http_handler = http2_connection->http_connection->base_handler;
    _activate_http2_connection(http2_connection, promised);
    promised->http_handler->set(http2_connection->http_connection);
    promised->http_handler = http2_connection->http_connection->http_handler;
    _activate_http2_connection(http2_connection, promised);

    if(promised->http_settings->on_message_begin) {
        promised->http_settings->on_message_begin(promised->request);
    }
    if(promised->http_settings->on_url) {
        promised->http_settings->on_url(
            promised->request,
            promised->request->path,
            promised->request->path_size
        );
    }
    if(promised->http_settings->on_headers_complete) {
        promised->http_settings->on_headers_complete(promised->request);
    }
    if(promised->http_settings->on_message_complete) {
        promised->http_settings->on_message_complete(promised->request);
    }

    /* takes the handler that is serving the promised stream again,
    the same way the block of a request does */
    promised->http_handler = http2_connection->http_connection->http_handler;

    /* makes the stream that has promised the resource the current one
    again, the handler of it is still writing the response that has
    referred to the resource being promised */
    _activate_http2_connection(http2_connection, http2_stream);

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Gathers a fragment of a header block into the buffer that
 * assembles it, bounding the block so that a peer is not able to
 * make it grow without any limit.
 *
 * @param http2_connection The session assembling the block.
 * @param data The fragment to be gathered.
 * @param data_size The size in bytes of the fragment.
 * @return The resulting error code.
 */
static ERROR_CODE _gather_http2_connection(struct http2_connection_t *http2_connection, const unsigned char *data, size_t data_size) {
    /* a block that grows beyond the accepted size is refused, a peer
    is otherwise able to stream continuation frames forever */
    if(http2_connection->block_size + data_size > HTTP2_MAX_BLOCK) {
        RAISE_ERROR_S(HTTP2_ENHANCE_YOUR_CALM);
    }

    /* grows the buffer of the block and gathers the fragment at the
    end of what has been assembled so far, the first allocation goes
    through the counted operation so that the accounting of the
    allocations stays balanced against its release */
    if(http2_connection->block == NULL) {
        http2_connection->block = (unsigned char *) MALLOC(data_size);
    } else {
        http2_connection->block = (unsigned char *) REALLOC(
            (void *) http2_connection->block,
            http2_connection->block_size + data_size
        );
    }
    memcpy(&http2_connection->block[http2_connection->block_size], data, data_size);
    http2_connection->block_size += data_size;

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Releases the buffer that assembles a header block and closes the
 * sequence of continuation frames that was open.
 *
 * @param http2_connection The session to be reset.
 */
static void _reset_block_http2_connection(struct http2_connection_t *http2_connection) {
    if(http2_connection->block != NULL) { FREE(http2_connection->block); }
    http2_connection->block = NULL;
    http2_connection->block_size = 0;
    http2_connection->continuation = 0;
    http2_connection->continuation_flags = 0;
}

ERROR_CODE handle_frame_http2_connection(struct http2_connection_t *http2_connection, struct http2_frame_t *http2_frame) {
    /* allocates space for the stream the frame belongs to and for
    the payload once the padding has been removed from it */
    struct http2_stream_t *http2_stream;
    struct http2_priority_t http2_priority;
    unsigned char *payload;
    size_t payload_size;
    size_t offset;
    ERROR_CODE return_value;

    /* verifies that the frame is a coherent one for its type before
    anything at all is done with it */
    return_value = verify_frame_http2(http2_frame, &http2_connection->settings);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* while a header block is being assembled the only frame that
    the peer is allowed to send is the continuation of it, on the
    very same stream, anything else breaks the block apart */
    if(http2_connection->continuation != 0) {
        if(http2_frame->type != HTTP2_CONTINUATION ||
           http2_frame->stream_id != http2_connection->continuation) {
            RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
        }
    }

    switch(http2_frame->type) {
        case HTTP2_DATA:
            http2_stream = find_stream_http2_connection(http2_connection, http2_frame->stream_id);

            /* the payload of a stream that is no longer open is
            still accounted against the window of the connection,
            the peer has already sent it either way */
            http2_connection->receive_window -= (long) http2_frame->length;
            if(http2_connection->receive_window < 0) { RAISE_ERROR_S(HTTP2_FLOW_CONTROL_ERROR); }

            if(http2_stream == NULL) { RAISE_ERROR_S(HTTP2_STREAM_CLOSED); }
            if(http2_stream->end_stream == TRUE) { RAISE_ERROR_S(HTTP2_STREAM_CLOSED); }

            http2_stream->receive_window -= (long) http2_frame->length;
            if(http2_stream->receive_window < 0) { RAISE_ERROR_S(HTTP2_FLOW_CONTROL_ERROR); }

            return_value = strip_padding_http2(http2_frame, &payload, &payload_size);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* accounts the payload against the size that the message
            has announced, the two are compared once it closes */
            http2_stream->received += payload_size;

            _activate_http2_connection(http2_connection, http2_stream);
            if(payload_size > 0 && http2_stream->http_settings->on_body) {
                http2_stream->http_settings->on_body(http2_stream->request, payload, payload_size);
            }

            /* widens the windows of both the connection and the
            stream by what has just been consumed, this end has no
            reason to hold the peer back */
            if(http2_frame->length > 0) {
                write_value_http2_connection(
                    http2_connection,
                    HTTP2_WINDOW_UPDATE,
                    0,
                    (unsigned int) http2_frame->length
                );
                http2_connection->receive_window += (long) http2_frame->length;
                write_value_http2_connection(
                    http2_connection,
                    HTTP2_WINDOW_UPDATE,
                    http2_frame->stream_id,
                    (unsigned int) http2_frame->length
                );
                http2_stream->receive_window += (long) http2_frame->length;
            }

            if(http2_frame->flags & HTTP2_FLAG_END_STREAM) {
                http2_stream->end_stream = TRUE;
                http2_stream->state = HTTP2_STATE_HALF_CLOSED_REMOTE;

                /* the payload that has arrived is required to be of
                the very size that the message has announced */
                if(http2_stream->announced == TRUE &&
                   http2_stream->received != http2_stream->content_length) {
                    RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
                }

                if(http2_stream->http_settings->on_message_complete) {
                    http2_stream->http_settings->on_message_complete(http2_stream->request);
                }
            }

            /* breaks the switch */
            break;

        case HTTP2_HEADERS:
            /* a header block that arrives for a stream that is
            already open is a trailer section, anything else opens
            a new stream */
            http2_stream = find_stream_http2_connection(http2_connection, http2_frame->stream_id);
            if(http2_stream == NULL) {
                return_value = open_stream_http2_connection(
                    http2_connection,
                    http2_frame->stream_id,
                    &http2_stream
                );
                if(return_value == HTTP2_REFUSED_STREAM) {
                    write_value_http2_connection(
                        http2_connection,
                        HTTP2_RST_STREAM,
                        http2_frame->stream_id,
                        HTTP2_REFUSED_STREAM
                    );
                    RAISE_NO_ERROR;
                }
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            } else if(http2_stream->headers_complete == TRUE) {
                /* a second block on an open stream is a trailer
                section, which is required to close the stream */
                if(!(http2_frame->flags & HTTP2_FLAG_END_STREAM)) {
                    RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
                }
                http2_stream->request->trailers = TRUE;
            }

            return_value = strip_padding_http2(http2_frame, &payload, &payload_size);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* the priority information sits between the padding and
            the block itself when the flag announces it */
            if(http2_frame->flags & HTTP2_FLAG_PRIORITY) {
                if(payload_size < HTTP2_PRIORITY_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }
                return_value = decode_priority_http2(payload, HTTP2_PRIORITY_SIZE, &http2_priority);
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

                /* places the stream at the position of the tree that
                the frame describes, a dependency on itself is refused
                by the placing as it would close a cycle */
                return_value = prioritise_stream_http2_connection(
                    http2_connection,
                    http2_stream,
                    &http2_priority
                );
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
                payload += HTTP2_PRIORITY_SIZE;
                payload_size -= HTTP2_PRIORITY_SIZE;
            }

            if(http2_frame->flags & HTTP2_FLAG_END_STREAM) {
                http2_stream->end_stream = TRUE;
                http2_stream->state = HTTP2_STATE_HALF_CLOSED_REMOTE;
            }

            return_value = _gather_http2_connection(http2_connection, payload, payload_size);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* a block that is not closed by this frame continues in
            the frames that follow it, nothing else may come between */
            if(!(http2_frame->flags & HTTP2_FLAG_END_HEADERS)) {
                http2_connection->continuation = http2_frame->stream_id;
                http2_connection->continuation_flags = http2_frame->flags;
                RAISE_NO_ERROR;
            }

            return_value = _block_http2_connection(
                http2_connection,
                http2_stream,
                http2_connection->block,
                http2_connection->block_size
            );
            _reset_block_http2_connection(http2_connection);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* breaks the switch */
            break;

        case HTTP2_CONTINUATION:
            /* a continuation that does not follow a block is not
            expected at all, the sequence has to be open */
            if(http2_connection->continuation == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

            return_value = _gather_http2_connection(
                http2_connection,
                http2_frame->payload,
                http2_frame->length
            );
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            if(!(http2_frame->flags & HTTP2_FLAG_END_HEADERS)) { RAISE_NO_ERROR; }

            http2_stream = find_stream_http2_connection(http2_connection, http2_connection->continuation);
            if(http2_stream == NULL) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

            return_value = _block_http2_connection(
                http2_connection,
                http2_stream,
                http2_connection->block,
                http2_connection->block_size
            );
            _reset_block_http2_connection(http2_connection);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* breaks the switch */
            break;

        case HTTP2_PRIORITY:
            return_value = decode_priority_http2(
                http2_frame->payload,
                http2_frame->length,
                &http2_priority
            );
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* a stream that depends on itself would make the tree
            carry a cycle and so it is refused */
            if(http2_priority.dependency == http2_frame->stream_id) {
                RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
            }

            /* the priority of a stream that is not open is simply
            discarded, the tree of this end holds only open ones */
            http2_stream = find_stream_http2_connection(http2_connection, http2_frame->stream_id);
            if(http2_stream != NULL) {
                return_value = prioritise_stream_http2_connection(
                    http2_connection,
                    http2_stream,
                    &http2_priority
                );
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            }

            /* breaks the switch */
            break;

        case HTTP2_RST_STREAM:
            /* the reset of a stream that was never open refers to
            an idle one, which the peer is not allowed to reset */
            http2_stream = find_stream_http2_connection(http2_connection, http2_frame->stream_id);
            if(http2_stream == NULL) {
                if(http2_frame->stream_id > http2_connection->last_stream_id) {
                    RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
                }
                RAISE_NO_ERROR;
            }

            close_stream_http2_connection(http2_connection, http2_stream);

            /* breaks the switch */
            break;

        case HTTP2_SETTINGS:
            /* the acknowledgement of the settings of this end
            carries nothing that has to be applied */
            if(http2_frame->flags & HTTP2_FLAG_ACK) { RAISE_NO_ERROR; }

            return_value = apply_settings_http2_connection(
                http2_connection,
                http2_frame->payload,
                http2_frame->length
            );
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* the settings of the peer are acknowledged as soon as
            they have been applied */
            return_value = _write_settings_http2_connection(http2_connection, TRUE);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* a change of the initial window is carried over to the
            streams that are already open, so the payload that they
            are holding back may have somewhere to go now */
            _schedule_http2_connection(http2_connection);

            /* breaks the switch */
            break;

        case HTTP2_PING:
            /* the acknowledgement of a ping of this end is not
            answered, only the ping of the peer is */
            if(http2_frame->flags & HTTP2_FLAG_ACK) { RAISE_NO_ERROR; }

            return_value = _write_ping_http2_connection(http2_connection, http2_frame->payload);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* breaks the switch */
            break;

        case HTTP2_GOAWAY:
            /* the peer is closing the connection, no new stream is
            accepted from this point on */
            http2_connection->goaway = TRUE;

            /* breaks the switch */
            break;

        case HTTP2_WINDOW_UPDATE:
            offset = decode_number_http2(http2_frame->payload) & (unsigned int) HTTP2_MAX_WINDOW_SIZE;
            return_value = update_window_http2_connection(
                http2_connection,
                http2_frame->stream_id,
                (unsigned int) offset
            );

            /* an increment that overflows the window of a stream is
            an error of that stream alone and not of the connection */
            if(return_value == HTTP2_FLOW_CONTROL_ERROR && http2_frame->stream_id != 0) {
                write_value_http2_connection(
                    http2_connection,
                    HTTP2_RST_STREAM,
                    http2_frame->stream_id,
                    HTTP2_FLOW_CONTROL_ERROR
                );
                RAISE_NO_ERROR;
            }
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* the widening of a window may let through the payload
            that the streams have been holding back, the order they
            write in is the one the tree of the priorities describes */
            _schedule_http2_connection(http2_connection);

            /* breaks the switch */
            break;

        case HTTP2_PUSH_PROMISE:
            /* a server never receives a promise, only a client does,
            so its presence is an error of the peer */
            RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);

        default:
            /* a frame of an unknown type is ignored, this is what
            allows the protocol to be extended */
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Retrieves the stream that is currently being served in the
 * provided connection, it is the one the message of the HTTP
 * connection belongs to.
 *
 * @param connection The connection being served.
 * @param http2_connection_pointer The pointer to the session that
 * drives the connection.
 * @return The stream being served or an unset value in case the
 * connection is not serving one.
 */
static struct http2_stream_t *_current_http2_connection(struct connection_t *connection, struct http2_connection_t **http2_connection_pointer) {
    /* walks the substrates of the connection down to the session
    that is driving the protocol over it */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct http2_connection_t *http2_connection = http_connection->http2_connection;

    *http2_connection_pointer = http2_connection;
    if(http_connection->request == NULL) { return NULL; }

    /* the message carries the identifier of the stream it belongs
    to, which is the one the response is written on */
    return find_stream_http2_connection(http2_connection, http_connection->request->stream_id);
}

/**
 * Structure that carries a stream through the completion of a
 * write, so that the handler resumes on the very same message it
 * was serving.
 * A connection serves several streams at the same time and the
 * writes of them complete in an order of their own, so the message
 * that is current when a write completes is not the one that
 * started it.
 */
typedef struct http2_callback_t {
    /**
     * The session the write belongs to.
     */
    struct http2_connection_t *http2_connection;

    /**
     * The identifier of the stream the write belongs to.
     */
    unsigned int stream_id;

    /**
     * The callback of the handler, driven once the stream has been
     * made the current one again.
     */
    connection_data_callback_h2 callback;

    /**
     * The value handed to the callback of the handler.
     */
    void *parameters;
} http2_callback;

/**
 * Restores the stream that a write belongs to and then drives the
 * callback that the handler was waiting on.
 *
 * @param connection The connection the write belongs to.
 * @param data The data that has been written.
 * @param parameters The structure carrying the stream.
 * @return The resulting error code.
 */
static ERROR_CODE _callback_http2_connection(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the structure that carries the stream and out of it
    the stream that the write belongs to */
    struct http2_callback_t *http2_callback = (struct http2_callback_t *) parameters;
    struct http2_stream_t *http2_stream = find_stream_http2_connection(
        http2_callback->http2_connection,
        http2_callback->stream_id
    );
    ERROR_CODE return_value = 0;

    /* the structure is no longer outstanding, so the session stops
    holding it as one that has to be released with it */
    remove_value_linked_list(
        http2_callback->http2_connection->callbacks,
        (void *) http2_callback,
        TRUE
    );

    /* makes the stream the current one again and only then drives the
    callback, a stream that is gone leaves nothing to resume */
    if(http2_stream != NULL) {
        _activate_http2_connection(http2_callback->http2_connection, http2_stream);
        if(http2_callback->callback != NULL) {
            return_value = http2_callback->callback(connection, data, http2_callback->parameters);
        }

        /* the handler may have let go of itself along the callback,
        so the stream takes over whatever the connection is left
        with, otherwise the closing would unset it a second time */
        http2_stream->http_handler = http2_callback->http2_connection->http_connection->http_handler;

        /* the response has been written in full and nothing is being
        held back, so the stream has served its purpose and the slot
        it takes is handed back to the connection */
        if(http2_stream->complete == TRUE && http2_stream->pending->size == 0) {
            close_stream_http2_connection(http2_callback->http2_connection, http2_stream);
        }
    }

    FREE(http2_callback);

    /* raises again the result of the callback of the handler */
    RAISE_AGAIN(return_value);
}

/**
 * Builds the structure that carries a stream through the completion
 * of a write.
 *
 * @param http2_connection The session the write belongs to.
 * @param stream_id The stream the write belongs to.
 * @param callback The callback of the handler.
 * @param parameters The value handed to the callback.
 * @return The structure that has been built.
 */
static struct http2_callback_t *_wrap_http2_connection(struct http2_connection_t *http2_connection, unsigned int stream_id, connection_data_callback_h2 callback, void *parameters) {
    struct http2_callback_t *http2_callback =
        (struct http2_callback_t *) MALLOC(sizeof(struct http2_callback_t));

    http2_callback->http2_connection = http2_connection;
    http2_callback->stream_id = stream_id;
    http2_callback->callback = callback;
    http2_callback->parameters = parameters;

    /* holds the structure as an outstanding one, so that it is
    released even when the write it belongs to never completes */
    append_value_linked_list(http2_connection->callbacks, (void *) http2_callback);

    return http2_callback;
}

ERROR_CODE write_flush_http2(struct connection_t *connection, unsigned char *data, size_t size, connection_data_callback_h2 callback, void *parameters) {
    /* allocates space for the session and for the stream that the
    response is being written on */
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream = _current_http2_connection(connection, &http2_connection);

    /* a response written for a stream that is no longer open is
    discarded, the buffer of it is released either way */
    if(http2_stream == NULL) {
        FREE(data);
        RAISE_NO_ERROR;
    }

    /* hands the buffer over together with the structure that restores
    the stream once the write of it completes */
    write_connection(
        connection,
        data,
        (unsigned int) size,
        _callback_http2_connection,
        (void *) _wrap_http2_connection(
            http2_connection,
            http2_stream->stream_id,
            callback,
            parameters
        )
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

size_t write_status_http2(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive) {
    /* allocates space for the session, for the field being written
    and for the position it is written at */
    struct http2_connection_t *http2_connection;
    struct hpack_header_t hpack_header;
    char status[8];
    size_t offset = HTTP2_HEADER_SIZE;

    /* retrieves the session, the encoder of it is the one that the
    block of the response is built with */
    _current_http2_connection(connection, &http2_connection);

    /* writes the status as the first field of the block, the
    specification requires the pseudo headers to come first */
    SPRINTF(status, sizeof(status), "%d", status_code);
    hpack_header.name = (unsigned char *) ":status";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) status;
    hpack_header.value_size = strlen(status);
    encode_hpack(http2_connection->encoder, (unsigned char *) buffer, size, &offset, &hpack_header, FALSE);

    /* writes the field that describes the server, it is the one the
    status of HTTP/1.1 carries together with the line */
    hpack_header.name = (unsigned char *) "server";
    hpack_header.name_size = 6;
    hpack_header.value = connection->service->description;
    hpack_header.value_size = strlen((char *) connection->service->description);
    encode_hpack(http2_connection->encoder, (unsigned char *) buffer, size, &offset, &hpack_header, FALSE);

    /* returns the position that follows the fields written so far */
    return offset;
}

size_t write_field_http2(struct connection_t *connection, char *buffer, size_t size, size_t offset, const char *name, const char *value) {
    /* allocates space for the session, for the field being written
    and for the lowered name of it */
    struct http2_connection_t *http2_connection;
    struct hpack_header_t hpack_header;
    unsigned char lowered[VIRIATUM_MAX_HEADER_SIZE];
    size_t name_size = strlen(name);
    size_t index;

    _current_http2_connection(connection, &http2_connection);

    /* a name that does not fit the buffer of the lowering is never
    a valid field, so the response is left untouched */
    if(name_size >= sizeof(lowered)) { return offset; }

    /* lowers the name of the field, the protocol requires them to be
    written in lower case and refuses a response that does not */
    for(index = 0; index < name_size; index++) {
        lowered[index] = (unsigned char) tolower((unsigned char) name[index]);
    }
    lowered[name_size] = '\0';

    /* drops the fields that only carry meaning under HTTP/1.1, the
    peer refuses a response that carries one of them */
    if(strcmp((char *) lowered, "connection") == 0) { return offset; }
    if(strcmp((char *) lowered, "keep-alive") == 0) { return offset; }
    if(strcmp((char *) lowered, "transfer-encoding") == 0) { return offset; }
    if(strcmp((char *) lowered, "upgrade") == 0) { return offset; }

    hpack_header.name = lowered;
    hpack_header.name_size = name_size;
    hpack_header.value = (unsigned char *) value;
    hpack_header.value_size = strlen(value);
    encode_hpack(http2_connection->encoder, (unsigned char *) buffer, size, &offset, &hpack_header, FALSE);

    /* returns the position that follows the field just written */
    return offset;
}

size_t write_end_http2(struct connection_t *connection, char *buffer, size_t size, size_t offset, char last) {
    /* allocates space for the session and for the stream that the
    response is being written on */
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream = _current_http2_connection(connection, &http2_connection);
    unsigned char flags = HTTP2_FLAG_END_HEADERS;

    /* a response that carries no payload closes the stream together
    with the block of the headers */
    if(last == TRUE) {
        flags |= HTTP2_FLAG_END_STREAM;
        if(http2_stream != NULL) { http2_stream->complete = TRUE; }
    }

    /* writes the header of the frame into the space that has been
    reserved for it at the start of the buffer */
    encode_frame_http2(
        (unsigned char *) buffer,
        size,
        offset - HTTP2_HEADER_SIZE,
        HTTP2_HEADERS,
        flags,
        http2_stream == NULL ? 0 : http2_stream->stream_id
    );

    /* the block is complete, so the position of the end of it is the
    size of the complete frame */
    return offset;
}

/**
 * Writes as much of the payload that the provided stream is
 * holding back as the windows of both the stream and the
 * connection allow through.
 *
 * @param http2_connection The session the stream belongs to.
 * @param http2_stream The stream to be flushed.
 * @return The resulting error code.
 */
static ERROR_CODE _flush_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream) {
    /* allocates space for the fragment being flushed, for the amount
    of it that the windows allow through and for the frame */
    struct http2_pending_t *http2_pending;
    struct connection_t *connection;
    unsigned char *header;
    unsigned char *chunk;
    size_t remaining;
    size_t allowed;
    long window;
    char complete;

    connection = http2_connection->http_connection->io_connection->connection;

    while(http2_stream->pending->size > 0) {
        /* takes the oldest of the fragments the stream is holding
        back, it is the one that goes out next */
        get_value_linked_list(http2_stream->pending, 0, (void **) &http2_pending);
        remaining = http2_pending->size - http2_pending->offset;

        /* the amount that goes out is bounded by the window of the
        stream, by the one of the connection and by the largest frame
        that the peer has announced */
        window = http2_stream->send_window < http2_connection->send_window
                     ? http2_stream->send_window
                     : http2_connection->send_window;
        if(window <= 0) { break; }

        allowed = remaining;
        if((long) allowed > window) { allowed = (size_t) window; }
        if(allowed > http2_connection->remote.max_frame_size) {
            allowed = http2_connection->remote.max_frame_size;
        }
        complete = allowed == remaining ? TRUE : FALSE;

        /* writes the header of the frame, it is queued right before
        the payload it describes */
        header = (unsigned char *) MALLOC(HTTP2_HEADER_SIZE);
        encode_frame_http2(
            header,
            HTTP2_HEADER_SIZE,
            allowed,
            HTTP2_DATA,
            http2_pending->last == TRUE && complete == TRUE ? HTTP2_FLAG_END_STREAM : 0x00,
            http2_stream->stream_id
        );
        write_connection(connection, header, HTTP2_HEADER_SIZE, NULL, NULL);

        http2_stream->send_window -= (long) allowed;
        http2_connection->send_window -= (long) allowed;

        /* the fragment that closes the message is the last one that
        the stream is going to write, so the stream is done as soon
        as the handler lets go of it */
        if(http2_pending->last == TRUE && complete == TRUE) { http2_stream->complete = TRUE; }

        /* the complete fragment fits in a single frame and none of it
        has gone out yet, so the buffer of the handler travels as it
        stands and no copy of it is required at all */
        if(http2_pending->offset == 0 && complete == TRUE) {
            pop_value_linked_list(http2_stream->pending, (void **) &http2_pending, TRUE);
            write_connection(
                connection,
                http2_pending->data,
                (unsigned int) allowed,
                _callback_http2_connection,
                (void *) _wrap_http2_connection(
                    http2_connection,
                    http2_stream->stream_id,
                    http2_pending->callback,
                    http2_pending->parameters
                )
            );
            FREE(http2_pending);
            continue;
        }

        /* only a part of the fragment goes out, so that part is
        copied, the buffer of the handler is never handed over in
        pieces as the connection releases it as a whole */
        chunk = (unsigned char *) MALLOC(allowed);
        memcpy(chunk, &http2_pending->data[http2_pending->offset], allowed);
        http2_pending->offset += allowed;

        if(complete == FALSE) {
            write_connection(connection, chunk, (unsigned int) allowed, NULL, NULL);
            continue;
        }

        /* the last part of the fragment carries the callback of the
        handler, the buffer of it has been copied in full and so it
        is released right away */
        pop_value_linked_list(http2_stream->pending, (void **) &http2_pending, TRUE);
        write_connection(
            connection,
            chunk,
            (unsigned int) allowed,
            _callback_http2_connection,
            (void *) _wrap_http2_connection(
                http2_connection,
                http2_stream->stream_id,
                http2_pending->callback,
                http2_pending->parameters
            )
        );
        FREE(http2_pending->data);
        FREE(http2_pending);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

/**
 * Verifies whether the provided stream is kept from writing by one
 * of the streams it depends on, which is the case as soon as any of
 * them still holds payload of its own.
 *
 * @param http2_connection The session holding the streams.
 * @param http2_stream The stream being verified.
 * @return The value one when a stream above it still has payload.
 */
static char _blocked_http2_connection(struct http2_connection_t *http2_connection, struct http2_stream_t *http2_stream) {
    /* allocates space for the stream being visited along the walk and
    for the bound that keeps a broken tree from looping forever */
    struct http2_stream_t *parent;
    unsigned int stream_id = http2_stream->priority.dependency;
    size_t index;

    for(index = 0; index < http2_connection->count; index++) {
        /* the walk has reached the root of the tree, so nothing above
        this stream is holding anything back */
        if(stream_id == 0) { return FALSE; }

        /* a stream that is no longer open holds nothing at all, so the
        walk carries on towards the root */
        parent = find_stream_http2_connection(http2_connection, stream_id);
        if(parent == NULL) { return FALSE; }

        /* the stream above still has payload of its own and a window
        that lets it through, so this one waits for it */
        if(parent->pending->size > 0 && parent->send_window > 0) { return TRUE; }

        stream_id = parent->priority.dependency;
    }

    return FALSE;
}

/**
 * Writes the payload that the streams of the connection are holding
 * back, in the order that the tree of the priorities describes.
 * A stream only writes once every one of the streams it depends on
 * has nothing left to write, and among the ones that are able to
 * write the heavier goes first, which is the ordering the tree of
 * the specification asks for.
 *
 * @param http2_connection The session to be flushed.
 * @return The resulting error code.
 */
static ERROR_CODE _schedule_http2_connection(struct http2_connection_t *http2_connection) {
    /* allocates space for the iteration over the streams and for the
    one that is chosen to write on each of the rounds */
    struct http2_stream_t *http2_stream;
    struct http2_stream_t *chosen;
    size_t index;
    size_t rounds;
    size_t pending;

    /* every round writes at most one of the streams, so the bound of
    the rounds is the number of streams that hold something */
    for(rounds = 0; rounds < HTTP2_STREAM_SLOTS; rounds++) {
        chosen = NULL;

        for(index = 0; index < http2_connection->count; index++) {
            http2_stream = &http2_connection->streams[index];
            if(http2_stream->pending->size == 0) { continue; }

            /* a stream whose window is closed is not able to write, so
            it never keeps the ones below it from doing so */
            if(http2_stream->send_window <= 0) { continue; }

            /* a stream only writes once the ones it depends on have
            nothing left, which is what the tree describes */
            if(_blocked_http2_connection(http2_connection, http2_stream)) { continue; }

            /* among the ones that are able to write the heavier goes
            first, the weight is what orders the siblings */
            if(chosen == NULL || http2_stream->priority.weight > chosen->priority.weight) {
                chosen = http2_stream;
            }
        }

        /* nothing is able to write, either because nothing is being
        held or because the windows do not allow any of it through */
        if(chosen == NULL) { break; }

        /* remembers what the stream was holding so that a round that
        writes nothing at all does not spin forever */
        pending = chosen->pending->size;
        _flush_http2_connection(http2_connection, chosen);
        if(chosen->pending->size == pending) { break; }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE write_chunk_http2(struct connection_t *connection, unsigned char *data, size_t size, char last, connection_data_callback_h2 callback, void *parameters) {
    /* allocates space for the session, for the stream the payload
    belongs to and for the fragment being queued */
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream = _current_http2_connection(connection, &http2_connection);
    struct http2_pending_t *http2_pending;

    /* a payload written for a stream that is no longer open is
    discarded, the buffer of it is released either way */
    if(http2_stream == NULL) {
        FREE(data);
        RAISE_NO_ERROR;
    }

    /* queues the fragment on the stream, the windows decide how much
    of it goes out right away and how much of it waits */
    http2_pending = (struct http2_pending_t *) MALLOC(sizeof(struct http2_pending_t));
    http2_pending->data = data;
    http2_pending->size = size;
    http2_pending->offset = 0;
    http2_pending->last = last;
    http2_pending->callback = callback;
    http2_pending->parameters = parameters;
    append_value_linked_list(http2_stream->pending, (void *) http2_pending);

    /* hands the writing over to the scheduling, which is the one that
    decides the order the streams write in */
    RAISE_AGAIN(_schedule_http2_connection(http2_connection));
}

ERROR_CODE data_handler_stream_http2(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the frame being handled and for the
    positions in the buffer of the connection */
    struct http2_frame_t http2_frame;
    unsigned char *_read;
    size_t _read_size;
    size_t pending;
    ERROR_CODE return_value;

    /* retrieves the references to both the connection layers and to
    the session that drives the protocol */
    struct connection_t *connection = io_connection->connection;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct http2_connection_t *http2_connection = http_connection->http2_connection;

    /* gathers the data that has arrived at the end of the buffer of
    the connection, growing it when it does not fit */
    if(buffer_size > 0) {
        if(http_connection->buffer_offset + buffer_size > http_connection->buffer_size) {
            http_connection->buffer_size = http_connection->buffer_offset + buffer_size;

            /* the first allocation goes through the counted operation
            so that the accounting of the allocations stays balanced
            against the release of the buffer */
            if(http_connection->buffer == NULL) {
                http_connection->buffer = (unsigned char *) MALLOC(http_connection->buffer_size);
            } else {
                http_connection->buffer = (unsigned char *) REALLOC(
                    (void *) http_connection->buffer,
                    http_connection->buffer_size
                );
            }
        }
        memcpy(&http_connection->buffer[http_connection->buffer_offset], buffer, buffer_size);
        http_connection->buffer_offset += buffer_size;
    }

    /* consumes the preface, no frame at all is accepted before it
    and the connection is closed when it does not match */
    if(http2_connection->preface == FALSE) {
        if(http_connection->buffer_offset - http_connection->read_offset < HTTP2_PREFACE_SIZE) {
            RAISE_NO_ERROR;
        }
        if(memcmp(&http_connection->buffer[http_connection->read_offset], HTTP2_PREFACE, HTTP2_PREFACE_SIZE) != 0) {
            V_DEBUG("Invalid HTTP/2 connection preface\n");
            connection->close_connection(connection);
            RAISE_NO_ERROR;
        }
        http_connection->read_offset += HTTP2_PREFACE_SIZE;
        http2_connection->preface = TRUE;

        /* announces the settings of this end, which is what the
        specification requires as the first frame of a connection */
        _write_settings_http2_connection(http2_connection, FALSE);
    }

    /* handles one frame at a time until the buffer no longer holds a
    complete one, at which point more data has to be gathered */
    while(TRUE) {
        _read = &http_connection->buffer[http_connection->read_offset];
        _read_size = http_connection->buffer_offset - http_connection->read_offset;

        if(_read_size < HTTP2_HEADER_SIZE) { break; }

        return_value = decode_frame_http2(_read, _read_size, &http2_frame);
        if(IS_ERROR_CODE(return_value)) { break; }
        if(http2_frame.payload == NULL) { break; }

        http_connection->read_offset += HTTP2_HEADER_SIZE + http2_frame.length;

        return_value = handle_frame_http2_connection(http2_connection, &http2_frame);
        if(IS_ERROR_CODE(return_value)) {
            /* a frame that this end is unable to handle takes the
            complete connection down, the peer is told the reason */
            V_DEBUG_F("HTTP/2 connection error: %d\n", (int) return_value);
            write_goaway_http2_connection(http2_connection, (unsigned int) return_value);
            RAISE_NO_ERROR;
        }
    }

    /* moves the part of the buffer that has not been handled yet
    back to its start, a connection is long lived and the buffer
    would otherwise grow with every frame that arrives on it */
    pending = http_connection->buffer_offset - http_connection->read_offset;
    if(http_connection->read_offset > 0) {
        if(pending > 0) { memmove(http_connection->buffer, _read, pending); }
        http_connection->buffer_offset = pending;
        http_connection->read_offset = 0;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE upgrade_handler_stream_http2(struct io_connection_t *io_connection) {
    /* allocates space for the session that is going to take over
    the reading of the connection */
    struct http2_connection_t *http2_connection;

    /* retrieves the HTTP connection, it is the one the session
    hangs from and the one the handlers keep reaching */
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* creates the session and points the reading of the connection
    at the handler that drives the frames */
    create_http2_connection(&http2_connection, http_connection);
    io_connection->on_data = data_handler_stream_http2;

    /* raises no error */
    RAISE_NO_ERROR;
}
