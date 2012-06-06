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

#include "../system/system.h"

typedef struct polling_select_t {
    /**
     * The polling reference, for top
     * level reference.
     */
    struct Polling_t *polling;

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
    struct Connection_t **read_connections;

    /**
     * The buffer that holds the connections
     * with data available for write.
     */
    struct Connection_t **write_connections;

    /**
     * The buffer that holds the connections
     * that are in an erroneous state
     */
    struct Connection_t **error_connections;

    /**
     * The buffer thatn holds the connections
     * to be house-jept (removed) at the end
     * of the polling cycle.
     */
    struct Connection_t **remove_connections;

    /**
     * The size of the read connections
     * buffer.
     */
    unsigned int read_connections_size;

    /**
     * The size of the write connections
     * buffer.
     */
    unsigned int write_connections_size;

    /**
     * The size of the error connections
     * buffer.
     */
    unsigned int error_connections_size;

    /**
     * The size of the remove connections
     * buffer.
     */
    unsigned int remove_connections_size;

    /**
     * The timeout value used for the
     * select call.
     */
    struct timeval select_timeout;

    /**
     * The temporary select timeout value.
     */
    struct timeval select_timeout_temporary;
} polling_select;

void create_polling_select(struct polling_select_t **polling_select_pointer, struct Polling_t *polling);
void delete_polling_select(struct polling_select_t *polling_select);
ERROR_CODE open_polling_select(struct Polling_t *polling);
ERROR_CODE close_polling_select(struct Polling_t *polling);
ERROR_CODE register_connection_polling_select(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE unregister_connection_polling_select(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE register_write_polling_select(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE unregister_write_polling_select(struct Polling_t *polling, struct Connection_t *connection);
ERROR_CODE poll_polling_select(struct Polling_t *polling);
ERROR_CODE call_polling_select(struct Polling_t *polling);
ERROR_CODE _poll_polling_select(struct polling_select_t *polling_select, struct Connection_t **read_connections, struct Connection_t **write_connections, struct Connection_t **error_connections, unsigned int *read_connections_size, unsigned int *write_connections_size, unsigned int *error_connections_size);
ERROR_CODE _call_polling_select(struct polling_select_t *polling_select, struct Connection_t **read_connections, struct Connection_t **write_connections, struct Connection_t **error_connections, struct Connection_t **remove_connections, unsigned int read_connections_size, unsigned int write_connections_size, unsigned int error_connections_size);
ERROR_CODE _register_sockets_set_polling_select(struct polling_select_t *polling_select, SOCKET_HANDLE socket_handle, SOCKET_SET *sockets_set);
ERROR_CODE _unregister_sockets_set_polling_select(struct polling_select_t *polling_select, SOCKET_HANDLE socket_handle, SOCKET_SET *sockets_set);

/**
 * Removes a connection from the remove connections array.
 * This method checks for duplicates and in case they exist, no
 * connection is added.
 *
 * @param remove_connections The list of connection for removal.
 * @param remove_connections_size_pointer A pointer to the sisze
 * of the remove connections array.
 * @param connection The connection to be removed (deleted).
 */
static __inline void remove_connection(struct Connection_t **remove_connections, unsigned int *remove_connections_size_pointer, struct Connection_t *connection) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current connection */
    struct Connection_t *current_connection;

    /* retrieves the remove connections size */
    unsigned int remove_connections_size = *remove_connections_size_pointer;

    /* iterates over the remove connections */
    for(index = 0; index < remove_connections_size; index++) {
        /* retrieves the current connection */
        current_connection = remove_connections[index];

        /* in case the current connection already exists */
        if(current_connection == connection) {
            /* returns immediately */
            return;
        }
    }

    /* adds the connection to the remove connections */
    remove_connections[remove_connections_size] = connection;

    /* increments the remove connections size */
    (*remove_connections_size_pointer)++;
}
