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

ERROR_CODE createHttpHandler(struct HttpHandler_t **httpHandlerPointer, unsigned char *name) {
    /* retrieves the http handler size */
    size_t httpHandlerSize = sizeof(struct HttpHandler_t);

    /* allocates space for the http handler */
    struct HttpHandler_t *httpHandler = (struct HttpHandler_t *) MALLOC(httpHandlerSize);

    /* sets the http handler attributes (default) values */
    httpHandler->name = name;
    httpHandler->set = NULL;
    httpHandler->unset = NULL;
    httpHandler->reset = NULL;

    /* sets the http handler in the http handler pointer */
    *httpHandlerPointer = httpHandler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpHandler(struct HttpHandler_t *httpHandler) {
    /* releases the http handler */
    FREE(httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *ioConnection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http connection size */
    size_t httpConnectionSize = sizeof(struct HttpConnection_t);

    /* allocates space for the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) MALLOC(httpConnectionSize);

    /* retrieves the service associated with the connection */
    struct Service_t *service = ioConnection->connection->service;

    /* sets the http handler attributes (default) values */
    httpConnection->ioConnection = ioConnection;
    httpConnection->httpHandler = NULL;
    httpConnection->buffer = NULL;
    httpConnection->bufferSize = 0;
    httpConnection->bufferOffset = 0;

    /* creates the http settings */
    createHttpSettings(&httpConnection->httpSettings);

    /* creates the http parser */
    createHttpParser(&httpConnection->httpParser);

    /* sets the connection as the parser parameter(s) */
    httpConnection->httpParser->parameters = ioConnection->connection;

    /* sets the http connection in the (upper) io connection substrate */
    ioConnection->lower = httpConnection;

    /* retrieves the current (default) service handler and sets the
    connection on it, then sets this handler as the base handler */
    httpHandler = service->httpHandler;
    httpConnection->baseHandler = httpHandler;

    /* sets the http connection in the http connection pointer */
    *httpConnectionPointer = httpConnection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpConnection(struct HttpConnection_t *httpConnection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *httpHandler;

    /* retrieves the currently assigned handler and usets the connection
    from with (unregister connection) */
    httpHandler = httpConnection->httpHandler;
    if(httpHandler) { httpHandler->unset(httpConnection); }

    /* in case the http connection buffer is defined must release it to
    avoid any memory leak */
    if(httpConnection->buffer) { FREE(httpConnection->buffer); }

    /* deletes the http parser */
    deleteHttpParser(httpConnection->httpParser);

    /* deletes the http settings */
    deleteHttpSettings(httpConnection->httpSettings);

    /* releases the http connection */
    FREE(httpConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize) {
    /* allocates space for the temporary variable to
    hold the ammount of bytes processed in a given http
    data parsing iteration */
    int processedSize;

    /* allocates the reference to the pointer to current
    buffer position to be parsed (initial position) */
    unsigned char *_buffer;

    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;

    /* iterates continuously, this allows the stream handler
    to split the stream into possible multiple messages, usefull
    for http pipelining (multiple sequenced requests) */
    while(1) {
        /* in case no http connection buffer is not currently set, time to start
        a new one from the provided buffer (fast access) */
        if(httpConnection->buffer == NULL) {
            httpConnection->bufferSize = bufferSize;
            httpConnection->buffer = (unsigned char *) MALLOC(httpConnection->bufferSize);
        }
        /* in case the http connection buffer is currently set but the available
        space is not enought must reallocate the buffer (increment size) */
        else if(httpConnection->bufferOffset + bufferSize > httpConnection->bufferSize) {
            if(httpConnection->httpParser->_contentLength > 0) {
                httpConnection->bufferSize += httpConnection->httpParser->_contentLength;
                httpConnection->buffer = REALLOC((void *) httpConnection->buffer, httpConnection->bufferSize);
            }
            else {
                httpConnection->bufferSize += bufferSize;
                httpConnection->buffer = REALLOC((void *) httpConnection->buffer, httpConnection->bufferSize);
            }
        }

        /* retrieves the pointer reference to the current position in the
        buffer to be used for writing then copies the current buffer data
        into it and updates the buffer size */
        _buffer = httpConnection->buffer + httpConnection->bufferOffset;
        memcpy(_buffer, buffer, bufferSize);
        httpConnection->bufferOffset += bufferSize;

        /* in case there's currently no http handler associated with the
        http connection (need to create one) */
        if(httpConnection->httpHandler == NULL) {
            /* retrieves the current connection's base handler and then sets
            it in the current connection then sets it as the current http handler */
            httpHandler = httpConnection->baseHandler;
            httpHandler->set(httpConnection);
            httpConnection->httpHandler = httpHandler;
        }

        /* process the http data for the http parser, this should be
        a partial processing and some data may remain unprocessed (in
        case there are multiple http requests) */
        processedSize = processDataHttpParser(httpConnection->httpParser, httpConnection->httpSettings, _buffer, bufferSize);

        /* in case the current state in the http parser is the
        start state, ther message is considered to be completly
        parsed (new message may come after) */
        if(httpConnection->httpParser->state == STATE_START_RES) {
            /* releases the current http connection buffer and then
            unsets the buffer from the connection and update the size
            of it to the initial empty value (buffer reset) */
            FREE(httpConnection->buffer);
            httpConnection->buffer = NULL;
            httpConnection->bufferSize = 0;
            httpConnection->bufferOffset = 0;

            /* resets the http parser state */
            httpConnection->httpParser->type = 2;
            httpConnection->httpParser->flags = 6;
            httpConnection->httpParser->state = STATE_START_REQ;
            httpConnection->httpParser->headerState = 0;
            httpConnection->httpParser->readCount = 0;
            httpConnection->httpParser->contentLength = -1;
            httpConnection->httpParser->httpMajor = 0;
            httpConnection->httpParser->httpMinor = 0;
            httpConnection->httpParser->statusCode = 0;
            httpConnection->httpParser->method = 0;
            httpConnection->httpParser->upgrade = 0;
            httpConnection->httpParser->_contentLength = 0;
        }

        /* in case all the remaining data has been processed
        no need to process more http data (breaks the loop) */
        if(processedSize > 0 && (size_t) processedSize == bufferSize) { break; }

        /* increments the buffer pointer and updates the buffer
        size value to the new size */
        buffer += processedSize;
        bufferSize -= processedSize;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE openHandlerStreamHttp(struct IoConnection_t *ioConnection) {
    /* allocates the http connection */
    struct HttpConnection_t *httpConnection;

    /* creates the http connection */
    createHttpConnection(&httpConnection, ioConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeHandlerStreamHttp(struct IoConnection_t *ioConnection) {
    /* retrieves the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;

    /* deletes the http connection */
    deleteHttpConnection(httpConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}
