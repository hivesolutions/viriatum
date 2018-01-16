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

#include "../system/service.h"

#define GET_HTTP_STATUS(code) http_status_codes[(code / 100) - 1][code % 100]

/* forward references (avoids loop) */
struct data_t;
struct service_t;
struct connection_t;
typedef ERROR_CODE (*connection_data_callback_hu) (struct connection_t *, struct data_t *, void *);

/**
 * The default authentication string to be used
 * when the default http mode is requested.
 */
#define DEFAULT_AUTH_HTTP "Basic cm9vdDpyb290"

/**
 * The string representations for the various
 * versions of the http protocol.
 * These values are useful for usage in the
 * status line printing part of the http message.
 */
static const char *http_version_codes[3] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1"
};

/**
 * The buffer containing sequences of the
 * various keep alive values.
 */
static const char *keep_alive_codes[2] = {
    "close",
    "keep-alive"
};

/**
 * The various values for the cache control
 * system values.
 */
static const char *cache_codes[2] = {
    "no-cache, must-revalidate",
    "max-age=604800"
};

/**
 * The buffer containing the various codes
 * for possible header buffer closing.
 * This values consider a boolean index as
 * the driver index for printing.
 */
static const char *close_codes[2] = {
    "",
    "\r\n"
};

/**
 * The buffer containing sequences of the
 * descriptions to the various http error
 * codes orderes by major and minor parts
 * of the error code.
 */
static const char *http_status_codes[5][64] = {
    {
        "Continue",
        "Switching Protocols",
        "Processing"
    },
    {
        "OK",
        "Created",
        "Accepted",
        "Non-Authoritative Information (since HTTP/1.1)",
        "No Content",
        "Reset Content",
        "Partial Content",
        "Multi-Status",
        "Already Reported",
        "IM Used"
    },
    {
        "Multiple Choices",
        "Moved Permanently",
        "Found",
        "See Other",
        "Not Modified",
        "Use Proxy",
        "Switch Proxy",
        "Temporary Redirect",
        "Permanent Redirect"
    },
    {
        "Bad Request",
        "Unauthorized",
        "Payment Required",
        "Forbidden",
        "Not Found",
        "Method Not Allowed",
        "Not Acceptable",
        "Proxy Authentication Required",
        "Request Timeout",
        "Conflict",
        "Gone",
        "Length Required"
        "Precondition Failed",
        "Request Entity Too Large",
        "Request-URI Too Long",
        "Unsupported Media Type",
        "Requested Range Not Satisfiable",
        "Expectation Failed",
        "I'm a teapot"
    },
    {
        "Internal Server Error",
        "Not Implemented",
        "Bad Gateway",
        "Service Unavailable",
        "Gateway Timeout",
        "HTTP Version Not Supported",
        "Variant Also Negotiates",
        "Insufficient Storage",
        "Loop Detected",
        "Bandwidth Limit Exceeded",
        "Not Extended",
        "Network Authentication Required"
    }
};

size_t write_http_headers(
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int status_code,
    char *status_message,
    enum http_keep_alive_e keep_alive,
    char close
);
size_t write_http_headers_c(
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int status_code,
    char *status_message,
    enum http_keep_alive_e keep_alive,
    size_t content_length,
    enum http_cache_e cache,
    int close
);
size_t write_http_headers_a(
    struct connection_t *connection,
    char *buffer, size_t size,
    enum http_version_e version,
    int status_code,
    char *status_message,
    enum http_keep_alive_e keep_alive,
    size_t content_length,
    enum http_cache_e cache,
    char *realm,
    int close
);
size_t write_http_headers_m(
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int status_code,
    char *status_message,
    enum http_keep_alive_e keep_alive,
    size_t content_length,
    enum http_cache_e cache,
    char *message
);
ERROR_CODE write_http_message(
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int status_code,
    char *status_message,
    char *message,
    enum http_keep_alive_e keep_alive,
    connection_data_callback_hu callback,
    void *callback_parameters
);
ERROR_CODE write_http_error(
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int error_code,
    char *error_message,
    char *error_description,
    enum http_keep_alive_e keep_alive,
    connection_data_callback_hu callback,
    void *callback_parameters
);
ERROR_CODE write_http_error_a(
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int error_code,
    char *error_message,
    char *error_description,
    char *realm,
    enum http_keep_alive_e keep_alive,
    connection_data_callback_hu callback,
    void *callback_parameters
);
ERROR_CODE get_http_range_limits(unsigned char *range, size_t *initial_byte, size_t *final_byte, size_t size);
ERROR_CODE log_http_request(char *host, char *identity, char *user, char *method, char *uri, enum http_version_e version, int error_code, size_t content_length);
ERROR_CODE auth_http(char *auth_file, char *authorization, unsigned char *result);
ERROR_CODE auth_default_http(char *auth_file, char *authorization, unsigned char *result);
ERROR_CODE auth_file_http(char *auth_file, char *authorization, unsigned char *result);
ERROR_CODE parameters_http(struct hash_map_t *hash_map, unsigned char **buffer_pointer, size_t *buffer_length_pointer);
ERROR_CODE parameters_http_c(char *buffer, size_t size, size_t count, ...);

__inline static const char *_get_http_version_code(size_t index) {
    return http_version_codes[index];
}

__inline static const char *_get_keep_alive_code(size_t index) {
    return keep_alive_codes[index];
}

__inline static const char *_get_cache_code(size_t index) {
    return cache_codes[index];
}

__inline static const char *_get_close_code(size_t index) {
    return close_codes[index];
}

__inline static const char *_get_http_status_code(size_t major, size_t minor) {
    return http_status_codes[major][minor];
}
