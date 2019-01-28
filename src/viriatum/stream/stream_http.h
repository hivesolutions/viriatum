/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../http/http.h"
#include "../system/system.h"
#include "../handlers/handlers.h"
#include "stream_io.h"

/* forward references (avoids loop) */
struct http_connection_t;
typedef ERROR_CODE (*connection_data_callback_sh) (struct connection_t *, struct data_t *, void *);

/**
 * Function used to update the given HTTP connection
 * with new information.
 *
 * @param http_connection The HTTP connection to be
 * updated with new information.
 */
typedef ERROR_CODE (*http_connection_update) (struct http_connection_t *http_connection);

/**
 * Function used to populate a provided buffer with HTTP
 * headers information using the context of the provided
 * connection.
 */
typedef size_t (*http_connection_headers) (struct connection_t *, char *, size_t, enum http_version_e, int, char *, enum http_keep_alive_e, char);

/**
 * Function used to populate a provided buffer with HTTP
 * headers information using the context of the provided
 * connection.
 * This functions uses a more complete approach to the fill
 * of the HTTP headers.
 */
typedef size_t (*http_connection_headers_c) (struct connection_t *, char *, size_t, enum http_version_e, int, char *, enum http_keep_alive_e, size_t, enum http_cache_e, int);

/**
 * Function used to populate a provided buffer with HTTP
 * headers information using the context of the provided
 * connection.
 * This functions also populates the buffer with the message
 * part (contents) of the HTTP message.
 */
typedef size_t (*http_connection_headers_m) (struct connection_t *, char *, size_t, enum http_version_e, int, char *, enum http_keep_alive_e, size_t, enum http_cache_e, char *);

/**
 * Function used to send a message through the provided connection
 * using the predefined structured pipes.
 */
typedef ERROR_CODE (*http_connection_message) (struct connection_t *, char *, size_t, enum http_version_e, int, char *, char *, enum http_keep_alive_e, connection_data_callback_sh, void *);

/**
 * Function used to log request information in the common
 * log format into the current output stream.
 *
 * @param host The (client) host associated with the request
 * to be logged.
 * @param identity The identity of the "driver" that is logging
 * the current request (eg: file, dispatch, default, etc).
 * @param user The user that is logging the current, request
 * in case it's an automated script the name should be used.
 * @param method The uppercased name of the HTTP method used.
 * @param uri The path to the resource refered by the url.
 * @param version The version of the HTTP protocol used in
 * the request to be loggerd.
 * @param error_code The error (status) code to be sent back to
 * the client in the associated response.
 * @param content_length The size of the content to be sent back
 * as the response.
 */
typedef ERROR_CODE (*http_connection_log) (char *host, char *identity, char *user, char *method, char *uri, enum http_version_e version, int error_code, size_t content_length);

/**
 * The structure that describes the structure
 * of an handler capable of interpreting an
 * HTTP request and construct a response based
 * on it.
 */
typedef struct http_handler_t {
    /**
     * The name that describes the HTTP
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
     * connection object. This operation should
     * be similar to the set and then unset.
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
 * HTTP connection.
 */
typedef struct http_connection_t {
    /**
     * The (upper) io connection that owns
     * and manages this connection.
     */
    struct io_connection_t *io_connection;

    /**
     * Structure containing the settings to be
     * used by the HTTP parser.
     */
    struct http_settings_t *http_settings;

    /**
     * Parser to be used during the interpretation
     * of the HTTP requests. The state of this parser
     * is variable and should not be trusted.
     */
    struct http_parser_t *http_parser;

    /**
     * The handler currently being used to handle
     * the connection, may change over the connection
     * life-time and should not be trusted as static.
     * This handles may be from an external resource.
     * Security issues apply.
     * This is an internal value and must be used
     * with care or else unexpected behavior may occur.
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
     * HTTP stream in processing.
     */
    unsigned char *buffer;

    /**
     * The current size of the buffer containing the HTTP
     * data in processing.
     * This value is used to retrieve the current position
     * to be written into the buffer.
     */
    size_t buffer_size;

    /**
     * The current offset of the buffer containing the HTTP
     * data in processing. This value may be used to determine
     * the final (valid part) of the current buffer that contains
     * data (the rest would be empty buffer).
     */
    size_t buffer_offset;

    /**
     * The current offset to the already read (processed) data from
     * the current buffer. This determined the next bytes for
     * processing in the buffer.
     */
    size_t read_offset;

    /**
     * The lock flag that controls the data stream flood, this
     * way only one message is processed at a given time, even
     * using HTTP pipeling.
     */
    unsigned char lock;

    /**
     * Function used to create and write header into a sent
     * buffer of data.
     */
    http_connection_headers write_headers;

    /**
     * Function used to create and write header into a sent
     * buffer of data.
     * This function writes an extended version of the heders.
     */
    http_connection_headers_c write_headers_c;

    /**
     * Function used to create and write header into a sent
     * buffer of data.
     * This function should also write the message into the
     * provided buffer.
     */
    http_connection_headers_m write_headers_m;

    /**
     * Function to be used for writing of message is a simplistic
     * but inefficient way.
     * This function is usefull for debugging purposes.
     */
    http_connection_message write_message;

    /**
     * Function to be used for the writing of the errors
     * in the connection context using the "normal" pipe.
     */
    http_connection_message write_error;

    /**
     * Function to be used for logging a connection into the
     * appropriate output streams in the default common log
     * format.
     */
    http_connection_log log_request;

    /**
     * Function to be used to acquire the lock on the processing
     * of the data in the connection, this lock avoid that two
     * different requests write to the same connection in parallel.
     */
    http_connection_update acquire;

    /**
     * Function to be used to release the lock on the processing
     * of the data in the connection.
     */
    http_connection_update release;
} http_connection;

ERROR_CODE create_http_handler(struct http_handler_t **http_handler_pointer, unsigned char *name);
ERROR_CODE delete_http_handler(struct http_handler_t *http_handler);
ERROR_CODE create_http_connection(struct http_connection_t **http_connection_pointer, struct io_connection_t *io_connection);
ERROR_CODE delete_http_connection(struct http_connection_t *http_connection);
ERROR_CODE acquire_http_connection(struct http_connection_t *http_connection);
ERROR_CODE release_http_connection(struct http_connection_t *http_connection);
ERROR_CODE data_handler_stream_http(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE open_handler_stream_http(struct io_connection_t *io_connection);
ERROR_CODE close_handler_stream_http(struct io_connection_t *io_connection);
