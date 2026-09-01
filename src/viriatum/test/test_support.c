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

#include "test_support.h"

#ifdef VIRIATUM_PLATFORM_WIN32
#define TEST_DUP(descriptor) _dup(descriptor)
#define TEST_DUP2(descriptor, target) _dup2(descriptor, target)
#define TEST_CLOSE(descriptor) _close(descriptor)
#define TEST_FILENO(stream) _fileno(stream)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#define TEST_DUP(descriptor) dup(descriptor)
#define TEST_DUP2(descriptor, target) dup2(descriptor, target)
#define TEST_CLOSE(descriptor) close(descriptor)
#define TEST_FILENO(stream) fileno(stream)
#endif

/**
 * The path of the file that the output of the process is gathered
 * into while the standard output of it is taken over.
 */
#define TEST_OUTPUT_PATH "./viriatum_test_output.txt"

/**
 * The descriptor that the standard output was pointing at before it
 * was taken over, it is what gives it back at the end.
 */
static int _output_test = -1;

/**
 * The file that whatever is written to the standard output lands in
 * while it is taken over.
 */
static FILE *_file_test = NULL;

void capture_test_output(void) {
    /* flushes whatever is still pending so that it lands on the
    output the process was pointing at and not on the file */
    fflush(stdout);

    /* keeps the descriptor of the standard output around and points
    it at the file that is going to gather what is written */
    _output_test = TEST_DUP(TEST_FILENO(stdout));
    FOPEN(&_file_test, TEST_OUTPUT_PATH, "wb+");
    if(_file_test == NULL) { return; }
    TEST_DUP2(TEST_FILENO(_file_test), TEST_FILENO(stdout));
}

size_t release_test_output(char *buffer, size_t buffer_size) {
    /* allocates space for the number of the bytes that have been
    gathered out of the file */
    size_t count = 0;

    /* flushes what has been written so that all of it is in the file
    before it is read back */
    fflush(stdout);

    /* gives the standard output back to the descriptor it was
    pointing at before the capture was started */
    TEST_DUP2(_output_test, TEST_FILENO(stdout));
    TEST_CLOSE(_output_test);
    _output_test = -1;

    /* in case the file was never opened there is nothing that could
    have been gathered out of it */
    if(_file_test == NULL) {
        if(buffer_size > 0) { buffer[0] = '\0'; }
        return 0;
    }

    /* reads what has been written back into the provided buffer and
    closes it with the end of string character */
    fseek(_file_test, 0, SEEK_SET);
    if(buffer_size > 0) {
        count = fread(buffer, sizeof(char), buffer_size - 1, _file_test);
        buffer[count] = '\0';
    }

    /* closes the file and takes it out of the file system, nothing of
    it is meant to be left behind by a run */
    fclose(_file_test);
    _file_test = NULL;
    remove(TEST_OUTPUT_PATH);

    return count;
}

void create_test_context(struct test_context_t **context_pointer) {
    /* retrieves the test context size */
    size_t context_size = sizeof(struct test_context_t);

    /* allocates space for the test context */
    struct test_context_t *context =
        (struct test_context_t *) MALLOC(context_size);

    /* creates the service options and populates the
    pre-resolved paths with the compile-time defaults
    so that handler callbacks can safely dereference them */
    create_service_options(&context->options);
    SPRINTF(
        (char *) context->options->contents_path,
        VIRIATUM_MAX_PATH_SIZE, "%s",
        VIRIATUM_CONTENTS_PATH
    );
    SPRINTF(
        (char *) context->options->resources_path,
        VIRIATUM_MAX_PATH_SIZE, "%s",
        VIRIATUM_RESOURCES_PATH
    );
    SPRINTF(
        (char *) context->options->modules_path,
        VIRIATUM_MAX_PATH_SIZE, "%s",
        VIRIATUM_MODULES_PATH
    );

    /* creates a minimal service and replaces its default
    options with the ones that were just configured */
    create_service(
        &context->service,
        (unsigned char *) "test",
        (unsigned char *) "test"
    );
    delete_service_options(context->service->options);
    context->service->options = context->options;

    /* creates a connection and wires it to the service
    so that the chain connection->service->options is
    fully valid for handler callback usage */
    create_connection(&context->connection, 0);
    context->connection->service = context->service;

    /* unsets the upper layers of the connection, only the
    tests that drive a complete connection build them */
    context->io_connection = NULL;
    context->http_connection = NULL;

    /* sets the context in the context pointer */
    *context_pointer = context;
}

/**
 * The number of times the closing of a test connection has been
 * requested, it is the way a test observes that a failure has
 * taken the connection down.
 */
static size_t _closed_test_connection = 0;

ERROR_CODE close_test_connection(struct connection_t *connection) {
    /* the connection of a test holds no socket at all, so only the
    request itself is recorded */
    _closed_test_connection++;
    RAISE_NO_ERROR;
}

void reset_closed_test_connection(void) {
    _closed_test_connection = 0;
}

size_t get_closed_test_connection(void) {
    return _closed_test_connection;
}

ERROR_CODE register_write_test_connection(struct connection_t *connection) {
    /* the connection of a test is not attached to a polling, so
    there's nothing at all to be registered, the data that has
    been queued stays in the queue for the test to observe */
    RAISE_NO_ERROR;
}

size_t flush_test_connection(struct test_context_t *context, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the data being taken out of the queue and
    for the position that the gathering of it reaches */
    struct data_t *data;
    size_t offset = 0;

    /* completes every one of the writes that are queued, in the very
    same sequence the io layer of a connection would, the completion
    of one of them may well queue the part that follows it */
    while(context->connection->write_queue->size > 0) {
        pop_value_linked_list(context->connection->write_queue, (void **) &data, TRUE);
        if(data == NULL) { break; }
        if(buffer != NULL && offset + data->size <= buffer_size) {
            memcpy(&buffer[offset], data->data, data->size);
            offset += data->size;
        }
        if(data->callback != NULL) {
            data->callback(context->connection, data, data->callback_parameters);
        }
        delete_data(data);
    }

    /* returns the number of the bytes that have been gathered */
    return offset;
}

void create_test_connection(struct test_context_t *context) {
    /* builds both of the layers that sit on top of the
    connection, the HTTP one is the substrate of the io one,
    the handler a message is served by is left unset so that
    a test installs the one it wants to observe */
    create_io_connection(&context->io_connection, context->connection);
    create_http_connection(&context->http_connection, context->io_connection);

    /* points the registration of the writing at a stub, the queue of
    the connection is otherwise never reachable in a test */
    context->connection->register_write = register_write_test_connection;
    context->connection->unregister_write = register_write_test_connection;
    context->connection->close_connection = close_test_connection;
    reset_closed_test_connection();
}

void delete_test_connection(struct test_context_t *context) {
    /* deletes the layers in the reverse order they were built,
    the HTTP one holds the parser and the settings */
    delete_http_connection(context->http_connection);
    delete_io_connection(context->io_connection);
    context->http_connection = NULL;
    context->io_connection = NULL;
}

void delete_test_context(struct test_context_t *context) {
    /* deletes the connection, note that the connection
    does not own the service reference */
    delete_connection(context->connection);

    /* deletes the service which in turn deletes the
    service options that were attached to it */
    delete_service(context->service);

    /* releases the test context structure */
    FREE(context);
}
