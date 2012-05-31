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

#include "stdafx.h"

#include "stream_torrent.h"

#define TORRENT_PROTOCOL_STRING "BitTorrent protocol"
#define TORRENT_PROTOCOL_SIZE 19

typedef struct TorrentHandshake_t {
    unsigned char pstrlen;
    char pstr[TORRENT_PROTOCOL_SIZE];
    unsigned char reserved[8];
    unsigned char info_hash[20];
    unsigned char peer_id[20];
} TorrentHandshake;

ERROR_CODE createTorrentConnection(struct TorrentConnection_t **torrentConnectionPointer, struct IoConnection_t *ioConnection) {
    /* retrieves the torrent connection size */
    size_t torrentConnectionSize = sizeof(struct TorrentConnection_t);

    /* allocates space for the torrent connection */
    struct TorrentConnection_t *torrentConnection = (struct TorrentConnection_t *) MALLOC(torrentConnectionSize);

    /* retrieves the service associated with the connection */
    struct Service_t *service = ioConnection->connection->service;

    /* sets the torrent handler attributes (default) values */
    torrentConnection->ioConnection = ioConnection;
    torrentConnection->torrentHandler = NULL;

    /* sets the torrent connection in the (upper) io connection substrate */
    ioConnection->lower = torrentConnection;

    /* retrieves the current (default) service handler and sets the
    connection on it, then sets this handler as the base handler */
  /*  torrentHandler = service->torrentHandler;
    torrentConnection->baseHandler = torrentHandler;*/

    /* sets the torrent connection in the torrent connection pointer */
    *torrentConnectionPointer = torrentConnection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteTorrentConnection(struct TorrentConnection_t *torrentConnection) {
    /* releases the torrent connection */
    FREE(torrentConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE dataHandlerStreamTorrent(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize) {
    char *_buffer = (char *) MALLOC(bufferSize + 1);

    memcpy(_buffer, buffer, bufferSize);
    _buffer[bufferSize] = '\0';

    printf("'%s'", _buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE openHandlerStreamTorrent(struct IoConnection_t *ioConnection) {
	/* allocates space for the data to be read from the
	info hash file */
	char *data;

    /* allocates the torrent connection */
    struct TorrentConnection_t *torrentConnection;

    /* allocates the response buffer */
    struct TorrentHandshake_t *responseBuffer = (struct TorrentHandshake_t *) MALLOC(sizeof(struct TorrentHandshake_t));

    FILE *file = fopen("C:/info_hash.txt", "rb");
	if(file == NULL) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, "Problem opening hash file"); }
    data = (char *) fread(responseBuffer->info_hash, 1, 20, file);
    fclose(file);


    /* creates the torrent connection */
    createTorrentConnection(&torrentConnection, ioConnection);



    responseBuffer->pstrlen = TORRENT_PROTOCOL_SIZE;
    memcpy(responseBuffer->pstr, TORRENT_PROTOCOL_STRING, TORRENT_PROTOCOL_SIZE);
    memset(responseBuffer->reserved, 0, 8);
    /*memcpy(responseBuffer->info_hash, "-AZ4702-UCahr9VNImUy", 20);*/
    memcpy(responseBuffer->peer_id, "-AZ4702-UCJhrsVNImUy", 20);


    /* writes the response to the connection, registers for the appropriate callbacks */
    writeConnection(ioConnection->connection, (unsigned char *) responseBuffer, sizeof(struct TorrentHandshake_t), NULL, NULL);


    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeHandlerStreamTorrent(struct IoConnection_t *ioConnection) {
    /* retrieves the torrent connection */
    struct TorrentConnection_t *torrentConnection = (struct TorrentConnection_t *) ioConnection->lower;

    /* deletes the torrent connection */
    deleteTorrentConnection(torrentConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}
