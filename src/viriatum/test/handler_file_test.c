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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"
#include "viriatum.h"

#include "handler_file_test.h"

const char *test_handler_file_context() {
    /* allocates space for the handler file context
    structure to be used in the test */
    struct handler_file_context_t *handler_file_context;

    /* creates the handler file context and verifies
    that the default values are properly initialized */
    create_handler_file_context(&handler_file_context);

    V_ASSERT(handler_file_context->base_path == NULL);
    V_ASSERT(handler_file_context->auth_basic == NULL);
    V_ASSERT(handler_file_context->auth_file == NULL);
    V_ASSERT(handler_file_context->file == NULL);
    V_ASSERT(handler_file_context->file_size == 0);
    V_ASSERT(handler_file_context->initial_byte == 0);
    V_ASSERT(handler_file_context->final_byte == 0);
    V_ASSERT(handler_file_context->flags == 0);
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);
    V_ASSERT(handler_file_context->template_handler == NULL);
    V_ASSERT(handler_file_context->cache_control_status == 0);
    V_ASSERT(handler_file_context->etag_status == 0);
    V_ASSERT(handler_file_context->authorization_status == 0);
    V_ASSERT(handler_file_context->range_status == 0);

    /* deletes the handler file context */
    delete_handler_file_context(handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_url() {
    /* allocates space for the error code returned by the
    callback and for the HTTP parser and context structures */
    ERROR_CODE error;
    struct http_parser_t *http_parser;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP parser and the handler file context
    then wires them together through the context pointer */
    create_http_parser(&http_parser, TRUE);
    create_handler_file_context(&handler_file_context);
    http_parser->context = handler_file_context;
    http_parser->method = HTTP_GET;

    /* tests that a normal url is properly parsed
    and stored in the handler file context */
    error = url_callback_handler_file(
        http_parser,
        (unsigned char *) "/index.html",
        11
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) handler_file_context->url, "/index.html") == 0);

    /* tests that a url with query string has
    the query parameters stripped from the path */
    error = url_callback_handler_file(
        http_parser,
        (unsigned char *) "/page?id=1",
        10
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) handler_file_context->url, "/page") == 0);

    /* tests that a path traversal url is rejected
    by the url callback returning an error code */
    error = url_callback_handler_file(
        http_parser,
        (unsigned char *) "/../etc/passwd",
        14
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a percent-encoded path traversal is
    also rejected after decoding takes place */
    error = url_callback_handler_file(
        http_parser,
        (unsigned char *) "/%2e%2e/etc/passwd",
        18
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* deletes both the context and the parser to
    avoid any memory leak from the test */
    delete_handler_file_context(handler_file_context);
    delete_http_parser(http_parser);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_header_field() {
    /* allocates space for the HTTP parser and the
    handler file context structures */
    struct http_parser_t *http_parser;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP parser and the handler file context
    then wires them together through the context pointer */
    create_http_parser(&http_parser, TRUE);
    create_handler_file_context(&handler_file_context);
    http_parser->context = handler_file_context;

    /* tests that the "Range" header (5 chars) is correctly
    identified and sets the appropriate next header state */
    header_field_callback_handler_file(
        http_parser,
        (unsigned char *) "Range",
        5
    );
    V_ASSERT(handler_file_context->range_status == 1);
    V_ASSERT(handler_file_context->next_header == RANGE);

    /* resets the next header to undefined before
    testing the next header field recognition */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* tests that the "Cache-Control" header (13 chars) is
    correctly identified as cache control type */
    header_field_callback_handler_file(
        http_parser,
        (unsigned char *) "Cache-Control",
        13
    );
    V_ASSERT(handler_file_context->cache_control_status == 1);
    V_ASSERT(handler_file_context->next_header == CACHE_CONTROL);

    /* resets the next header to undefined */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* tests that the "Authorization" header (13 chars) is
    correctly identified as authorization type */
    header_field_callback_handler_file(
        http_parser,
        (unsigned char *) "Authorization",
        13
    );
    V_ASSERT(handler_file_context->authorization_status == 1);
    V_ASSERT(handler_file_context->next_header == AUTHORIZATION);

    /* resets the next header to undefined */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* tests that the "If-None-Match" header (13 chars) is
    correctly identified as etag type */
    header_field_callback_handler_file(
        http_parser,
        (unsigned char *) "If-None-Match",
        13
    );
    V_ASSERT(handler_file_context->etag_status == 1);
    V_ASSERT(handler_file_context->next_header == ETAG);

    /* resets the next header to undefined */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* tests that an unknown header does not change
    the next header state from undefined */
    header_field_callback_handler_file(
        http_parser,
        (unsigned char *) "X-Custom",
        8
    );
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* deletes both the context and the parser */
    delete_handler_file_context(handler_file_context);
    delete_http_parser(http_parser);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_header_value() {
    /* allocates space for the HTTP parser and the
    handler file context structures */
    struct http_parser_t *http_parser;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP parser and the handler file context
    then wires them together through the context pointer */
    create_http_parser(&http_parser, TRUE);
    create_handler_file_context(&handler_file_context);
    http_parser->context = handler_file_context;

    /* simulates parsing a "Range" header by setting the
    next header state and then calling the value callback */
    handler_file_context->next_header = RANGE;
    header_value_callback_handler_file(
        http_parser,
        (unsigned char *) "bytes=0-499",
        11
    );
    V_ASSERT(strcmp((char *) handler_file_context->range, "bytes=0-499") == 0);
    V_ASSERT(handler_file_context->range_status == 2);
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* simulates parsing a "Cache-Control" header value */
    handler_file_context->next_header = CACHE_CONTROL;
    header_value_callback_handler_file(
        http_parser,
        (unsigned char *) "no-cache",
        8
    );
    V_ASSERT(strcmp((char *) handler_file_context->cache_control, "no-cache") == 0);
    V_ASSERT(handler_file_context->cache_control_status == 2);
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* simulates parsing an "Authorization" header value */
    handler_file_context->next_header = AUTHORIZATION;
    header_value_callback_handler_file(
        http_parser,
        (unsigned char *) "Basic dGVzdA==",
        14
    );
    V_ASSERT(strcmp((char *) handler_file_context->authorization, "Basic dGVzdA==") == 0);
    V_ASSERT(handler_file_context->authorization_status == 2);
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* verifies that an undefined next header causes the
    value callback to leave context fields unchanged */
    handler_file_context->next_header = UNDEFINED_HEADER;
    memset(handler_file_context->range, 0, 128);
    header_value_callback_handler_file(
        http_parser,
        (unsigned char *) "some-value",
        10
    );
    V_ASSERT(handler_file_context->range[0] == '\0');

    /* deletes both the context and the parser */
    delete_handler_file_context(handler_file_context);
    delete_http_parser(http_parser);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
