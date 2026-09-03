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

#include "polling_test.h"
#include "service_test.h"

/* the port that the pair of sockets of the tests is built over, kept
apart from the one the service of the other tests binds */
#define POLLING_TEST_PORT 9391

/* the port that the service of each of the tests is bound to, one of
them is taken per test so that the socket of the one before, which the
system holds for a while yet, is never bound a second time */
#define VIRIATUM_TEST_POLLING_PORT 19400

/* the number of times that the handler of the reading has been called,
which is how a test tells that the waiting handed a connection back */
static size_t _read_count = 0;

/* the number of times that the handler of the error has been called,
which is the other way a peer that has gone away shows up */
static size_t _error_count = 0;

/* the number of times that the handler of the writing has been
called, which is how a write left pending is told to have been driven */
static size_t _write_count = 0;

/* the number of times that the handler of the handshake has been
called, a connection is put through one before it is ever served */
static size_t _handshake_count = 0;

static ERROR_CODE _on_read_polling_test(struct connection_t *connection) {
    /* counts the call and reads whatever is waiting on the socket so
    that the mechanisms that report an edge do not report it again */
    unsigned char buffer[128];
    _read_count++;
    SOCKET_RECEIVE(connection->socket_handle, (char *) buffer, sizeof(buffer), 0);
    RAISE_NO_ERROR;
}

static ERROR_CODE _on_error_polling_test(struct connection_t *connection) {
    /* counts the call so that a test is able to tell that the
    mechanism reported the connection as being in trouble */
    _error_count++;
    RAISE_NO_ERROR;
}

static ERROR_CODE _on_handshake_polling_test(struct connection_t *connection) {
    /* counts the call and puts the connection through to the open
    state, which is what the finishing of a handshake amounts to */
    _handshake_count++;
    connection->status = STATUS_OPEN;
    RAISE_NO_ERROR;
}

static ERROR_CODE _on_write_polling_test(struct connection_t *connection) {
    /* counts the call so that a test is able to tell that a write
    left pending at the end of a cycle was driven at the start of
    the one that came after it */
    _write_count++;
    RAISE_NO_ERROR;
}

/* the number of values whose sending has been reported and the
order in which it was, each of them written as the digit it carries,
which is how a test tells that the values of a queue went out in turn */
static size_t _sent_count = 0;
static char _sent_order[8] = "";

static ERROR_CODE _on_sent_polling_test(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* records the value as sent, in its turn, so that a test is able
    to tell that the values of the queue went out in order and that
    the callback of every one of them was called */
    _sent_order[_sent_count] = (char) ('0' + (int) (size_t) parameters);
    _sent_count++;
    _sent_order[_sent_count] = '\0';
    RAISE_NO_ERROR;
}

/* builds a pair of connected sockets over the loopback, the accepted
end of it being the one that the polling is handed and the connected
one the end that a test writes into so that something is waiting */
static void _create_pair_polling_test(SOCKET_HANDLE *server, SOCKET_HANDLE *client, unsigned short port) {
    SOCKET_HANDLE listener;
    SOCKET_ADDRESS_INTERNET address;
    SOCKET_ADDRESS_INTERNET peer;
    SOCKET_ADDRESS_SIZE peer_size = sizeof(peer);
    SOCKET_OPTION_LARGE flag = 1;
    SOCKET_FLAGS flags = 1;

    /* opens the socket that the connecting end is going to reach and
    allows the address to be taken again, a suite that runs twice in
    a row would otherwise find the one of the run before still held */
    listener = SOCKET_CREATE(AF_INET, SOCK_STREAM, 0);
    SOCKET_SET_OPTIONS(
        listener,
        SOCKET_OPTIONS_LEVEL_SOCKET,
        SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET,
        flag
    );

    /* the address is populated the very same way the service does
    it, the macro that would build one is written against a wider
    structure than the one it fills and is used by nothing */
    memset(&address, 0, sizeof(address));
    address.sin_family = SOCKET_INTERNET_TYPE;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    SOCKET_BIND(listener, address);
    SOCKET_LISTEN(listener);

    /* the connecting end and the accepted one, the second of them
    being what the mechanism is handed and the first what a test
    writes into so that there is something waiting to be read */
    *client = SOCKET_CREATE(AF_INET, SOCK_STREAM, 0);
    SOCKET_CONNECT_SIZE(*client, address, sizeof(address));
    *server = SOCKET_ACCEPT(listener, &peer, peer_size);

    /* both ends stop waiting on the reading of them, which is what
    the service does with every connection it accepts, a handler that
    reached for bytes that never arrived would otherwise never return */
    SOCKET_SET_NON_BLOCKING(*server, flags);
    SOCKET_SET_NON_BLOCKING(*client, flags);

    SOCKET_CLOSE(listener);
}

/* opens a service that is not bound to anything, only the mechanism
that it waits through is wanted and the binding of a socket would
collide with whatever else the suite happens to be running */
static void _create_polling_test(struct service_t **service_pointer, unsigned short port) {
    struct service_t *service;
    struct hash_map_t *arguments;

    create_service(
        &service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    load_specifications(service);
    create_hash_map(&arguments, 0);
    _default_options_service(service, arguments);
    delete_hash_map(arguments);

    service->options->port = port;
    service->options->load_modules = 0;
    service->options->workers = 0;
    service->options->ip6 = 0;
    calculate_options_service(service);
    calculate_locations_service(service);

    open_service(service);

    *service_pointer = service;
}

static void _delete_polling_test(struct service_t *service) {
    close_service(service);
    delete_service(service);
}

const char *test_polling_connection(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 1);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 1);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    /* the adding of the connection to the service is what hands it
    to the mechanism, which from that point on reports whatever
    happens on its socket, and it is what the accepting of one does */
    error = add_connection_service(service, connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* and the removing takes it out again, the connection itself is
    left for the loop to delete at the end of the cycle */
    error = remove_connection_service(service, connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_read(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 2);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 2);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    add_connection_service(service, connection);
    connection->on_read = _on_read_polling_test;

    /* the interest in reading is turned on and off again through the
    operations of the connection, which are what the serving of a
    message reaches for and what keeps the flag of the connection in
    step with what the mechanism underneath has been told */
    /* a connection is born already registered for the reading of it,
    which is the state that the accepting of one leaves it in */
    V_ASSERT(connection->read_registered == TRUE);

    error = unregister_read_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->read_registered == FALSE);

    /* and on again, so that a connection that is served more than one
    message over its life is exercised rather than assumed */
    error = register_read_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->read_registered == TRUE);

    error = unregister_read_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->read_registered == FALSE);

    remove_connection_service(service, connection);
    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_write(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 3);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 3);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    add_connection_service(service, connection);
    connection->on_write = _on_write_polling_test;

    /* a connection that is not write valid is never taken up, the
    writing only becomes possible once the mechanism has said so */
    connection->write_valid = FALSE;
    error = register_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* one that is write valid is taken up, and the flag it carries
    is what keeps a second request from taking it up again */
    connection->write_valid = TRUE;
    error = register_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->write_registered == TRUE);

    error = register_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    error = unregister_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->write_registered == FALSE);

    /* a connection that carries no handler for the writing is never
    taken up either, there would be nothing at all to call for it */
    connection->write_valid = TRUE;
    connection->on_write = NULL;
    error = register_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* nor is one that is no longer open, a connection on its way out
    has nothing more that may be written to it */
    connection->on_write = _on_write_polling_test;
    connection->status = STATUS_CLOSED;
    error = register_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* and one that is open and able is taken up and then driven, a
    pending write being performed at the start of a cycle rather than
    at the moment it was asked for */
    connection->status = STATUS_OPEN;
    connection->write_registered = FALSE;
    error = register_write_connection(connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    _write_count = 0;
    service->polling->timeout = 0;
    error = service->polling->poll(service->polling);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(_write_count > 0);

    remove_connection_service(service, connection);
    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_gather(void) {
    /* allocates space for the pair of sockets, for the connection built
    over one of them, for the value at the head of its queue, for the
    buffers the other end reads into, for the value that is larger than
    the socket takes at once and for the last bytes that arrived */
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct connection_t *connection;
    struct data_t *data;
    unsigned char buffer[64];
    unsigned char drain[65536];
    unsigned char last[4];
    unsigned char *large;
    size_t large_size = 1 << 24;
    size_t total = 0;
    size_t index;
    long read_bytes;
    ERROR_CODE error;

    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 10);
    create_connection(&connection, server);
    connection->status = STATUS_OPEN;

    /* points the registration of the writing at a stub, the sending
    is driven by hand rather than by a mechanism */
    connection->register_write = register_write_test_connection;
    connection->unregister_write = register_write_test_connection;

    /* the headers of a response and the payload that follows them are
    queued apart and go out together, in the order they were queued and
    with the callback of each of them called in its turn */
    _sent_count = 0;
    _sent_order[0] = '\0';
    write_connection_c(connection, (unsigned char *) "HEAD", 4, _on_sent_polling_test, (void *) 1, FALSE);
    write_connection_c(connection, (unsigned char *) "BODY", 4, _on_sent_polling_test, (void *) 2, FALSE);
    error = write_handler_stream_io(connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(connection->write_queue->size, 0);
    V_ASSERT_EQ_S(_sent_order, "12");

    read_bytes = (long) SOCKET_RECEIVE(client, (char *) buffer, sizeof(buffer), 0);
    V_ASSERT_EQ_I((int) read_bytes, 8);
    V_ASSERT_MEM(buffer, "HEADBODY", 8);

    /* a value larger than the socket takes at once goes out in part
    and the rest of it waits, together with whatever was queued behind
    it, the value being shrunk to what is left rather than sent again,
    the sending reports that the socket would block from then on */
    large = (unsigned char *) MALLOC(large_size);
    memset(large, 'x', large_size);
    write_connection_c(connection, large, (unsigned int) large_size, NULL, NULL, TRUE);
    write_connection_c(connection, (unsigned char *) "TAIL", 4, _on_sent_polling_test, (void *) 3, FALSE);
    error = write_handler_stream_io(connection);
    V_ASSERT_EQ_U(error, 2);
    V_ASSERT_EQ_U(connection->write_queue->size, 2);
    peek_value_linked_list(connection->write_queue, (void **) &data);
    V_ASSERT(data->size < large_size);
    V_ASSERT(data->data > large);
    V_ASSERT_EQ_U(data->size + (size_t) (data->data - large), large_size);

    /* the other end is drained and the sending driven again for as
    long as something is left, every byte arrives in its order and the
    ones that close the whole of it belong to the value queued last */
    memset(last, 0, sizeof(last));
    while(TRUE) {
        while(TRUE) {
            read_bytes = (long) SOCKET_RECEIVE(client, (char *) drain, sizeof(drain), 0);
            if(read_bytes <= 0) { break; }
            if(read_bytes >= 4) {
                memcpy(last, drain + read_bytes - 4, 4);
            } else {
                for(index = 0; index < (size_t) read_bytes; index++) {
                    memmove(last, last + 1, 3);
                    last[3] = drain[index];
                }
            }
            total += (size_t) read_bytes;
        }
        if(connection->write_queue->size == 0) { break; }
        error = write_handler_stream_io(connection);
        V_ASSERT(error != 1);
    }
    V_ASSERT_EQ_U(total, large_size + 4);
    V_ASSERT_MEM(last, "TAIL", 4);
    V_ASSERT_EQ_S(_sent_order, "123");

    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    delete_connection(connection);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_event(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    size_t index;
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 4);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 4);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    connection->on_read = _on_read_polling_test;
    connection->on_handshake = _on_handshake_polling_test;
    connection->status = STATUS_HANDSHAKE;
    add_connection_service(service, connection);

    V_ASSERT(server > 0);
    V_ASSERT(client > 0);

    /* something is put on the other end of the pair, so that the
    waiting has an actual event to hand back rather than timing out */
    V_ASSERT_EQ_I((int) SOCKET_SEND(client, "viriatum", 8, 0), 8);

    /* the waiting is driven the way a loop drives it rather than once,
    a socket is writable from the moment it is opened so the very first
    wait comes back with that alone and is over before whatever was
    sent has had the chance to land on the other end */
    _read_count = 0;
    _handshake_count = 0;
    service->polling->timeout = 50;
    for(index = 0; index < 20; index++) {
        error = service->polling->poll(service->polling);
        V_ASSERT(!IS_ERROR_CODE(error));
        error = service->polling->call(service->polling);
        V_ASSERT(!IS_ERROR_CODE(error));
        if(_read_count > 0) { break; }
    }

    /* the connection that had something waiting on it is handed back
    and the handler of the reading is called for it */
    V_ASSERT(_read_count > 0);
    V_ASSERT(_handshake_count > 0);
    V_ASSERT(connection->read_valid == TRUE);

    remove_connection_service(service, connection);
    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_closed(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    size_t index;
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 6);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 6);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    connection->on_read = _on_read_polling_test;
    connection->on_error = _on_error_polling_test;
    add_connection_service(service, connection);

    /* the other end goes away without saying anything, which is what
    a client that is closed rather than shut down does and what a
    connection meets far more often than the orderly ending */
    SOCKET_CLOSE(client);

    _read_count = 0;
    _error_count = 0;
    service->polling->timeout = 50;
    for(index = 0; index < 20; index++) {
        error = service->polling->poll(service->polling);
        V_ASSERT(!IS_ERROR_CODE(error));
        error = service->polling->call(service->polling);
        V_ASSERT(!IS_ERROR_CODE(error));
        if(_read_count > 0 || _error_count > 0) { break; }
    }

    /* the connection is handed back one way or the other, a peer that
    has gone shows up either as something to read, which reads as
    nothing at all, or as an outright error of the connection */
    V_ASSERT(_read_count > 0 || _error_count > 0);

    remove_connection_service(service, connection);

    SOCKET_CLOSE(server);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_gone(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;
    struct connection_t *other;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 7);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 7);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    connection->on_read = _on_read_polling_test;
    add_connection_service(service, connection);

    /* the descriptor of the connection is closed without the
    mechanism being told, which is the state that every one of the
    operations below has an error path for, a server that is handed a
    descriptor that no longer exists must report it and carry on
    rather than take the whole of the serving down with it */
    SOCKET_CLOSE(server);

    unregister_read_connection(connection);
    register_read_connection(connection);
    remove_connection_service(service, connection);

    /* whatever the operations above made of it, the mechanism is
    still able to take another connection and wait on it, which is
    the thing that actually matters about an error of this kind */
    SOCKET_CLOSE(client);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 8);
    create_connection(&other, server);
    other->service = service;
    other->status = STATUS_OPEN;
    other->on_read = _on_read_polling_test;

    error = add_connection_service(service, other);
    V_ASSERT(!IS_ERROR_CODE(error));

    service->polling->timeout = 0;
    error = service->polling->poll(service->polling);
    V_ASSERT(!IS_ERROR_CODE(error));
    error = service->polling->call(service->polling);
    V_ASSERT(!IS_ERROR_CODE(error));

    remove_connection_service(service, other);

    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_outstanding(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 5);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 5);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    add_connection_service(service, connection);
    connection->on_read = _on_read_polling_test;

    /* the connection is left pending a read at the beginning of the
    next cycle, which is what a handler that could not take all of
    what was waiting asks for */
    error = service->polling->add_outstanding(service->polling, connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->is_outstanding == TRUE);

    /* asking for it twice never queues it twice, the flag above is
    what the second of the calls is turned away by */
    error = service->polling->add_outstanding(service->polling, connection);
    V_ASSERT(!IS_ERROR_CODE(error));

    /* the pending operations are driven before the waiting, so the
    handler of the reading is called without anything having arrived */
    _read_count = 0;
    service->polling->timeout = 0;
    error = service->polling->poll(service->polling);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(_read_count > 0);
    V_ASSERT(connection->is_outstanding == FALSE);

    remove_connection_service(service, connection);
    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_polling_discarded(void) {
    /* allocates space for the service that carries the mechanism, for
    the pair of sockets and for the connection built over one of them */
    ERROR_CODE error;
    SOCKET_HANDLE server;
    SOCKET_HANDLE client;
    struct service_t *service;
    struct connection_t *connection;

    _create_polling_test(&service, VIRIATUM_TEST_POLLING_PORT + 9);
    _create_pair_polling_test(&server, &client, POLLING_TEST_PORT + 9);

    create_connection(&connection, server);
    connection->service = service;
    connection->status = STATUS_OPEN;
    add_connection_service(service, connection);
    connection->on_read = _on_read_polling_test;

    /* the connection is left pending a read and is then taken out
    of the service before the cycle that would drive it, which is
    what a handler that closes the connection it could not finish
    reading ends up asking for */
    error = service->polling->add_outstanding(service->polling, connection);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT(connection->is_outstanding == TRUE);

    remove_connection_service(service, connection);

    /* the connection is no longer pending anything, the taking of
    it out of the service is what says so, and it is still around
    to be asked because the release of it is only queued */
    V_ASSERT(connection->is_outstanding == FALSE);

    /* the call is what releases the connection that was queued and
    the waiting that follows is where a connection that stayed
    pending would be reached again, by then already released */
    _read_count = 0;
    service->polling->timeout = 0;
    error = service->polling->call(service->polling);
    V_ASSERT(!IS_ERROR_CODE(error));
    error = service->polling->poll(service->polling);
    V_ASSERT(!IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(_read_count, 0);

    SOCKET_CLOSE(server);
    SOCKET_CLOSE(client);
    _delete_polling_test(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
