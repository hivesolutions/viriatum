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

#include "stdafx.h"

#include "torrent.h"

ERROR_CODE _create_tracker_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
    /* allocates space for the connection reference */
    struct connection_t *connection;

    /* alocates dynamic space for the parameters to the
    http stream (http client) this structure will be able
    to guide the stream of http client */
    struct http_client_parameters_t *parameters =\
        (struct http_client_parameters_t *) MALLOC(sizeof(struct http_client_parameters_t));

    /* populates the parameters structure with the
    required values for the http client request */
    parameters->url = "/ptorrent/announce.php";

    /* creates a general http client connection structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(connection_pointer, service, hostname, port);
    connection = *connection_pointer;

    /* sets the http client protocol as the protocol to be
    "respected" for this client connection, this should
    be able to set the apropriate handlers in io then sets
    the parameters structure in the connection so that the
    lower layers "know" what to do */
    connection->protocol = HTTP_CLIENT_PROTOCOL;
    connection->parameters = (void *) parameters;

    /* opens the connection */
    connection->open_connection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _create_torrent_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
    /* allocates space for the connection reference */
    struct connection_t *connection;

    /* creates a general client conneciton structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(connection_pointer, service, hostname, port);
    connection = *connection_pointer;

    /* sets the torrent protocol as the protocol to be
    "respected" for this client connection, this should
    be able to set the apropriate handlers in io */
    connection->protocol = TORRENT_PROTOCOL;

    /* opens the connection */
    connection->open_connection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
