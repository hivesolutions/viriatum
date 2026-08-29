/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "handler.h"

ERROR_CODE set_handler_diag(struct http_connection_t *http_connection) {
    /* sets the HTTP setting values (callback handlers) and then returns
    normally to the caller function */
    http_connection->http_settings->on_message_complete = message_complete_callback_handler_diag;
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_diag(struct http_connection_t *http_connection) {
    /* unsets the HTTP setting values (callback handlers) and then returns
    normally to the caller function */
    http_connection->http_settings->on_message_complete = NULL;
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_diag(struct http_request_t *http_request) {
    /* sends (and creates) the response from the currently parsed
    messages and then returns with no error the caller function */
    _send_response_handler_diag(http_request);
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_diag(struct http_request_t *http_request) {
    /* allocates the space for the buffer to be used to
    send the data to the client side and for the counter
    that stores the size of the buffer to be sent*/
    char *buffer;
    char *body;
    size_t count;
    char length[32];

    /* reserves space for the json based info string for its
    size and for the complete size of the resulting message */
    char info[10240];
    size_t info_size;
    size_t message_size;

    /* retrieves the connection from the HTTP parser parameters and
    uses it to retrieve the associated service instance */
    struct connection_t *connection = (struct connection_t *) http_request->parameters;
    struct service_t *service = (struct service_t *) connection->service;

    /* retrieves the HTTP connection from the io connection to be used
    for the HTTP level operation to be done */
    struct http_connection_t *http_connection =
        (struct http_connection_t *) ((struct io_connection_t *) connection->lower)->lower;

    /* create the info json string with the complete set of diagnostics
    information regarding the viriatum service structure and then updates
    the message size value with the size of the information string */
    info_size = SPRINTF(
        info,
        10240,
        "{"
        "\"name\":\"%s\","
        "\"version\":\"%s\","
        "\"platform\":\"%s\","
        "\"flags\":\"%s\","
        "\"compiler\":\"%s\","
        "\"compiler_version\":\"%s\","
        "\"compilation_date\":\"%s\","
        "\"compilation_time\":\"%s\","
        "\"compilation_flags\":\"%s\","
        "\"modules\":\"%s\","
        "\"description\":\"%s\","
        "\"status\":%d,"
        "\"port\":%d,"
        "\"uptime\":\"%s\","
        "\"connections\":%ld"
        "}",
        service->name,
        service->version,
        service->platform,
        service->flags,
        service->compiler,
        service->compiler_version,
        service->compilation_date,
        service->compilation_time,
        service->compilation_flags,
        service->modules,
        service->description,
        service->status,
        service->options->port,
        service->get_uptime(service, 2),
        (long int) service->connections_list->size
    );
    message_size = VIRIATUM_HTTP_MAX_SIZE + info_size;

    /* allocates the buffer to be used to send the empty diag data, it must
    be allocated inside the viriatum memory area so that the deallocation of
    it is handled by the main engine */
    connection->alloc_data(connection, message_size, (void **) &buffer);

    /* acquires the lock on the HTTP connection, this will avoid further
    messages to be processed, no parallel request handling problems, then
    writes the message into the current HTTP connection, the message should
    be composed of an empty diag */
    http_connection->acquire(http_connection);
    count = http_connection->write_status(
        connection,
        buffer,
        message_size,
        http_request->version,
        200,
        "OK",
        http_request->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE
    );
    SPRINTF(length, sizeof(length), "%lu", (long unsigned int) info_size);
    count = http_connection->write_field(
        connection,
        buffer,
        message_size,
        count,
        CONTENT_LENGTH_H,
        length
    );
    count = http_connection->write_field(
        connection,
        buffer,
        message_size,
        count,
        CACHE_CONTROL_H,
        cache_codes[NO_CACHE - 1]
    );
    count = http_connection->write_field(
        connection,
        buffer,
        message_size,
        count,
        CONTENT_TYPE_H,
        "application/json"
    );
    count = http_connection->write_end(connection, buffer, message_size, count, FALSE);

    /* gathers the payload into a buffer of its own, the protocol is
    the one framing it and so it travels as a write of its own */
    connection->alloc_data(connection, info_size, (void **) &body);
    memcpy(body, info, info_size);

    /* writes the response to the connection, this will write the
    complete message to the connection, upon finishing the sent operation
    the callback will be called for finish operations */
    http_connection->write_flush(connection, (unsigned char *) buffer, count, NULL, NULL);
    http_connection->write_chunk(
        connection,
        (unsigned char *) body,
        info_size,
        TRUE,
        _send_response_callback_handler_diag,
        (void *) (size_t) http_request->flags
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_diag(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current HTTP flags as the provided parameters
    these flags are going to be used for conditional execution */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* checks if the current connection should be kept alive, this must
    be done prior to the unseting of the connection as the current diag
    context structure will be destroyed there */
    unsigned char keep_alive = flags & FLAG_KEEP_ALIVE;

    /* in case there is an HTTP handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current HTTP connection and then sets the reference
        to it in the HTTP connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive must be closed
    in the normal manner (using the close connection function) otherwise
    releases the lock on the HTTP connection, this will allow further
    messages to be processed, an update event should raised following this
    lock releasing call */
    if(!keep_alive) {
        connection->close_connection(connection);
    } else {
        http_connection->release(http_connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
