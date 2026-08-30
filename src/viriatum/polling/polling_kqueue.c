/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#ifdef VIRIATUM_KQUEUE

#include "polling_kqueue.h"

void create_polling_kqueue(struct polling_kqueue_t **polling_kqueue_pointer, struct polling_t *polling) {
    /* retrieves the polling kqueue size */
    size_t polling_kqueue_size = sizeof(struct polling_kqueue_t);

    /* allocates space for the polling kqueue */
    struct polling_kqueue_t *polling_kqueue = (struct polling_kqueue_t *) MALLOC(polling_kqueue_size);

    /* initializes the polling kqueue structure default values
    so that the default setting is used by default */
    polling_kqueue->poll_count = 0;
    polling_kqueue->read_connections_size = 0;
    polling_kqueue->write_connections_size = 0;
    polling_kqueue->error_connections_size = 0;
    polling_kqueue->remove_connections_size = 0;
    polling_kqueue->read_outstanding_size = 0;
    polling_kqueue->write_outstanding_size = 0;

    /* resets the polling in the polling kqueue */
    polling_kqueue->polling = polling;

    /* creates the kqueue file descriptor, the queue of the kernel
    that the events of every connection are read out of */
    polling_kqueue->kqueue_fd = kqueue();

    /* sets the polling kqueue in the polling kqueue pointer */
    *polling_kqueue_pointer = polling_kqueue;
}

void delete_polling_kqueue(struct polling_kqueue_t *polling_kqueue) {
    /* in case the polling fd is defined it must be closed
    in order to avoid any memory leak problem */
    if(polling_kqueue->kqueue_fd != -1) { close(polling_kqueue->kqueue_fd); }

    /* releases the polling kqueue */
    FREE(polling_kqueue);
}

ERROR_CODE open_polling_kqueue(struct polling_t *polling) {
    /* allocates the polling kqueue */
    struct polling_kqueue_t *polling_kqueue;

    /* creates the polling kqueue */
    create_polling_kqueue(&polling_kqueue, polling);

    /* sets the polling kqueue in the polling as
    the lower substrate */
    polling->lower = (void *) polling_kqueue;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_polling_kqueue(struct polling_t *polling) {
    /* allocates space for the temporary variable that will
    store the various index values in iteration */
    size_t index;
    struct connection_t *current_connection;

    /* retrieves the polling kqueue in order to be used locally
    for the removal of the connections */
    struct polling_kqueue_t *polling_kqueue =
        (struct polling_kqueue_t *) polling->lower;

    /* iterates over the set of connections that are meant to
    be removed from the kqueue list as they are no longer available */
    for(index = 0; index < polling_kqueue->remove_connections_size; index++) {
        /* retrieves the current connection for the iteration
        and then deletes the current connection (house keeping) */
        current_connection = polling_kqueue->remove_connections[index];
        delete_connection(current_connection);
    }

    /* deletes the polling kqueue in order to avoid any extra memory
    leak from the kqueue structures */
    delete_polling_kqueue(polling_kqueue);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_connection_polling_kqueue(struct polling_t *polling, struct connection_t *connection) {
    /* allocates space for the result of the poll call
    to add a new element to the poll structure */
    SOCKET_ERROR_CODE result_code;

    /* allocates space for the pair of changes to be handed to the
    queue of the kernel, one for each of the two filters */
    struct kevent _events[2];

    /* retrieves the polling kqueue structure from the upper
    polling control structure */
    struct polling_kqueue_t *polling_kqueue = (struct polling_kqueue_t *) polling->lower;

    /* populates both of the changes with the connection as the data
    that travels with them, so that an event names the connection it
    belongs to without any lookup being required, the edge triggered
    flag matching the behaviour the other mechanism is asked for */
    EV_SET(&_events[0], connection->socket_handle, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, (void *) connection);
    EV_SET(&_events[1], connection->socket_handle, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, (void *) connection);
    result_code = kevent(polling_kqueue->kqueue_fd, _events, 2, NULL, 0, NULL);

    /* in case there was an error in kqueue need to correctly
    handle it and propagate it to the caller */
    if(SOCKET_TEST_ERROR(result_code)) {
        SOCKET_ERROR_CODE kqueue_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_WARNING_F("Problem registering connection kqueue: %d\n", kqueue_error_code);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem registering connection kqueue");
    }

    /* increments the counter that controls the number of events
    currently in the polling state in the kqueue */
    polling_kqueue->poll_count++;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_connection_polling_kqueue(
    struct polling_t *polling,
    struct connection_t *connection,
    unsigned char remove_c
) {
    /* allocates space for the result of the poll call
    to add a new element to the poll structure */
    SOCKET_ERROR_CODE result_code;

    /* retrieves the polling kqueue structure from the upper
    polling control structure */
    struct polling_kqueue_t *polling_kqueue = (struct polling_kqueue_t *) polling->lower;

    /* allocates space for the pair of changes that take both of the
    filters of the connection out of the queue of the kernel */
    struct kevent _events[2];

    /* removes the associated socket handle (fd) from the kqueue
    structure to avoid any leak */
    EV_SET(&_events[0], connection->socket_handle, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&_events[1], connection->socket_handle, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    result_code = kevent(polling_kqueue->kqueue_fd, _events, 2, NULL, 0, NULL);

    /* in case there was an error in kqueue it is only reported, a
    descriptor that has already gone is no longer in the queue of the
    kernel either, which drops the events of a closed one on its own,
    so the operation has reached what it was after, stopping here
    would leave the connection below out of the removal for good */
    if(SOCKET_TEST_ERROR(result_code)) {
        SOCKET_ERROR_CODE kqueue_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_WARNING_F(
            "Problem unregistering connection kqueue: %d\n",
            kqueue_error_code
        );
    }

    /* takes the connection out of the buffers of the pending
    operations, one that stays in them is reached again on the
    next round of the loop, by then possibly already released */
    discard_connection(
        polling_kqueue->read_outstanding,
        &polling_kqueue->read_outstanding_size,
        connection
    );
    discard_connection(
        polling_kqueue->write_outstanding,
        &polling_kqueue->write_outstanding_size,
        connection
    );
    connection->is_outstanding = FALSE;

    /* in case the remove connection flag is set the connection
    is also added to the list of connections to be removed
    after the io driven logic part is processed (at the end
    of the logic loop) */
    if(remove_c == TRUE) {
        remove_connection(
            polling_kqueue->remove_connections,
            &polling_kqueue->remove_connections_size,
            connection
        );
    }

    /* decrements the counter that controls the number of events
    currently in the polling state in the kqueue */
    polling_kqueue->poll_count--;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_read_polling_kqueue(struct polling_t *polling, struct connection_t *connection) {
    /* allocates space for the change that puts the reading filter
    of the connection back into the queue of the kernel */
    struct kevent _event;

    /* retrieves the polling kqueue structure as the lower
    structure from the provided polling object */
    struct polling_kqueue_t *polling_kqueue =
        (struct polling_kqueue_t *) polling->lower;

    /* allocates space for the result of the poll call
    to add a new element to the poll structure */
    SOCKET_ERROR_CODE result_code;

    /* enables the reading filter of the connection again, each of the
    filters is held apart by the queue of the kernel so the one of the
    writing is left exactly as it stands */
    EV_SET(&_event, connection->socket_handle, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, (void *) connection);
    result_code = kevent(polling_kqueue->kqueue_fd, &_event, 1, NULL, 0, NULL);

    /* in case there was an error in kqueue need to correctly
    handle it and propagate it to the caller */
    if(SOCKET_TEST_ERROR(result_code)) {
        SOCKET_ERROR_CODE kqueue_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_WARNING_F(
            "Problem registering connection kqueue for read: %d\n",
            kqueue_error_code
        );
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem registering connection kqueue for read"
        );
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_read_polling_kqueue(struct polling_t *polling, struct connection_t *connection) {
    /* allocates space for the result of the poll call
    to add a new element to the poll structure */
    SOCKET_ERROR_CODE result_code;

    /* retrieves the polling kqueue structure from the upper
    polling control structure */
    struct polling_kqueue_t *polling_kqueue = (struct polling_kqueue_t *) polling->lower;

    /* allocates space for the change that takes the reading filter
    of the connection out of the queue of the kernel */
    struct kevent _event;

    /* turns the reading filter of the connection off and leaves the
    one of the writing exactly as it stands, the queue of the kernel
    holds each of them apart from the other */
    EV_SET(&_event, connection->socket_handle, EVFILT_READ, EV_DISABLE, 0, 0, (void *) connection);
    result_code = kevent(polling_kqueue->kqueue_fd, &_event, 1, NULL, 0, NULL);

    /* in case there was an error in kqueue need to correctly
    handle it and propagate it to the caller */
    if(SOCKET_TEST_ERROR(result_code)) {
        SOCKET_ERROR_CODE kqueue_error_code = SOCKET_GET_ERROR_CODE(socket_result);
        V_WARNING_F(
            "Problem unregistering connection kqueue for read: %d\n",
            kqueue_error_code
        );
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem unregistering connection kqueue for read"
        );
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_polling_kqueue(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling kqueue structure as the lower
    structure from the provided polling object */
    struct polling_kqueue_t *polling_kqueue =
        (struct polling_kqueue_t *) polling->lower;

    /* in case the connection is write registered must
    return immediately to avoid double outstanding for connection */
    if(connection->write_registered == TRUE) { RAISE_NO_ERROR; }

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
    polling_kqueue->write_outstanding[polling_kqueue->write_outstanding_size] = connection;
    polling_kqueue->write_outstanding_size++;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_polling_kqueue(struct polling_t *polling, struct connection_t *connection) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE add_outstanding_polling_kqueue(struct polling_t *polling, struct connection_t *connection) {
    /* retrieves the polling kqueue structure as the lower
    structure from the provided polling object */
    struct polling_kqueue_t *polling_kqueue =
        (struct polling_kqueue_t *) polling->lower;

    /* in case the connection is already registered for outstanding
    read operations this request must be ignored */
    if(connection->is_outstanding == TRUE) { RAISE_NO_ERROR; }

    /* sets the connection for the current outstanding position and
    then increments the size of the outstanding connection pending */
    polling_kqueue->read_outstanding[polling_kqueue->read_outstanding_size] = connection;
    polling_kqueue->read_outstanding_size++;

    /* sets the connection as outstanding as the connection has just
    been registered in the kqueue polling mechanism for extra reads */
    connection->is_outstanding = TRUE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE poll_polling_kqueue(struct polling_t *polling) {
    /* retrieves the polling kqueue structure from the upper
    polling control structure */
    struct polling_kqueue_t *polling_kqueue =
        (struct polling_kqueue_t *) polling->lower;

    /* triggers the processing of the outstanding connection
    operations (pending operations) that meant to be done before
    the main poll operation blocks the control flow */
    _outstanding_polling_kqueue(
        polling_kqueue,
        polling_kqueue->read_outstanding,
        polling_kqueue->write_outstanding,
        polling_kqueue->_read_outstanding,
        polling_kqueue->_write_outstanding,
        polling_kqueue->read_outstanding_size,
        polling_kqueue->write_outstanding_size
    );

    /* polls the polling kqueue */
    _poll_polling_kqueue(
        polling_kqueue,
        polling_kqueue->read_connections,
        polling_kqueue->write_connections,
        polling_kqueue->error_connections,
        &polling_kqueue->read_connections_size,
        &polling_kqueue->write_connections_size,
        &polling_kqueue->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE call_polling_kqueue(struct polling_t *polling) {
    /* retrieves the polling kqueue structure from the upper
    polling control structure */
    struct polling_kqueue_t *polling_kqueue =
        (struct polling_kqueue_t *) polling->lower;

    /* calls the polling kqueue */
    _call_polling_kqueue(
        polling_kqueue,
        polling_kqueue->read_connections,
        polling_kqueue->write_connections,
        polling_kqueue->error_connections,
        polling_kqueue->read_connections_size,
        polling_kqueue->write_connections_size,
        polling_kqueue->error_connections_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _poll_polling_kqueue(
    struct polling_kqueue_t *polling_kqueue,
    struct connection_t **read_connections,
    struct connection_t **write_connections,
    struct connection_t **error_connections,
    size_t *read_connections_size,
    size_t *write_connections_size,
    size_t *error_connections_size
) {
    /* allocates space for the buffer to hold the various
    events generated from the wait call and for the event
    pointer for the iteration cycle */
    struct kevent events[VIRIATUM_MAX_EVENTS];
    struct kevent *_event;

    /* allocates space for the moment that the wait is allowed to go
    on for, handed over as a structure rather than as a count */
    struct timespec timeout;
    struct timespec *_timeout;

    /* allocates space for the index counter to be used in
    the iteration and for the counter of event from the wait */
    int index;
    int event_count;

    /* allocates space for the reference to the connection
    to be used in the iteration cycle (temporary object) */
    struct connection_t *connection;

    /* starts the various temporary index counters for
    the various types of socket operations */
    size_t read_index = 0;
    size_t write_index = 0;
    size_t error_index = 0;

    /* retrieves the service associated with the current polling
    structure in order to be able to modify it */
    struct service_t *service = polling_kqueue->polling->service;

    /* prints a debug message (include the poll count) */
    V_DEBUG_F("Entering kqueue statement (%lu)\n", polling_kqueue->poll_count);

    /* runs the wait process in the kqueue, this is the main call
    of the kqueue loop as it is the one responsible for the polling
    operation and generation of the events, note that the embed
    version of the call is managed by a timeout so that the event
    loop does not block (the normal version is unblocked by signals) */
#ifdef VIRIATUM_EMBED
    timeout.tv_sec = polling_kqueue->polling->timeout < 0
                         ? VIRIATUM_SELECT_TIMEOUT
                         : polling_kqueue->polling->timeout / 1000;
    timeout.tv_nsec = polling_kqueue->polling->timeout < 0
                          ? 0
                          : (polling_kqueue->polling->timeout % 1000) * 1000000;
    _timeout = &timeout;
#else
    timeout.tv_sec = polling_kqueue->polling->timeout / 1000;
    timeout.tv_nsec = (polling_kqueue->polling->timeout % 1000) * 1000000;
    _timeout = polling_kqueue->polling->timeout < 0 ? NULL : &timeout;
#endif
    event_count = kevent(
        polling_kqueue->kqueue_fd,
        NULL,
        0,
        events,
        VIRIATUM_MAX_EVENTS,
        _timeout
    );

    /* prints a debug message */
    V_DEBUG_F("Exiting kqueue statement with value: %d\n", event_count);

    /* in case there was an error in kqueue, in case there was this is
    considered to be a critical error */
    if(SOCKET_TEST_ERROR(event_count)) {
        /* retrieves the kqueue error code */
        SOCKET_ERROR_CODE kqueue_error_code = SOCKET_GET_ERROR_CODE(socket_result);

        /* prints an info message */
        V_INFO_F("Problem running kqueue: %d\n", kqueue_error_code);

        /* resets the values for the various read values,
        this avoid possible problems in next actions */
        *read_connections_size = 0;
        *write_connections_size = 0;
        *error_connections_size = 0;

        /* in case the interupt error code has been received the
        system should fail gracefully to unblock the call */
        if(kqueue_error_code == EINTR) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Interrupted system call in kqueue"
            );
        }

        /* closes the service socket, this will disable any
        more interaction with the service socket */
        SOCKET_CLOSE(service->service_socket_handle);

        /* raises an error */
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem running kqueue"
        );
    }

    /* iterates over the range of "generated" events in order
    to correctly handle them to the upper levels */
    for(index = 0; index < event_count; index++) {
        /* retrieves the current event in iteration (kqueue structure)
        and uses it to retrieve the associated pointer to the connection
        for event resolution */
        _event = &events[index];
        connection = (struct connection_t *) _event->udata;

        /* checks if the event is of type input (read) must be
        added to the read operations queue */
        if(_event->filter == EVFILT_READ) {
            /* sets the current connection in the read connections
            and then increments the read index counter */
            read_connections[read_index] = connection;
            read_index++;
        }

        /* checks if the event is of type output (write) must be
        added to the write operations queue */
        if(_event->filter == EVFILT_WRITE) {
            /* sets the current connection in the write connections
            and then increments the write index counter */
            write_connections[write_index] = connection;
            write_index++;
        }

        /* checks if the event is of type exception (error) must be
        added to the error operations queue */
        if(_event->flags & (EV_ERROR | EV_EOF)) {
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

ERROR_CODE _call_polling_kqueue(
    struct polling_kqueue_t *polling_kqueue,
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
        section and the handshake handler is correctly set (must
        call it to initialize the connection) */
        if(current_connection->status == STATUS_HANDSHAKE &&
           current_connection->on_handshake != NULL) {
            /* prints a series of debug messages about the handshake
            operation in it and then calls the hadhaske handler */
            V_DEBUG("Calling on handshake handler\n");
            CALL_V(current_connection->on_handshake, current_connection);
            V_DEBUG("Finished calling on handshake handler\n");
        }

        /* in case the current connection is open and the read
        handler is correctly set (must call it) */
        if(current_connection->status == STATUS_OPEN &&
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
        if(current_connection->status == STATUS_OPEN &&
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
        if(current_connection->status == STATUS_OPEN &&
           current_connection->on_error != NULL) {
            /* prints a series of debug messages and then calls the
            correct on error handler for the notification */
            V_DEBUG("Calling on error handler\n");
            CALL_V(current_connection->on_error, current_connection);
            V_DEBUG("Finished calling on error handler\n");
        }
    }

    /* iterates over the set of connections that are meant to
    be removed from the kqueue list as they are no longer available */
    for(index = 0; index < polling_kqueue->remove_connections_size; index++) {
        /* retrieves the current connection for the iteration
        and then deletes the current connection (house keeping) */
        current_connection = polling_kqueue->remove_connections[index];
        delete_connection(current_connection);
    }

    /* resets the remove connections size to the default
    zero value, no connections are removed by default */
    polling_kqueue->remove_connections_size = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _outstanding_polling_kqueue(
    struct polling_kqueue_t *polling_kqueue,
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
    polling_kqueue->read_outstanding_size = 0;
    polling_kqueue->write_outstanding_size = 0;

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
        if(current_connection->status == STATUS_OPEN &&
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
        if(current_connection->status == STATUS_OPEN &&
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

#endif
