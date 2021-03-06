/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "stream_io.h"

void create_io_connection(struct io_connection_t **io_connection_pointer, struct connection_t *connection) {
    /* retrieves the io connection size */
    size_t io_connection_size = sizeof(struct io_connection_t);

    /* allocates space for the io connection */
    struct io_connection_t *io_connection =\
        (struct io_connection_t *) MALLOC(io_connection_size);

    /* sets the default values for the io connection
    attributes, this avoids problems in initialization */
    io_connection->connection = connection;
    io_connection->on_data = NULL;
    io_connection->on_open = NULL;
    io_connection->on_close = NULL;
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

    /* allocates the option value and sets it to one (valid) */
    SOCKET_OPTION option_value = 1;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* calculates the size of the socket address */
    SOCKET_ADDRESS_SIZE client_socket_address_size = sizeof(SOCKET_ADDRESS);

#ifdef VIRIATUM_SSL
    /* allocates space for the "possible" ssl handle and
    for the socket result for the ssl operations */
    SSL *ssl_handle;
    SOCKET_ERROR_CODE socket_result;
    unsigned long ssl_code;
    char ssl_message[VIRIATUM_MAX_SSL_SIZE];
#endif

    /* iterates continuously to try to accept as most
    connections as possible from the service socket */
    while(TRUE) {
        /* accepts the socket, retrieving the socket handle */
        socket_handle = SOCKET_ACCEPT(
            connection->socket_handle,
            &socket_address,
            client_socket_address_size
        );

        /* in case there was an error accepting the socket */
        if(SOCKET_TEST_ERROR(socket_handle)) {
            /* breaks the loop, nothing to be handled not possible
            to accept the connection for now */
            break;
        }
        /* otherwise the socket was accepted corretly */
        else {
            /* in case viriatum is set to non blocking, changes the current
            socket behavior to non blocking mode then sets the socket to the
            non push mode in case it's required by configuration this implies
            also checking for the no (tcp) wait variable */
            if(VIRIATUM_NON_BLOCKING) { SOCKET_SET_NON_BLOCKING(socket_handle, flags); }
            if(VIRIATUM_KEEPALIVE) { SOCKET_SET_KEEPALIVE(socket_handle, option_value); }
            if(VIRIATUM_NO_WAIT) { SOCKET_SET_NO_WAIT(socket_handle, option_value); }
            if(VIRIATUM_NO_PUSH) { SOCKET_SET_NO_PUSH(socket_handle, option_value); }

            /* prints a debug message about the accepting of the new
            connection with the value of the socket handle */
            V_DEBUG_F("Accepted connection: %d\n", socket_handle);

            /* creates the (client) connection using the provided
            socket handle as the reference socket and then resolves
            it so that the proper host and port fields are populated */
            create_connection(&client_connection, socket_handle);
            resolve_connection(client_connection, &socket_address);

            /* sets the socket address in the (client) connection
            this is going to be very usefull for later connection
            identification (address, port, etc.) */
            client_connection->socket_address = socket_address;

            /* sets the service select service as the service in the (client)  connection */
            client_connection->service = connection->service;

            /* sets the base handling functions in the client connection */
            client_connection->open_connection = open_connection;
            client_connection->close_connection = close_connection;
            client_connection->write_connection = write_connection;
            client_connection->register_read = register_read_connection;
            client_connection->unregister_read = unregister_read_connection;
            client_connection->register_write = register_write_connection;
            client_connection->unregister_write = unregister_write_connection;
            client_connection->invalidate_read = invalidate_read_connection;
            client_connection->invalidate_write = invalidate_write_connection;
            client_connection->add_outstanding = add_outstanding_connection;

            /* sets the various stream io connection callbacks
            in the client connection */
            client_connection->on_read = read_handler_stream_io;
            client_connection->on_write = write_handler_stream_io;
            client_connection->on_handshake = handshake_handler_stream_io;
            client_connection->on_error = error_handler_stream_io;
            client_connection->on_open = open_handler_stream_io;
            client_connection->on_close = close_handler_stream_io;

            /* opens the connection, this should add the client
            connection to the polling system */
            client_connection->open_connection(client_connection);

#ifdef VIRIATUM_SSL
            /* in case the connection "contains" an ssl context defined
            the client connection "should also" be considered an encrypted
            ssl connection (secure connection) */
            if(connection->ssl_context) {
                /* creates the new ssl handle using the current service connection
                ssl conext and sets the correct socked file descriptor in it */
                ssl_handle = SSL_new(connection->ssl_context);
                SSL_set_fd(ssl_handle, socket_handle);

                /* prints a debug message about the creation of the new ssl handle
                to be able to trace it on latter connections */
                V_DEBUG("Created new SSL handle\n");

                /* updates boths the ssl context and ssl handle reference in the
                client connection structure reference */
                client_connection->is_secure = TRUE;
                client_connection->ssl_context = connection->ssl_context;
                client_connection->ssl_handle = ssl_handle;

                /* runs the accept operation in the ssl handle, this is possible to
                break as this operation involves the handshake operation non blocking
                sockets may block on this */
                socket_result = SSL_accept(ssl_handle);

                /* in case the socket result is zero the connection has been closed
                from the other side must close it from here also */
                if(socket_result == 0) {
                    /* retrieves the last ssl error code and then converts
                    it into a readable error string so that it's possible to
                    print it in a proper manner */
                    ssl_code = ERR_get_error();
                    ERR_error_string_n(ssl_code, ssl_message, VIRIATUM_MAX_SSL_SIZE);

                    /* retrieves the current ssl error description, to be displayed
                    as a warning message */
                    socket_result = SSL_get_error(ssl_handle, socket_result);
                    V_WARNING_F("Closing the SSL connection (%s)\n", ssl_error_codes[socket_result]);
                    V_WARNING_F("%s\n", ssl_message);

                    /* closes the (client) connection, the ssl connection has been
                    closed from the other side (client side) */
                    client_connection->close_connection(client_connection);
                }
                /* in case the result of the accept operation in the ssl handle is not
                the socket result (error) must handle it gracefully (normal) */
                else if(socket_result < 0) {
                    /* retrieves the error code for the current problem
                    in order to gracefully handle it */
                    socket_result = SSL_get_error(ssl_handle, socket_result);

                    /* switches over the result of the error checking
                    in order to properly handle it */
                    switch(socket_result) {
                        case SSL_ERROR_WANT_READ:
                        case SSL_ERROR_WANT_WRITE:
                            /* prints a debug message about the skip of the process
                            of handshaking for the current connection*/
                            V_DEBUG_F("Skipping SSL handshake (%s)\n", ssl_error_codes[socket_result]);

                            /* updates the client connection status to handshake
                            status so that the complete handshake may be resumed */
                            client_connection->status = STATUS_HANDSHAKE;

                            /* breaks the switch must try to accept
                            the connection again in a different loop */
                            break;

                        default:
                            /* retrieves the last ssl error code and then converts
                            it into a readable error string so that it's possible to
                            print it in a proper manner */
                            ssl_code = ERR_get_error();
                            ERR_error_string_n(ssl_code, ssl_message, VIRIATUM_MAX_SSL_SIZE);

                            /* prints a warning message about the closing of
                            the ssl connection (due to a connection problem) */
                            V_WARNING_F("Failed to accept SSL connection (%s)\n", ssl_error_codes[socket_result]);
                            V_WARNING_F("%s\n", ssl_message);

                            /* closes the (client) connection, the ssl connection has been
                            closed from the other side (client side) */
                            client_connection->close_connection(client_connection);
                            break;
                    }
                }
            }
#endif
        }

        /* in case the viriatum is set to blocking must break
        the loop, no more that one read operation is allowed */
        if(!VIRIATUM_NON_BLOCKING) { break; }
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE read_handler_stream_io(struct connection_t *connection) {
    /* allocates the variable that will hold number of bytes
    that have been read in the current read operation */
    SOCKET_ERROR_CODE number_bytes;

    /* allocates the "simple" buffer and sets the pointer value
    of it with its current defined address */
    unsigned char buffer[VIRIATUM_READB_SIZE];
    unsigned char *buffer_pointer = (unsigned char *) buffer;

    /* reserves space for the counters for the size of the
    current buffer and for the total bytes that have been
    read in the current io operation */
    size_t buffer_size = 0;
    size_t total_bytes = 0;

    /* flag and value controlling the state of the read */
    ERROR_CODE error = 0;

    /* retrieves the io connection as the lower substrate of the
    current connection object, in order to be used */
    struct io_connection_t *io_connection =\
        (struct io_connection_t *) connection->lower;

    /* iterates continuously to read the data that is currently
    available in the connection for reading */
    while(TRUE) {
        /* in case the current total number of bytes read during
        this read operation is greater than the expected maximum
        the read operation should be delayed so that the write
        doesn't starve (only read operations and no writing) */
        if(total_bytes > VIRIATUM_MAX_READ) {
            connection->add_outstanding(connection);
            break;
        }

        /* in case the buffer size is so big that may
        overflow the current allocated buffer, must
        flush the current buffer avoid corruption */
        if(buffer_size + VIRIATUM_READ_SIZE > VIRIATUM_READB_SIZE) {
            /* in case the on data handler is defined must call
            the handler to flush the buffer */
            if(io_connection->on_data != NULL) {
                /* prints some debugging information about the calling
                of the data handler for the partial buffered data and
                then runs the call for the data handler */
                V_DEBUG("Calling on data handler\n");
                io_connection->on_data(io_connection, buffer, buffer_size);
                V_DEBUG("Finished calling on data handler\n");
            }

            /* resets the buffer size to the original (initial)
            position so that it can be reused, note that both
            the buffer pointer and the buffer size are reseted */
            buffer_pointer = (unsigned char *) buffer;
            buffer_size = 0;
        }

        /* receives from the connection socket (takes into account the type
        of socket in use) should be able to take care of secure connections */
        number_bytes = CONNECTION_RECEIVE(
            connection,
            (char *) buffer_pointer,
            VIRIATUM_READ_SIZE,
            0
        );
        connection->received += number_bytes;

        /* in case the number of bytes is zero (connection closed) */
        if(number_bytes == 0) {
            /* sets the error flag so that the connection is closed
            and then breaks the current loop */
            error = 1;
            break;
        }

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(number_bytes)) {
            /* retrieves the (receving) error code */
            SOCKET_ERROR_CODE error_code = CONNECTION_GET_ERROR_CODE(
                connection,
                number_bytes
            );

            /* switches over the receiving error code */
            switch(error_code) {
                case SOCKET_WOULDBLOCK:
                    /* prints a debug message, sets the error flag (non fatal)
                    and then breaks the current switch statement */
                    V_DEBUG("Read structures not ready: WOULDBLOCK\n");
                    error = 2;
                    break;

#ifdef VIRIATUM_SSL
                case SSL_ERROR_WANT_READ:
                    /* prints a debug message, sets the error flag (non fatal)
                    and then breaks the current switch statement */
                    V_DEBUG("Read structures not ready: SSL_ERROR_WANT_READ\n");
                    error = 2;
                    break;
#endif

                default:
                    /* prints a debug message, sets the error flag
                    and then breaks the current switch statement */
                    V_DEBUG_F("Problem receiving from socket: %d\n", error_code);
                    error = 1;
                    break;
            }

            /* breaks the loop */
            break;
        }

        /* increments the buffer position, then increments
        the buffer size with the number of bytes and then
        increments the total ammount of bytes */
        buffer_pointer += number_bytes;
        buffer_size += number_bytes;
        total_bytes += number_bytes;

        /* in case the viriatum is set to blocking must break
        the loop, no more that one read operation is allowed */
        if(!VIRIATUM_NON_BLOCKING) { break; }
    }

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* in case the on data handler is defined and there
            is data to be provided (buffer size available )*/
            if(buffer_size > 0 && io_connection->on_data != NULL) {
                /* prints a set of debugging information abour the
                calling of the handler for the data and then calls the
                data handler, this should be one of the main areas in
                the event loop and should block for a while */
                V_DEBUG("Calling on data handler\n");
                io_connection->on_data(io_connection, buffer, buffer_size);
                V_DEBUG("Finished calling on data handler\n");
            }

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* in case there's data pending to be processed must flush
            it even though there was a fatal error the data must be
            correctly processed by the underlying layer */
            if(buffer_size > 0) {
                V_DEBUG("Calling on data handler\n");
                io_connection->on_data(io_connection, buffer, buffer_size);
                V_DEBUG("Finished calling on data handler\n");
            }

            /* closes the connection as the error was fatal, so this is the
            best escape approach to solve the issue */
            connection->close_connection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* in case the on data handler is defined and there
            is data to be provided (buffer size available )*/
            if(buffer_size > 0 && io_connection->on_data != NULL) {
                /* prints some debugging information about the data
                callback function call and then performs the call on
                the data callback for the current io connection */
                V_DEBUG("Calling on data handler\n");
                io_connection->on_data(io_connection, buffer, buffer_size);
                V_DEBUG("Finished calling on data handler\n");
            }

            /* invalidates the read operation for the current
            connection meaning that a new "level up" event must
            be performed before the connection is triggered for
            the read operation (useful for epoll) */
            CALL_V(connection->invalidate_read, connection);

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
    while(TRUE) {
        /* prints a debug message */
        V_DEBUG("Peeking value from write queue\n");

        /* peeks a value (data) from the linked list (write queue) and
        in case the data is invalid (list is empty) it considered the
        end of iteration and must break the loop */
        peek_value_linked_list(connection->write_queue, (void **) &data);
        if(data == NULL) { break; }

        /* prints a debug message about the data that is going
        to be sent through the socket */
        V_DEBUG_F(
            "Sending %ld bytes through socket: %d\n",
            (long int) data->size,
            connection->socket_handle
        );

        /* sends the value into the connection socket (takes into account the type
        of socket in use) should be able to take care of secure connections */
        number_bytes = CONNECTION_SEND(
            connection,
            (char *) data->data,
            data->size,
            0
        );
        connection->sent += number_bytes;

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(number_bytes)) {
            /* retrieves the (sending) error code */
            SOCKET_ERROR_CODE error_code = CONNECTION_GET_ERROR_CODE(connection, number_bytes);

            /* switches over the (sending) error code */
            switch(error_code) {
                case SOCKET_WOULDBLOCK:
                    /* prints a debug message */
                    V_DEBUG("Write structures not ready: WOULDBLOCK\n");

                    /* sets the error flag (non fatal) */
                    error = 2;

                    /* breaks the switch */
                    break;

#ifdef VIRIATUM_SSL
                case SSL_ERROR_WANT_WRITE:
                    /* prints a debug message */
                    V_DEBUG("Write structures not ready: SSL_ERROR_WANT_WRITE\n");

                    /* sets the error flag (non fatal) */
                    error = 2;

                    /* breaks the switch */
                    break;
#endif

                default:
                    /* prints the error */
                    V_DEBUG_F("Problem sending from socket: %d\n", error_code);

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

        /* pops a value (data) from the linked list (write queue)
        this way the item will not be re-used again */
        pop_value_linked_list(connection->write_queue, (void **) &data, TRUE);

        /* in case the data callback is set */
        if(data->callback != NULL) {
            /* prints some debug information about the calling of the callback
            associated with the data and then calls the callback associated */
            V_DEBUG("Calling write callback\n");
            data->callback(connection, data, data->callback_parameters);
            V_DEBUG("Finished calling write callback\n");
        }

        /* prints a debug message */
        V_DEBUG("Deleting data (cleanup structures)\n");

        /* deletes the data, this should remove the underlying
        buffer in case the flag for such operation is set */
        delete_data(data);

        /* in case the connection has been closed sets the
        error flag to non fatal and then breaks the loop */
        if(connection->status == STATUS_CLOSED) { error = 2; break; }
    }

    /* prints a debug message */
    V_DEBUG_F("Processing write error code: %d\n", error);

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* unregisters the connection from write as the
            connection write queue should be empty */
            connection->unregister_write(connection);

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection because that's the proper
            way to escape from a fatal error */
            connection->close_connection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* invalidates the write operation for the current
            connection meaning that a new "level up" event must
            be performed before the connection is triggered for
            the write operation (useful for epoll) */
            CALL_V(connection->invalidate_write, connection);

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

ERROR_CODE handshake_handler_stream_io(struct connection_t *connection) {
#ifdef VIRIATUM_SSL
    /* allocates space for the variable that will hold the result of
    the operation, to take the appropriate measures */
    SOCKET_ERROR_CODE result;

    /* allocates space for both the integer that will hold the error code
    for the ssl specifi purposes and for the buffer that will hold the
    descriptive message of the ssl error to be "raised" */
    unsigned long ssl_code;
    char ssl_message[VIRIATUM_MAX_SSL_SIZE];

    /* prints a debug message about the retry of the handshake for the
    provided socket handle in the ssl context */
    V_DEBUG("Trying handshake for SSL handle\n");

    /* runs the accept operation in the ssl handle, this is possible to
    break as this operation involves the handshake operation non blocking
    sockets may block on this */
    result = SSL_accept(connection->ssl_handle);

    /* in case the result of the accept operation in the ssl handle is not
    the expected (error) must handle it gracefully (normal) */
    if(result == 1) {
        /* prints a debug message about the success in the ssl handshake
        operation so that it may be used for debugging purposes */
        V_DEBUG("SSL handshake successful\n");

        /* udpates the status of the (client) connection to open
        so that any further data is correctly handled by read */
        connection->status = STATUS_OPEN;
    } else if(result == 0) {
        /* retrieves the last ssl error code and then converts
        it into a readable error string so that it's possible to
        print it in a proper manner */
        ssl_code = ERR_get_error();
        ERR_error_string_n(ssl_code, ssl_message, VIRIATUM_MAX_SSL_SIZE);

        /* retrieves the current ssl error description, to be displayed
        as a warning message */
        result = SSL_get_error(connection->ssl_handle, result);
        V_WARNING_F("Closing the SSL connection (%s)\n", ssl_error_codes[result]);
        V_WARNING_F("%s\n", ssl_message);

        /* closes the connection, the ssl connection has been closed
        from the other side */
        connection->close_connection(connection);
    } else {
        /* retrieves the error code for the current problem
        and updates the client connection to the handshake
        status so that any further read request is redirected
        as an handshake data request */
        result = SSL_get_error(connection->ssl_handle, result);
        connection->status = STATUS_HANDSHAKE;

        /* switches over the result of the error checking
        in order to properly handle it */
        switch(result) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                /* prints a debug message about the skip of the process
                of handshaking for the current connection*/
                V_DEBUG_F("Skipping SSL handshake (%s)\n", ssl_error_codes[result]);

                /* breaks the switch must try to accept
                the connection again in a different loop */
                break;

            default:
                /* retrieves the last ssl error code and then converts
                it into a readable error string so that it's possible to
                print it in a proper manner */
                ssl_code = ERR_get_error();
                ERR_error_string_n(ssl_code, ssl_message, VIRIATUM_MAX_SSL_SIZE);

                /* prints a warning message about the closing of
                the ssl connection (due to a connection problem) */
                V_WARNING_F("Closing the SSL connection (%s)\n", ssl_error_codes[result]);
                V_WARNING_F("%s\n", ssl_message);

                /* closes the connection, the ssl connection it has been
                closed from the other side (client side) */
                connection->close_connection(connection);
                break;
        }
    }
#endif

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE error_handler_stream_io(struct connection_t *connection) {
    /* closes the connection */
    connection->close_connection(connection);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_handler_stream_io(struct connection_t *connection) {
    /* allocates the io connection and for the parameters
    structure reference that may be used for custom protocols */
    struct io_connection_t *io_connection;
    struct custom_parameters_t *parameters;

    /* creates the io connection from the provided connection
    object, this should apply the proper handlers in case they
    are set as part of the parameters */
    create_io_connection(&io_connection, connection);

    /* switches over the connection "desired" protocol to
    set the appropriate connection handlers */
    switch(connection->protocol) {
        case CUSTOM_PROTOCOL:
            parameters = connection->parameters;
            io_connection->on_data = parameters->on_data;
            io_connection->on_open = parameters->on_open;
            io_connection->on_close = parameters->on_close;
            break;

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

    /* in case the on open handler (function) is defined it must be
    called to notify the lower layers about the opening of the connection */
    if(io_connection->on_open != NULL) {
        /* prints a series of debug messages about the opening
        of the connection and call the proper open handler for
        the io connection */
        V_DEBUG("Calling on open handler\n");
        io_connection->on_open(io_connection);
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
        /* prints some debugging information about the handler function
        that is going to be called and the runs the call */
        V_DEBUG("Calling on close handler\n");
        io_connection->on_close(io_connection);
        V_DEBUG("Finished calling on close handler\n");
    }

    /* deletes the io connection */
    delete_io_connection(io_connection);

    /* raise no error */
    RAISE_NO_ERROR;
}
