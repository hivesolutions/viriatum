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

#include "client.h"

ERROR_CODE _create_client_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
    /* allocates the various scoket related structures
    to be used to create the client connection */
    SOCKET_HANDLE socket_handle;
    SOCKET_ADDRESS_INTERNET serv_addr;
    SOCKET_HOSTENT *server;
    SOCKET_ERROR_CODE error;

    /* allocates the (client) connection, this is the
    structure representing the connection */
    struct connection_t *connection;

    /* allocates the option value and sets it to one (valid) */
    SOCKET_OPTION option_value = 1;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    socket_handle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);
    if(SOCKET_TEST_ERROR(socket_handle)) { fprintf(stderr, "ERROR opening socket"); }

    /* tries to retrieve the server host value from the provided
    hostname string value in case the "resolution" fails raises
    an error indicating the problem */
    server = SOCKET_GET_HOST_BY_NAME(hostname);
    if(server == NULL) { fprintf(stderr, "ERROR, no such host\n"); }

    memset(&serv_addr, 0, sizeof(SOCKET_ADDRESS_INTERNET));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    error = SOCKET_CONNECT_SIZE(socket_handle, serv_addr, sizeof(SOCKET_ADDRESS_INTERNET));
    if(SOCKET_TEST_ERROR(error)) { fprintf(stderr, "ERROR connecting host\n"); }

    /* in case viriatum is set to non blocking, changes the current
    socket behavior to non blocking mode then sets the socket to the
    non push mode in case it's required by configuration this implies
    also checking for the no (tcp) wait variable */
    if(VIRIATUM_NON_BLOCKING) { SOCKET_SET_NON_BLOCKING(socket_handle, flags); }
    if(VIRIATUM_NO_WAIT) { SOCKET_SET_NO_WAIT(socket_handle, option_value); }
    if(VIRIATUM_NO_PUSH) { SOCKET_SET_NO_PUSH(socket_handle, option_value); }

    /* creates the (client) connection */
    create_connection(&connection, socket_handle);

    /* sets the socket address in the (client) connection
    this is going to be very usefull for later connection
    identification (address, port, etc.) */
    /*connection->socket_address = (SOCKET_ADDRESS) serv_addr;*/

    /* sets the service select service as the service in the (client)  connection */
    connection->service = service;

    /* sets the base handling functions in the client connection,
    these are just the default connection set for every connection*/
    connection->open_connection = open_connection;
    connection->close_connection = close_connection;
    connection->write_connection = write_connection;
    connection->register_write = register_write_connection;
    connection->unregister_write = unregister_write_connection;

    /* sets the various stream io connection callbacks
    in the client connection */
    connection->on_read = read_handler_stream_io;
    connection->on_write = write_handler_stream_io;
    connection->on_error = error_handler_stream_io;
    connection->on_handshake = handshake_handler_stream_io;
    connection->on_open = open_handler_stream_io;
    connection->on_close = close_handler_stream_io;

    /* updats the connection pointer with the refernce
    to the connection structure */
    *connection_pointer = connection;

    /* raises no error */
    RAISE_NO_ERROR;
}
