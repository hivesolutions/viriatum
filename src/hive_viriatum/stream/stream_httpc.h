/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../http/http.h"
#include "stream_io.h"

/* forward references (avoids loop) */
struct http_client_connection_t;

/**
 * Function used to update the given http client connection
 * with new information.
 *
 * @param http_client_connection The http client connection to be
 * update with new information.
 */
typedef ERROR_CODE (*http_client_cConnection_update) (struct http_client_connection_t *http_client_connection);

/**
 * Structure defining a logical
 * http client connection.
 */
typedef struct http_client_connection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct IoConnection_t *io_connection;

    /**
     * Structure containig the settings to be
     * used by the http parser.
     */
    struct http_settings_t *http_settings;

    /**
     * Parser to be used during the interpretation
     * of the http requests.
     */
    struct http_parser_t *http_parser;
} http_client_connection;

typedef struct http_client_parameters_t {
    char *url;
} HttpClientParameters;

ERROR_CODE create_http_client_connection(struct http_client_connection_t **http_client_connection_pointer, struct IoConnection_t *io_connection);
ERROR_CODE delete_http_client_connection(struct http_client_connection_t *http_client_connection);
ERROR_CODE data_handler_stream_http_client(struct IoConnection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE open_handler_stream_http_client(struct IoConnection_t *io_connection);
ERROR_CODE close_handler_stream_http_client(struct IoConnection_t *io_connection);
