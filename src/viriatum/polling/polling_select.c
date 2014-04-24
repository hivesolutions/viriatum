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

#include "stdafx.h"

#include "polling_select.h"

void create_polling_select(struct polling_select_t **polling_select_pointer, struct polling_t *polling) {
    /* retrieves the polling select size */
    size_t polling_select_size = sizeof(struct polling_select_t);

    /* allocates space for the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) MALLOC(polling_select_size);

    /* resets the various internal values for the select
    that are going to be used in the polling */
    polling_select->polling = polling;
    polling_select->sockets_set_highest = 0;
    polling_select->read_connections_size = 0;
    polling_select->write_connections_size = 0;
    polling_select->error_connections_size = 0;
    polling_select->remove_connections_size = 0;
    polling_select->read_outstanding_size = 0;
    polling_select->write_outstanding_size = 0;

    /* zerifies the variuous socket sets that are going
    to be used during the select type polling */
    SOCKET_SET_ZERO(&polling_select->sockets_read_set);
    SOCKET_SET_ZERO(&polling_select->sockets_write_set);
    SOCKET_SET_ZERO(&polling_select->sockets_error_set);
    SOCKET_SET_ZERO(&polling_select->sockets_read_set_temporary);
    SOCKET_SET_ZERO(&polling_select->sockets_write_set_temporary);
    SOCKET_SET_ZERO(&polling_select->sockets_error_set_temporary);

    /* sets the default timeout */
    polling_select->select_timeout.tv_sec = VIRIATUM_SELECT_TIMEOUT;
    polling_select->select_timeout.tv_usec = 0;

    /* sets the polling select in the polling select pointer */
    *polling_select_pointer = polling_select;
}

void delete_polling_select(struct polling_select_t *polling_select) {
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
    /* allocates space for the temporary variable that will
    store the various index values in iteration */
    size_t index;
    struct connection_t *current_connection;

    /* retrieves the polling select structure as the concrete
    underlying substrate of the polling structure */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* iterates over the set of connections that are meant to
    be removed from the select list as they are no longer available */
    for(index = 0; index < polling_select->remove_connections_size; index++) {
        /* retrieves the current connection for the iteration
        and then deletes the current connection (house keeping) */
        current_connection = polling_select->remove_connections[index];
        delete_connection(current_connection);
    }

    /* deletes the polling select, avoiding any memory leaks
    occuring from the select structures */
    delete_polling_select(polling_select);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_connection_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* registers the socket handle in the sockets read set */
    _register_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_read_set
    );

    /* in case the socket error are meant to be processed */
    if(VIRIATUM_SOCKET_ERROR) {
        /* registers the socket handle in the sockets error set */
        _register_sockets_set_polling_select(
            polling_select,
            connection->socket_handle,
            &polling_select->sockets_read_set
        );
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_connection_polling_select(
    struct polling_t *polling,
    struct connection_t *connection,
    unsigned char remove_c
) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* unregister the socket handle from the sockets read set */
    _unregister_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_read_set
    );

    /* in case the socket error are meant to be processed, must
    unregister the socket handle from the sockets error set */
    if(VIRIATUM_SOCKET_ERROR) {
        _unregister_sockets_set_polling_select(
            polling_select,
            connection->socket_handle,
            &polling_select->sockets_error_set
        );
    }

    /* in case the connection write is currently registered, must
    unregister the socket handle from the sockets write set */
    if(connection->write_registered == TRUE) {
        _unregister_sockets_set_polling_select(
            polling_select,
            connection->socket_handle,
            &polling_select->sockets_write_set
        );
    }

    /* in case the remove connection flag is set the connection
    is also added to the list of connections to be removed
    after the io driven logic part is processed (at the end
    of the logic loop) */
    if(remove_c == TRUE) {
        remove_connection(
            polling_select->remove_connections,
            &polling_select->remove_connections_size,
            connection
        );
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_read_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select structure as the lower
    structure from the provided polling object */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* in case the connection is read registered must
    raise an error indicating the problem */
    if(connection->read_registered == TRUE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Connection already registered"
        );
    }

    /* register the socket handle from the sockets read set */
    _register_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_read_set
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_read_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select structure as the lower
    structure from the provided polling object */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* in case the connection is not read registered must
    raise an error indicating the problem */
    if(connection->read_registered == FALSE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Connection already unregistered"
        );
    }

    /* unregister the socket handle from the sockets read set */
    _unregister_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_read_set
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select structure as the lower
    structure from the provided polling object */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* in case the connection is write registered must
    raise an error indicating the problem */
    if(connection->write_registered == TRUE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Connection already registered"
        );
    }

    /* register the socket handle from the sockets write set */
    _register_sockets_set_polling_select(
        polling_select,
        connection->socket_handle,
        &polling_select->sockets_write_set
    );

    /* in case the current state for the connection is not write
    valid must return immediately with no error */
    if(connection->write_valid == FALSE) { RAISE_NO_ERROR; }

    /* in case the current connection is not open or the on write
    callback function is not currently set must return */
    if(connection->status != STATUS_OPEN || connection->on_write == NULL) {
        RAISE_NO_ERROR;
    }

    /* sets the connection for the current outstanding position and
    then increments the size of the outstanding connection pending */
    polling_select->write_outstanding[polling_select->write_outstanding_size] = connection;
    polling_select->write_outstanding_size++;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select structure as the lower
    structure from the provided polling object */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* in case the connection is not write registered must
    raise an error indicating the problem */
    if(connection->write_registered == FALSE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Connection already unregistered"
        );
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

ERROR_CODE add_outstanding_polling_select(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling select structure as the lower
    structure from the provided polling object */
    struct polling_select_t *polling_select =\
        (struct polling_select_t *) polling->lower;

    /* in case the connection is already registered for outstanding
    read operations this request must be ignored */
    if(connection->is_outstanding == TRUE) { RAISE_NO_ERROR; }

    /* sets the connection for the current outstanding position and
    then increments the size of the outstanding connection pending */
    polling_select->read_outstanding[polling_select->read_outstanding_size] = connection;
    polling_select->read_outstanding_size++;

    /* sets the connection as outstanding as the connection has just
    been registered in the select polling mechanism for extra reads */
    connection->is_outstanding = TRUE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE poll_polling_select(struct polling_t *polling) {
    /* retrieves the polling select */
    struct polling_select_t *polling_select = (struct polling_select_t *) polling->lower;

    /* triggers the processing of the outstanding connection
    operations (pending operations) that meant to be done before
    the main poll operation blocks the control flow */
    _outstanding_polling_select(
        polling_select,
        polling_select->read_outstanding,
        polling_select->write_outstanding,
        polling_select->_read_outstanding,
        polling_select->_write_outstanding,
        polling_select->read_outstanding_size,
        polling_select->write_outstanding_size
    );

    /* polls the polling select, this call should block until
    either a timeout occurs or data is received */
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
        polling_select->read_connections_size,
        polling_select->write_connections_size,
        polling_select->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _poll_polling_select(
    struct polling_select_t *polling_select,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t *read_connections_size,
    size_t *write_connections_size,
    size_t *error_connections_size
) {
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

    /* initializes the various index values for the diferent
    operations to be polled in the current operation */
    unsigned int read_index = 0;
    unsigned int write_index = 0;
    unsigned int error_index = 0;

    /* copies the socket sets to the socket sets temporary */
    polling_select->sockets_read_set_temporary = polling_select->sockets_read_set;
    polling_select->sockets_write_set_temporary = polling_select->sockets_write_set;

    /* copies the select timeout to the select timeout temporary */
    polling_select->select_timeout_temporary = polling_select->select_timeout;

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

        /* in case the interupt error code has been received the
        system should fail gracefully to unblock the call */
        if(select_error_code == EINTR) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Interrupted system call in select"
            );
        }

        /* closes the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem running select"
        );
    }

    /* creates the iterator for the connections list that wil be used
    to iterate arround all the connections for the is set testing */
    create_iterator_linked_list(connections_list, &connections_list_iterator);

    /* iterates continuously */
    while(TRUE) {
        /* in case the select count is zero, time to break the
        current loop of iteration */
        if(select_count == 0) { break; }

        /* retrieves the next value from the iterator, this will
        provide the pointer to the next connection to be tested */
        get_next_iterator(
            connections_list_iterator,
            (void **) &current_connection
        );

        /* in case the current connection is null (end of iterator)
        should break the loop */
        if(current_connection == NULL) { break; }

        /* prints a debug message */
        V_DEBUG_F("Testing file for select: %d\n", current_connection->socket_handle);

        /* in case the current connection socket handle is set in
        the sockets read ready set */
        if(SOCKET_SET_IS_SET(
            current_connection->socket_handle,
            &polling_select->sockets_read_set_temporary
        ) > 0)  {
            /* sets the current connection in the read connections */
            read_connections[read_index] = current_connection;

            /* increments the read index and the select count (one
            more read and one more count) */
            read_index++;
            select_count--;
        }

        /* in case the current connection socket handle is set in
        the sockets write ready set */
        if(SOCKET_SET_IS_SET(
            current_connection->socket_handle,
            &polling_select->sockets_write_set_temporary
        ) > 0)  {
            /* sets the current connection in the write connections */
            write_connections[write_index] = current_connection;

            /* increments the write index and the select count (one
            more write and one more count) */
            write_index++;
            select_count--;
        }

        /* in case the current connection socket handle is set in
        the sockets error ready set */
        if(SOCKET_SET_IS_SET(
            current_connection->socket_handle,
            &polling_select->sockets_error_set_temporary
        ) > 0)  {
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

    /* in case the select count is bigger than zero, prints
    a debug message indicating the amount of read values */
    if(select_count > 0) {
        V_DEBUG_F(
            "Extraordinary select file descriptors not found: %d\n",
            select_count
        );
    }

    /* updates the various operation counters for the three
    operation to be "polled" (this is done by reference) */
    *read_connections_size = read_index;
    *write_connections_size = write_index;
    *error_connections_size = error_index;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _call_polling_select(
    struct polling_select_t *polling_select,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t read_connections_size,
    size_t write_connections_size,
    size_t error_connections_size
) {
    /* allocates the index, to be used during the iterations
    over the various connection bundles */
    size_t index;

    /* allocates the current connection */
    struct connection_t *current_connection;

    /* prints a debug message */
    V_DEBUG_F("Processing %lu read connections\n", (long unsigned int) read_connections_size);

    /* iterates over the complete set of read connections
    to run either the read operations on them, the handshake
    or event the close operation (no data received) */
    for(index = 0; index < read_connections_size; index++) {
        /* retrieves the current connection and then prints
        a debug message with the socket handle of the connection */
        current_connection = read_connections[index];
        V_DEBUG_F("Processing read connection: %d\n", current_connection->socket_handle);

        /* updates the current connection so that it's set as read
        valid meaning that any read operation in it will be immediately
        performed, fast operations */
        current_connection->read_valid = TRUE;

        /* in case the current connection is in the handshake
        section and the handshake handler is correclty set (must
        call it to initialize the connection) */
        if(current_connection->status == STATUS_HANDSHAKE &&\
            current_connection->on_handshake != NULL) {
            /* prints a series of debug messages about the handshake
            operation in it and then calls the hadhaske handler */
            V_DEBUG("Calling on handshake handler\n");
            CALL_V(current_connection->on_handshake, current_connection);
            V_DEBUG("Finished calling on handshake handler\n");
        }

        /* in case the current connection is open and the read
        handler is correclty set (must call it) */
        if(current_connection->status == STATUS_OPEN &&\
            current_connection->on_read != NULL) {
            /* prints a series of debug messages and then calls the
            correct on read handler for the notification */
            V_DEBUG("Calling on read handler\n");
            CALL_V(current_connection->on_read, current_connection);
            V_DEBUG("Finished calling on read handler\n");
        }
    }

    /* prints a debug message */
    V_DEBUG_F(
        "Processing %lu write connections\n",
        (long unsigned int) write_connections_size
    );

    /* iterates over all of the connection that are currently registerd
    for the write operation to correctly call their callbacks */
    for(index = 0; index < write_connections_size; index++) {
        /* retrieves the current connection and then prints
        a debug message with the socket handle of the connection */
        current_connection = write_connections[index];
        V_DEBUG_F("Processing write connection: %d\n", current_connection->socket_handle);

        /* updates the current connection so that it's set as write
        valid meaning that any write operation in it will be immediately
        performed, fast operations */
        current_connection->write_valid = TRUE;

        /* in case the current connection is open */
        if(current_connection->status == STATUS_OPEN &&\
            current_connection->on_write != NULL) {
            /* prints a series of debug messages and then calls the
            correct on write handler for the notification */
            V_DEBUG("Calling on write handler\n");
            CALL_V(current_connection->on_write, current_connection);
            V_DEBUG("Finished calling on write handler\n");
        }
    }

    /* prints a debug message */
    V_DEBUG_F("Processing %lu error connections\n", (long unsigned int) error_connections_size);

    /* iterates over all the connections that are currently set in an
    error state to escape from that state (via notification) */
    for(index = 0; index < error_connections_size; index++) {
        /* retrieves the current connection and then prints
        a debug message with the socket handle of the connection */
        current_connection = error_connections[index];
        V_DEBUG_F("Processing error connection: %d\n", current_connection->socket_handle);

        /* in case the current connection is open */
        if(current_connection->status == STATUS_OPEN &&\
            current_connection->on_error != NULL) {
            /* prints a series of debug messages and then calls the
            correct on error handler for the notification */
            V_DEBUG("Calling on error handler\n");
            CALL_V(current_connection->on_error, current_connection);
            V_DEBUG("Finished calling on error handler\n");
        }
    }

    /* iterates over the set of connections that are meant to
    be removed from the select list as they are no longer available */
    for(index = 0; index < polling_select->remove_connections_size; index++) {
        /* retrieves the current connection for the iteration
        and then deletes the current connection (house keeping) */
        current_connection = polling_select->remove_connections[index];
        delete_connection(current_connection);
    }

    /* resets the remove connections size to the default
    zero value, no connections are removed by default */
    polling_select->remove_connections_size = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _outstanding_polling_select(
    struct polling_select_t *polling_select,
    struct connection_t **read_outstanding,
    struct connection_t **write_outstanding,
    struct connection_t **_read_outstanding,
    struct connection_t **_write_outstanding,
    size_t read_outstanding_size,
    size_t write_outstanding_size
) {
    /* allocates the index, to be used during the iterations
    over the various connection bundles */
    size_t index;

    /* allocates the current connection, the temporary variable
    that will hold the unpacked connection on iteration */
    struct connection_t *current_connection;

    /* resets the current sizes associated with the outstanding
    events any new values coming from the event triggering will
    be set in a "fresh" set of outstanding buffers */
    polling_select->read_outstanding_size = 0;
    polling_select->write_outstanding_size = 0;

    /* runs the initial backup operation of the read outstanding
    operations to the extra buffer so that the outstanding operations
    may themselves change the read outstanding buffers */
    for(index = 0; index < read_outstanding_size; index++) {
        current_connection = read_outstanding[index];
        current_connection->is_outstanding = FALSE;
        _read_outstanding[index] = current_connection;
    }

    /* runs the initial backup operation of the write outstanding
    operations to the extra buffer so that the outstanding operation
    may themselves change the write outstanding buffers */
    for(index = 0; index < write_outstanding_size; index++) {
        current_connection = write_outstanding[index];
        _write_outstanding[index] = current_connection;
    }

    /* prints a debug message */
    V_DEBUG_F(
        "Processing %lu outstanding read connections\n",
        (long unsigned int) read_outstanding_size
    );

    /* iterates over all the connections that have outstanding
    read operations to be performed and triggers the on data
    event for each of them, starting the read operations */
    for(index = 0; index < read_outstanding_size; index++) {
        /* retrieves the current connection and then prints
        a debug message with the socket handle of the connection */
        current_connection = _read_outstanding[index];
        V_DEBUG_F(
            "Processing outstanding read connection: %d\n",
            current_connection->socket_handle
        );

        /* in case the current connection is open and the on read
        event handler is set performs the read call */
        if(current_connection->status == STATUS_OPEN &&\
            current_connection->on_read != NULL) {
            /* prints a series of debug messages and then calls the
            correct on read handler for the notification */
            V_DEBUG("Calling on read handler\n");
            CALL_V(current_connection->on_read, current_connection);
            V_DEBUG("Finished calling on read handler\n");
        }
    }

    /* prints a debug message */
    V_DEBUG_F(
        "Processing %lu outstanding write connections\n",
        (long unsigned int) write_outstanding_size
    );

    /* iterates over all the connections that have outstanding
    write operations to be performed and triggers the on write
    event for each of them, starting the write operations */
    for(index = 0; index < write_outstanding_size; index++) {
        /* retrieves the current connection and then prints
        a debug message with the socket handle of the connection */
        current_connection = _write_outstanding[index];
        V_DEBUG_F(
            "Processing outstanding write connection: %d\n",
            current_connection->socket_handle
        );

        /* in case the current connection is open and the on write
        event handler is set performs the write call */
        if(current_connection->status == STATUS_OPEN &&\
            current_connection->on_write != NULL) {
            /* prints a series of debug messages and then calls the
            correct on write handler for the notification */
            V_DEBUG("Calling on write handler\n");
            CALL_V(current_connection->on_write, current_connection);
            V_DEBUG("Finished calling on write handler\n");
        }
    }

    /* raises no error as the process of the outstanding
    connection operations has been successfull */
    RAISE_NO_ERROR;
}

ERROR_CODE _register_sockets_set_polling_select(
    struct polling_select_t *polling_select,
    SOCKET_HANDLE socket_handle,
    SOCKET_SET *sockets_set
) {
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

ERROR_CODE _unregister_sockets_set_polling_select(
    struct polling_select_t *polling_select,
    SOCKET_HANDLE socket_handle,
    SOCKET_SET *sockets_set
) {
    /* removes the socket handle from the sockets set */
    SOCKET_SET_CLEAR(socket_handle, sockets_set);

    /* raises no error */
    RAISE_NO_ERROR;
}
