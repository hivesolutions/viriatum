/*
 Hive Viriatum Modules
 Copyright (c) 2008-2017 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2017 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
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
    /* sets the http setting values (callback handlers) and then returns
    normally to the caller function */
    http_connection->http_settings->on_message_complete = message_complete_callback_handler_gif;
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_gif(struct http_connection_t *http_connection) {
    /* unsets the http setting values (callback handlers) and then returns
    normally to the caller function */
    http_connection->http_settings->on_message_complete = NULL;
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_gif(struct http_parser_t *http_parser) {
    /* sends (and creates) the reponse from the currently parsed
    messages and then returns with no error the caller function */
    _send_response_handler_gif(http_parser);
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

    /* retrieves the http connection from the io connection to be used
    for the http level operation to be done */
    struct http_connection_t *http_connection =\
        (struct http_connection_t *) ((struct io_connection_t *) connection->lower)->lower;

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
        http_parser->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
        _empty_gif_size,
        MAX_AGE,
        FALSE
    );
    count += SPRINTF(
        &buffer[count],
        VIRIATUM_HTTP_SIZE - count,
        CONTENT_TYPE_H ": %s\r\n\r\n",
        "image/gif"
    );
    memcpy(&buffer[count], _empty_gif, _empty_gif_size);
    count += _empty_gif_size;

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
    unsigned char keep_alive = flags & FLAG_KEEP_ALIVE;

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
