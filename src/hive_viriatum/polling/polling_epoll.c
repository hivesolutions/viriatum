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

/*#ifdef VIRIATUM_EPOLL*/

#include "polling_epoll.h"

void create_polling_epoll(struct polling_epoll_t **polling_epoll_pointer, struct polling_t *polling) {
    /* retrieves the polling epoll size */
    size_t polling_epoll_size = sizeof(struct polling_epoll_t);

    /* retrieves the connection pointer size */
    size_t connection_pointer_size = sizeof(struct connection_t *);

    /* allocates space for the polling epoll */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) MALLOC(polling_epoll_size);

    /* resets the polling in the polling epoll */
    polling_epoll->polling = polling;

	/* creates the epoll file descriptor with an arbitrary
	size (this no longer used by the kernel) */
	polling_epoll->epoll_fd = epoll_create(1024);

	/* creates the hash map for the connection this hash map
	will be responsible for the mapping of the file descriptors
	(fds) into references to connection structures */
	create_hash_map(&polling_epoll->connections, 0);



    /* allocates the read connection for internal
    polling epoll usage */
    polling_epoll->read_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* allocates the write connection for internal
    polling epoll usage */
    polling_epoll->write_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* allocates the error connection for internal
    polling epoll usage */
    polling_epoll->error_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);

    /* allocates the remove connection for internal
    polling epoll usage */
    polling_epoll->remove_connections = (struct connection_t **) MALLOC(VIRIATUM_MAXIMUM_CONNECTIONS * connection_pointer_size);




    /* sets the polling epoll in the polling epoll pointer */
    *polling_epoll_pointer = polling_epoll;
}

void delete_polling_epoll(struct polling_epoll_t *polling_epoll) {
	/* in case the polling fd is defined it must be closed
	in order to avoid any memory leak problem */
	if(polling_epoll->epoll_fd) { close(polling_epoll->epoll_fd); } 

	/* deletes the hash map used for the connections to fd assiciations
	(this is no longer required) */
	delete_hash_map(polling_epoll->connections);

    /* releases the remove connections */
    FREE(polling_epoll->remove_connections);

    /* releases the error connection */
    FREE(polling_epoll->error_connections);

    /* releases the write connection */
    FREE(polling_epoll->write_connections);

    /* releases the read connection */
    FREE(polling_epoll->read_connections);

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
	/* allocates space for teh new event to be inserted into
	the epoll polling system (this is an internal kernel structure) */
	struct epoll_event _event;

    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

	/* populates the event structure with the appropriate
	structures in order to register for the right events 
	and then inserts the event request into the epoll fd */
	_event.events = EPOLLIN | EPOLLET;
    _event.data.fd = connection->socket_handle;
	epoll_ctl(polling_epoll->epoll_fd, EPOLL_CTL_ADD, connection->socket_handle, &_event);
	
	/* sets the connection value in the hash map this should
	be able to provide a file descriptor to connection association */
	set_value_hash_map(polling_epoll->connections, connection->socket_handle, NULL, (void *) connection);

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection)  {
    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;


	/*   !!!! PENDING IMPLEMENTATION    !!!!!! */

	/* unsets the connection value from the hash map this should
	be able to remove the file descriptor connection association */
	set_value_hash_map(polling_epoll->connections, connection->socket_handle, NULL, NULL);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_write_polling_epoll(struct polling_t *polling, struct connection_t *connection) {
	/* allocates space for teh new event to be inserted into
	the epoll polling system (this is an internal kernel structure) */
	struct epoll_event _event;

    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

	/* populates the event structure with the appropriate
	structures in order to register for the right events 
	and then inserts the event request into the epoll fd */
	_event.events = EPOLLOUT | EPOLLET;
    _event.data.fd = connection->socket_handle;
	epoll_ctl(polling_epoll->epoll_fd, EPOLL_CTL_ADD, connection->socket_handle, &_event);
	
	/* sets the connection value in the hash map this should
	be able to provide a file descriptor to connection association */
	set_value_hash_map(polling_epoll->connections, connection->socket_handle, NULL, (void *) connection);

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_write_polling_epoll(struct polling_t *polling, struct connection_t *connection)  {
    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;


	/*   !!!! PENDING IMPLEMENTATION    !!!!!! */

	/* unsets the connection value from the hash map this should
	be able to remove the file descriptor connection association */

	/* TENHO DE TER CUIDADO COM ISTO ELE AINDA PODE ESTAR LA  VER read / write relations*/

	/*set_value_hash_map(polling_epoll->connections, connection->socket_handle, NULL, NULL);*/

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE poll_polling_epoll(struct polling_t *polling) {
	struct epoll_event *events;
	struct epoll_event *_event;
	int event_count;
	int index;

	struct connection_t *connection;

    size_t read_index = 0;
    size_t write_index = 0;
    size_t error_index = 0;

    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* polls the polling epoll */
    /*_poll_polling_epoll(
        polling_epoll,
        polling_epoll->read_connections,
        polling_epoll->write_connections,
        polling_epoll->error_connections,
        &polling_epoll->read_connections_size,
        &polling_epoll->write_connections_size,
        &polling_epoll->error_connections_size
    )*/

	/* TODO: se isto funcionar ten ho de tentar com memoria estatica
	events[64] */
	events = calloc(64, sizeof(struct epoll_event));

	printf("entrou epoll\n");

	event_count = epoll_wait(polling_epoll->epoll_fd, events, 64, -1);

	printf("acabou epoll com %d\n", event_count);

	for(index = 0; index < event_count; index++) {
        _event = &events[index];
		get_value_hash_map(polling_epoll->connections, _event->data.fd, NULL, (void **) &connection);

		if(_event->events & EPOLLIN) {
			printf("encontrou read '%d' -> '%d'\n", _event->data.fd, connection);

            /* sets the current connection in the read connections
			and then increments the read index counter */
			polling_epoll->read_connections[read_index] = connection;
			read_index++;
		}

		if(_event->events & EPOLLOUT) {
			printf("encontrou write '%d' -> '%d'\n", _event->data.fd, connection);

            /* sets the current connection in the write connections
			and then increments the write index counter */
			polling_epoll->write_connections[write_index] = connection;
			write_index++;
		}

		if(_event->events & (EPOLLERR | EPOLLHUP)) {
			printf("encontrou error '%d' -> '%d'\n", _event->data.fd, connection);

            /* sets the current connection in the error connections
			and then increments the error index counter */
			polling_epoll->error_connections[write_index] = connection;
			write_index++;
		}
    }
	
    /* updates the various operation counters for the three
	operation to be "polled" (this is done by reference) */
    polling_epoll->read_connections_size = read_index;
    polling_epoll->write_connections_size = write_index;
    polling_epoll->error_connections_size = error_index;

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

ERROR_CODE _call_polling_epoll(struct polling_epoll_t *polling_epoll, struct connection_t **read_connections, struct connection_t **write_connections, struct connection_t **error_connections, struct connection_t **remove_connections, size_t read_connections_size, size_t write_connections_size, size_t error_connections_size) {
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

/*#endif*/
