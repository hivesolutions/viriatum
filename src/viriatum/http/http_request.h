/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2018 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#define ETAG_H "ETag"
#define HOST_H "Host"
#define COOKIE_H "Cookie"
#define SERVER_H "Server"
#define LOCATION_H "Location"
#define LOCATION_H "Location"
#define CONNECTION_H "Connection"
#define CONTENT_TYPE_H "Content-Type"
#define ACCEPT_RANGES_H "Accept-Ranges"
#define CONTENT_RANGE_H "Content-Range"
#define CACHE_CONTROL_H "Cache-Control"
#define CONTENT_LENGTH_H "Content-Length"
#define WWW_AUTHENTICATE_H "WWW-Authenticate"

/**
 * The array defining the various strings indicating
 * the respective HTTP protocol version according to
 * the pre-defined enumeration.
 */
static const char *http_version_strings[3] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1"
};

/**
 * Enumeration defining the various possible
 * and "recognizable" HTTP header types, this
 * may be used to provided static reference.
 */
typedef enum http_header_e {
    UNDEFINED_HEADER = 1,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    COOKIE,
    HOST,
    ETAG,
    CACHE_CONTROL,
    AUTHORIZATION,
    RANGE
} http_header;

/**
 * Enumeration defining the various possible
 * and "recognizable" protocol versions for
 * the HTTP protocol.
 */
typedef enum http_version_e {
    HTTP09 = 1,
    HTTP10,
    HTTP11
} http_version;

/**
 * Enumeration defining the various possible
 * methods for an HTTP request.
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
 * HTTP cache control strategies.
 */
typedef enum http_cache_e {
    NO_CACHE = 1,
    MAX_AGE
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
 * Constructor of the HTTP request.
 *
 * @param http_request_pointer The pointer to the HTTP request to be constructed.
 */
void create_http_request(struct http_request_t **http_request_pointer);

/**
 * Destructor of the HTTP request.
 *
 * @param http_request The HTTP request to be destroyed.
 */
void delete_http_request(struct http_request_t *http_request);

/**
 * Retrieves the string representing the HTTP version for
 * the given HTTP version integer represented in the
 * enumeration.
 *
 * @param http_method The HTTP version integer to be "converted"
 * into string representation.
 * @return The string representation of the HTTP version.
 */
static __inline const char *get_http_version_string(enum http_version_e http_version) {
    return http_version_strings[http_version - 1];
}

/**
 * Convers the provided major and minor based version set
 * of integer values into a more standard representation
 * of the HTTP version values using enumerations.
 *
 * @param http_major The major version value for the HTTP
 * protocol to be used in the enumeration conversion.
 * @param http_minor The minor version value for the htto
 * protocol  to be used in the enumeration conversion.
 * @return The standard enumeration oriented version of the
 * HTTP version value converted accordingly.
 */
static __inline enum http_version_e get_http_version(unsigned short http_major, unsigned short http_minor) {
    switch(http_major) {
        case 0:
            switch(http_minor) {
                case 9:
                    return HTTP09;
            }

            break;

        case 1:
            switch(http_minor) {
                case 0:
                    return HTTP10;

                case 1:
                    return HTTP11;
            }

            break;
    }

    return HTTP11;
}
