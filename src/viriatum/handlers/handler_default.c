/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2017 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2017 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "handler_default.h"

ERROR_CODE register_handler_default(struct service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* creates the http handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "default");

    /* sets the http handler attributes */
    http_handler->resolve_index = FALSE;
    http_handler->set = set_handler_default;
    http_handler->unset = unset_handler_default;
    http_handler->reset = NULL;

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_default(struct service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* retrieves the http handler from the service, then
    remove it from the service after that delete the handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "default");
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_default(struct http_connection_t *http_connection) {
    /* sets the http parser values */
    _set_http_parser_handler_default(http_connection->http_parser);

    /* sets the http settings values */
    _set_http_settings_handler_default(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_default(struct http_connection_t *http_connection) {
    /* unsets the http parser values */
    _unset_http_parser_handler_default(http_connection->http_parser);

    /* unsets the http settings values */
    _unset_http_settings_handler_default(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_default(struct http_parser_t *http_parser) {
    /* prints an information */
    V_DEBUG("http request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates the required space for the url */
    unsigned char *url = (unsigned char *) MALLOC(data_size + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, data_size);

    /* puts the end of strng in the url */
    url[data_size] = '\0';

    /* prints the url */
    V_DEBUG_F("url: %s\n", url);

    /* releases the url */
    FREE(url);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates the required space for the header field */
    unsigned char *header_field = (unsigned char *) MALLOC(data_size + 1);

    /* copies the memory from the data to the header field */
    memcpy(header_field, data, data_size);

    /* puts the end of strng in the header field */
    header_field[data_size] = '\0';

    /* prints the header field */
    V_DEBUG_F("header field: %s\n", header_field);

    /* releases the header field */
    FREE(header_field);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates the required space for the header value */
    unsigned char *header_value = (unsigned char *) MALLOC(data_size + 1);

    /* copies the memory from the data to the header value */
    memcpy(header_value, data, data_size);

    /* puts the end of strng in the header value */
    header_value[data_size] = '\0';

    /* prints the header value */
    V_DEBUG_F("header value: %s\n", header_value);

    /* releases the header value */
    FREE(header_value);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_default(struct http_parser_t *http_parser) {
    /* prints an information */
    V_DEBUG("http headers parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates the required space for the body */
    unsigned char *body = (unsigned char *) MALLOC(data_size + 1);

    /* copies the memory from the data to the body */
    memcpy(body, data, data_size);

    /* puts the end of strng in the body */
    body[data_size] = '\0';

    /* prints the body */
    V_DEBUG_F("body: %s\n", body);

    /* releases the body */
    FREE(body);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_default(struct http_parser_t *http_parser) {
    /* prints an information */
    V_DEBUG("http request parsed\n");

    /* sends (and creates) the reponse */
    _send_response_handler_default(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_default(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_default(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_default(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_default(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_default;
    http_settings->on_url = url_callback_handler_default;
    http_settings->on_header_field = header_field_callback_handler_default;
    http_settings->on_header_value = header_value_callback_handler_default;
    http_settings->on_headers_complete = headers_complete_callback_handler_default;
    http_settings->on_body = body_callback_handler_default;
    http_settings->on_message_complete = message_complete_callback_handler_default;
    http_settings->on_path = path_callback_handler_default;
    http_settings->on_location = location_callback_handler_default;
    http_settings->on_virtual_url = virtual_url_callback_handler_default;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_default(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the http settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;
    http_settings->on_path = NULL;
    http_settings->on_location = NULL;
    http_settings->on_virtual_url = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_default(struct http_parser_t *http_parser) {
    /* allocates the response buffer */
    char *response_buffer = MALLOC(256);

    /* retrieves the connection from the http parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for register */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* acquires the lock on the http connection, this will avoids further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* writes the http static headers (and message) as the response and
    registers for the appropriate callbacks (for cleanup) */
    write_http_message(
        connection,
        response_buffer,
        256,
        HTTP11,
        200,
        "OK",
        "Hello Viriatum",
        http_parser->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
        _send_response_callback_handler_default,
        (void *) (size_t) http_parser->flags
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_default(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current http flags */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* in case there is an http handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current http connection and then sets the reference
        to it in the http connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive */
    if(!(flags & FLAG_KEEP_ALIVE)) {
        /* closes the connection */
        connection->close_connection(connection);
    } else {
        /* releases the lock on the http connection, this will allow further
        messages to be processed, an update event should raised following this
        lock releasing call */
        http_connection->release(http_connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
