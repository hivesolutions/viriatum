/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2014 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2014 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "stream_http.h"

ERROR_CODE create_http_handler(struct http_handler_t **http_handler_pointer, unsigned char *name) {
    /* retrieves the http handler size */
    size_t http_handler_size = sizeof(struct http_handler_t);

    /* allocates space for the http handler */
    struct http_handler_t *http_handler = (struct http_handler_t *) MALLOC(http_handler_size);

    /* sets the http handler attributes (default) values */
    http_handler->name = name;
    http_handler->resolve_index = FALSE;
    http_handler->set = NULL;
    http_handler->unset = NULL;
    http_handler->reset = NULL;

    /* sets the http handler in the http handler pointer */
    *http_handler_pointer = http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http_handler(struct http_handler_t *http_handler) {
    /* releases the http handler */
    FREE(http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_http_connection(struct http_connection_t **http_connection_pointer, struct io_connection_t *io_connection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct http_handler_t *http_handler;

    /* retrieves the http connection size */
    size_t http_connection_size = sizeof(struct http_connection_t);

    /* allocates space for the http connection */
    struct http_connection_t *http_connection = (struct http_connection_t *) MALLOC(http_connection_size);

    /* retrieves the service associated with the connection */
    struct service_t *service = io_connection->connection->service;

    /* sets the http handler attributes (default) values */
    http_connection->io_connection = io_connection;
    http_connection->http_handler = NULL;
    http_connection->buffer = NULL;
    http_connection->buffer_size = 0;
    http_connection->buffer_offset = 0;
    http_connection->read_offset = 0;
    http_connection->lock = FALSE;

    /* sets the proper functions in the http connection
    they may be used by the handlers to interact with
    the connection (at the http level) */
    http_connection->write_headers = write_http_headers;
    http_connection->write_headers_c = write_http_headers_c;
    http_connection->write_headers_m = write_http_headers_m;
    http_connection->write_message = write_http_message;
    http_connection->write_error = write_http_error;
    http_connection->log_request = log_http_request;
    http_connection->acquire = acquire_http_connection;
    http_connection->release = release_http_connection;

    /* creates the http settings */
    create_http_settings(&http_connection->http_settings);

    /* creates the http parser (for a request) */
    create_http_parser(&http_connection->http_parser, 1);

    /* sets the connection as the parser parameter(s) */
    http_connection->http_parser->parameters = io_connection->connection;

    /* sets the http connection in the (upper) io connection substrate */
    io_connection->lower = http_connection;

    /* retrieves the current (default) service handler and sets the
    connection on it, then sets this handler as the base handler */
    http_handler = service->http_handler;
    http_connection->base_handler = http_handler;

    /* sets the http connection in the http connection pointer */
    *http_connection_pointer = http_connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http_connection(struct http_connection_t *http_connection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct http_handler_t *http_handler;

    /* retrieves the currently assigned handler and then usets
    the connection from associated handler (unregister connection) */
    http_handler = http_connection->http_handler;
    if(http_handler) { http_handler->unset(http_connection); }

    /* in case the http connection buffer is defined must release it to
    avoid any memory leak */
    if(http_connection->buffer) { FREE(http_connection->buffer); }

    /* deletes the http parser */
    delete_http_parser(http_connection->http_parser);

    /* deletes the http settings */
    delete_http_settings(http_connection->http_settings);

    /* releases the http connection */
    FREE(http_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE acquire_http_connection(struct http_connection_t *http_connection) {
    /* sets the http connection lock flag, any further
    message processing should be blocked until this
    value is released */
    http_connection->lock = TRUE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE release_http_connection(struct http_connection_t *http_connection) {
    /* retrieves the parent io connection from the http
    connection to be used for event triggering */
    struct io_connection_t *io_connection = http_connection->io_connection;

    /* retieves the current locked state in order to make
    assumptions if pending should be processed again in this
    event loop, then unsests the lock flag */
    unsigned char locked = http_connection->lock;
    http_connection->lock = FALSE;
    if(locked) { CALL_V(http_connection->io_connection->on_data, io_connection, NULL, (size_t) 0); }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE data_handler_stream_http(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the temporary variable to
    hold the ammount of bytes processed in a given http
    data parsing iteration */
    int processed_size;

    /* allocates the space to be used for the the calculus
    arround the new buffer sizes, both for the offset and the
    content length strategies */
    size_t size_offset;
    size_t size_length;

    /* allocates the reference to the pointer to current
    buffer position to be used in writing (initial position) */
    unsigned char *_buffer;

    /* allocates the reference to the pointer to the position
    in the connection buffer to continue the parsing */
    unsigned char *_read;
    size_t _read_size;

    /* allocates space for the http handler reference
    to be used in this connection */
    struct http_handler_t *http_handler;

    /* retrieves the references to both the connection (upper)
    and the http connection (lower) data structures */
    struct connection_t *connection = io_connection->connection;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* retrieves the content length value from the parser associated
    with the current connection, this value is maitained accross multiple
    parsing loops and is not considered transient, the value remains at the
    initial zero value until the proper header is parsed */
    size_t content_length = http_connection->http_parser->_content_length;

    /* in case no http connection buffer is currently set, time to start
    a new one from the provided buffer (fast access) */
    if(http_connection->buffer == NULL) {
        http_connection->buffer_size = buffer_size;
        http_connection->buffer = (unsigned char *) MALLOC(http_connection->buffer_size);
    }
    /* in case the http connection buffer is currently set but the available
    space is not enough must reallocate the buffer (increment size) in order
    to hold the required space for the new contents */
    else if(http_connection->buffer_offset + buffer_size > http_connection->buffer_size) {
        /* creates two strategies for the reallocation of the buffer, one based on the
        minimal size required for the handling the memory copy (offset strategy) and
        another that aims at pre-allocating size for the complete message buffer (content
        length strategy) the maximum value of both is selected as the new buffer size
        and is used for the reallocation of the buffer */
        size_offset = http_connection->buffer_offset + buffer_size;
        size_length = http_connection->buffer_size + content_length;
        http_connection->buffer_size = size_length > size_offset ? size_length : size_offset;
        http_connection->buffer = REALLOC(
            (void *) http_connection->buffer,
            http_connection->buffer_size
        );
    }

    /* retrieves the pointer reference to the current position in the
    buffer to be used for writing then copies the current buffer data
    into it and updates the buffer size */
    _buffer = http_connection->buffer + http_connection->buffer_offset;
    memcpy(_buffer, buffer, buffer_size);
    http_connection->buffer_offset += buffer_size;

    /* iterates continuously, this allows the stream handler
    to split the stream into possible multiple messages, usefull
    for http pipelining (multiple sequenced requests) */
    while(TRUE) {
        /* retrieves the pointer to the read position as the connection buffer
        added to the read offset and then retrieves the current size for reading
        by removing the read offset from the buffer offset (must deduct the already
        processed part of the buffer from the buffer offset) */
        _read = http_connection->buffer + http_connection->read_offset;
        _read_size = http_connection->buffer_offset - http_connection->read_offset;

        /* in case the http connection is currently locked
        breaks immediately to avoid multiple simultaneously
        processing of messages in the same connection */
        if(http_connection->lock) { break; }

        /* in case there's currently no http handler associated with the
        http connection (need to create one) */
        if(http_connection->http_handler == NULL) {
            /* retrieves the current connection's base handler and then sets
            it in the current connection then sets it as the current http handler */
            http_handler = http_connection->base_handler;
            http_handler->set(http_connection);
            http_connection->http_handler = http_handler;
        }

        /* process the http data for the http parser, this should be
        a partial processing and some data may remain unprocessed (in
        case there are multiple http requests) */
        processed_size = process_data_http_parser(
            http_connection->http_parser,
            http_connection->http_settings,
            _read,
            _read_size
        );

        /* in case the processed size value is set to an invalid value
        an error has occurred and proper handling must occur */
        if(processed_size == -1) {
            /* closes the connection, this is the default response
            to an error in the parsing (fallback) */
            connection->close_connection(connection);
            break;
        }

        /* increments the (buffer) read offset with the processed
        size so that the next process starts at the read offset */
        http_connection->read_offset += processed_size;

        /* in case the current state in the http parser is the
        start state, the message is considered to be completly
        parsed (new message may come after) */
        if(http_connection->http_parser->state == STATE_START_RES) {
            /* resets the http parser state */
            http_connection->http_parser->type = 2;
            http_connection->http_parser->flags = 6;
            http_connection->http_parser->state = STATE_START_REQ;
            http_connection->http_parser->header_state = 0;
            http_connection->http_parser->read_count = 0;
            http_connection->http_parser->content_length = -1;
            http_connection->http_parser->http_major = 0;
            http_connection->http_parser->http_minor = 0;
            http_connection->http_parser->status_code = 0;
            http_connection->http_parser->method = 0;
            http_connection->http_parser->upgrade = 0;
            http_connection->http_parser->_content_length = 0;

            /* resets the internal parser state values */
            http_connection->http_parser->header_field_mark = 0;
            http_connection->http_parser->header_value_mark = 0;
            http_connection->http_parser->url_mark = 0;

            /* in case the current http connection read offset has reached
            the buffer size it's time to reset the buffer (no more data to
            be processed), the buffer size should come from the content length
            value of the actual message */
            if(http_connection->read_offset == http_connection->buffer_size) {
                /* releases the current http connection buffer and then
                unsets the buffer from the connection and updates the size
                of it to the initial empty value (buffer reset) */
                FREE(http_connection->buffer);
                http_connection->buffer = NULL;
                http_connection->buffer_size = 0;
                http_connection->buffer_offset = 0;
                http_connection->read_offset = 0;
            }
        }

        /* in case all the data has been read from the connection
        buffer must break the loop */
        if(http_connection->read_offset == http_connection->buffer_size) { break; }

        /* in case all the remaining data has been processed
        no need to process more http data (breaks the loop) */
        if(processed_size == 0 || (size_t) processed_size == _read_size) { break; }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_handler_stream_http(struct io_connection_t *io_connection) {
    /* allocates the http connection */
    struct http_connection_t *http_connection;

    /* creates the http connection */
    create_http_connection(&http_connection, io_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_handler_stream_http(struct io_connection_t *io_connection) {
    /* retrieves the http connection */
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* deletes the http connection */
    delete_http_connection(http_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
