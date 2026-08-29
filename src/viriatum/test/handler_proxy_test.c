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

#include "handler_proxy_test.h"

/**
 * Builds the complete chain of the connection of a client together
 * with the context of the proxy and the connection that stands in
 * for the one of the upstream.
 * The connection of the upstream carries no socket at all, only the
 * queue of it is ever reached by a test.
 *
 * @param context_pointer The pointer to the test context that has
 * been built.
 * @param handler_proxy_context_pointer The pointer to the context
 * of the proxy that has been built.
 * @param connection_pointer The pointer to the connection that
 * stands in for the one of the upstream.
 */
static void _create_handler_proxy_test(
    struct test_context_t **context_pointer,
    struct handler_proxy_context_t **handler_proxy_context_pointer,
    struct connection_t **connection_pointer
) {
    struct test_context_t *context;
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_c;
    struct http_request_t *http_request;

    create_test_context(&context);
    create_test_connection(context);

    /* builds the connection that stands in for the one of the
    upstream, only the queue of it is ever observed */
    create_connection(&connection_c, 0);
    connection_c->service = context->service;
    connection_c->register_write = register_write_test_connection;
    connection_c->unregister_write = register_write_test_connection;

    create_handler_proxy_context(&handler_proxy_context);

    /* builds the message of the client, it is the one the response of
    the upstream is written for */
    create_http_request(&http_request);
    http_request->context = handler_proxy_context;
    http_request->parameters = context->connection;
    http_request->method = HTTP_GET;
    http_request->version = HTTP11;
    http_request->flags = FLAG_KEEP_ALIVE;

    handler_proxy_context->connection = context->connection;
    handler_proxy_context->connection_c = connection_c;
    handler_proxy_context->http_request = http_request;
    handler_proxy_context->pending = TRUE;

    /* the message of the parser of the upstream already carries the
    context, it is the one the callbacks of it are driven with */
    handler_proxy_context->http_parser->request->version = HTTP11;
    handler_proxy_context->http_parser->request->flags = FLAG_KEEP_ALIVE;

    *context_pointer = context;
    *handler_proxy_context_pointer = handler_proxy_context;
    *connection_pointer = connection_c;
}

/**
 * Releases the chain of the connection of a client together with
 * the context of the proxy and the connection of the upstream.
 *
 * @param context The test context to be released.
 * @param handler_proxy_context The context to be released.
 * @param connection_c The connection of the upstream.
 */
static void _delete_handler_proxy_test(
    struct test_context_t *context,
    struct handler_proxy_context_t *handler_proxy_context,
    struct connection_t *connection_c
) {
    delete_http_request(handler_proxy_context->http_request);
    delete_handler_proxy_context(handler_proxy_context);
    delete_connection(connection_c);
    context->http_connection->request = NULL;
    delete_test_connection(context);
    delete_test_context(context);
}

const char *test_handler_proxy_request(void) {
    /* allocates space for the chain of the connection, for the
    context of the proxy and for the field that is gathered */
    struct test_context_t *context;
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_c;
    struct http_request_t *http_request;
    struct data_t *data;
    char value[512];
    size_t index;
    ERROR_CODE error;

    _create_handler_proxy_test(&context, &handler_proxy_context, &connection_c);
    http_request = handler_proxy_context->http_request;

    /* builds a value that is long enough to take the buffer past the
    size it starts its life with, so that it has to grow */
    for(index = 0; index < sizeof(value) - 1; index++) { value[index] = 'a'; }
    value[sizeof(value) - 1] = '\0';

    /* gathers the message of the client into the buffer that is
    handed to the upstream, one callback after the other */
    error = message_begin_callback_handler_proxy(http_request);
    V_ASSERT(error == 0);
    V_ASSERT_EQ_U(handler_proxy_context->buffer_size, 0);

    error = url_callback_handler_proxy(http_request, (unsigned char *) "/proxied", 8);
    V_ASSERT(error == 0);

    for(index = 0; index < 4; index++) {
        error = header_field_callback_handler_proxy(http_request, (unsigned char *) "X-Long", 6);
        V_ASSERT(error == 0);
        error = header_value_callback_handler_proxy(
            http_request,
            (unsigned char *) value,
            sizeof(value) - 1
        );
        V_ASSERT(error == 0);
    }

    error = headers_complete_callback_handler_proxy(http_request);
    V_ASSERT(error == 0);
    error = body_callback_handler_proxy(http_request, (unsigned char *) "body", 4);
    V_ASSERT(error == 0);

    /* the buffer has grown past the size it started at and it holds
    every one of the fields that have been gathered */
    V_ASSERT(handler_proxy_context->buffer_max_size > 1024);
    V_ASSERT(handler_proxy_context->buffer_size > 1024);
    V_ASSERT_MEM(handler_proxy_context->buffer, "X-Long: ", 8);

    /* the line that opens the message of the upstream names the
    version of HTTP/1.1 whatever the one serving the client is, the
    most recent one carries no request line at all and an upstream
    speaking the older one would refuse the name of it */
    handler_proxy_context->buffer_size = 0;
    http_request->version = HTTP20;
    write_request_handler_proxy(handler_proxy_context, "GET", "/proxied");
    V_ASSERT_EQ_U(handler_proxy_context->buffer_size, 23);
    V_ASSERT_MEM(handler_proxy_context->buffer, "GET /proxied HTTP/1.1\r\n", 23);

    handler_proxy_context->buffer_size = 0;
    http_request->version = HTTP11;
    write_request_handler_proxy(handler_proxy_context, "POST", "/other");
    V_ASSERT_MEM(handler_proxy_context->buffer, "POST /other HTTP/1.1\r\n", 22);

    /* the completion of the message hands the buffer to the upstream,
    the connection of it is the one that takes it */
    error = message_complete_callback_handler_proxy(http_request);
    V_ASSERT(error == 0);
    V_ASSERT_EQ_U(connection_c->write_queue->size, 1);
    get_value_linked_list(connection_c->write_queue, 0, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    V_ASSERT_EQ_U(data->size, handler_proxy_context->buffer_size);

    _delete_handler_proxy_test(context, handler_proxy_context, connection_c);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_proxy_response(void) {
    /* allocates space for the chain of the connection, for the
    context of the proxy and for the response it produces */
    struct test_context_t *context;
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_c;
    struct http_request_t *http_request;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_proxy_test(&context, &handler_proxy_context, &connection_c);

    /* the message of the parser of the upstream is the one that the
    callbacks of the response are driven with */
    http_request = handler_proxy_context->http_parser->request;
    http_request->status_code = 200;

    error = line_callback_backend(http_request);
    V_ASSERT(error == 0);

    /* hands a field over in the pieces that a read of it would have
    produced, the pair is only written once the next one arrives */
    error = header_field_callback_backend(http_request, (unsigned char *) "Content-", 8);
    V_ASSERT(error == 0);
    error = header_field_callback_backend(http_request, (unsigned char *) "Type", 4);
    V_ASSERT(error == 0);
    error = header_value_callback_backend(http_request, (unsigned char *) "text/", 5);
    V_ASSERT(error == 0);
    error = header_value_callback_backend(http_request, (unsigned char *) "plain", 5);
    V_ASSERT(error == 0);

    error = header_field_callback_backend(http_request, (unsigned char *) "Content-Length", 14);
    V_ASSERT(error == 0);
    error = header_value_callback_backend(http_request, (unsigned char *) "4", 1);
    V_ASSERT(error == 0);

    error = headers_complete_callback_backend(http_request);
    V_ASSERT(error == 0);
    error = body_callback_backend(http_request, (unsigned char *) "body", 4);
    V_ASSERT(error == 0);
    error = message_complete_callback_backend(http_request);
    V_ASSERT(error == 0);

    /* the message of the upstream is over, so a disconnection of it
    no longer affects the client of the proxy */
    V_ASSERT(handler_proxy_context->pending == FALSE);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    /* the response reaches the client encoded in the protocol that is
    serving it, carrying the fields of the upstream and the one that
    announces that it has travelled through this end */
    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "X-Viriatum-Proxy: 1\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Type: text/plain\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 4\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\nbody"));

    _delete_handler_proxy_test(context, handler_proxy_context, connection_c);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_proxy_reuse(void) {
    /* allocates space for the chain of the connection, for the two
    exchanges that travel on it and for the parameters that bind one
    of them to the connection of the upstream */
    struct test_context_t *context;
    struct handler_proxy_context_t *handler_proxy_context;
    struct handler_proxy_context_t *other;
    struct connection_t *connection_c;
    struct custom_parameters_t parameters;

    _create_handler_proxy_test(&context, &handler_proxy_context, &connection_c);
    create_handler_proxy_context(&other);

    /* a connection that does not exist yet is never reused, one of
    its own has to be opened for the exchange */
    V_ASSERT(reuse_backend_handler_proxy(NULL, handler_proxy_context) == FALSE);

    /* a connection that carries no parameters at all sits between two
    exchanges and so it is free to take */
    connection_c->parameters = NULL;
    V_ASSERT(reuse_backend_handler_proxy(connection_c, handler_proxy_context) == TRUE);

    /* the exchange that is already on it is the one of this very
    message whenever the connection is being reused in sequence, so
    it is taken whether it is still under way or not */
    parameters.parameters = (void *) handler_proxy_context;
    connection_c->parameters = (void *) &parameters;
    handler_proxy_context->pending = TRUE;
    V_ASSERT(reuse_backend_handler_proxy(connection_c, handler_proxy_context) == TRUE);

    /* an exchange of another message that is over leaves the
    connection free for the one that comes after it */
    parameters.parameters = (void *) other;
    other->pending = FALSE;
    V_ASSERT(reuse_backend_handler_proxy(connection_c, handler_proxy_context) == TRUE);

    /* one that is still under way keeps it, the response of it would
    otherwise be written for the message of this one */
    other->pending = TRUE;
    V_ASSERT(reuse_backend_handler_proxy(connection_c, handler_proxy_context) == FALSE);

    connection_c->parameters = NULL;
    delete_handler_proxy_context(other);
    _delete_handler_proxy_test(context, handler_proxy_context, connection_c);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_proxy_gateway(void) {
    /* allocates space for the chain of the connection, for the
    context of the proxy and for the response it produces */
    struct test_context_t *context;
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_c;
    struct http_request_t *http_request;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_proxy_test(&context, &handler_proxy_context, &connection_c);
    http_request = handler_proxy_context->http_request;

    /* a request that reaches no upstream at all is answered with the
    error that tells the client where the fault is */
    handler_proxy_context->connection_c = NULL;
    error = message_complete_callback_handler_proxy(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 502 Bad gateway\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n502 - Bad gateway - "));

    /* the message is meant to be kept alive, so the completion of the
    write of the error does not take the connection down */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    handler_proxy_context->connection_c = connection_c;
    _delete_handler_proxy_test(context, handler_proxy_context, connection_c);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
const char *test_handler_proxy_upstream(void) {
    /* allocates space for the chain of the connection, for the
    context of the proxy and for the response of the upstream */
    struct test_context_t *context;
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_c;
    unsigned char written[2048];
    size_t size;
    size_t index;

    /* the response of the upstream as it travels on the wire, the
    parser of the backend is the one that takes it apart, one of the
    fields of it carries no value at all which a valid message is
    allowed to do */
    static const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "X-Empty: \r\n"
        "X-After: kept\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "gone!";

    /* the headers of a response whose payload has not arrived yet,
    the size it announces differs from the one of the response above
    so that a value left over from it is told apart */
    static const char *headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 9\r\n"
        "\r\n";

    _create_handler_proxy_test(&context, &handler_proxy_context, &connection_c);

    /* two responses travel on the connection of the upstream one
    after the other, the second of them is taken apart from a message
    that carries nothing at all of the one that came before it */
    for(index = 0; index < 2; index++) {
        handler_proxy_context->pending = TRUE;

        /* hands the bytes of the upstream to the parser of the
        backend, the callbacks of it are what write the response */
        process_data_http_parser(
            handler_proxy_context->http_parser,
            handler_proxy_context->http_settings,
            (unsigned char *) response,
            strlen(response)
        );

        size = flush_test_connection(context, written, sizeof(written));
        V_ASSERT(size > 0 && size < sizeof(written));
        written[size] = '\0';

        /* the status of the upstream reaches the client together with
        the fields and the payload that came with it */
        V_ASSERT(strstr((char *) written, "HTTP/1.1 404 Not Found\r\n") == (char *) written);
        V_ASSERT_NOT_NULL(strstr((char *) written, "X-Viriatum-Proxy: 1\r\n"));
        V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Type: text/html\r\n"));
        V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 5\r\n"));
        V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\ngone!"));

        /* the field whose value carries nothing at all is written
        just the same, and the one that follows it reaches the client
        under its own name rather than appended to that one */
        V_ASSERT_NOT_NULL(strstr((char *) written, "X-Empty: \r\n"));
        V_ASSERT_NOT_NULL(strstr((char *) written, "X-After: kept\r\n"));

        /* the message of the upstream is over, so a disconnection of
        it no longer affects the client of the proxy */
        V_ASSERT(handler_proxy_context->pending == FALSE);
    }

    /* the size that a message announces is already saved when the
    handler is told that the headers are over, so a message whose
    payload has not arrived yet still carries it, one that is only
    saved by the parsing of the payload would carry the size of the
    message that came before it on the very same connection */
    handler_proxy_context->pending = TRUE;
    process_data_http_parser(
        handler_proxy_context->http_parser,
        handler_proxy_context->http_settings,
        (unsigned char *) headers,
        strlen(headers)
    );
    V_ASSERT_EQ_U(handler_proxy_context->http_parser->request->content_length, 9);

    _delete_handler_proxy_test(context, handler_proxy_context, connection_c);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_proxy_handler(void) {
    /* allocates space for the chain of the connection, for the
    handler of the proxy and for the location it carries */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_proxy_context_t *handler_proxy_context;
    struct proxy_handler_t *proxy_handler;
    struct http_handler_t http_handler;
    struct proxy_location_t location;
    struct http_settings_t *http_settings;
    ERROR_CODE error;

    create_test_context(&context);
    create_test_connection(context);

    create_http_request(&http_request);
    http_request->parameters = context->connection;
    http_request->method = HTTP_GET;
    http_request->version = HTTP11;
    http_request->flags = FLAG_KEEP_ALIVE;
    context->http_connection->request = http_request;
    http_settings = context->http_connection->http_settings;

    /* the setting of the handler builds the context of the message
    and installs the complete pipeline of it */
    set_handler_proxy(context->http_connection);
    V_ASSERT_NOT_NULL(http_request->context);
    V_ASSERT_NOT_NULL(http_settings->on_message_begin);
    V_ASSERT_NOT_NULL(http_settings->on_location);
    V_ASSERT_NOT_NULL(http_settings->on_virtual_url);
    handler_proxy_context = (struct handler_proxy_context_t *) http_request->context;

    /* builds the handler of the proxy together with the single
    location that the message is going to be matched against */
    create_proxy_handler(&proxy_handler, &http_handler);
    location.proxy_pass = (unsigned char *) "http://localhost:8080/";
    parse_url_static((char *) location.proxy_pass, 22, &location.url_s);
    proxy_handler->locations = &location;
    proxy_handler->locations_count = 1;

    http_handler.name = (unsigned char *) "proxy";
    context->http_connection->http_handler = &http_handler;

    /* the matching of a location carries the upstream of it into the
    context, which is where the message is going to be handed to */
    error = location_callback_handler_proxy(http_request, 0, 0);
    V_ASSERT(error == 0);
    V_ASSERT_EQ_P(handler_proxy_context->proxy_pass, location.proxy_pass);

    /* a message that follows another one on the same connection
    forgets what the one before it has gathered */
    handler_proxy_context->buffer_size = 128;
    handler_proxy_context->out_buffer_size = 128;
    reset_handler_proxy(context->http_connection);
    V_ASSERT_EQ_U(handler_proxy_context->buffer_size, 0);

    /* the buffer of the locations belongs to the test rather than to
    the handler, so it is forgotten before the release of it */
    proxy_handler->locations = NULL;
    proxy_handler->locations_count = 0;
    delete_proxy_handler(proxy_handler);
    context->http_connection->http_handler = NULL;

    /* the unsetting releases the context and takes the pipeline
    down, so that another handler may take the connection */
    unset_handler_proxy(context->http_connection);
    V_ASSERT_NULL(http_request->context);
    V_ASSERT_NULL(http_settings->on_message_begin);
    V_ASSERT_NULL(http_settings->on_virtual_url);

    context->http_connection->request = NULL;
    delete_http_request(http_request);
    delete_test_connection(context);
    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
