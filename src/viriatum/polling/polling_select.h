/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2014 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2014 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../system/system.h"

typedef struct polling_select_t {
    /**
     * The polling reference, for top
     * level reference.
     */
    struct polling_t *polling;

    /**
     * The value of the highest socket
     * reference (performance issue).
     */
    SOCKET_HANDLE sockets_set_highest;

    /**
     * The socket set for the read sockets.
     */
    SOCKET_SET sockets_read_set;

    /**
     * The socket set for the write sockets.
     */
    SOCKET_SET sockets_write_set;

    /**
     * The socket set for the error sockets.
     */
    SOCKET_SET sockets_error_set;

    /**
     * The temporary socket set for the
     * read sockets.
     */
    SOCKET_SET sockets_read_set_temporary;

    /**
     * The temporary socket set for the
     * write sockets.
     */
    SOCKET_SET sockets_write_set_temporary;

    /**
     * The temporary socket set for the
     * error sockets.
     */
    SOCKET_SET sockets_error_set_temporary;

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
     * The buffer that contains the various
     * connections that have data pending to
     * be read from the socket at the begining
     * of the next poll operation.
     */
    struct connection_t *read_outstanding[VIRIATUM_MAX_EVENTS];

    /**
     * The buffer that contains the various
     * connections that have data pending to
     * be writen to the socket at the begining
     * of the next poll operation.
     */
    struct connection_t *write_outstanding[VIRIATUM_MAX_EVENTS];

    /**
     * Auxiliary buffer to be used in the performing
     * of the outstaind read operations.
     */
    struct connection_t *_read_outstanding[VIRIATUM_MAX_EVENTS];

    /**
     * Auxiliary buffer to be used in the performing
     * of the outstaind write operations.
     */
    struct connection_t *_write_outstanding[VIRIATUM_MAX_EVENTS];

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

    /**
     * The size as items of the sequence of
     * connections that are pending to be read
     * at he beginning of the poll operation.
     */
    size_t read_outstanding_size;

    /**
     * The size as items of the sequence of
     * connections that are pending to be writen
     * at he beginning of the poll operation.
     */
    size_t write_outstanding_size;

    /**
     * The timeout value used for the
     * select call.
     */
    struct timeval select_timeout;

    /**
     * The temporary select timeout value, that
     * is used in the select operation in order
     * to avoid complete blocking in the operation.
     */
    struct timeval select_timeout_temporary;
} polling_select;

void create_polling_select(struct polling_select_t **polling_select_pointer, struct polling_t *polling);
void delete_polling_select(struct polling_select_t *polling_select);
ERROR_CODE open_polling_select(struct polling_t *polling);
ERROR_CODE close_polling_select(struct polling_t *polling);
ERROR_CODE register_connection_polling_select(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_connection_polling_select(
    struct polling_t *polling,
    struct connection_t *connection,
    unsigned char remove
);
ERROR_CODE register_read_polling_select(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_read_polling_select(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE register_write_polling_select(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_write_polling_select(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE add_outstanding_polling_select(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE poll_polling_select(struct polling_t *polling);
ERROR_CODE call_polling_select(struct polling_t *polling);
ERROR_CODE _poll_polling_select(
    struct polling_select_t *polling_select,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t *read_connections_size,
    size_t *write_connections_size,
    size_t *error_connections_size
);
ERROR_CODE _call_polling_select(
    struct polling_select_t *polling_select,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t read_connections_size,
    size_t write_connections_size,
    size_t error_connections_size
);
ERROR_CODE _outstanding_polling_select(
    struct polling_select_t *polling_select,
    struct connection_t **read_outstanding,
    struct connection_t **write_outstanding,
    struct connection_t **_read_outstanding,
    struct connection_t **_write_outstanding,
    size_t read_outstanding_size,
    size_t write_outstanding_size
);
ERROR_CODE _register_sockets_set_polling_select(
    struct polling_select_t *polling_select,
    SOCKET_HANDLE socket_handle,
    SOCKET_SET *sockets_set
);
ERROR_CODE _unregister_sockets_set_polling_select(
    struct polling_select_t *polling_select,
    SOCKET_HANDLE socket_handle,
    SOCKET_SET *sockets_set
);

/**
 * Removes a connection and insert it into the remove connections
 * array, in order for it to be removed in next main loop.
 * This method checks for duplicates and in case they exist, no
 * connection is added.
 *
 * @param remove_connections The list of connection for removal.
 * @param remove_connections_size_pointer A pointer to the sisze
 * of the remove connections array.
 * @param connection The connection to be removed (deleted).
 */
static __inline void remove_connection(
    struct connection_t **remove_connections,
    size_t *remove_connections_size_pointer,
    struct connection_t *connection
) {
    /* allocates the index counter for the interation and
    the space for the temporary connection pointer */
    size_t index;
    struct connection_t *current_connection;

    /* retrieves the remove connections size value from the
    providade pointer value (indirect value) */
    unsigned int remove_connections_size = *remove_connections_size_pointer;

    /* iterates over all the connections to be removed in order
    to find out if there's a duplicated value */
    for(index = 0; index < remove_connections_size; index++) {
        /* retrieves the current connection and in case the
        current connection already exists in the list of
        connections to be removed must return immediately
        in to avoid duplicated values (possible problems) */
        current_connection = remove_connections[index];
        if(current_connection == connection) { return; }
    }

    /* adds the connection to the remove connections and
    then increments the remove connections size */
    remove_connections[remove_connections_size] = connection;
    (*remove_connections_size_pointer)++;
}
