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

#pragma once

#include "../http/http.h"
#include "../handlers/handlers.h"
#include "stream_io.h"

/* forward references (avoids loop) */
struct HttpConnection_t;

/**
 * Function used to update the given http connection
 * with new information.
 *
 * @param httpConnection The http connection to be
 * update with new information.
 */
typedef ERROR_CODE (*httpConnectionUpdate) (struct HttpConnection_t *httpConnection);

/**
 * The structure that describes the structure
 * of an handler capable of interpreting an
 * http request and construct a response based
 * on it.
 */
typedef struct HttpHandler_t {
    httpConnectionUpdate set;
    httpConnectionUpdate unset;
    httpConnectionUpdate reset;

    /**
     * Reference to the lower level
     * handler substrate (child).
     */
    void *lower;
} HttpHandler;

/**
 * Structure defining a locgical
 * http connection.
 */
typedef struct HttpConnection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct IoConnection_t *ioConnection;

    /**
     * Structure containig the settings to be
     * used by the http parser.
     */
    struct HttpSettings_t *httpSettings;

    /**
     * Parser to be used during the interpretation
     * of the http requests.
     */
    struct HttpParser_t *httpParser;

    /**
     * The handler currently being used to handle
     * the connection.
     * This handles may be from an external resource.
     * Security issues apply.
     */
    struct HttpHandler_t *httpHandler;
} HttpConnection;

ERROR_CODE createHttpHandler(struct HttpHandler_t **httpHandlerPointer);
ERROR_CODE deleteHttpHandler(struct HttpHandler_t *httpHandler);
ERROR_CODE createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *ioConnection);
ERROR_CODE deleteHttpConnection(struct HttpConnection_t *httpConnection);
ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize);
ERROR_CODE openHandlerStreamHttp(struct IoConnection_t *ioConnection);
ERROR_CODE closeHandlerStreamHttp(struct IoConnection_t *ioConnection);
