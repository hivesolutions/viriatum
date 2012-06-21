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

#include "polling_select.h"

void create_polling_select(struct polling_select_t **polling_select_pointer, struct polling_t *polling) {
    /* retrieves the polling select size */
    size_t polling_select_size = sizeof(struct polling_select_t);

    /* retrieves the connection pointer size */
    size_t connection_pointer_size = sizeof(struct connection_t *);

    /* allocates space for the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) MALLOC(polling_select_size);

    /* resets the polling in the polling select */
    polling_select->polling = polling;

    /* resets the sockets set highest value */
    polling_select->sockets_set_highest = 0;

    /* zeros the sockets read set */
    SOCKET_SET_ZERO(&polling_select->sockets_read_set);

    /* zeros the sockets write set */
    SOCKET_SET_ZERO(&polling_select->sockets_write_set);

    /* zeros the sockets error set */
    SOCKET_SET_ZERO(&polling_select->sockets_error_set);

    /* zeros the sockets read set temporary */
    SOCKET_SET_ZERO(&polling_select->sockets_read_set_temporary);

    /* zeros the sockets write set temporary */
    SOCKET_SET_ZERO(&polling_select->sockets_write_set_temporary);

    /* zeros the sockets error set temporary */
    SOCKET_SET_ZERO(&polling_select->sockets_error_set_temporary);

    /* allocates the read connection for internal
    polling select usage */
    polling_select->read_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* allocates the write connection for internal
    polling select usage */
    polling_select->write_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* allocates the error connection for internal
    polling select usage */
    polling_select->error_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* allocates the remove connection for internal
    polling select usage */
    polling_select->remove_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* sets the default timeout */
    polling_select->select_timeout.tv_sec = VIRIATUM_SELECT_TIMEOUT;
    polling_select->select_timeout.tv_usec = 0;

    /* sets the polling select in the polling select pointer */
    *polling_select_pointer = polling_select;
}

void delete_polling_select(struct polling_select_t *polling_select) {
    /* releases the remove connections */
    FREE(polling_select->remove_connections);

    /* releases the error connection */
    FREE(polling_select->error_connections);

    /* releases the write connection */
    FREE(polling_select->write_connections);

    /* releases the read connection */
    FREE(polling_select->read_connections);

    /* releases the polling select */
    FREE(polling_select);
}

ERROR_CODE open_polling_select(struct polling_t *polling) {
    /* allocates the polling select */
    struct polling_select_t *polling_select;

    /* creates the polling select */
    create_polling_select(&polling_select, polling);

    /* sets the polling select in the polling as
    the lower substrate */
    polling->lower = (void *) polling_select;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_polling_select(struct polling_t *polling) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* deletes the polling select */
    delete_polling_select(polling_select);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_connection_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* registers the socket handle in the sockets read set */
    _register_sockets_set_polling_select(polling_select, connection->socket_handle, &polling_select->sockets_read_set);

    /* in case the socket error are meant to be processed */
    if(VIRIATUM_SOCKET_ERROR) {
        /* registers the socket handle in the sockets error set */
        _register_sockets_set_polling_select(polling_select, connection->socket_handle, &polling_select->sockets_read_set);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_connection_polling_select(struct polling_t *polling, struct connection_t *connection)  {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* unregister the socket handle from the sockets read set */
    _unregister_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_read_set
    );

    /* in case the socket error are meant to be processed */
    if(VIRIATUM_SOCKET_ERROR) {
        /* unregister the socket handle from the sockets error set */
        _unregister_sockets_set_polling_select(
            polling_select,
            connection->socket_handle,
            &polling_select->sockets_error_set
        );
    }

    /* in case the connection write is registered */
    if(connection->write_registered == 1) {
        /* unregister the socket handle from the sockets write set */
        _unregister_sockets_set_polling_select(
            polling_select,
            connection->socket_handle,
            &polling_select->sockets_write_set
        );
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* in case the connection is write registered */
    if(connection->write_registered == 1) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Connection already registered");
    }

    /* register the socket handle from the sockets write set */
    _register_sockets_set_polling_select(polling_select, connection->socket_handle, &polling_select->sockets_write_set);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* in case the connection is not write registered */
    if(connection->write_registered == 0) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Connection already unregistered");
    }

    /* unregister the socket handle from the sockets write set */
    _unregister_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_write_set
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE poll_polling_select(struct polling_t *polling) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* polls the polling select */
    _poll_polling_select(
        polling_select,
        polling_select->read_connections,
        polling_select->write_connections,
        polling_select->error_connections,
        &polling_select->read_connections_size,
        &polling_select->write_connections_size,
        &polling_select->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE call_polling_select(struct polling_t *polling) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* calls the polling select */
    _call_polling_select(
        polling_select,
        polling_select->read_connections,
        polling_select->write_connections,
        polling_select->error_connections,
        polling_select->remove_connections,
        polling_select->read_connections_size,
        polling_select->write_connections_size,
        polling_select->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _poll_polling_select(struct polling_select_t *polling_select, struct connection_t **read_connections, struct connection_t **write_connections, struct connection_t **error_connections, unsigned int *read_connections_size, unsigned int *write_connections_size, unsigned int *error_connections_size) {
    /* allocates space for the select count */
    int select_count;

    /* allocates space for the current connection */
    struct connection_t *current_connection;

    /* allocates space for the connections list iterator */
    struct iterator_t *connections_list_iterator;

    /* retrieves the service */
    struct service_t *service = polling_select->polling->service;

    /* retrieves the service connections list */
    struct linked_list_t *connections_list = service->connections_list;

    /* initializes the read index */
    unsigned int read_index = 0;

    /* initializes the write index */
    unsigned int write_index = 0;

    /* initializes the error index */
    unsigned int error_index = 0;

    /* copies the socket sets to the socket sets temporary */
    polling_select->sockets_read_set_temporary = polling_select->sockets_read_set;
    polling_select->sockets_write_set_temporary = polling_select->sockets_write_set;

    /* copies the select timeout to the select timeout temporary */
    polling_select->select_timeout_temporary = polling_select->select_timeout;

    /* creates the iterator for the linked list */
    create_iterator_linked_list(connections_list, &connections_list_iterator);

    /* prints a debug message */
    V_DEBUG("Entering select statement\n");

    /* runs the select over the sockets set */
    select_count = SOCKET_SELECT(
        polling_select->sockets_set_highest + 1,
        &polling_select->sockets_read_set_temporary,
        &polling_select->sockets_write_set_temporary,
        &polling_select->sockets_error_set_temporary,
        &polling_select->select_timeout_temporary
    );

    /* prints a debug message */
    V_DEBUG_F("Exiting select statement with value: %d\n", select_count);

    /* in case there was an error in select */
    if(SOCKET_TEST_ERROR(select_count)) {
        /* retrieves the select error code */
        SOCKET_ERROR_CODE select_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints an info message */
        V_INFO_F("Problem running select: %d\n", select_error_code);

        /* resets the values for the various read values,
        this avoid possible problems in next actions */
        *read_connections_size = 0;
        *write_connections_size = 0;
        *error_connections_size = 0;

        /* closes the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem running select");
    }

    /* iterates continuously */
    while(1) {
        /* in case the select count is zero, time to break the
        current loop of iteration */
        if(select_count == 0) { break; }

        /* retrieves the next value from the iterator */
        get_next_iterator(connections_list_iterator, (void **) &current_connection);

        /* in case the current connection is null (end of iterator)
        should break the loop */
        if(current_connection == NULL) { break; }

        /* prints a debug message */
        V_DEBUG_F("Testing file for select: %d\n", current_connection->socket_handle);

        /* in case the current connection socket handle is set in
        the sockets read ready set */
        if(SOCKET_SET_IS_SET(current_connection->socket_handle, &polling_select->sockets_read_set_temporary) > 0)  {
            /* sets the current connection in the read connections */
            read_connections[read_index] = current_connection;

            /* increments the read index and the select count (one
            more read and one more count) */
            read_index++;
            select_count--;
        }

        /* in case the current connection socket handle is set in
        the sockets write ready set */
        if(SOCKET_SET_IS_SET(current_connection->socket_handle, &polling_select->sockets_write_set_temporary) > 0)  {
            /* sets the current connection in the write connections */
            write_connections[write_index] = current_connection;

            /* increments the write index and the select count (one
            more write and one more count) */
            write_index++;
            select_count--;
        }

        /* in case the current connection socket handle is set in
        the sockets error ready set */
        if(SOCKET_SET_IS_SET(current_connection->socket_handle, &polling_select->sockets_error_set_temporary) > 0)  {
            /* sets the current connection in the error connections */
            error_connections[error_index] = current_connection;

            /* increments the error index and the select count (one
            more error and one more count) */
            error_index++;
            select_count--;
        }
    }

    /* prints a debug message */
    V_DEBUG("Finished file testing\n");

    /* deletes the iterator linked list */
    delete_iterator_linked_list(connections_list, connections_list_iterator);

    /* prints a debug message */
    V_DEBUG("Deleted iterator linked list\n");

    /* in case the select count is bigger than zero */
    if(select_count > 0) {
        /* prints a debug message */
        V_DEBUG_F("Extraordinary select file descriptors not found: %d\n", select_count);
    }

    /* updates the various operation counters for the three
	operation to be "polled" (this is done by reference) */
    *read_connections_size = read_index;
    *write_connections_size = write_index;
    *error_connections_size = error_index;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _call_polling_select(struct polling_select_t *polling_select, struct connection_t **read_connections, struct connection_t **write_connections, struct connection_t **error_connections, struct connection_t **remove_connections, unsigned int read_connections_size, unsigned int write_connections_size, unsigned int error_connections_size) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current connection */
    struct connection_t *current_connection;

    /* resets the remove connections size */
    unsigned int remove_connections_size = 0;

    /* prints a debug message */
    V_DEBUG_F("Processing %d read connections\n", read_connections_size);

    /* iterates over the read connections */
    for(index = 0; index < read_connections_size; index++) {
        /* retrieves the current connection */
        current_connection = read_connections[index];

        /* prints a debug message */
        V_DEBUG_F("Processing read connection: %d\n", current_connection->socket_handle);

        /* in case the current connection is open and the read
        handler is correclty set (must call it) */
        if(current_connection->status == STATUS_OPEN && current_connection->on_read != NULL) {
            /* prints a debug message */
            V_DEBUG("Calling on read handler\n");

            /* calls the on read handler */
            current_connection->on_read(current_connection);

            /* prints a debug message */
            V_DEBUG("Finished calling on read handler\n");
        }

        /* in case the current connection is in the handshake
        section and the handshake handler is correclty set (must
        call it to initialize the connection) */
        if(current_connection->status == STATUS_HANDSHAKE && current_connection->on_handshake != NULL) {
            /* prints a debug message */
            V_DEBUG("Calling on handshake handler\n");

            /* calls the on read handler */
            current_connection->on_handshake(current_connection);

            /* prints a debug message */
            V_DEBUG("Finished calling on read handler\n");
        }

        /* in case the current connection is closed */
        if(current_connection->status == STATUS_CLOSED) {
            /* tries to add the current connection to the remove connections list */
            remove_connection(remove_connections, &remove_connections_size, current_connection);
        }
    }

    /* prints a debug message */
    V_DEBUG_F("Processing %d write connections\n", write_connections_size);

    /* iterates over the write connections */
    for(index = 0; index < write_connections_size; index++) {
        /* retrieves the current connection */
        current_connection = write_connections[index];

        /* prints a debug message */
        V_DEBUG_F("Processing write connection: %d\n", current_connection->socket_handle);

        /* in case the current connection is open */
        if(current_connection->status == STATUS_OPEN && current_connection->on_write != NULL) {
            /* prints a debug message */
            V_DEBUG("Calling on write handler\n");

            /* calls the on write handler */
            current_connection->on_write(current_connection);

            /* prints a debug message */
            V_DEBUG("Finished calling on write handler\n");
        }

        /* in case the current connection is closed */
        if(current_connection->status == STATUS_CLOSED) {
            /* tries to add the current connection to the remove connections list */
            remove_connection(remove_connections, &remove_connections_size, current_connection);
        }
    }

    /* prints a debug message */
    V_DEBUG_F("Processing %d error connections\n", error_connections_size);

    /* iterates over the error connections */
    for(index = 0; index < error_connections_size; index++) {
        /* retrieves the current connection */
        current_connection = error_connections[index];

        /* prints a debug message */
        V_DEBUG_F("Processing error connection: %d\n", current_connection->socket_handle);

        /* in case the current connection is open */
        if(current_connection->status == STATUS_OPEN && current_connection->on_error != NULL) {
            /* prints a debug message */
            V_DEBUG("Calling on error handler\n");

            /* calls the on error handler */
            current_connection->on_error(current_connection);

            /* prints a debug message */
            V_DEBUG("Finished calling on error handler\n");
        }

        /* in case the current connection is closed */
        if(current_connection->status == STATUS_CLOSED) {
            /* tries to add the current connection to the remove connections list */
            remove_connection(remove_connections, &remove_connections_size, current_connection);
        }
    }

    /* iterates over the remove connections */
    for(index = 0; index < remove_connections_size; index++) {
        /* retrieves the current connection */
        current_connection = remove_connections[index];

        /* deletes the current connection (house keeping) */
        delete_connection(current_connection);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _register_sockets_set_polling_select(struct polling_select_t *polling_select, SOCKET_HANDLE socket_handle,  SOCKET_SET *sockets_set) {
    /* in case the current socket handle is bigger than the polling
    select sockets set highest value */
    if(socket_handle > polling_select->sockets_set_highest) {
        /* sets the socket handle as the sockets set highest */
        polling_select->sockets_set_highest = socket_handle;
    }

    /* adds the socket handle to the sockets set */
    SOCKET_SET_SET(socket_handle, sockets_set);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unregister_sockets_set_polling_select(struct polling_select_t *polling_select, SOCKET_HANDLE socket_handle,  SOCKET_SET *sockets_set) {
    /* removes the socket handle from the sockets set */
    SOCKET_SET_CLEAR(socket_handle, sockets_set);

    /* raises no error */
    RAISE_NO_ERROR;
}
