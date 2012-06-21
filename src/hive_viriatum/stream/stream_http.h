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
struct http_connection_t;

/**
 * Function used to update the given http connection
 * with new information.
 *
 * @param http_connection The http connection to be
 * update with new information.
 */
typedef ERROR_CODE (*http_connection_update) (struct http_connection_t *http_connection);

/**
 * Function used to log request information in the common
 * log format into the current output stream.
 *
 * @param host The (client) host associated with the request
 * to be logged.
 * @param identity The identity of the "driver" that is logging
 * the current request.
 * @param user The user that is logging the current, request
 * in case it's an automated script the name should be used.
 * @param method The uppercased name of the http method used.
 * @param uri The path to the resource refered by the url.
 * @param protocol The version of the http protocol used in
 * the request to be loggerd.
 * @param error_code The error (status) code to be sent back to
 * the client in the associated response.
 * @param content_length The size of the content to be sent back
 * as the response.
 */
typedef ERROR_CODE (*http_connection_log) (char *host, char *identity, char *user, char *method, char *uri, char *protocol, int error_code, size_t content_length);

/**
 * The structure that describes the structure
 * of an handler capable of interpreting an
 * http request and construct a response based
 * on it.
 */
typedef struct http_handler_t {
    /**
     * The name that describes the http
     * handler.
     * This is going to be used as id and
     * so it hsould be unique to the system.
     */
    unsigned char *name;

    /**
     * If the index path should be automatically
     * "resolved" by external handlers or any other
     * associated object.
     */
    char resolve_index;

    /**
     * Callback method called uppon setting
     * the current handler in the provided
     * connection object.
     */
    http_connection_update set;

    /**
     * Callback method called uppon unsetting
     * the current handler from the provided
     * connection object.
     */
    http_connection_update unset;

    /**
     * Callback method called uppon resetting
     * the current handler from the provided
     * connection object.
     */
    http_connection_update reset;

    /**
     * The reference to the service that "owns"
     * the current handler.
     * The creation of the handler implies the
     * registration of it in this service.
     */
    struct service_t *service;

    /**
     * Reference to the lower level
     * handler substrate (child).
     */
    void *lower;
} http_handler;

/**
 * Structure defining a logical
 * http connection.
 */
typedef struct http_connection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct io_connection_t *io_connection;

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
    struct http_handler_t *http_handler;

    /**
     * The base handler for the connection every new
     * request entering this connection will be handled
     * initialy by this handler.
     */
    struct http_handler_t *base_handler;

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
    size_t buffer_offset;

    /**
     * Function to be used for logging a connection into the
     * appropriate output streams in the default common log
     * format.
     */
    http_connection_log log_request;
} http_connection;

ERROR_CODE create_http_handler(struct http_handler_t **http_handler_pointer, unsigned char *name);
ERROR_CODE delete_http_handler(struct http_handler_t *http_handler);
ERROR_CODE create_http_connection(struct http_connection_t **http_connection_pointer, struct io_connection_t *io_connection);
ERROR_CODE delete_http_connection(struct http_connection_t *http_connection);
ERROR_CODE data_handler_stream_http(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE open_handler_stream_http(struct io_connection_t *io_connection);
ERROR_CODE close_handler_stream_http(struct io_connection_t *io_connection);
