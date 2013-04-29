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

#ifdef VIRIATUM_EPOLL

#include "polling_epoll.h"

void create_polling_epoll(struct polling_epoll_t **polling_epoll_pointer, struct polling_t *polling) {
    /* retrieves the polling epoll size */
    size_t polling_epoll_size = sizeof(struct polling_epoll_t);

    /* allocates space for the polling epoll */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) MALLOC(polling_epoll_size);

    /* initializes the polling epoll structure default values
    so that the default setting is used by default */
    polling_epoll->poll_count = 0;

    /* resets the polling in the polling epoll */
    polling_epoll->polling = polling;

    /* creates the epoll file descriptor with an arbitrary
    size (this no longer used by the kernel) */
    polling_epoll->epoll_fd = epoll_create(VIRIATUM_MAX_EVENTS);

    /* sets the polling epoll in the polling epoll pointer */
    *polling_epoll_pointer = polling_epoll;
}

void delete_polling_epoll(struct polling_epoll_t *polling_epoll) {
    /* in case the polling fd is defined it must be closed
    in order to avoid any memory leak problem */
    if(polling_epoll->epoll_fd) { close(polling_epoll->epoll_fd); }

    /* releases the polling epoll */
    FREE(polling_epoll);
}

ERROR_CODE open_polling_epoll(struct polling_t *polling) {
    /* allocates the polling epoll */
    struct polling_epoll_t *polling_epoll;

    /* creates the polling epoll */
    create_polling_epoll(&polling_epoll, polling);

    /* sets the polling epoll in the polling as
    the lower substrate */
    polling->lower = (void *) polling_epoll;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_polling_epoll(struct polling_t *polling) {
    /* retrieves the polling epoll */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* deletes the polling epoll */
    delete_polling_epoll(polling_epoll);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection) {
    /* allocates space for the result of the poll call
    to add a new element to the poll structure */
    SOCKET_ERROR_CODE result_code;

    /* allocates space for the new event to be inserted into
    the epoll polling system (this is an internal kernel structure) */
    struct epoll_event _event;

    /* retrieves the polling epoll structure from the upper
    polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* populates the event structure with the appropriate
    structures in order to register for the right events
    and then inserts the event request into the epoll fd */
    _event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    _event.data.ptr = (void *) connection;
    result_code = epoll_ctl(
	    polling_epoll->epoll_fd,
		EPOLL_CTL_ADD,
		connection->socket_handle,
		&_event
	);

    /* in case there was an error in epoll need to correctly
    handle it and propagate it to the caller */
    if(SOCKET_TEST_ERROR(result_code)) {
        SOCKET_ERROR_CODE epoll_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_INFO_F("Problem registering connection epoll: %d\n", epoll_error_code);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem registering connection epoll");
    }

    /* increments the counter that controls the number of events
    currently in the polling state in the epoll */
    polling_epoll->poll_count++;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection)  {
    /* allocates space for the result of the poll call
    to add a new element to the poll structure */
    SOCKET_ERROR_CODE result_code;

    /* retrieves the polling epoll structure from the upper
    polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* removes the associated socket handle (fd) from the epoll
    structure to avoid any leak */
    result_code = epoll_ctl(polling_epoll->epoll_fd, EPOLL_CTL_DEL, connection->socket_handle, NULL);

    /* in case there was an error in epoll need to correctly
    handle it and propagate it to the caller */
    if(SOCKET_TEST_ERROR(result_code)) {
        SOCKET_ERROR_CODE epoll_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_INFO_F("Problem unregistering connection epoll: %d\n", epoll_error_code);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem unregistering connection epoll");
    }

    /* decrements the counter that controls the number of events
    currently in the polling state in the epoll */
    polling_epoll->poll_count--;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_polling_epoll(struct polling_t *polling, struct connection_t *connection) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_polling_epoll(struct polling_t *polling, struct connection_t *connection)  {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE poll_polling_epoll(struct polling_t *polling) {
    /* retrieves the polling epoll structure from the upper
    polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* polls the polling epoll */
    _poll_polling_epoll(
        polling_epoll,
        polling_epoll->read_connections,
        polling_epoll->write_connections,
        polling_epoll->error_connections,
        &polling_epoll->read_connections_size,
        &polling_epoll->write_connections_size,
        &polling_epoll->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE call_polling_epoll(struct polling_t *polling) {
    /* retrieves the polling epoll structure from the upper
    polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* calls the polling epoll */
    _call_polling_epoll(
        polling_epoll,
        polling_epoll->read_connections,
        polling_epoll->write_connections,
        polling_epoll->error_connections,
        polling_epoll->remove_connections,
        polling_epoll->read_connections_size,
        polling_epoll->write_connections_size,
        polling_epoll->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _poll_polling_epoll(struct polling_epoll_t *polling_epoll, struct connection_t **read_connections, struct connection_t **write_connections, struct connection_t **error_connections, size_t *read_connections_size, size_t *write_connections_size, size_t *error_connections_size) {
    /* allocates space for the buffer to hold the various
    events generated from the wait call and for the event
    pointer for the iteration cycle */
    struct epoll_event events[VIRIATUM_MAX_EVENTS];
    struct epoll_event *_event;

    /* allocates space for the index counter to be used in
    the iteration and for the counter of event from the wait */
    int index;
    int event_count;

    /* allocates space for the reference to the connection
    to eb used in the iteration cycle (temporary object) */
    struct connection_t *connection;

    /* starts the various temporary index counters for
    the various types of socket operations */
    size_t read_index = 0;
    size_t write_index = 0;
    size_t error_index = 0;

    /* retrieves the service associated with the current polling
    structure in order to be able to modify it */
    struct service_t *service = polling_epoll->polling->service;

    /* prints a debug message (include the poll count) */
    V_DEBUG_F("Entering epoll statement (%lu)\n", polling_epoll->poll_count);

    /* runs the wait process in the epoll, this is the main call
    of the epoll loop as it si the on responsible for the polling
    operation and generation of the events */
#ifdef VIRIATUM_PLATFORM_ANDROID
    event_count = epoll_wait(
		polling_epoll->epoll_fd,
		events,
		VIRIATUM_MAX_EVENTS,
		VIRIATUM_SELECT_TIMEOUT * 1000
	);
#else
	event_count = epoll_wait(
		polling_epoll->epoll_fd,
		events,
		VIRIATUM_MAX_EVENTS,
		-1
	);
#endif

    /* prints a debug message */
    V_DEBUG_F("Exiting epoll statement with value: %d\n", event_count);

	/* in case there was an error in epoll, in case there was this is
    considered to be a critical error */
    if(SOCKET_TEST_ERROR(event_count)) {
        /* retrieves the epoll error code */
        SOCKET_ERROR_CODE epoll_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints an info message */
        V_INFO_F("Problem running epoll: %d\n", epoll_error_code);

        /* resets the values for the various read values,
        this avoid possible problems in next actions */
        *read_connections_size = 0;
        *write_connections_size = 0;
        *error_connections_size = 0;

        /* closes the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem running epoll");
    }

    /* iterates over the range of "generated" events in order
    to correctly handle them to the upper levels */
    for(index = 0; index < event_count; index++) {
        /* retrieves the current event in iteration (epoll structure)
        and uses it to retrieve the associated pointer to the connection
        for event resolution */
        _event = &events[index];
        connection = (struct connection_t *) _event->data.ptr;

        /* checks if the event is of type input (read) must be
        added to the read operations queue */
        if(_event->events & EPOLLIN) {
            /* sets the current connection in the read connections
            and then increments the read index counter */
            read_connections[read_index] = connection;
            read_index++;
        }

        /* checks if the event is of type output (write) must be
        added to the write operations queue */
        if(_event->events & EPOLLOUT) {
            /* sets the current connection in the write connections
            and then increments the write index counter */
            write_connections[write_index] = connection;
            write_index++;
        }

        /* checks if the event is of type exception (error) must be
        added to the error operations queue */
        if(_event->events & (EPOLLERR | EPOLLHUP)) {
            /* sets the current connection in the error connections
            and then increments the error index counter */
            error_connections[error_index] = connection;
            error_index++;
        }
    }

    /* updates the various operation counters for the three
    operations to be "polled" (this is done by reference) */
    *read_connections_size = read_index;
    *write_connections_size = write_index;
    *error_connections_size = error_index;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _call_polling_epoll(struct polling_epoll_t *polling_epoll, struct connection_t **read_connections, struct connection_t **write_connections, struct connection_t **error_connections, struct connection_t **remove_connections, size_t read_connections_size, size_t write_connections_size, size_t error_connections_size) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current connection */
    struct connection_t *current_connection;

    /* resets the remove connections size */
    unsigned int remove_connections_size = 0;

    /* prints a debug message */
    V_DEBUG_F("Processing %lu read connections\n", (long unsigned int) read_connections_size);

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
            CALL_V(current_connection->on_read, current_connection);

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
            CALL_V(current_connection->on_handshake, current_connection);

            /* prints a debug message */
            V_DEBUG("Finished calling on handshake handler\n");
        }

        /* in case the current connection is closed */
        if(current_connection->status == STATUS_CLOSED) {
            /* tries to add the current connection to the remove connections list */
            remove_connection(remove_connections, &remove_connections_size, current_connection);
        }
    }

    /* prints a debug message */
    V_DEBUG_F("Processing %lu write connections\n", (long unsigned int) write_connections_size);

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
            CALL_V(current_connection->on_write, current_connection);

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
    V_DEBUG_F("Processing %lu error connections\n", (long unsigned int) error_connections_size);

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
            CALL_V(current_connection->on_error, current_connection);

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

#endif
