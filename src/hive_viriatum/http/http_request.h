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

#define ETAG_H "ETag"
#define HOST_H "Host"
#define SERVER_H "Server"
#define LOCATION_H "Location"
#define CONNECTION_H "Connection"
#define CACHE_CONTROL_H "Cache-Control"
#define CONTENT_LENGTH_H "Content-Length"

/**
 * Enumeration defining the various possible
 * and "recognizable" http header types, this
 * may be used to provided static reference.
 */
typedef enum http_header_e {
    UNDEFINED_HEADER = 1,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    COOKIE,
    HOST
} http_header;

/**
 * Enumeration defining the various possible
 * and "recognizable" protocol versions for
 * the http protocol.
 */
typedef enum http_version_e {
    HTTP09 = 1,
    HTTP10,
    HTTP11
} http_version;

/**
 * Enumeration defining the various possible
 * methods for an http request.
 */
typedef enum http_request_method_e {
    GET_REQUEST_METHOD = 1,
    POST_REQUEST_METHOD
} http_request_method;

/**
 * Enumeration defining the various possible
 * keep alive situations (modes).
 */
typedef enum http_keep_alive_e {
    KEEP_CLOSE = 1,
    KEEP_ALIVE
} http_keep_alive;

/**
 * Enumeration defining the various possible
 * http cache control strategies.
 */
typedef enum http_cache_e {
    NO_CACHE = 1
} http_cache;

typedef struct http_header_value_t {
    char name[VIRIATUM_MAX_HEADER_SIZE];
    char value[VIRIATUM_MAX_HEADER_V_SIZE];
    size_t name_size;
    size_t value_size;
} http_header_value;

typedef struct http_headers_t {
    struct http_header_value_t values[VIRIATUM_MAX_HEADER_COUNT];
    size_t count;
} http_headers_value;

typedef struct http_request_t {
    enum http_request_method_e method;
    unsigned char *data;
} http_request;

/**
 * Constructor of the http request.
 *
 * @param http_request_pointer The pointer to the http request to be constructed.
 */
void create_http_request(struct http_request_t **http_request_pointer);

/**
 * Destructor of the http request.
 *
 * @param http_request The http request to be destroyed.
 */
void delete_http_request(struct http_request_t *http_request);
