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
#include "../system/system.h"
#include "../handlers/handlers.h"
#include "stream_io.h"

/* forward references (avoids loop) */
struct HttpConnection_t;

/**
 * Function used to update the given http connection
 * with new information.
 *
 * @param http_connection The http connection to be
 * update with new information.
 */
typedef ERROR_CODE (*httpConnectionUpdate) (struct HttpConnection_t *http_connection);

/**
 * The structure that describes the structure
 * of an handler capable of interpreting an
 * http request and construct a response based
 * on it.
 */
typedef struct HttpHandler_t {
    /**
     * The name that describes the http
     * handler.
     * This is going to be used as id and
     * so it hsould be unique to the system.
     */
    unsigned char *name;

    httpConnectionUpdate set;
    httpConnectionUpdate unset;
    httpConnectionUpdate reset;

    /**
     * The reference to the service that "owns"
     * the current handler.
     * The creation of the handler implies the
     * registration of it in this service.
     */
    struct Service_t *service;

    /**
     * Reference to the lower level
     * handler substrate (child).
     */
    void *lower;
} HttpHandler;

/**
 * Structure defining a logical
 * http connection.
 */
typedef struct HttpConnection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct IoConnection_t *io_connection;

    /**
     * Structure containig the settings to be
     * used by the http parser.
     */
    struct http_settings_t *http_settings;

    /**
     * Parser to be used during the interpretation
     * of the http requests.
     */
    struct http_parser_t *http_parser;

    /**
     * The handler currently being used to handle
     * the connection.
     * This handles may be from an external resource.
     * Security issues apply.
     * This is an internal value and must be used
     * with care.
     */
    struct HttpHandler_t *http_handler;

    /**
     * The base handler for the connection every new
     * request entering this connection will be handled
     * initialy by this handler.
     */
    struct HttpHandler_t *baseHandler;

    /**
     * The current contiguous buffer for the complete
     * http stream in processing.
     */
    unsigned char *buffer;

    /**
     * The current size of the buffer containing the http
     * data in processing.
     * This value is used to retrieve the current position
     * to be written into the buffer.
     */
    size_t buffer_size;

    /**
     * The current offset of the buffer containing the http
     * data in processing.
     */
    size_t bufferOffset;
} HttpConnection;

ERROR_CODE create_http_handler(struct HttpHandler_t **httpHandlerPointer, unsigned char *name);
ERROR_CODE delete_http_handler(struct HttpHandler_t *http_handler);
ERROR_CODE createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *io_connection);
ERROR_CODE deleteHttpConnection(struct HttpConnection_t *http_connection);
ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE openHandlerStreamHttp(struct IoConnection_t *io_connection);
ERROR_CODE closeHandlerStreamHttp(struct IoConnection_t *io_connection);
