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

#include "stream_io.h"

/* forward references (avoids loop) */
struct torrent_connection_t;

/**
 * Function used to update the given torrent connection
 * with new information.
 *
 * @param http_connection The http connection to be
 * update with new information.
 */
typedef ERROR_CODE (*torrent_connection_update) (struct torrent_connection_t *torrent_connection);

/**
 * Structure defining a logical
 * torrent connection.
 */
typedef struct torrent_connection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct io_connection_t *io_connection;

    /**
     * The handler currently being used to handle
     * the connection.
     * This handles may be from an external resource.
     * Security issues apply.
     * This is an internal value and must be used
     * with care.
     */
    struct TorrentHandler_t *torrent_handler;
} torrent_connection;

ERROR_CODE create_torrent_connection(struct torrent_connection_t **torrent_connection_pointer, struct io_connection_t *io_connection);
ERROR_CODE delete_torrent_connection(struct torrent_connection_t *torrent_connection);
ERROR_CODE data_handler_stream_torrent(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size);
ERROR_CODE open_handler_stream_torrent(struct io_connection_t *io_connection);
ERROR_CODE close_handler_stream_torrent(struct io_connection_t *io_connection);
