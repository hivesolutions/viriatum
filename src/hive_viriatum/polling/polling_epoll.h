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

#pragma once

#include "../system/system.h"

#ifdef VIRIATUM_EPOLL

typedef struct polling_epoll_t {
    /**
     * The polling reference, for top
     * level reference.
     */
    struct polling_t *polling;

    /**
     * The file descriptor tp the object
     * that control the epoll structure.
     * This object is used for each polling
     * operation to be done.
     */
    int epoll_fd;

    /**
     * Counter that controls the number of events
     * currently present in the epoll structure.
     * This counter is very usefull for debugging
     * purposes and printing information.
     */
    size_t poll_count;

    /**
     * The buffer that holds the connections
     * with data available for read.
     */
    struct connection_t *read_connections[VIRIATUM_MAX_EVENTS];

    /**
     * The buffer that holds the connections
     * with data available for write.
     */
    struct connection_t *write_connections[VIRIATUM_MAX_EVENTS];

    /**
     * The buffer that contains the various
     * connections that have data pending to
     * be writen to the socket at the begining
     * of the next poll operation.
     */
    struct connection_t *write_outstanding[VIRIATUM_MAX_EVENTS];

    /**
     * The buffer that holds the connections
     * that are in an erroneous state
     */
    struct connection_t *error_connections[VIRIATUM_MAX_EVENTS];

    /**
     * The buffer thatn holds the connections
     * to be house-kept (removed) at the end
     * of the polling cycle.
     */
    struct connection_t *remove_connections[VIRIATUM_MAX_EVENTS];

    /**
     * The size as items of the sequence of
     * connection that are pending to be writen
     * at he beginning of the poll operation.
     */
    size_t write_outstanding_size;

    /**
     * The size of the read connections
     * buffer.
     */
    size_t read_connections_size;

    /**
     * The size of the write connections
     * buffer.
     */
    size_t write_connections_size;

    /**
     * The size of the error connections
     * buffer.
     */
    size_t error_connections_size;

    /**
     * The size of the remove connections
     * buffer.
     */
    size_t remove_connections_size;
} polling_epoll;

void create_polling_epoll(struct polling_epoll_t **polling_epoll_pointer, struct polling_t *polling);
void delete_polling_epoll(struct polling_epoll_t *polling_epoll);
ERROR_CODE open_polling_epoll(struct polling_t *polling);
ERROR_CODE close_polling_epoll(struct polling_t *polling);
ERROR_CODE register_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_connection_polling_epoll(
    struct polling_t *polling,
    struct connection_t *connection,
    unsigned char remove
);
ERROR_CODE register_write_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_write_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE poll_polling_epoll(struct polling_t *polling);
ERROR_CODE call_polling_epoll(struct polling_t *polling);
ERROR_CODE _poll_polling_epoll(
    struct polling_epoll_t *polling_epoll,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t *read_connections_size,
    size_t *write_connections_size,
    size_t *error_connections_size
);
ERROR_CODE _call_polling_epoll(
    struct polling_epoll_t *polling_epoll,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t read_connections_size,
    size_t write_connections_size,
    size_t error_connections_size
);
ERROR_CODE _outstanding_polling_epoll(
    struct polling_epoll_t *polling_epoll,
    struct connection_t **write_outstanding,
    size_t write_outstanding_size
);

#endif
