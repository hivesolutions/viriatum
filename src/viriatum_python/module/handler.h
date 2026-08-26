/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../stdafx.h"

/**
 * The name under which the python (WSGI) handler is
 * registered in the service handlers map.
 */
#define VIRIATUM_PYTHON_HANDLER_NAME ((unsigned char *) "python")

/**
 * The maximum number of headers that may be gathered
 * from a request or set by an application response.
 */
#define VIRIATUM_PYTHON_MAX_HEADERS 64

/**
 * Structure holding the state of a single request being
 * handled, one of these exists per connection so that no
 * global state is required (contrary to mod_wsgi).
 */
typedef struct handler_python_context_t {
    /**
     * The url as received from the request line, the
     * query string is still part of this value.
     */
    unsigned char *url;

    /**
     * The set of header names gathered from the request
     * together with the associated values.
     */
    unsigned char *header_fields[VIRIATUM_PYTHON_MAX_HEADERS];
    unsigned char *header_values[VIRIATUM_PYTHON_MAX_HEADERS];

    /**
     * The number of headers currently gathered from the
     * request, limited by the maximum header count.
     */
    size_t header_count;

    /**
     * The payload of the request, gathered from the various
     * body callbacks raised by the parser.
     */
    unsigned char *body;
    size_t body_size;

    /**
     * The status code and message set by the application
     * through the start response callable.
     */
    int status_code;
    unsigned char *status_message;

    /**
     * The set of headers set by the application, already
     * formatted as complete header lines.
     */
    unsigned char *response_headers[VIRIATUM_PYTHON_MAX_HEADERS];
    size_t response_header_count;

    /**
     * Flag controlling if the start response callable has
     * already been called for the current request.
     */
    char started;

    /**
     * The reference to the handler that owns the current
     * context, used to reach the application callable.
     */
    struct handler_python_t *handler;
} handler_python_context;

/**
 * Structure describing the python handler, holding the
 * application callable that is going to be called for
 * every request received by the service.
 */
typedef struct handler_python_t {
    struct http_handler_t *http_handler;
    PyObject *application;
} handler_python;

ERROR_CODE create_handler_python_context(struct handler_python_context_t **handler_python_context_pointer);
ERROR_CODE delete_handler_python_context(struct handler_python_context_t *handler_python_context);
ERROR_CODE register_handler_python(struct service_t *service, PyObject *application);
ERROR_CODE unregister_handler_python(struct service_t *service);
ERROR_CODE set_handler_python(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_python(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_python(struct http_parser_t *http_parser);
ERROR_CODE url_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_python(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_python(struct http_parser_t *http_parser);
ERROR_CODE path_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_python(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_python(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_parser_handler_python(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_python(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_python(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_python(struct http_settings_t *http_settings);
ERROR_CODE _send_response_handler_python(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_python(struct connection_t *connection, struct data_t *data, void *parameters);
