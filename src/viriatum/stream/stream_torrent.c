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

#include "stream_torrent.h"

ERROR_CODE create_torrent_connection(struct torrent_connection_t **torrent_connection_pointer, struct io_connection_t *io_connection) {
    /* retrieves the torrent connection size */
    size_t torrent_connection_size = sizeof(struct torrent_connection_t);

    /* allocates space for the torrent connection */
    struct torrent_connection_t *torrent_connection = (struct torrent_connection_t *) MALLOC(torrent_connection_size);

    /* retrieves the service associated with the connection */
    struct service_t *service = io_connection->connection->service;

    /* sets the torrent handler attributes (default) values */
    torrent_connection->io_connection = io_connection;
    torrent_connection->torrent_handler = NULL;

    /* sets the torrent connection in the (upper) io connection substrate */
    io_connection->lower = torrent_connection;

    /* sets the current torrent handler for the created connection as
    not valid no delegation will occur */
    torrent_connection->torrent_handler = NULL;

    /* sets the torrent connection in the torrent connection pointer */
    *torrent_connection_pointer = torrent_connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_torrent_connection(struct torrent_connection_t *torrent_connection) {
    /* releases the torrent connection */
    FREE(torrent_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE data_handler_stream_torrent(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    char *_buffer = (char *) MALLOC(buffer_size + 1);

    memcpy(_buffer, buffer, buffer_size);
    _buffer[buffer_size] = '\0';

    printf("'%s'", _buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_handler_stream_torrent(struct io_connection_t *io_connection) {
    /* allocates the torrent connection and the space
    to be used for the buffer for the response */
    struct torrent_connection_t *torrent_connection;
    struct torrent_handshake_t *response_buffer = (struct torrent_handshake_t *) MALLOC(sizeof(struct torrent_handshake_t));



    FILE *file = fopen("C:/info_hash.txt", "rb");
    if(file == NULL) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem opening hash file"
        );
    }
    fread(response_buffer->info_hash, 1, 20, file);
    fclose(file);


    /* creates the torrent connection for the currently used
    io connection structure */
    create_torrent_connection(&torrent_connection, io_connection);


    /* populates the various elements of the response buffer structure
    using the static values and the info hash and peer id provided */
    response_buffer->torrent_size = TORRENT_PROTOCOL_SIZE;
    memcpy(response_buffer->torrent_string, TORRENT_PROTOCOL_STRING, TORRENT_PROTOCOL_SIZE);
    memset(response_buffer->reserved, 0, 8);
    /*memcpy(response_buffer->info_hash, "-AZ4702-UCahr9VNImUy", 20);*/
    memcpy(response_buffer->peer_id, "-AZ4702-UCJhrsVNImUy", 20);


    /* writes the response to the connection, registers for the appropriate callbacks */
    write_connection(
        io_connection->connection,
        (unsigned char *) response_buffer,
        sizeof(struct torrent_handshake_t),
        NULL,
        NULL
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_handler_stream_torrent(struct io_connection_t *io_connection) {
    /* retrieves the torrent connection */
    struct torrent_connection_t *torrent_connection = (struct torrent_connection_t *) io_connection->lower;

    /* deletes the torrent connection */
    delete_torrent_connection(torrent_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
