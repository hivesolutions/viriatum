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

#include "loop.h"

/**
 * The name under which the asgi handler is registered
 * in the service handlers map.
 */
#define VIRIATUM_ASGI_HANDLER_NAME ((unsigned char *) "asgi")

/**
 * The version of both the asgi specification and of the
 * application interface that are implemented.
 */
#define VIRIATUM_ASGI_VERSION "3.0"
#define VIRIATUM_ASGI_SPEC_VERSION "2.3"

/**
 * The maximum number of headers that may be gathered
 * from a request or set by an application response.
 */
#define VIRIATUM_ASGI_MAX_HEADERS 64

/**
 * The maximum size of the payload of a request, anything
 * beyond this value is discarded.
 */
#define VIRIATUM_ASGI_MAX_BODY 16777216

/**
 * The initial capacity of the buffer that accumulates the
 * payload of a request, grown geometrically from it.
 */
#define VIRIATUM_ASGI_BODY_CAPACITY 4096

/**
 * The number of iterations of the serving loop that the
 * lifespan operations may take before being abandoned.
 */
#define VIRIATUM_ASGI_LIFESPAN_ITERATIONS 1000

/**
 * The maximum size of the subprotocol that may be selected
 * by the application as part of the handshake.
 */
#define VIRIATUM_ASGI_MAX_PROTOCOL 128

/**
 * The response that refuses a handshake, sent whenever the
 * application closes the connection before accepting it.
 */
#define VIRIATUM_ASGI_REJECTED \
    "HTTP/1.1 403 Forbidden\r\n" \
    "Content-Length: 0\r\n" \
    "Connection: close\r\n" \
    "\r\n"

/**
 * Enumeration describing the state of the response that is
 * being produced for the request of the current context.
 */
typedef enum asgi_state_e {
    ASGI_STATE_PENDING = 1,
    ASGI_STATE_STARTED,
    ASGI_STATE_HEADERS,
    ASGI_STATE_COMPLETE
} asgi_state;

/**
 * Enumeration describing the state of the websocket
 * connection of the current context.
 */
typedef enum asgi_websocket_e {
    ASGI_WEBSOCKET_NONE = 1,
    ASGI_WEBSOCKET_CONNECTING,
    ASGI_WEBSOCKET_CONNECTED,
    ASGI_WEBSOCKET_CLOSED
} asgi_websocket;

/**
 * Structure holding the state of a single request being
 * handled, one of these exists per connection so that no
 * global state is required (contrary to mod_wsgi).
 */
typedef struct handler_asgi_context_t {
    /**
     * The url as received from the request line, the
     * query string is still part of this value.
     */
    unsigned char *url;

    /**
     * The set of header names gathered from the request
     * together with the associated values.
     */
    unsigned char *header_fields[VIRIATUM_ASGI_MAX_HEADERS];
    unsigned char *header_values[VIRIATUM_ASGI_MAX_HEADERS];

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
    size_t body_capacity;

    /**
     * The status code set by the application through the
     * response start message of the send callable.
     */
    int status_code;

    /**
     * The set of headers set by the application, already
     * formatted as complete header lines.
     */
    unsigned char *response_headers[VIRIATUM_ASGI_MAX_HEADERS];
    size_t response_header_count;

    /**
     * The state of the response, it advances as the various
     * messages are received from the application.
     */
    enum asgi_state_e state;

    /**
     * Flag controlling if the application has set a content
     * length of its own, in which case no chunked framing
     * is used for the sending of the payload.
     */
    char has_length;

    /**
     * Flag controlling if the response may carry a payload,
     * unset for both the head requests and the statuses that
     * are defined as carrying no body.
     */
    char has_body;

    /**
     * The flags of the request as gathered from the parser,
     * they control the keeping alive of the connection.
     */
    unsigned char flags;

    /**
     * The state of the websocket connection, set to the none
     * value for the plain http requests.
     */
    enum asgi_websocket_e websocket_state;

    /**
     * The buffer accumulating the data received from the peer
     * once the connection has been upgraded, it holds at most
     * one incomplete frame at a time.
     */
    unsigned char *websocket_buffer;
    size_t websocket_buffer_size;
    size_t websocket_buffer_capacity;

    /**
     * The buffer reassembling a fragmented message together
     * with the operation code of the first frame of it.
     */
    unsigned char *websocket_message;
    size_t websocket_message_size;
    size_t websocket_message_capacity;
    unsigned char websocket_opcode;

    /**
     * The list of events that are pending delivery to the
     * application through the receive callable.
     */
    PyObject *events;

    /**
     * The future that a pending receive call is waiting on,
     * resolved as soon as an event becomes available.
     */
    PyObject *future;

    /**
     * The task running the application for the current
     * request, cancelled on the destruction of the context.
     */
    PyObject *task;

    /**
     * The capsule carrying the context to both the receive
     * and the send callables, invalidated on destruction.
     */
    PyObject *capsule;

    /**
     * The connection that originated the request, required
     * for the writing of the various parts of the response.
     */
    struct connection_t *connection;

    /**
     * Flag controlling if the current context is the one of the
     * lifespan protocol, in which case the various messages are
     * handled apart from the ones of a request.
     */
    char lifespan;

    /**
     * The reference to the handler that owns the current
     * context, used to reach the application callable.
     */
    struct handler_asgi_t *handler;
} handler_asgi_context;

/**
 * Structure describing the asgi handler, holding the
 * application callable that is going to be called for
 * every request received by the service.
 */
typedef struct handler_asgi_t {
    struct http_handler_t *http_handler;
    PyObject *application;
    struct loop_python_t *loop_python;

    /**
     * The context of the lifespan protocol, it lives for the
     * complete duration of the serving operation.
     */
    struct handler_asgi_context_t *lifespan_context;

    /**
     * The state of both the startup and the shutdown events of
     * the lifespan protocol, unset while they are still pending.
     */
    char lifespan_startup;
    char lifespan_shutdown;
} handler_asgi;

ERROR_CODE create_handler_asgi_context(struct handler_asgi_context_t **handler_asgi_context_pointer);
ERROR_CODE delete_handler_asgi_context(struct handler_asgi_context_t *handler_asgi_context);
ERROR_CODE register_handler_asgi(struct service_t *service, PyObject *application, struct loop_python_t *loop_python);
ERROR_CODE unregister_handler_asgi(struct service_t *service);
ERROR_CODE startup_handler_asgi(struct service_t *service);
ERROR_CODE shutdown_handler_asgi(struct service_t *service);
ERROR_CODE set_handler_asgi(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_asgi(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_asgi(struct http_parser_t *http_parser);
ERROR_CODE url_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_asgi(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_asgi(struct http_parser_t *http_parser);
ERROR_CODE path_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_asgi(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_asgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE data_handler_websocket_asgi(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE _set_http_parser_handler_asgi(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_asgi(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_asgi(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_asgi(struct http_settings_t *http_settings);
ERROR_CODE _call_application_handler_asgi(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_asgi(struct connection_t *connection, struct data_t *data, void *parameters);
