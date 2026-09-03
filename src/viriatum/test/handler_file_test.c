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

#include "handler_file_test.h"

const char *test_handler_file_context(void) {
    /* allocates space for the handler file context
    structure to be used in the test */
    struct handler_file_context_t *handler_file_context;

    /* creates the handler file context and verifies
    that the default values are properly initialized */
    create_handler_file_context(&handler_file_context);

    V_ASSERT(handler_file_context->base_path == NULL);
    V_ASSERT(handler_file_context->auth_basic == NULL);
    V_ASSERT(handler_file_context->auth_file == NULL);
    V_ASSERT(handler_file_context->descriptor == -1);
    V_ASSERT(handler_file_context->offset == 0);
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

void *setup_handler_file_test(void) {
    /* allocates space for the fixture that is going to carry
    the complete chain of structures required by the callbacks */
    struct handler_file_fixture_t *fixture =
        (struct handler_file_fixture_t *) MALLOC(sizeof(struct handler_file_fixture_t));

    /* creates the test context providing a minimal connection,
    service and options chain for the handler callbacks */
    create_test_context(&fixture->test_context);

    /* creates the HTTP request and the handler file context
    then wires them together through the context pointer,
    also sets the connection as the request parameters so
    that callbacks can access the service options */
    create_http_request(&fixture->http_request);
    create_handler_file_context(&fixture->handler_file_context);
    fixture->http_request->context = fixture->handler_file_context;
    fixture->http_request->parameters = fixture->test_context->connection;
    fixture->http_request->method = HTTP_GET;

    /* returns the fixture as the opaque context that is going
    to be handed over to the test function */
    return (void *) fixture;
}

void cleanup_handler_file_test(void *context) {
    /* casts the opaque context back into the fixture so that
    every one of the structures it carries may be destroyed */
    struct handler_file_fixture_t *fixture = (struct handler_file_fixture_t *) context;

    /* deletes the context, the request and the test
    context to avoid any memory leak from the test */
    delete_handler_file_context(fixture->handler_file_context);
    delete_http_request(fixture->http_request);
    delete_test_context(fixture->test_context);

    /* releases the fixture itself, it has been allocated by
    the setup that created the chain of structures */
    FREE(fixture);
}

const char *test_handler_file_url(void *context) {
    /* allocates space for the error code returned by the
    callback and retrieves both the request and the context of
    the handler from the fixture created by the setup */
    ERROR_CODE error;
    struct handler_file_fixture_t *fixture = (struct handler_file_fixture_t *) context;
    struct http_request_t *http_request = fixture->http_request;
    struct handler_file_context_t *handler_file_context = fixture->handler_file_context;

    /* tests that a normal url is properly parsed
    and stored in the handler file context */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/index.html",
        11
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) handler_file_context->url, "/index.html") == 0);

    /* tests that a url with query string has
    the query parameters stripped from the path */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/page?id=1",
        10
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) handler_file_context->url, "/page") == 0);

    /* tests that a path traversal url is rejected
    by the url callback returning an error code */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/../etc/passwd",
        14
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a percent-encoded path traversal is
    also rejected after decoding takes place */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/%2e%2e/etc/passwd",
        18
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution, the structures are
    destroyed by the teardown of the fixture */
    return NULL;
}

const char *test_handler_file_header_field(void) {
    /* allocates space for the HTTP request and the
    handler file context structures */
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP request and the handler file context
    then wires them together through the context pointer */
    create_http_request(&http_request);
    create_handler_file_context(&handler_file_context);
    http_request->context = handler_file_context;

    /* tests that the "Range" header (5 chars) is correctly
    identified and sets the appropriate next header state */
    header_field_callback_handler_file(
        http_request,
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
        http_request,
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
        http_request,
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
        http_request,
        (unsigned char *) "If-None-Match",
        13
    );
    V_ASSERT(handler_file_context->etag_status == 1);
    V_ASSERT(handler_file_context->next_header == ETAG);

    /* resets the next header to undefined */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* the name of a field is never case sensitive and the most
    recent version of the protocol carries them in lower case
    alone, so every one of them is recognised in that shape too */
    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "range",
        5
    );
    V_ASSERT(handler_file_context->next_header == RANGE);
    handler_file_context->next_header = UNDEFINED_HEADER;

    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "cache-control",
        13
    );
    V_ASSERT(handler_file_context->next_header == CACHE_CONTROL);
    handler_file_context->next_header = UNDEFINED_HEADER;

    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "authorization",
        13
    );
    V_ASSERT(handler_file_context->next_header == AUTHORIZATION);
    handler_file_context->next_header = UNDEFINED_HEADER;

    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "if-none-match",
        13
    );
    V_ASSERT(handler_file_context->next_header == ETAG);

    /* resets the next header to undefined */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* tests that an unknown header does not change
    the next header state from undefined */
    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "X-Custom",
        8
    );
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* a name of the very same size that is not one of them is left
    alone just the same, whatever the case of it */
    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "x-something-x",
        13
    );
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* the case of a name carries no meaning at all, so one written
    in any mixture of the two is recognised just the same */
    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "RANGE",
        5
    );
    V_ASSERT(handler_file_context->next_header == RANGE);
    handler_file_context->next_header = UNDEFINED_HEADER;

    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "cAcHe-CoNtRoL",
        13
    );
    V_ASSERT(handler_file_context->next_header == CACHE_CONTROL);
    handler_file_context->next_header = UNDEFINED_HEADER;

    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "aUtHoRiZaTiOn",
        13
    );
    V_ASSERT(handler_file_context->next_header == AUTHORIZATION);
    handler_file_context->next_header = UNDEFINED_HEADER;

    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "IF-none-MATCH",
        13
    );
    V_ASSERT(handler_file_context->etag_status == 1);
    V_ASSERT(handler_file_context->next_header == ETAG);
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* a name shorter than the bytes the matching looks at is none of
    the ones being looked for and is never read past its end */
    header_field_callback_handler_file(
        http_request,
        (unsigned char *) "te",
        2
    );
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* deletes both the context and the request */
    delete_handler_file_context(handler_file_context);
    delete_http_request(http_request);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_header_value(void) {
    /* allocates space for the HTTP request and the
    handler file context structures */
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP request and the handler file context
    then wires them together through the context pointer */
    create_http_request(&http_request);
    create_handler_file_context(&handler_file_context);
    http_request->context = handler_file_context;

    /* simulates parsing a "Range" header by setting the
    next header state and then calling the value callback */
    handler_file_context->next_header = RANGE;
    header_value_callback_handler_file(
        http_request,
        (unsigned char *) "bytes=0-499",
        11
    );
    V_ASSERT(strcmp((char *) handler_file_context->range, "bytes=0-499") == 0);
    V_ASSERT(handler_file_context->range_status == 2);
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* simulates parsing a "Cache-Control" header value */
    handler_file_context->next_header = CACHE_CONTROL;
    header_value_callback_handler_file(
        http_request,
        (unsigned char *) "no-cache",
        8
    );
    V_ASSERT(strcmp((char *) handler_file_context->cache_control, "no-cache") == 0);
    V_ASSERT(handler_file_context->cache_control_status == 2);
    V_ASSERT(handler_file_context->next_header == UNDEFINED_HEADER);

    /* simulates parsing an "Authorization" header value */
    handler_file_context->next_header = AUTHORIZATION;
    header_value_callback_handler_file(
        http_request,
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
        http_request,
        (unsigned char *) "some-value",
        10
    );
    V_ASSERT(handler_file_context->range[0] == '\0');

    /* deletes both the context and the request */
    delete_handler_file_context(handler_file_context);
    delete_http_request(http_request);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
/**
 * The name of the file that the tests of the response serve, it is
 * written into the directory the process is running from and taken
 * out of it once the test that uses it is done.
 */
#define HANDLER_FILE_TEST_NAME "viriatum_handler_file_test.txt"

/**
 * The path of that very same file, both of the values are kept
 * apart as the url of a request carries the leading slash.
 */
#define HANDLER_FILE_TEST_PATH "./" HANDLER_FILE_TEST_NAME

/**
 * The contents that the file carries, the response of a request
 * for it is verified against them.
 */
#define HANDLER_FILE_TEST_CONTENTS "hello"

/**
 * The path that stands in for the resource once it is gone, no file
 * is ever written under it and opening it is meant to fail.
 */
#define HANDLER_FILE_TEST_GONE "./viriatum_handler_file_gone.txt"

/**
 * The directory that the templates of the tests are written into,
 * it is where the handler looks for them once the resources of the
 * service are pointed at the directory the process is running from.
 */
#define HANDLER_FILE_TEST_TEMPLATES "./templates"

/**
 * The paths of the templates of the listing and of the error page
 * under that directory, the names being the very ones the handler
 * and the writer of an error look for.
 */
#define HANDLER_FILE_TEST_LISTING "." VIRIATUM_LISTING_PATH
#define HANDLER_FILE_TEST_ERROR "." VIRIATUM_ERROR_PATH

/**
 * The name of the file that is added to the directory between two
 * listings of it, so that the second of them is expected to carry
 * an entry the first did not.
 */
#define HANDLER_FILE_TEST_ADDED "viriatum_handler_file_added.txt"

/**
 * The handler that stands in for the one a message is served by, it
 * is only ever required so that the release of a message reaches
 * something, it installs no callback of its own.
 */
static struct http_handler_t _handler;

/**
 * Writes a template under the directory the handler looks for them
 * in, creating the directory when it is not there yet.
 *
 * @param path The path of the template to be written.
 * @param contents The contents the template is written with.
 */
static void _write_template_handler_file_test(char *path, char *contents) {
#ifdef VIRIATUM_PLATFORM_WIN32
    _mkdir(HANDLER_FILE_TEST_TEMPLATES);
#else
    mkdir(HANDLER_FILE_TEST_TEMPLATES, 0755);
#endif
    write_file(path, (unsigned char *) contents, strlen(contents));
}

/**
 * Takes a template written by the above out of the file system,
 * together with the directory once nothing else is left in it.
 *
 * @param path The path of the template to be removed.
 */
static void _remove_template_handler_file_test(char *path) {
    remove(path);
#ifdef VIRIATUM_PLATFORM_WIN32
    _rmdir(HANDLER_FILE_TEST_TEMPLATES);
#else
    rmdir(HANDLER_FILE_TEST_TEMPLATES);
#endif
}

static ERROR_CODE _set_handler_file_test(struct http_connection_t *http_connection) {
    RAISE_NO_ERROR;
}

static ERROR_CODE _unset_handler_file_test(struct http_connection_t *http_connection) {
    RAISE_NO_ERROR;
}

/**
 * Builds a message together with the context of the handler on top
 * of the provided chain of a connection, wired the very same way
 * the parser of a connection would wire them.
 *
 * @param context The test context holding the connection.
 * @param http_request_pointer The pointer to the message that is
 * going to be served.
 * @param handler_file_context_pointer The pointer to the context
 * that the handler carries along the message.
 */
static void _create_request_handler_file_test(
    struct test_context_t *context,
    struct http_request_t **http_request_pointer,
    struct handler_file_context_t **handler_file_context_pointer
) {
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;

    create_http_request(&http_request);
    create_handler_file_context(&handler_file_context);
    http_request->context = handler_file_context;
    http_request->parameters = context->connection;
    http_request->method = HTTP_GET;
    http_request->version = HTTP11;
    http_request->flags = FLAG_KEEP_ALIVE;

    *http_request_pointer = http_request;
    *handler_file_context_pointer = handler_file_context;
}

/**
 * Builds the complete chain of a connection together with the
 * message and the context that a request for a file is served
 * through, so that the response of it may be observed.
 * The contents that are served are written to the file system as
 * the handler reads the resource out of it.
 *
 * @param context_pointer The pointer to the test context that has
 * been built.
 * @param http_request_pointer The pointer to the message that is
 * going to be served.
 * @param handler_file_context_pointer The pointer to the context
 * that the handler carries along the message.
 */
static void _create_handler_file_test(
    struct test_context_t **context_pointer,
    struct http_request_t **http_request_pointer,
    struct handler_file_context_t **handler_file_context_pointer
) {
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;

    /* writes the file that is going to be served, the directory of
    the process is the one the handler resolves the url against */
    write_file(
        (char *) HANDLER_FILE_TEST_PATH,
        (unsigned char *) HANDLER_FILE_TEST_CONTENTS,
        sizeof(HANDLER_FILE_TEST_CONTENTS) - 1
    );

    create_test_context(&context);
    create_test_connection(context);

    /* points the contents of the service at the directory of the
    process, which is where the file has just been written */
    SPRINTF((char *) context->options->contents_path, VIRIATUM_MAX_PATH_SIZE, "%s", ".");

    /* sets the type of the extension of the file being served, the
    response of it announces the one that matches */
    set_value_string_hash_map(
        context->options->mime_types,
        (unsigned char *) ".txt",
        "text/plain"
    );

    /* installs the handler that stands in for the one a message is
    served by, the release of a message reaches it */
    _handler.name = (unsigned char *) "file";
    _handler.resolve_index = FALSE;
    _handler.set = _set_handler_file_test;
    _handler.unset = _unset_handler_file_test;
    _handler.reset = NULL;
    context->http_connection->base_handler = &_handler;
    context->http_connection->http_handler = &_handler;

    _create_request_handler_file_test(context, &http_request, &handler_file_context);

    *context_pointer = context;
    *http_request_pointer = http_request;
    *handler_file_context_pointer = handler_file_context;
}

/**
 * Drives a request for the provided url through the handler on the
 * provided chain of a connection and gathers the response of it, the
 * message and the context of it being built and released around it,
 * so that a test may serve a second request through the very same
 * service, which is what the caches of it are shared across.
 *
 * @param context The test context holding the connection.
 * @param url The url of the resource being asked for.
 * @param written The buffer to gather the response into.
 * @param size The size in bytes of the provided buffer.
 * @return The number of the bytes that have been gathered.
 */
static size_t _serve_handler_file_test(struct test_context_t *context, char *url, unsigned char *written, size_t size) {
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    size_t count;

    _create_request_handler_file_test(context, &http_request, &handler_file_context);
    url_callback_handler_file(http_request, (unsigned char *) url, strlen(url));
    message_complete_callback_handler_file(http_request);
    count = flush_test_connection(context, written, size - 1);
    written[count] = '\0';
    delete_handler_file_context(handler_file_context);
    delete_http_request(http_request);

    return count;
}

/**
 * Counts the entries that a page built out of the template of the
 * tests names, each of them being written between square brackets,
 * and reads the count that closes the page, the two of them being
 * expected to agree.
 *
 * @param written The response the page is part of.
 * @param counted_pointer The pointer to the number of entries named.
 * @param announced_pointer The pointer to the count that closes the
 * page, minus one when there is none.
 */
static void _count_listing_handler_file_test(unsigned char *written, int *counted_pointer, int *announced_pointer) {
    char *pointer = (char *) written;
    int counted = 0;

    while((pointer = strchr(pointer, '[')) != NULL) {
        counted++;
        pointer++;
    }

    pointer = strrchr((char *) written, '(');
    *counted_pointer = counted;
    *announced_pointer = pointer == NULL ? -1 : atoi(pointer + 1);
}

/**
 * Releases the chain of a connection together with the message and
 * the context that were being served over it, taking the file that
 * has been served out of the file system.
 *
 * @param context The test context to be released.
 * @param http_request The message to be released.
 * @param handler_file_context The context to be released.
 */
static void _delete_handler_file_test(
    struct test_context_t *context,
    struct http_request_t *http_request,
    struct handler_file_context_t *handler_file_context
) {
    delete_handler_file_context(handler_file_context);
    delete_http_request(http_request);
    delete_test_connection(context);
    delete_test_context(context);
    remove(HANDLER_FILE_TEST_PATH);
}

const char *test_handler_file_response(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* drives the message through the callbacks of the handler, the
    very same sequence that the parser of a connection produces */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/" HANDLER_FILE_TEST_NAME,
        sizeof(HANDLER_FILE_TEST_NAME)
    );
    V_ASSERT(error == 0);
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    /* completes the writes, the payload of the file is queued by the
    completion of the write that carries the headers */
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    /* the response carries the line of the status, the size of the
    payload and the fields that describe the resource */
    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 5\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Accept-Ranges: bytes\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Type: text/plain\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "ETag: \""));

    /* the payload of the file follows the headers, the empty line is
    what separates the two of them */
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n" HANDLER_FILE_TEST_CONTENTS));

    /* the message is meant to be kept alive, so the completion of the
    last of the writes does not take the connection down */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_range(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* asks for a part of the resource rather than the complete one,
    the field is the one that announces the range being asked for */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/" HANDLER_FILE_TEST_NAME,
        sizeof(HANDLER_FILE_TEST_NAME)
    );
    V_ASSERT(error == 0);
    header_field_callback_handler_file(http_request, (unsigned char *) "Range", 5);
    header_value_callback_handler_file(http_request, (unsigned char *) "bytes=1-3", 9);
    V_ASSERT(handler_file_context->range_status == 2);

    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    /* only the part that has been asked for is served, the status of
    the response and the field of the range describe it */
    V_ASSERT(strstr((char *) written, "HTTP/1.1 206 Partial content\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 3\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Range: bytes 1-3/5\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\nell"));

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_missing(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* asks for a resource that does not exist at all, the handler
    answers it with the error that describes the absence of it */
    error = url_callback_handler_file(http_request, (unsigned char *) "/absent.txt", 11);
    V_ASSERT(error == 0);
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 404 Not Found\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n404 - Not Found - "));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* a message that is not meant to be kept alive takes the
    connection down once the response of it has gone out */
    _create_handler_file_test(&context, &http_request, &handler_file_context);
    http_request->flags = 0;
    url_callback_handler_file(http_request, (unsigned char *) "/absent.txt", 11);
    message_complete_callback_handler_file(http_request);
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    flush_test_connection(context, NULL, 0);
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_missing_template(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the responses it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    struct template_cache_entry_t *entry;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* asks for the errors to be built out of their template and
    points the resources of the service at the directory of the
    process, which is where the template is written */
    context->options->use_template = 1;
    SPRINTF((char *) context->options->resources_path, VIRIATUM_MAX_PATH_SIZE, "%s", ".");
    _write_template_handler_file_test(
        (char *) HANDLER_FILE_TEST_ERROR,
        "<e>${out value=error_code /}</e>"
    );

    /* the page of the error is built out of the template, the code
    of the error being what the template prints */
    error = url_callback_handler_file(http_request, (unsigned char *) "/absent.txt", 11);
    V_ASSERT(error == 0);
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 404 Not Found\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 10\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n<e>404</e>"));

    /* the template is rewritten underneath the live service, the
    next page is built out of it as it now stands and never out of
    the tree that the service is holding of it */
    _write_template_handler_file_test(
        (char *) HANDLER_FILE_TEST_ERROR,
        "<err>${out value=error_message /}</err>"
    );
    size = _serve_handler_file_test(context, "/absent.txt", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT(strstr((char *) written, "HTTP/1.1 404 Not Found\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n<err>Not Found</err>"));

    /* with the template gone and the entry of it past the time it is
    trusted for, the page falls back on the text of the error, the
    very way it does for an installation that carries no template */
    _remove_template_handler_file_test((char *) HANDLER_FILE_TEST_ERROR);
    entry = &context->service->template_cache->entries[_calculate_string_hash_map((unsigned char *) HANDLER_FILE_TEST_ERROR) % CACHE_SIZE_TEMPLATE_HANDLER];
    V_ASSERT_EQ_S((char *) entry->path, HANDLER_FILE_TEST_ERROR);
    entry->checked = 0;
    size = _serve_handler_file_test(context, "/absent.txt", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\n404 - Not Found - "));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_gone(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* drives the message through the callbacks of the handler, the
    response of it is decided upon while the resource is still there */
    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/" HANDLER_FILE_TEST_NAME,
        sizeof(HANDLER_FILE_TEST_NAME)
    );
    V_ASSERT(error == 0);
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    /* the resource goes away between the deciding of the response and
    the sending of the payload of it, which the handler only ever
    finds out about when it opens the file for the first of the
    chunks, the cache is still holding the one that was there */
    remove(HANDLER_FILE_TEST_PATH);
    SPRINTF(
        (char *) handler_file_context->file_path_d,
        VIRIATUM_MAX_PATH_SIZE,
        "%s",
        HANDLER_FILE_TEST_GONE
    );

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    /* the headers went out because the resource was there when they
    were decided upon, and the payload of it never followed them */
    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NULL(strstr((char *) written, "\r\n\r\n" HANDLER_FILE_TEST_CONTENTS));

    /* the connection is handed back rather than left locked with the
    handler of it still in place, one that kept either of them would
    answer no further message for as long as it stayed open */
    V_ASSERT_NULL(context->http_connection->http_handler);
    V_ASSERT_EQ_U(context->http_connection->lock, FALSE);
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* a message that is not meant to be kept alive takes the
    connection down rather than being handed back */
    _create_handler_file_test(&context, &http_request, &handler_file_context);
    http_request->flags = 0;
    url_callback_handler_file(
        http_request,
        (unsigned char *) "/" HANDLER_FILE_TEST_NAME,
        sizeof(HANDLER_FILE_TEST_NAME)
    );
    message_complete_callback_handler_file(http_request);
    remove(HANDLER_FILE_TEST_PATH);
    SPRINTF(
        (char *) handler_file_context->file_path_d,
        VIRIATUM_MAX_PATH_SIZE,
        "%s",
        HANDLER_FILE_TEST_GONE
    );
    flush_test_connection(context, NULL, 0);
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_push(void) {
#ifdef VIRIATUM_HTTP2
    /* allocates space for the chain of the connection, for the
    session and for the promises that the location produces */
    struct test_context_t *context;
    struct http2_connection_t *http2_connection;
    struct http2_stream_t *http2_stream;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    struct http2_frame_t http2_frame;
    struct data_t *data;
    size_t queued;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* the promising of a resource only exists under HTTP/2, so the
    connection is driven by a session of it */
    create_http2_connection(&http2_connection, context->http_connection);
    open_stream_http2_connection(http2_connection, 1, &http2_stream);

    /* the message of the stream is the one that is served, the one
    that was built for HTTP/1.1 is released right away */
    delete_http_request(http_request);
    http_request = http2_stream->request;
    http_request->context = handler_file_context;
    http_request->method = HTTP_GET;
    http_request->scheme = HTTP_SCHEME;
    http2_stream->http_handler = &_handler;
    context->http_connection->request = http_request;

    /* the location of the request lists the resources that travel
    together with it, they are separated by spaces */
    handler_file_context->push = (unsigned char *) "/first.css /second.js";

    error = url_callback_handler_file(
        http_request,
        (unsigned char *) "/" HANDLER_FILE_TEST_NAME,
        sizeof(HANDLER_FILE_TEST_NAME)
    );
    V_ASSERT(error == 0);

    queued = context->connection->write_queue->size;
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    /* every one of the resources of the list has reserved a stream
    of its own, the identifiers of them are the even ones */
    V_ASSERT_EQ_U(http2_connection->push_stream_id, 4);
    V_ASSERT_NOT_NULL(find_stream_http2_connection(http2_connection, 2));
    V_ASSERT_NOT_NULL(find_stream_http2_connection(http2_connection, 4));

    /* the first of the frames that have gone out is the promise of
    the first of the resources, it travels on the stream that has
    asked for the one referring to them */
    get_value_linked_list(context->connection->write_queue, queued, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    error = decode_frame_http2(data->data, data->size, &http2_frame);
    V_ASSERT(error == 0);
    V_ASSERT(http2_frame.type == HTTP2_PUSH_PROMISE);
    V_ASSERT(http2_frame.stream_id == 1);
    V_ASSERT(decode_number_http2(http2_frame.payload) == 2);

    /* the promise of the second of them follows the first and it
    reserves the identifier that comes next */
    get_value_linked_list(context->connection->write_queue, queued + 1, (void **) &data);
    V_ASSERT_NOT_NULL(data);
    error = decode_frame_http2(data->data, data->size, &http2_frame);
    V_ASSERT(error == 0);
    V_ASSERT(http2_frame.type == HTTP2_PUSH_PROMISE);
    V_ASSERT(decode_number_http2(http2_frame.payload) == 4);

    /* the response of the request has been written on the stream it
    belongs to, so that one closes once every write of it completes
    and the ones the promises reserved stay open */
    flush_test_connection(context, NULL, 0);
    V_ASSERT_NULL(find_stream_http2_connection(http2_connection, 1));
    V_ASSERT_NOT_NULL(find_stream_http2_connection(http2_connection, 2));

    delete_http2_connection(http2_connection);
    delete_handler_file_context(handler_file_context);
    delete_test_connection(context);
    delete_test_context(context);
    remove(HANDLER_FILE_TEST_PATH);
#endif

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
const char *test_handler_file_directory(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* asks for a directory without the trailing slash, which is the
    form that carries the confusion between a file and a directory */
    error = url_callback_handler_file(http_request, (unsigned char *) "/.", 2);
    V_ASSERT(error == 0);
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    /* the peer is sent to the very same place with the slash added,
    so that the resources below it resolve against it */
    V_ASSERT(strstr((char *) written, "HTTP/1.1 307 Temporary Redirect\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 0\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Location: /./\r\n"));

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_listing(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the response it produces, the
    listing of the directory of the process being of a size that
    depends on whatever else is sitting there */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[16384];
    size_t size;
    int counted;
    int announced;
    char *body;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* points the resources of the service at the directory of the
    process and writes the template of the listing there, one that
    names every entry between brackets and closes with the count */
    SPRINTF((char *) context->options->resources_path, VIRIATUM_MAX_PATH_SIZE, "%s", ".");
    _write_template_handler_file_test(
        (char *) HANDLER_FILE_TEST_LISTING,
        "${foreach item=entry from=entries}[${out value=entry.name /}]${/foreach}(${out value=items /})"
    );

    /* asks for the directory the process runs from, the file that
    the fixture wrote into it is among the entries the page lists
    and so is the directory the template was written into */
    error = url_callback_handler_file(http_request, (unsigned char *) "/", 1);
    V_ASSERT(error == 0);
    error = message_complete_callback_handler_file(http_request);
    V_ASSERT(error == 0);

    size = flush_test_connection(context, written, sizeof(written) - 1);
    V_ASSERT(size > 0 && size < sizeof(written) - 1);
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "[" HANDLER_FILE_TEST_NAME "]"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "[templates]"));

    /* the count that closes the page agrees with the entries that
    it names and the length that is announced is the length of the
    page that follows the headers */
    _count_listing_handler_file_test(written, &counted, &announced);
    V_ASSERT(counted > 0);
    V_ASSERT_EQ_I(announced, counted);
    body = strstr((char *) written, "\r\n\r\n");
    V_ASSERT_NOT_NULL(body);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: "));
    V_ASSERT_EQ_U(
        (size_t) atoi(strstr((char *) written, "Content-Length: ") + 16),
        strlen(body + 4)
    );
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _remove_template_handler_file_test((char *) HANDLER_FILE_TEST_LISTING);
    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_listing_changed(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the responses it produces */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    unsigned char written[16384];
    size_t size;
    int counted;
    int announced;
    int before;

    _create_handler_file_test(&context, &http_request, &handler_file_context);
    SPRINTF((char *) context->options->resources_path, VIRIATUM_MAX_PATH_SIZE, "%s", ".");
    _write_template_handler_file_test(
        (char *) HANDLER_FILE_TEST_LISTING,
        "${foreach item=entry from=entries}[${out value=entry.name /}]${/foreach}(${out value=items /})"
    );

    /* the directory is listed as it stands, the entry that is about
    to be added to it is nowhere in the page */
    size = _serve_handler_file_test(context, "/", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT_NULL(strstr((char *) written, "[" HANDLER_FILE_TEST_ADDED "]"));
    _count_listing_handler_file_test(written, &counted, &announced);
    V_ASSERT_EQ_I(announced, counted);
    before = counted;

    /* a file is added to the directory between two requests, the
    second of them lists it, a listing held from the first would
    have answered with a directory that no longer exists */
    write_file((char *) "./" HANDLER_FILE_TEST_ADDED, (unsigned char *) "viriatum", 8);
    size = _serve_handler_file_test(context, "/", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT_NOT_NULL(strstr((char *) written, "[" HANDLER_FILE_TEST_ADDED "]"));
    _count_listing_handler_file_test(written, &counted, &announced);
    V_ASSERT_EQ_I(announced, counted);
    V_ASSERT_EQ_I(counted, before + 1);

    /* and once the file is gone again so is the entry of it */
    remove("./" HANDLER_FILE_TEST_ADDED);
    size = _serve_handler_file_test(context, "/", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT_NULL(strstr((char *) written, "[" HANDLER_FILE_TEST_ADDED "]"));
    _count_listing_handler_file_test(written, &counted, &announced);
    V_ASSERT_EQ_I(counted, before);
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _remove_template_handler_file_test((char *) HANDLER_FILE_TEST_LISTING);
    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_listing_template(void) {
    /* allocates space for the chain of the connection, for the
    message being served, for the responses it produces and for the
    entry of the cache that holds the template of the listing */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    struct template_cache_entry_t *entry;
    struct template_node_t *root;
    unsigned char written[16384];
    size_t size;

    _create_handler_file_test(&context, &http_request, &handler_file_context);
    SPRINTF((char *) context->options->resources_path, VIRIATUM_MAX_PATH_SIZE, "%s", ".");
    _write_template_handler_file_test(
        (char *) HANDLER_FILE_TEST_LISTING,
        "A:${out value=items /}"
    );

    /* the first page is built out of the template as written and
    leaves the service holding the tree that was parsed out of it */
    size = _serve_handler_file_test(context, "/", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\nA:"));

    entry = &context->service->template_cache->entries[_calculate_string_hash_map((unsigned char *) HANDLER_FILE_TEST_LISTING) % CACHE_SIZE_TEMPLATE_HANDLER];
    V_ASSERT_EQ_S((char *) entry->path, HANDLER_FILE_TEST_LISTING);
    V_ASSERT_NOT_NULL(entry->root);
    root = entry->root;

    /* a second page out of the very same template is rendered out of
    the tree that is held, the file is not parsed again for it */
    size = _serve_handler_file_test(context, "/", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\nA:"));
    V_ASSERT_EQ_P(entry->root, root);

    /* the template is rewritten underneath the live service, the
    next page is built out of it as it now stands, the tree that was
    held describing a file that is no longer there */
    _write_template_handler_file_test(
        (char *) HANDLER_FILE_TEST_LISTING,
        "BB:${out value=items /}"
    );
    size = _serve_handler_file_test(context, "/", written, sizeof(written));
    V_ASSERT(size > 0);
    V_ASSERT_NOT_NULL(strstr((char *) written, "\r\n\r\nBB:"));
    V_ASSERT_NULL(strstr((char *) written, "\r\n\r\nA:"));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _remove_template_handler_file_test((char *) HANDLER_FILE_TEST_LISTING);
    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_path(void) {
    /* allocates space for the chain of the connection, for the
    message being served and for the context of the handler */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* a message that carries no base path at all resolves against
    the contents of the service */
    error = path_callback_handler_file(http_request, (unsigned char *) "/plain.txt", 10);
    V_ASSERT(error == 0);
    V_ASSERT_EQ_S((char *) handler_file_context->url, "/plain.txt");
    V_ASSERT_EQ_S((char *) handler_file_context->file_path_d, "./plain.txt");

    /* a base path takes the place of the contents of the service,
    which is what a location of its own produces */
    handler_file_context->base_path = (unsigned char *) "./base";
    error = path_callback_handler_file(http_request, (unsigned char *) "/other.txt", 10);
    V_ASSERT(error == 0);
    V_ASSERT_EQ_S((char *) handler_file_context->file_path_d, "./base/other.txt");

    /* the virtual url is the part of the url that is left once the
    location has taken its own out of it */
    error = virtual_url_callback_handler_file(http_request, (unsigned char *) "/virtual", 8);
    V_ASSERT(error == 0);

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_location(void) {
    /* allocates space for the chain of the connection, for the
    handler of the files and for the location it carries */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    struct file_handler_t *file_handler;
    struct http_handler_t http_handler;
    struct file_location_t location;
    ERROR_CODE error;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* gathers the url of the message first, the matching of a
    location happens over the one that has already been gathered */
    error = url_callback_handler_file(http_request, (unsigned char *) "/located/one.txt", 16);
    V_ASSERT(error == 0);

    /* builds the handler of the files together with the single
    location that the message is going to be matched against */
    create_file_handler(&file_handler, &http_handler);
    location.base_path = (unsigned char *) "./located";
    location.auth_basic = (unsigned char *) "realm";
    location.auth_file = (unsigned char *) "./passwords";
    location.push = (unsigned char *) "/promised.css";
    file_handler->locations = &location;
    file_handler->locations_count = 1;

    http_handler.name = (unsigned char *) "file";
    context->http_connection->http_handler = &http_handler;

    /* the matching of a location carries the settings of it into the
    context, the resources to be promised among them, and the path of
    the resource is resolved against the base path of it */
    error = location_callback_handler_file(http_request, 0, 8);
    V_ASSERT(error == 0);
    V_ASSERT_EQ_S((char *) handler_file_context->file_path_d, "./located/one.txt");
    V_ASSERT_EQ_P(handler_file_context->base_path, location.base_path);
    V_ASSERT_EQ_P(handler_file_context->push, location.push);
    V_ASSERT_EQ_P(handler_file_context->auth_basic, location.auth_basic);
    V_ASSERT_EQ_P(handler_file_context->auth_file, location.auth_file);

    /* the buffer of the locations belongs to the test rather than to
    the handler, so it is forgotten before the release of it */
    file_handler->locations = NULL;
    file_handler->locations_count = 0;
    delete_file_handler(file_handler);
    context->http_connection->http_handler = NULL;

    _delete_handler_file_test(context, http_request, handler_file_context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_file_handler(void) {
    /* allocates space for the chain of the connection and for the
    context that the setting of the handler builds */
    struct test_context_t *context;
    struct http_request_t *http_request;
    struct handler_file_context_t *handler_file_context;
    struct http_settings_t *http_settings;

    _create_handler_file_test(&context, &http_request, &handler_file_context);

    /* the context that the fixture carries is released here, the
    setting of the handler builds one of its own */
    delete_handler_file_context(handler_file_context);
    http_request->context = NULL;
    context->http_connection->request = http_request;
    http_settings = context->http_connection->http_settings;

    /* the setting of the handler builds the context of the message
    and installs the complete pipeline of it */
    set_handler_file(context->http_connection);
    V_ASSERT_NOT_NULL(http_request->context);
    V_ASSERT_NOT_NULL(http_settings->on_message_begin);
    V_ASSERT_NOT_NULL(http_settings->on_url);
    V_ASSERT_NOT_NULL(http_settings->on_location);
    V_ASSERT_NOT_NULL(http_settings->on_message_complete);

    /* a message that follows another one on the same connection
    resets the values of the context rather than building it again,
    the descriptor of a transfer that was cut short included, one
    left behind would be held for the whole of the connection */
    handler_file_context = (struct handler_file_context_t *) http_request->context;
    handler_file_context->offset = 1024;
    handler_file_context->etag_status = 2;
    handler_file_context->range_status = 2;
    open_read_file((char *) HANDLER_FILE_TEST_PATH, &handler_file_context->descriptor);
    V_ASSERT(handler_file_context->descriptor != -1);
    reset_handler_file(context->http_connection);
    V_ASSERT_EQ_U(handler_file_context->offset, 0);
    V_ASSERT_EQ_U(handler_file_context->etag_status, 0);
    V_ASSERT_EQ_U(handler_file_context->range_status, 0);
    V_ASSERT_EQ_I(handler_file_context->descriptor, -1);

    /* the unsetting releases the context and takes the pipeline
    down, so that another handler may take the connection */
    unset_handler_file(context->http_connection);
    V_ASSERT_NULL(http_request->context);
    V_ASSERT_NULL(http_settings->on_message_begin);
    V_ASSERT_NULL(http_settings->on_message_complete);

    context->http_connection->request = NULL;
    delete_http_request(http_request);
    delete_test_connection(context);
    delete_test_context(context);
    remove(HANDLER_FILE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

/* the file that the tests of the cache write and serve, kept apart
from the one of the handler so that the two never interfere */
#define FILE_CACHE_TEST_PATH "./viriatum_file_cache_test.txt"

/* the contents that the file above is written with, the size of it
is what the entry of the cache is expected to report */
#define FILE_CACHE_TEST_CONTENTS "viriatum"

/* another set of contents of the very same length as the one
above, so that a file replaced by it is told apart from the one
that was there by nothing but what it holds */
#define FILE_CACHE_TEST_OTHER "serviced"

const char *test_file_cache(void) {
    /* allocates space for the cache and for the index to be used in
    the walking of the entries it is made of */
    size_t index;
    struct file_cache_t *file_cache;

    /* creates the cache and verifies that every one of its entries
    starts out holding no file at all */
    create_file_cache(&file_cache);
    V_ASSERT_NOT_NULL(file_cache);
    V_ASSERT_NOT_NULL(file_cache->entries);

    for(index = 0; index < CACHE_SIZE_HANDLER_FILE; index++) {
        V_ASSERT_EQ_I(file_cache->entries[index].descriptor, -1);
        V_ASSERT_EQ_U(file_cache->entries[index].size, 0);
    }

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_acquire(void) {
    /* allocates space for the cache and for the entries that the
    acquiring of the very same path hands back */
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *first;
    struct file_cache_entry_t *second;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);

    /* the first acquisition opens the file and learns both its size
    and the moment of the last write to it */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &first
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(first);
    V_ASSERT_EQ_U(first->size, sizeof(FILE_CACHE_TEST_CONTENTS) - 1);
    V_ASSERT(first->descriptor != -1);
    V_ASSERT(first->time.year >= 1970);

    /* the second one falls on the very same entry and hands back the
    file that was already open rather than opening it again */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &second
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_P(first, second);
    V_ASSERT_EQ_I(first->descriptor, second->descriptor);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_missing(void) {
    /* allocates space for the cache and for the entry that the
    acquiring of a file that is not there would have handed back */
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry = NULL;
    ERROR_CODE error;

    create_file_cache(&file_cache);

    /* a file that does not exist raises rather than handing back an
    entry that describes nothing at all */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) "./viriatum_file_cache_gone.txt",
        &entry
    );
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_changed(void) {
    /* allocates space for the cache and for the entry that describes
    the file before and after it has been written over */
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);

    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(entry->size, sizeof(FILE_CACHE_TEST_CONTENTS) - 1);

    /* the file is written over in place, which keeps the very same
    descriptor reaching it, so an entry that went on trusting the size
    it learnt before would answer with a body cut short to match it,
    and the writing itself has to succeed while the cache is holding
    the file open, which is not something every platform allows of
    its own accord and which a deployment of a new file depends on */
    error = write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS FILE_CACHE_TEST_CONTENTS,
        (sizeof(FILE_CACHE_TEST_CONTENTS) - 1) * 2
    );
    V_ASSERT_EQ_U(error, 0);

    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(entry->size, (sizeof(FILE_CACHE_TEST_CONTENTS) - 1) * 2);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_collision(void) {
    /* allocates space for the cache and for the entries of the two
    files that are made to fall on the very same slot */
    size_t index;
    size_t taken;
    char path[VIRIATUM_MAX_PATH_SIZE];
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);

    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    taken = _calculate_string_hash_map((unsigned char *) FILE_CACHE_TEST_PATH) %
            CACHE_SIZE_HANDLER_FILE;

    /* looks for a second path that falls on the very same slot as the
    first one, which is what a cache of this shape has instead of a
    chain and what makes one of the files give way to the other */
    for(index = 0; index < 100000; index++) {
        SPRINTF(path, VIRIATUM_MAX_PATH_SIZE, "./viriatum_file_cache_%d.txt", (int) index);
        if(_calculate_string_hash_map((unsigned char *) path) % CACHE_SIZE_HANDLER_FILE == taken) {
            break;
        }
    }
    V_ASSERT(index < 100000);

    write_file(
        path,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS FILE_CACHE_TEST_CONTENTS,
        (sizeof(FILE_CACHE_TEST_CONTENTS) - 1) * 2
    );

    /* the second of them takes the slot over and is described by it,
    a cache that handed back the entry of the first would be serving
    one file under the name of another */
    error = acquire_file_cache(file_cache, (unsigned char *) path, &entry);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_S((char *) entry->path, path);
    V_ASSERT_EQ_U(entry->size, (sizeof(FILE_CACHE_TEST_CONTENTS) - 1) * 2);

    /* and the first of them takes it back again, describing itself
    and never what had displaced it */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_S((char *) entry->path, FILE_CACHE_TEST_PATH);
    V_ASSERT_EQ_U(entry->size, sizeof(FILE_CACHE_TEST_CONTENTS) - 1);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);
    remove(path);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_clear(void) {
    /* allocates space for the cache and for the entry that is going
    to be emptied out of it */
    size_t index;
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(entry->descriptor != -1);

    /* the clearing closes every file that was being held, an entry
    left with a descriptor behind is a descriptor leaked */
    clear_file_cache(file_cache);
    for(index = 0; index < CACHE_SIZE_HANDLER_FILE; index++) {
        V_ASSERT_EQ_I(file_cache->entries[index].descriptor, -1);
    }

    /* and the cache goes on working afterwards, the clearing empties
    it rather than taking it out of use */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(entry->descriptor != -1);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_open(void) {
    /* allocates space for the cache, for the entry of the file and
    for the descriptors that are handed out of it */
    char buffer[32];
    long read_bytes;
    int first;
    int second;
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);

    /* the opening hands back a descriptor of its own rather than the
    one the cache is holding, so that the closing of it by whoever
    asked never takes the one of the cache down with it */
    error = open_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &first
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(first != -1);

    acquire_file_cache(file_cache, (unsigned char *) FILE_CACHE_TEST_PATH, &entry);
    V_ASSERT(first != entry->descriptor);

    /* the contents are read through it at the position that is asked
    for, without the descriptor ever being seeked towards it */
    read_bytes = (long) READ_AT(first, buffer, sizeof(FILE_CACHE_TEST_CONTENTS) - 1, 0);
    V_ASSERT_EQ_I((int) read_bytes, (int) sizeof(FILE_CACHE_TEST_CONTENTS) - 1);
    V_ASSERT_MEM(buffer, FILE_CACHE_TEST_CONTENTS, sizeof(FILE_CACHE_TEST_CONTENTS) - 1);

    /* a second one is handed out apart from the first, two requests
    for the very same file never share what they read through */
    error = open_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &second
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(second != first);

    /* the closing of one of them leaves the other one working, which
    is the whole reason they are handed out apart */
    CLOSE_READ(first);
    read_bytes = (long) READ_AT(second, buffer, sizeof(FILE_CACHE_TEST_CONTENTS) - 1, 0);
    V_ASSERT_EQ_I((int) read_bytes, (int) sizeof(FILE_CACHE_TEST_CONTENTS) - 1);
    CLOSE_READ(second);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_long(void) {
    /* allocates space for the cache and for the path that is longer
    than an entry of it is able to carry */
    char path[VIRIATUM_MAX_PATH_SIZE * 2];
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry = NULL;
    ERROR_CODE error;

    create_file_cache(&file_cache);

    /* a path that does not fit inside an entry is refused rather than
    being copied past the end of the buffer that is meant to hold it */
    memset(path, 'a', sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    error = acquire_file_cache(file_cache, (unsigned char *) path, &entry);
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_expired(void) {
    /* allocates space for the cache and for the entry that is made
    to look older than the time it is trusted for */
    int descriptor;
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    descriptor = entry->descriptor;

    /* the entry is made to look as though it had been sitting there
    since well before the time it is trusted for, which is what sends
    the next acquisition to look at the path again */
    entry->checked = 0;

    /* the file has not moved on, so the very same descriptor goes on
    being used and only the moment it was looked at is renewed */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_I(entry->descriptor, descriptor);
    V_ASSERT(entry->checked > 0);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_replaced(void) {
    /* allocates space for the cache and for the entry of the file
    that another one is put in the place of */
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);

    /* another file is put in the place of the one that is held, the
    descriptor of the entry goes on reaching the one that was there
    before and only a look at the path is able to tell, the removal
    has to have gone through or the writing lands on the very same
    file and there would be nothing to tell apart at all */
    V_ASSERT_EQ_I(remove(FILE_CACHE_TEST_PATH), 0);
    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS FILE_CACHE_TEST_CONTENTS,
        (sizeof(FILE_CACHE_TEST_CONTENTS) - 1) * 2
    );
    entry->checked = 0;

    /* the file that is now under the path is opened in place of the
    one that was, and the entry describes the one being served */
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(entry->size, (sizeof(FILE_CACHE_TEST_CONTENTS) - 1) * 2);

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_rewritten(void) {
    /* allocates space for the cache, for the entry of the file and
    for the buffer the contents of it are read into */
    char buffer[32];
    long read_bytes;
    int descriptor;
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);

    /* another file of the very same length is put in the place of
    the one that is held, which the size of it is unable to tell
    apart, so the entry has to reach the one now under the path
    rather than the one its descriptor still opens, the removal has
    to have gone through or the two of them are the same file */
    V_ASSERT_EQ_I(remove(FILE_CACHE_TEST_PATH), 0);
    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_OTHER,
        sizeof(FILE_CACHE_TEST_OTHER) - 1
    );
    entry->checked = 0;

    /* the length is the one it always was, so only the contents
    that are handed out say which of the two files is served */
    error = open_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &descriptor
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(descriptor != -1);

    read_bytes = (long) READ_AT(descriptor, buffer, sizeof(FILE_CACHE_TEST_OTHER) - 1, 0);
    V_ASSERT_EQ_I((int) read_bytes, (int) sizeof(FILE_CACHE_TEST_OTHER) - 1);
    V_ASSERT_MEM(buffer, FILE_CACHE_TEST_OTHER, sizeof(FILE_CACHE_TEST_OTHER) - 1);

    CLOSE_READ(descriptor);
    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_cache_stale(void) {
    /* allocates space for the cache and for the entry whose file is
    taken out from underneath it */
    struct file_cache_t *file_cache;
    struct file_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) FILE_CACHE_TEST_PATH,
        (unsigned char *) FILE_CACHE_TEST_CONTENTS,
        sizeof(FILE_CACHE_TEST_CONTENTS) - 1
    );

    create_file_cache(&file_cache);
    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);

    /* the file that the entry is holding is closed behind its back
    and the entry is left pointing at a descriptor that reaches
    nothing, which is the state the cache has to answer for rather
    than go on describing a file it can no longer reach */
    CLOSE_READ(entry->descriptor);
    entry->descriptor = -2;

    error = acquire_file_cache(
        file_cache,
        (unsigned char *) FILE_CACHE_TEST_PATH,
        &entry
    );
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    /* the entry is emptied by hand as the cache is no longer able to
    close what it was left holding */
    entry->descriptor = -1;

    delete_file_cache(file_cache);
    remove(FILE_CACHE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
