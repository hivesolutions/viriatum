/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../system/service.h"
#include "stream_http.h"
#include "stream_httpc.h"
#include "stream_torrent.h"

struct io_connection_t;

typedef ERROR_CODE (*data_io_connection_callback) (struct io_connection_t *, unsigned char *, size_t);
typedef ERROR_CODE (*io_connection_callback) (struct io_connection_t *);

/**
 * Structure to be used for the parameters to be
 * sent for custom based protocol (want their own
 * handling implementation).
 */
typedef struct custom_parameters_t {
    /**
     * The callback function to be used when new data
     * is available in the current connection.
     */
    data_io_connection_callback on_data;

    /**
     * Reference to the callback function to be called
     * once when the connection is considered as open.
     */
    io_connection_callback on_open;

    /**
     * Reference to the callback function to be called
     * once when the connection is considered as closed.
     */
    io_connection_callback on_close;

    /**
     * The reference to the "type transparent" value to
     * be used as parameter (erxtra one) to the current
     * parameters structure.
     */
    void *parameters;
} custom_parameters;

/**
 * Structure defining a logical
 * io connection.
 */
typedef struct io_connection_t {
    /**
     * The (upper) connection that owns
     * manages this connection.
     */
    struct connection_t *connection;

    /**
     * Callback function reference to be called
     * when data is available for processing.
     */
    data_io_connection_callback on_data;

    /**
     * Callback function reference to be called
     * when a connection is has been open.
     */
    io_connection_callback on_open;

    /**
     * Callback function reference to be called
     * when a connection is going to be closed.
     */
    io_connection_callback on_close;

    /**
     * Reference to the lower level
     * stream substrate (child).
     */
    void *lower;
} io_connection;

void create_io_connection(struct io_connection_t **io_connection_pointer, struct connection_t *connection);
void delete_io_connection(struct io_connection_t *io_connection);
ERROR_CODE accept_handler_stream_io(struct connection_t *connection);
ERROR_CODE read_handler_stream_io(struct connection_t *connection);
ERROR_CODE write_handler_stream_io(struct connection_t *connection);
ERROR_CODE handshake_handler_stream_io(struct connection_t *connection);
ERROR_CODE error_handler_stream_io(struct connection_t *connection);
ERROR_CODE open_handler_stream_io(struct connection_t *connection);
ERROR_CODE close_handler_stream_io(struct connection_t *connection);
