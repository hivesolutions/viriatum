/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "../stdafx.h"

#include "mod_php_test.h"

/**
 * The path to the directory holding the scripts that the tests
 * serve, handed by the build so that a run from anywhere finds it.
 */
#ifdef VIRIATUM_TEST_CONTENTS_PATH
#define PHP_TEST_CONTENTS_PATH VIRIATUM_TEST_CONTENTS_PATH
#else
#define PHP_TEST_CONTENTS_PATH "."
#endif

/**
 * The handler that the tests install as the one a new message
 * is served by, it carries the operations of the PHP one.
 */
static struct http_handler_t _handler;

/**
 * Builds the complete chain of a connection with the PHP handler
 * installed as the one a new message is served by, the contents of
 * the service pointing at the directory of the scripts of the suite.
 *
 * @param context_pointer The pointer to the test context that has
 * been built.
 */
static void _create_handler_php_test(struct test_context_t **context_pointer) {
    struct test_context_t *context;

    create_test_context(&context);
    create_test_connection(context);

    /* points the contents of the service at the scripts of the suite
    and resolves the port the very way the loading of the options of
    a service does, the interpreter is handed both of them */
    SPRINTF(
        (char *) context->options->contents_path,
        VIRIATUM_MAX_PATH_SIZE,
        "%s",
        PHP_TEST_CONTENTS_PATH
    );
    context->options->port = 9090;
    SPRINTF((char *) context->options->_port, 128, "%d", context->options->port);
    string_populate(&context->options->_port_string, context->options->_port, 0, 1);

    /* clears the address of the peer of the connection, the one of a
    test is never accepted from a socket and the interpreter is handed
    it as the remote address of every message */
    memset(&context->connection->socket_address, 0, sizeof(SOCKET_ADDRESS));

    /* installs the handler as the one a new message is served by,
    the reading of the connection is what reaches it */
    _handler.name = (unsigned char *) "php";
    _handler.resolve_index = FALSE;
    _handler.set = set_handler_php;
    _handler.unset = unset_handler_php;
    _handler.reset = NULL;
    context->http_connection->base_handler = &_handler;

    *context_pointer = context;
}

/**
 * Releases the chain of a connection, the handler is only unset
 * when the completion of a write has not done it already.
 *
 * @param context The test context to be released.
 */
static void _delete_handler_php_test(struct test_context_t *context) {
    if(context->http_connection->http_handler != NULL) {
        unset_handler_php(context->http_connection);
        context->http_connection->http_handler = NULL;
    }
    delete_test_connection(context);
    delete_test_context(context);
}

const char *test_handler_php_response(void) {
    /* allocates space for the chain of the connection and for the
    response that the message produces */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    /* the message as it travels on the wire, the parser of the
    connection is the one that takes it apart, this version of the
    protocol keeps a connection alive without being asked to */
    static const char *request =
        "GET /test_handler.php?name=viriatum HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    _create_handler_php_test(&context);

    /* reads the message, which runs the script to its end and queues
    the headers of the response, the payload of it travelling as a
    write of its own that follows the one of the headers */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request,
        strlen(request)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Connection: keep-alive\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 54\r\n"));
    V_ASSERT_NOT_NULL(strstr((char *) written, "X-Test: viriatum\r\n"));
    V_ASSERT_NOT_NULL(strstr(
        (char *) written,
        "\r\n\r\nmethod=GET\nuri=/test_handler.php?name=viriatum\ninput=\n"
    ));

    /* the message is meant to be kept alive, so the completion of the
    last of the writes does not take the connection down, it unsets
    the handler which is what releases the context of the message */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    V_ASSERT_NULL(context->http_connection->http_handler);

    _delete_handler_php_test(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_php_post(void) {
    /* allocates space for the chain of the connection and for the
    response that the message produces */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    /* the message carries a payload after its headers, which is held
    in the buffer of the connection when the script is run */
    static const char *request =
        "POST /test_handler.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 8\r\n"
        "\r\n"
        "viriatum";

    _create_handler_php_test(&context);

    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request,
        strlen(request)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    /* the script reads its input out of the very bytes that travelled
    after the headers, none of them lost and none of the ones of the
    headers carried along with them */
    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 49\r\n"));
    V_ASSERT_NOT_NULL(strstr(
        (char *) written,
        "\r\n\r\nmethod=POST\nuri=/test_handler.php\ninput=viriatum\n"
    ));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    _delete_handler_php_test(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_php_close(void) {
    /* allocates space for the chain of the connection and for the
    response that the message produces */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    /* the message asks for the connection to be closed once it has
    been answered, rather than kept alive for the ones that follow */
    static const char *request =
        "GET /test_handler.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n";

    _create_handler_php_test(&context);

    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request,
        strlen(request)
    );
    V_ASSERT_EQ_U(error, 0);

    /* the closing only happens once the response has actually left
    this end, the peer is answered before it */
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Connection: close\r\n"));
    V_ASSERT_EQ_U(get_closed_test_connection(), 1);
    V_ASSERT_NULL(context->http_connection->http_handler);

    _delete_handler_php_test(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_php_missing(void) {
    /* allocates space for the chain of the connection and for the
    responses that the messages produce */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    /* the first message names a script that is not there, the second
    one names the script of the suite and travels after it over the
    very same connection */
    static const char *request =
        "GET /test_handler_absent.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    static const char *request_next =
        "GET /test_handler.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    _create_handler_php_test(&context);

    /* a script that cannot be opened is answered with an error, the
    reference to it used to be released after its request had already
    shut down, which took the whole of the process down with it */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request,
        strlen(request)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 500 Internal Server Error\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 0\r\n"));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    /* the interpreter is left able to serve the message that follows,
    which runs its script to the end as if nothing had happened */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request_next,
        strlen(request_next)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr(
        (char *) written,
        "\r\n\r\nmethod=GET\nuri=/test_handler.php\ninput=\n"
    ));

    _delete_handler_php_test(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_php_directory(void) {
    /* allocates space for the chain of the connection and for the
    responses that the messages produce */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    /* the first message names the directory of the scripts itself, a
    path that exists but that may never be run, the second one names
    the script of the suite over the very same connection */
    static const char *request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    static const char *request_next =
        "GET /test_handler.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    _create_handler_php_test(&context);

    /* the opening of a directory as a script is refused and answered
    with an error, the very same way a script that is not there is */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request,
        strlen(request)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 500 Internal Server Error\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 0\r\n"));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    /* the interpreter is left able to serve the message that follows,
    which runs its script to the end as if nothing had happened */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request_next,
        strlen(request_next)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr(
        (char *) written,
        "\r\n\r\nmethod=GET\nuri=/test_handler.php\ninput=\n"
    ));

    _delete_handler_php_test(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_handler_php_error(void) {
    /* allocates space for the chain of the connection and for the
    responses that the messages produce */
    struct test_context_t *context;
    unsigned char written[2048];
    size_t size;
    ERROR_CODE error;

    /* the first message names a script that fails while it runs, the
    second one names the script of the suite over the very same
    connection */
    static const char *request =
        "GET /test_handler_error.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    static const char *request_next =
        "GET /test_handler.php HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    _create_handler_php_test(&context);

    /* an error that the script never catches is fatal to it, which is
    answered with an error that carries none of what it had written */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request,
        strlen(request)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 500 Internal Server Error\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr((char *) written, "Content-Length: 0\r\n"));
    V_ASSERT_EQ_U(get_closed_test_connection(), 0);

    /* the interpreter is left able to serve the message that follows,
    which runs its script to the end as if nothing had happened */
    error = data_handler_stream_http(
        context->io_connection,
        (unsigned char *) request_next,
        strlen(request_next)
    );
    V_ASSERT_EQ_U(error, 0);
    size = flush_test_connection(context, written, sizeof(written));
    V_ASSERT(size > 0 && size < sizeof(written));
    written[size] = '\0';

    V_ASSERT(strstr((char *) written, "HTTP/1.1 200 OK\r\n") == (char *) written);
    V_ASSERT_NOT_NULL(strstr(
        (char *) written,
        "\r\n\r\nmethod=GET\nuri=/test_handler.php\ninput=\n"
    ));

    _delete_handler_php_test(context);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

void exec_mod_php_tests(struct test_case_t *test_case) {
    /* the serving of a script that runs to its end */
    V_RUN_TEST(test_handler_php_response, test_case);
    V_RUN_TEST(test_handler_php_post, test_case);
    V_RUN_TEST(test_handler_php_close, test_case);

    /* the serving of a script that fails, which must leave the
    interpreter able to serve the message that follows it */
    V_RUN_TEST(test_handler_php_missing, test_case);
    V_RUN_TEST(test_handler_php_directory, test_case);
    V_RUN_TEST(test_handler_php_error, test_case);
}

ERROR_CODE run_mod_php_tests(void) {
    ERROR_CODE return_value = run_test_case(exec_mod_php_tests, "mod_php_tests");
    RAISE_AGAIN(return_value);
}
