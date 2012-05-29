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
struct TorrentConnection_t;

/**
 * Function used to update the given torrent connection
 * with new information.
 *
 * @param httpConnection The http connection to be
 * update with new information.
 */
typedef ERROR_CODE (*torrentConnectionUpdate) (struct TorrentConnection_t *torrentConnection);

/**
 * The structure that describes the structure
 * of an handler capable of interpreting an
 * torrent request and construct a response based
 * on it.
 */
typedef struct TorrentHandler_t {
    /**
     * The name that describes the torrent
     * handler.
     * This is going to be used as id and
     * so it hsould be unique to the system.
     */
    unsigned char *name;

    torrentConnectionUpdate set;
    torrentConnectionUpdate unset;
    torrentConnectionUpdate reset;

    /**
     * The reference to the service that "owns"
     * the current handler.
     * The creation of the handler implies the
     * registration of it in this service.
     */
    struct Service_t *service;

    /**
     * Reference to the lower level
     * handler substrate (child).
     */
    void *lower;
} TorrentHandler;

/**
 * Structure defining a logical
 * torrent connection.
 */
typedef struct TorrentConnection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct IoConnection_t *ioConnection;

    /**
     * The handler currently being used to handle
     * the connection.
     * This handles may be from an external resource.
     * Security issues apply.
     * This is an internal value and must be used
     * with care.
     */
    struct TorrentHandler_t *torrentHandler;
} TorrentConnection;

ERROR_CODE createTorrentConnection(struct TorrentConnection_t **torrentConnectionPointer, struct IoConnection_t *ioConnection);
ERROR_CODE deleteTorrentConnection(struct TorrentConnection_t *torrentConnection);
ERROR_CODE dataHandlerStreamTorrent(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize);
ERROR_CODE openHandlerStreamTorrent(struct IoConnection_t *ioConnection);
ERROR_CODE closeHandlerStreamTorrent(struct IoConnection_t *ioConnection);
