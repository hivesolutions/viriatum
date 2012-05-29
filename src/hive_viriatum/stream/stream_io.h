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

#include "../system/service.h"
#include "stream_http.h"
#include "stream_httpc.h"
#include "stream_torrent.h"

struct IoConnection_t;

typedef ERROR_CODE (*dataIoConnectionCallback) (struct IoConnection_t *, unsigned char *, size_t);
typedef ERROR_CODE (*ioConnectionCallback) (struct IoConnection_t *);

/**
 * Structure defining a logical
 * io connection.
 */
typedef struct IoConnection_t {
    /**
     * The (upper) connection that owns
     * manages this connection.
     */
    struct Connection_t *connection;

    /**
     * Callback function reference to be called
     * when data is available for processing.
     */
    dataIoConnectionCallback onData;

    /**
     * Callback function reference to be called
     * when a connection is has been open.
     */
    ioConnectionCallback onOpen;

    /**
     * Callback function reference to be called
     * when a connection is going to be closed.
     */
    ioConnectionCallback onClose;

    /**
     * Reference to the lower level
     * stream substrate (child).
     */
    void *lower;
} IoConnection;

void createIoConnection(struct IoConnection_t **ioConnectionPointer, struct Connection_t *connection);
void deleteIoConnection(struct IoConnection_t *ioConnection);
ERROR_CODE acceptHandlerStreamIo(struct Connection_t *connection);
ERROR_CODE readHandlerStreamIo(struct Connection_t *connection);
ERROR_CODE writeHandlerStreamIo(struct Connection_t *connection);
ERROR_CODE errorHandlerStreamIo(struct Connection_t *connection);
ERROR_CODE openHandlerStreamIo(struct Connection_t *connection);
ERROR_CODE closeHandlerStreamIo(struct Connection_t *connection);
