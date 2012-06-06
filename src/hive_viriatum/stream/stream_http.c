/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "stream_http.h"

ERROR_CODE create_http_handler(struct HttpHandler_t **httpHandlerPointer, unsigned char *name) {
    /* retrieves the http handler size */
    size_t httpHandlerSize = sizeof(struct HttpHandler_t);

    /* allocates space for the http handler */
    struct HttpHandler_t *http_handler = (struct HttpHandler_t *) MALLOC(httpHandlerSize);

    /* sets the http handler attributes (default) values */
    http_handler->name = name;
    http_handler->set = NULL;
    http_handler->unset = NULL;
    http_handler->reset = NULL;

    /* sets the http handler in the http handler pointer */
    *httpHandlerPointer = http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http_handler(struct HttpHandler_t *http_handler) {
    /* releases the http handler */
    FREE(http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *io_connection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *http_handler;

    /* retrieves the http connection size */
    size_t httpConnectionSize = sizeof(struct HttpConnection_t);

    /* allocates space for the http connection */
    struct HttpConnection_t *http_connection = (struct HttpConnection_t *) MALLOC(httpConnectionSize);

    /* retrieves the service associated with the connection */
    struct Service_t *service = io_connection->connection->service;

    /* sets the http handler attributes (default) values */
    http_connection->io_connection = io_connection;
    http_connection->http_handler = NULL;
    http_connection->buffer = NULL;
    http_connection->buffer_size = 0;
    http_connection->bufferOffset = 0;

    /* creates the http settings */
    createHttpSettings(&http_connection->http_settings);

    /* creates the http parser (for a request) */
    createHttpParser(&http_connection->http_parser, 1);

    /* sets the connection as the parser parameter(s) */
    http_connection->http_parser->parameters = io_connection->connection;

    /* sets the http connection in the (upper) io connection substrate */
    io_connection->lower = http_connection;

    /* retrieves the current (default) service handler and sets the
    connection on it, then sets this handler as the base handler */
    http_handler = service->http_handler;
    http_connection->baseHandler = http_handler;

    /* sets the http connection in the http connection pointer */
    *httpConnectionPointer = http_connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpConnection(struct HttpConnection_t *http_connection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *http_handler;

    /* retrieves the currently assigned handler and usets the connection
    from with (unregister connection) */
    http_handler = http_connection->http_handler;
    if(http_handler) { http_handler->unset(http_connection); }

    /* in case the http connection buffer is defined must release it to
    avoid any memory leak */
    if(http_connection->buffer) { FREE(http_connection->buffer); }

    /* deletes the http parser */
    deleteHttpParser(http_connection->http_parser);

    /* deletes the http settings */
    deleteHttpSettings(http_connection->http_settings);

    /* releases the http connection */
    FREE(http_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the temporary variable to
    hold the ammount of bytes processed in a given http
    data parsing iteration */
    int processedSize;

    /* allocates the reference to the pointer to current
    buffer position to be parsed (initial position) */
    unsigned char *_buffer;

    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *http_handler;

    /* retrieves the http connection */
    struct HttpConnection_t *http_connection = (struct HttpConnection_t *) io_connection->lower;

    /* iterates continuously, this allows the stream handler
    to split the stream into possible multiple messages, usefull
    for http pipelining (multiple sequenced requests) */
    while(1) {
        /* in case no http connection buffer is not currently set, time to start
        a new one from the provided buffer (fast access) */
        if(http_connection->buffer == NULL) {
            http_connection->buffer_size = buffer_size;
            http_connection->buffer = (unsigned char *) MALLOC(http_connection->buffer_size);
        }
        /* in case the http connection buffer is currently set but the available
        space is not enough must reallocate the buffer (increment size) */
        else if(http_connection->bufferOffset + buffer_size > http_connection->buffer_size) {
            if(http_connection->http_parser->_contentLength > 0) {
                http_connection->buffer_size += http_connection->http_parser->_contentLength;
                http_connection->buffer = REALLOC((void *) http_connection->buffer, http_connection->buffer_size);
            }
            else {
                http_connection->buffer_size += buffer_size;
                http_connection->buffer = REALLOC((void *) http_connection->buffer, http_connection->buffer_size);
            }
        }

        /* retrieves the pointer reference to the current position in the
        buffer to be used for writing then copies the current buffer data
        into it and updates the buffer size */
        _buffer = http_connection->buffer + http_connection->bufferOffset;
        memcpy(_buffer, buffer, buffer_size);
        http_connection->bufferOffset += buffer_size;

        /* in case there's currently no http handler associated with the
        http connection (need to create one) */
        if(http_connection->http_handler == NULL) {
            /* retrieves the current connection's base handler and then sets
            it in the current connection then sets it as the current http handler */
            http_handler = http_connection->baseHandler;
            http_handler->set(http_connection);
            http_connection->http_handler = http_handler;
        }

        /* process the http data for the http parser, this should be
        a partial processing and some data may remain unprocessed (in
        case there are multiple http requests) */
        processedSize = processDataHttpParser(http_connection->http_parser, http_connection->http_settings, _buffer, buffer_size);

        /* in case the current state in the http parser is the
        start state, ther message is considered to be completly
        parsed (new message may come after) */
        if(http_connection->http_parser->state == STATE_START_RES) {
            /* releases the current http connection buffer and then
            unsets the buffer from the connection and update the size
            of it to the initial empty value (buffer reset) */
            FREE(http_connection->buffer);
            http_connection->buffer = NULL;
            http_connection->buffer_size = 0;
            http_connection->bufferOffset = 0;

            /* resets the http parser state */
            http_connection->http_parser->type = 2;
            http_connection->http_parser->flags = 6;
            http_connection->http_parser->state = STATE_START_REQ;
            http_connection->http_parser->headerState = 0;
            http_connection->http_parser->read_count = 0;
            http_connection->http_parser->contentLength = -1;
            http_connection->http_parser->httpMajor = 0;
            http_connection->http_parser->httpMinor = 0;
            http_connection->http_parser->statusCode = 0;
            http_connection->http_parser->method = 0;
            http_connection->http_parser->upgrade = 0;
            http_connection->http_parser->_contentLength = 0;
        }

        /* in case all the remaining data has been processed
        no need to process more http data (breaks the loop) */
        if(processedSize > 0 && (size_t) processedSize == buffer_size) { break; }

        /* increments the buffer pointer and updates the buffer
        size value to the new size */
        buffer += processedSize;
        buffer_size -= processedSize;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE openHandlerStreamHttp(struct IoConnection_t *io_connection) {
    /* allocates the http connection */
    struct HttpConnection_t *http_connection;

    /* creates the http connection */
    createHttpConnection(&http_connection, io_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeHandlerStreamHttp(struct IoConnection_t *io_connection) {
    /* retrieves the http connection */
    struct HttpConnection_t *http_connection = (struct HttpConnection_t *) io_connection->lower;

    /* deletes the http connection */
    deleteHttpConnection(http_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
