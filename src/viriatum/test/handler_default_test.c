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

#include "handler_default_test.h"

/**
 * The handler that the tests install as the one the message is
 * being served by, it carries the operations of the default one.
 */
static struct http_handler_t _handler;

/**
 * Builds the complete chain of a connection together with the
 * message that the default handler is going to be driven for,
 * setting the handler the very same way a connection would.
 *
 * @param context_pointer The pointer to the test context that has
 * been built.
 * @param http_request_pointer The pointer to the message that is
 * going to be served.
 * @param flags The flags that the message carries, they decide
 * whether the connection is kept alive.
 */
static void _create_handler_default_test(
    struct test_context_t **context_pointer,
    struct http_request_t **http_request_pointer,
    unsigned char flags
) {
    struct test_context_t *context;
    struct http_request_t *http_request;

    create_test_context(&context);
    create_test_connection(context);

    create_http_request(&http_request);
    http_request->parameters = context->connection;
    http_request->method = HTTP_GET;
    http_request->version = HTTP11;
    http_request->flags = flags;

    /* installs the handler as the one the message is being served
    by, the setting of it is what builds the pipeline */
    _handler.name = (unsigned char *) "default";
    _handler.resolve_index = FALSE;
    _handler.set = set_handler_default;
    _handler.unset = unset_handler_default;
    _handler.reset = NULL;
    context->http_connection->request = http_request;
    context->http_connection->http_handler = &_handler;
    set_handler_default(context->http_connection);

    *context_pointer = context;
    *http_request_pointer = http_request;
}

/**
 * Releases the chain of a connection together with the message
 * that was being served over it, the handler is only unset when
 * the completion of a write has not done it already.
 *
 * @param context The test context to be released.
 * @param http_request The message to be released.
 */
static void _delete_handler_default_test(
    struct test_context_t *context,
    struct http_request_t *http_request
) {
    if(context->http_connection->http_handler != NULL) {
        unset_handler_default(context->http_connection);
        context->http_connection->http_handler = NULL;
    }
    context->http_connection->request = NULL;
    delete_http_request(http_request);
    delete_test_connection(context);
    delete_test_context(context);
}

const char *test_handler_default_response(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct http_settings_t *http_settings;
    unsigned char written[1024];
    size_t size;
    ERROR_CODE error;

    _create_handler_default_test(&context, &http_request, FLAG_KEEP_ALIVE);
    http_settings = context->http_connection->http_settings;

    /* the setting of the handler installs the complete pipeline, so
    every one of the callbacks of a message is reachable */
    V_ASSERT_NOT_NULL(http_settings->on_message_begin);
    V_ASSERT_NOT_NULL(http_settings->on_url);
    V_ASSERT_NOT_NULL(http_settings->on_message_complete);

    /* drives the message through the callbacks of the handler, the
    very same sequence that the parser of a connection produces */
    error = http_settings->on_message_begin(http_request);
    V_ASSERT(error == 0);
    error = http_settings->on_url(http_request, (unsigned char *) "/index.html", 11);
    V_ASSERT(error == 0);
    error = http_settings->on_header_field(http_request, (unsigned char *) "Accept", 6);
    V_ASSERT(error == 0);
    error = http_settings->on_header_value(http_request, (unsigned char *) "text/html", 9);
    V_ASSERT(error == 0);
    error = http_settings->on_headers_complete(http_request);
    V_ASSERT(error == 0);
    error = http_settings->on_body(http_request, (unsigned char *) "body", 4);
    V_ASSERT(error == 0);
    error = http_settings->on_path(http_request, (unsigned char *) "/index.html", 11);
    V_ASSERT(error == 0);
    error = http_settings->on_location(http_request, 0, 0);
    V_ASSERT(error == 0);
    error = http_settings->on_virtual_url(http_request, (unsigned char *) "/index.html", 11);
    V_ASSERT(error == 0);
    error = http_settings->on_message_complete(http_request);
    V_ASSERT(error == 0);

    /* completes the writes, the payload of the response travels as a
    write of its own that follows the one of the headers */
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Connection: keep-alive\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 14\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n" HANDLER_DEFAULT_MESSAGE));

    /* the message is meant to be kept alive, so the completion of the
    last of the writes does not take the connection down */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    /* the completion of the write has unset the handler, which is
    what releases whatever it was carrying for the message */
    V_ASSERT_NULL(context->http_connection->http_handler);

    _delete_handler_default_test(context, http_request);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_default_close(void) {
    /* allocates space for the chain of the connection and for the
    message that is not meant to be kept alive */
    struct test_context_t *context;
    struct http_request_t *http_request;
    unsigned char written[1024];
    size_t size;

    _create_handler_default_test(&context, &http_request, 0);

    message_complete_callback_handler_default(http_request);

    /* the closing only happens once the response has actually left
    this end, the peer is answered before it */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';
    V_ASSERT_NOT_NULL(strstr((char *) written, "Connection: close\r\n"));
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);

    _delete_handler_default_test(context, http_request);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
const char *test_handler_default_stream(void) {
    /* allocates space for the chain of the connection and for the
    responses that the messages produce */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    size_t index;
    ERROR_CODE error;

    /* the message as it travels on the wire, the parser of the
    connection is the one that takes it apart, this version of the
    protocol keeps a connection alive without being asked to */
    static const char *request =
        "POST /submit HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 4\r\n"
        "\r\n"
        "body";

    create_test_context(&context);
    create_test_connection(context);

    /* installs the handler as the one a new message is served by,
    the reading of the connection is what reaches it */
    _handler.name = (unsigned char *) "default";
    _handler.resolve_index = FALSE;
    _handler.set = set_handler_default;
    _handler.unset = unset_handler_default;
    _handler.reset = NULL;
    context->http_connection->base_handler = &_handler;

    /* two messages travel on the connection one after the other, the
    second of them is served from a message that carries nothing at
    all of the one that came before it */
    for(index = 0; index < 2; index++) {
        error = data_handler_stream_http(
            context->io_connection,
            (unsigned char *) request,
            strlen(request)
        );
        V_ASSERT(error == 0);

        size = flush_test_connection(context, written, sizeof(written));
        V_ASSERT(size > 0 && size < sizeof(written));
        written[size] = '\0';

        V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
        V_ASSERT_NOT_NULL(strstr((char *) written, "Connection: keep-alive\r\n"));
        V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 14\r\n"));
        V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n" HANDLER_DEFAULT_MESSAGE));
    }

    /* the connection is kept alive through both of the messages, so
    nothing has taken it down along the way */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    if(context->http_connection->http_handler != NULL) {
        unset_handler_default(context->http_connection);
        context->http_connection->http_handler = NULL;
    }
    delete_test_connection(context);
    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
const char *test_handler_default_persistence(void) {
    /* allocates space for the chain of the connection and for the
    responses that the messages produce */
    struct test_context_t *context;
    unsigned char written[1024];
    size_t size;
    size_t index;
    ERROR_CODE error;

    /* the messages that decide whether the connection is kept alive,
    together with what each one of them is answered with and with the
    number of the closings that follows it */
    static const char *messages[] = {
        "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",
        "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n",
        "GET / HTTP/1.0\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
    };
    static const char *answers[] = {
        "Connection: keep-alive\r\n",
        "Connection: close\r\n",
        "Connection: close\r\n",
        "Connection: keep-alive\r\n"
    };
    static const size_t closings[] = { 0, 1, 1, 0 };

    /* the most recent version of the protocol keeps a connection
    alive unless it is closed on purpose and the older one does the
    opposite, which is what the specification requires of each */
    for(index = 0; index < sizeof(messages) / sizeof(messages[0]); index++) {
        create_test_context(&context);
        create_test_connection(context);

        _handler.name = (unsigned char *) "default";
        _handler.resolve_index = FALSE;
        _handler.set = set_handler_default;
        _handler.unset = unset_handler_default;
        _handler.reset = NULL;
        context->http_connection->base_handler = &_handler;

        error = data_handler_stream_http(
            context->io_connection,
            (unsigned char *) messages[index],
            strlen(messages[index])
        );
        V_ASSERT(error == 0);

        size = flush_test_connection(context, written, sizeof(written));
        V_ASSERT(size > 0 && size < sizeof(written));
        written[size] = '\0';

        V_ASSERT_NOT_NULL(strstr((char *) written, answers[index]));
        V_ASSERT_EQ_U(get_closed_test_connection(), closings[index]);

        if(context->http_connection->http_handler != NULL) {
            unset_handler_default(context->http_connection);
            context->http_connection->http_handler = NULL;
        }
        delete_test_connection(context);
        delete_test_context(context);
    }

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

