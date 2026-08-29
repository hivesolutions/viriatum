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

#include "handler_dispatch_test.h"

const char *test_dispatch_handler_context_keepalive(void) {
    /* allocates space for the HTTP request and for a
    handler file context to simulate keep-alive reuse */
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP request and the handler file context,
    wiring them together through the context pointer to
    simulate a file handler that has just processed a request */
    create_http_request(&http_request);
    create_handler_file_context(&handler_file_context);
    http_request->context = handler_file_context;

    /* simulates the file handler unset which deletes the
    context structure and nullifies the pointer, matching
    the corrected behavior of _unset_http_request_handler_file */
    delete_handler_file_context(handler_file_context);
    http_request->context = NULL;

    /* simulates the dispatch handler being re-set for the
    next keep-alive request, context should be NULL at this
    point because the file handler properly cleaned up */
    _set_http_request_handler_dispatch(http_request);

    /* calls the dispatch handler unset, which is triggered
    when the dispatch handler finds a target handler and
    switches to it, context is NULL so no free should occur */
    _unset_http_request_handler_dispatch(http_request);

    /* verifies that the context pointer remains NULL after
    the dispatch handler unset (no corruption occurred) */
    V_ASSERT(http_request->context == NULL);

    /* cleans up the request */
    delete_http_request(http_request);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
const char *test_dispatch_handler_response(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct http_settings_t *http_settings;
    unsigned char written[2048];
    size_t size;
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

    /* the setting of the handler installs the complete pipeline, the
    dispatching happens as the url of a message reaches it */
    set_handler_dispatch(context->http_connection);
    V_ASSERT_NOT_NULL(http_settings->on_message_begin);
    V_ASSERT_NOT_NULL(http_settings->on_url);
    V_ASSERT_NOT_NULL(http_settings->on_message_complete);

    error = http_settings->on_message_begin(http_request);
    V_ASSERT(error == 0);
    error = http_settings->on_header_field(http_request, (unsigned char *) "Accept", 6);
    V_ASSERT(error == 0);
    error = http_settings->on_header_value(http_request, (unsigned char *) "*/*", 3);
    V_ASSERT(error == 0);
    error = http_settings->on_headers_complete(http_request);
    V_ASSERT(error == 0);
    error = http_settings->on_body(http_request, (unsigned char *) "body", 4);
    V_ASSERT(error == 0);
    error = http_settings->on_path(http_request, (unsigned char *) "/none", 5);
    V_ASSERT(error == 0);
    error = http_settings->on_location(http_request, 0, 0);
    V_ASSERT(error == 0);
    error = http_settings->on_virtual_url(http_request, (unsigned char *) "/none", 5);
    V_ASSERT(error == 0);

    /* a message that has reached no handler at all is answered with
    the error that tells the client that this end is at fault */
    error = http_settings->on_message_complete(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 500 Internal Server Error\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n500 - Internal Server Error - "));

    /* the message is meant to be kept alive, so the completion of the
    write of the error does not take the connection down */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    unset_handler_dispatch(context->http_connection);
    V_ASSERT_NULL(http_settings->on_message_begin);

    context->http_connection->request = NULL;
    delete_http_request(http_request);
    delete_test_connection(context);
    delete_test_context(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
