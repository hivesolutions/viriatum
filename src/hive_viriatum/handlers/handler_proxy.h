/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../http/http.h"
#include "../system/system.h"
#include "../stream/stream.h"

struct io_connection_t;

typedef ERROR_CODE (*io_connection_callback) (struct io_connection_t *);

/**
 * Structure describing the internal parameters
 * for a location in the proxy context.
 */
typedef struct proxy_location_t {
    /**
     * The pass (host) to be used as the base for the
     * forward operation, this is the base path and the
     * relative path value will be computed.
     */
    unsigned char *proxy_pass;

    /**
     * The result object from the parsing of the url structure
     * present in the proxy pass value. Should containg the
     * description of each of the fields in the url.
     */
    struct url_static_t url_s;
} proxy_location_t;

/**
 * The structure that holds the internal
 * structure to support the proxy handler
 * should contain configuration info.
 */
typedef struct proxy_handler_t {
    /**
     * The various locations loaded from the configuration
     * they refer the cofiruation attributes associated
     * with the proxy structures.
     */
    struct proxy_location_t *locations;

    /**
     * The number of locations currently loaded in the handler
     * this value is used for iteration arround the locations
     * buffer.
     */
    size_t locations_count;

    /**
     * The map that associates the client connection pointer
     * with the backend connection pointer. This is usefull for
     * the re-usage of the various connections along a persistent
     * connection to the viriatum server.
     */
    struct hash_map_t *connections_map;

	/**
	 * Map structure that associates the client connection with
	 * the on close handler that was previously set in the connection
	 * before the re-mapping. This is used so that the close handler
	 * is able to call the old on close handler.
	 */
    struct hash_map_t *on_close_map;
} proxy_handler;

/**
 * The context structure to be used allong
 * the interpretation of the request for
 * the peoxy handler.
 */
typedef struct handler_proxy_context_t {
    /**
     * The pass (host) to be used as the base for the
     * forward operation, this is the base path and the
     * relative path value will be computed.
     */
    unsigned char *proxy_pass;

    /**
     * The description of the url to be used as the proxy pass for the
     * proxy operation that is "inside" the current context.
     */
    struct url_static_t url_s;

    /**
     * The buffer that is going to be used before the data may
     * be flushed to the target proxy client, the size of this buffer
     * is meant to be controlled so that not many reallocations occur
     */
    char *buffer;

    /**
     * The current size of the buffer to be used for the proxy
     * operations when this values exceed that of the maximum size
     * the buffer must be expanded (reallocation).
     */
    size_t buffer_size;

    /**
     * The current maximum size of the buffer (amount of memory allocated
     * for the buffer).
     */
    size_t buffer_max_size;

    /**
     * The buffer to be used to store the data that will be considered the
     * output and will be sent back to the client of the proxy.
     */
    char *out_buffer;

    /**
     * Current size of the output buffer, if this value is greater that the
     * maximum size there must be an error and a proper escape operation
     * must be performed in order to avoid buffer overflow.
     */
    size_t out_buffer_size;

    /**
     * The maximum possible number of bytes that can be stored in the output
     * buffer, this value should be changed when the buffer storage grows or
     * when it shrinks.
     */
    size_t out_buffer_max_size;

    /**
     * The current service connection that "communicates" with the client
     * of the proxy service. This connection may be used to return the data
     * from the target proxy server back to the client.
     */
    struct connection_t *connection;

    /**
     * The client connection of the proxy service, that is used to forward
     * the request from the consumer of the proxy to the target. This is
     * considered to be the backend connection.
     */
    struct connection_t *connection_c;

    /**
     * The current http settings instance to be used for the connection with
     * the backend server to be able to parse its responses.
     */
    struct http_settings_t *http_settings;

    /**
     * Parser instance to be used when parsing the responses provided by the
     * backend server associated with the current context.
     */
    struct http_parser_t *http_parser;
} handler_proxy_context;

ERROR_CODE create_proxy_handler(struct proxy_handler_t **proxy_handler_pointer, struct http_handler_t *http_handler);
ERROR_CODE delete_proxy_handler(struct proxy_handler_t *proxy_handler);
ERROR_CODE create_handler_proxy_context(struct handler_proxy_context_t **handler_proxy_context_pointer);
ERROR_CODE delete_handler_proxy_context(struct handler_proxy_context_t *handler_proxy_context);
ERROR_CODE register_handler_proxy(struct service_t *service);
ERROR_CODE unregister_handler_proxy(struct service_t *service);
ERROR_CODE set_handler_proxy(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_proxy(struct http_connection_t *http_connection);
ERROR_CODE reset_handler_proxy(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_proxy(struct http_parser_t *http_parser);
ERROR_CODE url_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_proxy(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_proxy(struct http_parser_t *http_parser);
ERROR_CODE location_callback_handler_proxy(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE data_backend_handler(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE open_backend_handler(struct io_connection_t *io_connection);
ERROR_CODE close_backend_handler(struct io_connection_t *io_connection);
ERROR_CODE line_callback_backend(struct http_parser_t *http_parser);
ERROR_CODE header_field_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_backend(struct http_parser_t *http_parser);
ERROR_CODE body_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_backend(struct http_parser_t *http_parser);
ERROR_CODE _set_http_parser_handler_proxy(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_proxy(struct http_parser_t *http_parser);
ERROR_CODE _reset_http_parser_handler_proxy(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_proxy(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_proxy(struct http_settings_t *http_settings);
ERROR_CODE _cleanup_handler_proxy(struct connection_t *connection, struct data_t *data, void *parameters);

static __inline void write_proxy_out_buffer(struct handler_proxy_context_t *context, char *data, size_t size) {
    if(context->out_buffer_size + size > context->out_buffer_max_size) {
        context->out_buffer_max_size = (context->out_buffer_size + size) * 2;
        context->out_buffer = REALLOC(context->out_buffer, context->out_buffer_max_size);
    }

    memcpy(&context->out_buffer[context->out_buffer_size], data, size);
    context->out_buffer_size += size;
}

static __inline void write_proxy_buffer(struct handler_proxy_context_t *context, char *data, size_t size) {
    if(context->buffer_size + size > context->buffer_max_size) {
        context->buffer_max_size = (context->buffer_size + size) * 2;
        context->out_buffer = REALLOC(context->buffer, context->buffer_max_size);
    }

    memcpy(&context->buffer[context->buffer_size], data, size);
    context->buffer_size += size;
}
