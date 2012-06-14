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

#include "stream_io.h"

void create_io_connection(struct io_connection_t **io_connection_pointer, struct connection_t *connection) {
    /* retrieves the io connection size */
    size_t io_connection_size = sizeof(struct io_connection_t);

    /* allocates space for the io connection */
    struct io_connection_t *io_connection = (struct io_connection_t *) MALLOC(io_connection_size);

    /* sets the io connection connection */
    io_connection->connection = connection;

    /* sets the on data to unset */
    io_connection->on_data = NULL;

    /* sets the on open to unset */
    io_connection->on_open = NULL;

    /* sets the on close to unset */
    io_connection->on_close = NULL;

    /* sets the lower to unset */
    io_connection->lower = NULL;

    /* sets the io connection in the (upper) connection substrate */
    connection->lower = (void *) io_connection;

    /* sets the io connection in the io connection pointer */
    *io_connection_pointer = io_connection;
}

void delete_io_connection(struct io_connection_t *io_connection) {
    /* releases the io connection */
    FREE(io_connection);
}

ERROR_CODE accept_handler_stream_io(struct connection_t *connection) {
    /* allocates the socket handle */
    SOCKET_HANDLE socket_handle;

    /* allocates the socket address */
    SOCKET_ADDRESS socket_address;

    /* allocates the (client) connection */
    struct connection_t *client_connection;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* calculates the size of the socket address */
    SOCKET_ADDRESS_SIZE client_socket_address_size = sizeof(SOCKET_ADDRESS);

    /* iterates continuously */
    while(1) {
        /* accepts the socket, retrieving the socket handle */
        socket_handle = SOCKET_ACCEPT(connection->socket_handle, &socket_address, client_socket_address_size);

        /* in case there was an error accepting the socket */
        if(SOCKET_TEST_ERROR(socket_handle)) {
            /* breaks the loop */
            break;
        }
        /* otherwise the socket was accepted corretly */
        else {
			/* in case viriatum is set to non blocking, changes the current
			socket behavior to non blocking mode then sets the socket to then
			non push mode in case it's required by configuration */
            if(VIRIATUM_NON_BLOCKING) { SOCKET_SET_NON_BLOCKING(socket_handle, flags); }
			if(VIRIATUM_NO_WAIT) { SOCKET_SET_NO_WAIT(socket_handle, flags); }
			if(VIRIATUM_NO_PUSH) { SOCKET_SET_NO_PUSH(socket_handle, flags); }

            /* prints a debug message */
            V_DEBUG_F("Accepted connection: %d\n", socket_handle);

            /* creates the (client) connection */
            create_connection(&client_connection, socket_handle);

            /* sets the socket address in the (client) connection
            this is going to be very usefull for later connection
            identification (address, port, etc.) */
            client_connection->socket_address = socket_address;

            /* sets the service select service as the service in the (client)  connection */
            client_connection->service = connection->service;

            /* sets the base hanlding functions in the client connection */
            client_connection->open_connection = open_connection;
            client_connection->close_connection = close_connection;
            client_connection->write_connection = write_connection;
            client_connection->register_write = register_write_connection;
            client_connection->unregister_write = unregister_write_connection;

            /* sets the various stream io connection callbacks
            in the client connection */
            client_connection->on_read = read_handler_stream_io;
            client_connection->on_write = write_handler_stream_io;
            client_connection->on_error = error_handler_stream_io;
            client_connection->on_open = open_handler_stream_io;
            client_connection->on_close = close_handler_stream_io;

            /* opens the connection */
            client_connection->open_connection(client_connection);
        }

        /* in case viriatum is set to blocking */
        if(!VIRIATUM_NON_BLOCKING) {
            /* breaks the loop (avoid blocking) */
            break;
        }
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE read_handler_stream_io(struct connection_t *connection) {
    /* allocates the number of bytes */
    SOCKET_ERROR_CODE number_bytes;

    /* allocates the "simple" buffer */
    unsigned char buffer[10240];

    /* retrieves the buffer pointer */
    unsigned char *buffer_pointer = (unsigned char *) buffer;

    /* allocates the buffer size */
    size_t buffer_size = 0;

    /* flag and value controlling the state of the read */
    ERROR_CODE error = 0;

    /* retrieves the io connection */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;

    /* iterates continuously */
    while(1) {
        /* in case the buffer size is so big that may
        overflow the current allocated buffer */
        if(buffer_size + 1024 > 10240) {
            /* breaks the loop */
            break;
        }

        /* receives from the socket */
        number_bytes = SOCKET_RECEIVE(connection->socket_handle, (char *) buffer_pointer, 1024, 0);

        /* in case the number of bytes is zero (connection closed) */
        if(number_bytes == 0) {
            /* sets the error flag */
            error = 1;

            /* breaks the loop */
            break;
        }

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(number_bytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receiving_error_code = SOCKET_GET_ERROR_CODE(number_bytes);

            /* switches over the receiving error code */
            switch(receiving_error_code) {
                case SOCKET_WOULDBLOCK:
                    /* prints a debug message */
                    V_DEBUG("Read structures not ready: WOULDBLOCK\n");

                    /* sets the error flag (non fatal) */
                    error = 2;

                    /* breaks the switch */
                    break;

                default:
                    /* prints the error */
                    V_DEBUG_F("Problem receiving from socket: %d\n", receiving_error_code);

                    /* sets the error flag */
                    error = 1;

                    /* breaks the switch */
                    break;
            }

            /* breaks the loop */
            break;
        }

        /* increments the buffer position */
        buffer_pointer += number_bytes;

        /* increments the buffer size with the number of bytes */
        buffer_size += number_bytes;

        /* in case the viriatum is set to blocking */
        if(!VIRIATUM_NON_BLOCKING) {
            /* breaks the loop */
            break;
        }
    }

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* in case the on data handler is defined */
            if(io_connection->on_data != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on data handler\n");

                /* calls the on data handler */
                io_connection->on_data(io_connection, buffer, buffer_size);

                /* prints a debug message */
                V_DEBUG("Finished calling on data handler\n");
            }

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection */
            connection->close_connection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* in case the on data handler is defined */
            if(io_connection->on_data != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on data handler\n");

                /* calls the on data handler */
                io_connection->on_data(io_connection, buffer, buffer_size);

                /* prints a debug message */
                V_DEBUG("Finished calling on data handler\n");
            }

            /* breaks the switch */
            break;

        /* default case */
        default:
            /* breaks the switch */
            break;
    }

    /* raises the error code */
    RAISE_ERROR(error);
}

ERROR_CODE write_handler_stream_io(struct connection_t *connection) {
    /* allocates the number of bytes */
    SOCKET_ERROR_CODE number_bytes;

    /* allocates the data */
    struct data_t *data;

    /* flag and value controlling the state of the write */
    ERROR_CODE error = 0;

    /* iterates continuously */
    while(1) {
        /* prints a debug message */
        V_DEBUG("Peeking value from write queue\n");

        /* peeks a value (data) from the linked list (write queue) */
        peek_value_linked_list(connection->write_queue, (void **) &data);

        /* in case the data is invalid (list is empty) */
        if(data == NULL) {
            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG_F("Sending %ld bytes through socket: %d\n", (long int) data->size, connection->socket_handle);

        /* sends the value retrieving the number of bytes sent */
        number_bytes = SOCKET_SEND(connection->socket_handle, (char *) data->data, data->size, 0);

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(number_bytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receiving_error_code = SOCKET_GET_ERROR_CODE(number_bytes);

            /* switches over the receiving error code */
            switch(receiving_error_code) {
                case SOCKET_WOULDBLOCK:
                    /* prints a debug message */
                    V_DEBUG("Write structures not ready: WOULDBLOCK\n");

                    /* sets the error flag (non fatal) */
                    error = 2;

                    /* breaks the switch */
                    break;

                default:
                    /* prints the error */
                    V_DEBUG_F("Problem sending from socket: %d\n", receiving_error_code);

                    /* sets the error flag */
                    error = 1;

                    /* breaks the switch */
                    break;
            }

            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG_F("Data [%d bytes] sent without errors\n", number_bytes);

        /* in case the number of bytes sent is the same as the value size
        (not all data has been sent) */
        if(number_bytes != data->size) {
            /* prints a debug message */
            V_DEBUG_F("Shrinking data [%d bytes] (partial message sent)\n", number_bytes);

            /* updates the data internal structure
            to allow the sending of the pending partial data */
            data->data += number_bytes;
            data->size -= number_bytes;

            /* sets the error flag (non fatal) */
            error = 2;

            /* breaks the loop */
            break;
        }

        /* pops a value (data) from the linked list (write queue) */
        pop_value_linked_list(connection->write_queue, (void **) &data, 1);

        /* in case the data callback is set */
        if(data->callback != NULL) {
            /* prints a debug message */
            V_DEBUG("Calling write callback\n");

            /* calls the callback with the callback parameters */
            data->callback(connection, data, data->callback_parameters);

            /* prints a debug message */
            V_DEBUG("Finished calling write callback\n");
        }

        /* prints a debug message */
        V_DEBUG("Deleting data (cleanup structures)\n");

        /* deletes the data */
        delete_data(data);

        /* in case the connection has been closed */
        if(connection->status == STATUS_CLOSED) {
            /* sets the error flag (non fatal) */
            error = 2;

            /* breaks the loop */
            break;
        }
    }

    /* prints a debug message */
    V_DEBUG_F("Processing write error code: %d\n", error);

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* unregisters the connection for write */
            connection->unregister_write(connection);

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection */
            connection->close_connection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* breaks the switch */
            break;

        /* default case */
        default:
            /* breaks the switch */
            break;
    }

    /* raises the error code */
    RAISE_ERROR(error);
}

ERROR_CODE error_handler_stream_io(struct connection_t *connection) {
    /* closes the connection */
    connection->close_connection(connection);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_handler_stream_io(struct connection_t *connection) {
    /* allocates the io connection */
    struct io_connection_t *io_connection;

    /* creates the io connection */
    create_io_connection(&io_connection, connection);

    /* switches over the connection "desired" protocol to
    set the appropriate connection handlers */
    switch(connection->protocol) {
        case HTTP_PROTOCOL:
            io_connection->on_data = data_handler_stream_http;
            io_connection->on_open = open_handler_stream_http;
            io_connection->on_close = close_handler_stream_http;
            break;

        case HTTP_CLIENT_PROTOCOL:
            io_connection->on_data = data_handler_stream_http_client;
            io_connection->on_open = open_handler_stream_http_client;
            io_connection->on_close = close_handler_stream_http_client;
            break;

        case TORRENT_PROTOCOL:
            io_connection->on_data = data_handler_stream_torrent;
            io_connection->on_open = open_handler_stream_torrent;
            io_connection->on_close = close_handler_stream_torrent;
            break;

        default:
            io_connection->on_data = data_handler_stream_http;
            io_connection->on_open = open_handler_stream_http;
            io_connection->on_close = close_handler_stream_http;
            break;
    }

    /* in case the on open handler is defined */
    if(io_connection->on_open != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on open handler\n");

        /* calls the on open handler */
        io_connection->on_open(io_connection);

        /* prints a debug message */
        V_DEBUG("Finished calling on open handler\n");
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_handler_stream_io(struct connection_t *connection) {
    /* retrieves the io connection */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;

    /* in case the on close handler is defined */
    if(io_connection->on_close != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on close handler\n");

        /* calls the on close handler */
        io_connection->on_close(io_connection);

        /* prints a debug message */
        V_DEBUG("Finished calling on close handler\n");
    }

    /* deletes the io connection */
    delete_io_connection(io_connection);

    /* raise no error */
    RAISE_NO_ERROR;
}
