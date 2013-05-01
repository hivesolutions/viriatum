/*
 Hive Viriatum Modules
 Copyright (C) 2008-2012 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "handler.h"

unsigned char _empty_gif[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00,
    0x01, 0x00, 0xf0, 0x01, 0x00, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x21, 0xf9, 0x04, 0x01, 0x0a,
    0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44,
    0x01, 0x00, 0x3b
};
unsigned int _empty_gif_size = 43;

ERROR_CODE set_handler_gif(struct http_connection_t *http_connection) {
    /* sets both the http parser values and the settings
    and then returns normally to the caller function */
    _set_http_parser_handler_gif(http_connection->http_parser);
    _set_http_settings_handler_gif(http_connection->http_settings);
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_gif(struct http_connection_t *http_connection) {
    /* unsets both the http parser values and the settings
    and then returns normally to the caller function */
    _unset_http_parser_handler_gif(http_connection->http_parser);
    _unset_http_settings_handler_gif(http_connection->http_settings);
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_gif(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_gif(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_gif(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_gif(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_gif(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_gif(struct http_parser_t *http_parser) {
    /* sends (and creates) the reponse */
    _send_response_handler_gif(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_gif(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_gif(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_gif(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_gif(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_gif(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_gif(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_module;
    http_settings->on_url = url_callback_handler_gif;
    http_settings->on_header_field = header_field_callback_handler_gif;
    http_settings->on_header_value = header_value_callback_handler_gif;
    http_settings->on_headers_complete = headers_complete_callback_handler_gif;
    http_settings->on_body = body_callback_handler_gif;
    http_settings->on_message_complete = message_complete_callback_handler_gif;
    http_settings->on_path = path_callback_handler_gif;
    http_settings->on_location = location_callback_handler_gif;
    http_settings->on_virtual_url = virtual_url_callback_handler_gif;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_gif(struct http_settings_t *http_settings) {
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

ERROR_CODE _send_response_handler_gif(struct http_parser_t *http_parser) {
    /* allocates the space for the buffer to be used to
    send the data to the client side and for the counter
    that stores the size of the buffer to be sent*/
    char *buffer;
    size_t count;

    /* retrieves the connection from the http parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* retrieves the http connection from the io connection and uses it to retrieve
    the correct (mod gif) handler to operate around it */
    struct http_connection_t *http_connection = (struct http_connection_t *) ((struct io_connection_t *) connection->lower)->lower;
    struct mod_gif_http_handler_t *mod_gif_http_handler = (struct mod_gif_http_handler_t *) http_connection->http_handler->lower;

    /* allocates the buffer to be used to send the empty gif data, it must
    be allocated inside the viriatum memory area so that the deallocation of
    it is handled by the main engine */
    connection->alloc_data(connection, VIRIATUM_HTTP_MAX_SIZE, (void **) &buffer);

    /* acquires the lock on the http connection, this will avoids further
    messages to be processed, no parallel request handling problems, then
    writes the message into the current http connection, the message should
    be composed of an empty gif */
    http_connection->acquire(http_connection);
    count = http_connection->write_headers_c(
        connection,
        buffer,
        VIRIATUM_HTTP_SIZE,
        HTTP11,
        200,
        "OK",
        KEEP_ALIVE,
        _empty_gif_size,
        NO_CACHE,
        TRUE
    );
    count += SPRINTF(
        &buffer[count],
        VIRIATUM_HTTP_SIZE - count,
        "%s",
        _empty_gif
    );

    /* writes the response to the connection, this will write the
    complete message to the connection, upon finishing the sent operation
    the callback will be called for finish operations */
    connection->write_connection(
        connection,
        (unsigned char *) buffer,
        (unsigned int) count,
        _send_response_callback_handler_gif,
        (void *) (size_t) http_parser->flags
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_gif(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current http flags as the provided parameters
    these flags are going to be used for conditional execution */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* checks if the current connection should be kept alive, this must
    be done prior to the unseting of the connection as the current gif
    context structrue will be destroyed there */
    unsigned char keep_alive = flags & FLAG_CONNECTION_KEEP_ALIVE;

    /* in case there is an http handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current http connection and then sets the reference
        to it in the http connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive must be closed
    in the normal manner (using the close connection function) otherwise
    releases the lock on the http connection, this will allow further
    messages to be processed, an update event should raised following this
    lock releasing call */
    if(!keep_alive) { connection->close_connection(connection); }
    else { http_connection->release(http_connection); }

    /* raise no error */
    RAISE_NO_ERROR;
}
