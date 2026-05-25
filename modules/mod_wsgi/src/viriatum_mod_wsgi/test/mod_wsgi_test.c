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

#include "mod_wsgi_test.h"

const char *test_handler_wsgi_context(void) {
    /* allocates space for the handler WSGI context
    structure to be used in the test */
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the handler WSGI context and verifies
    that the default values are properly initialized */
    create_handler_wsgi_context(&handler_wsgi_context);

    V_ASSERT(handler_wsgi_context->flags == 0);
    V_ASSERT(handler_wsgi_context->module == NULL);
    V_ASSERT(handler_wsgi_context->module_pointer == NULL);
    V_ASSERT(handler_wsgi_context->module_name == NULL);
    V_ASSERT(handler_wsgi_context->reload == UNSET);
    V_ASSERT(handler_wsgi_context->iterator == NULL);
    V_ASSERT(handler_wsgi_context->_next_header == UNDEFINED_HEADER);
    V_ASSERT(handler_wsgi_context->_url_string.length == 0);
    V_ASSERT(handler_wsgi_context->_file_name_string.length == 0);
    V_ASSERT(handler_wsgi_context->_query_string.length == 0);
    V_ASSERT(handler_wsgi_context->_file_path_string.length == 0);
    V_ASSERT(handler_wsgi_context->_content_type_string.length == 0);
    V_ASSERT(handler_wsgi_context->_content_length_string.length == 0);
    V_ASSERT(handler_wsgi_context->_cookie_string.length == 0);
    V_ASSERT(handler_wsgi_context->_host_string.length == 0);
    V_ASSERT(handler_wsgi_context->_server_name_string.length == 0);
    V_ASSERT(handler_wsgi_context->headers == &_headers);
    V_ASSERT(handler_wsgi_context->headers->count == 0);

    /* deletes the handler WSGI context */
    delete_handler_wsgi_context(handler_wsgi_context);

    return NULL;
}

const char *test_handler_wsgi_url(void) {
    ERROR_CODE error;
    struct http_parser_t *http_parser;
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the HTTP parser and the handler WSGI context
    then wires them together through the context pointer */
    create_http_parser(&http_parser, TRUE);
    create_handler_wsgi_context(&handler_wsgi_context);
    http_parser->context = handler_wsgi_context;

    /* tests that a normal URL is properly parsed,
    note that the file_name is normalized to the platform
    path separator while the url is kept as-is */
    error = url_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "/index.html",
        11
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) handler_wsgi_context->url, "/index.html") == 0);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp((char *) handler_wsgi_context->file_name, "\\index.html") == 0);
#else
    V_ASSERT(strcmp((char *) handler_wsgi_context->file_name, "/index.html") == 0);
#endif
    V_ASSERT(handler_wsgi_context->query[0] == '\0');

    /* tests that a URL with query string has the query
    parameters properly separated from the path */
    error = url_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "/page?id=1&name=test",
        20
    );
    V_ASSERT(error == 0);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp((char *) handler_wsgi_context->file_name, "\\page") == 0);
#else
    V_ASSERT(strcmp((char *) handler_wsgi_context->file_name, "/page") == 0);
#endif
    V_ASSERT(strcmp((char *) handler_wsgi_context->query, "id=1&name=test") == 0);
    V_ASSERT(strcmp((char *) handler_wsgi_context->url, "/page?id=1&name=test") == 0);

    /* tests that the prefix name is cleared */
    V_ASSERT(handler_wsgi_context->prefix_name[0] == '\0');

    /* tests the string length tracking */
    V_ASSERT(handler_wsgi_context->_file_name_string.length == 5);
    V_ASSERT(handler_wsgi_context->_query_string.length == 14);

    delete_handler_wsgi_context(handler_wsgi_context);
    delete_http_parser(http_parser);

    return NULL;
}

const char *test_handler_wsgi_header_field(void) {
    struct http_parser_t *http_parser;
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the HTTP parser and the handler WSGI context
    then wires them together */
    create_http_parser(&http_parser, TRUE);
    create_handler_wsgi_context(&handler_wsgi_context);
    http_parser->context = handler_wsgi_context;

    /* tests that "Content-Type" is recognized */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Content-Type",
        12
    );
    V_ASSERT(handler_wsgi_context->_next_header == CONTENT_TYPE);
    V_ASSERT(strcmp(handler_wsgi_context->header->name, "Content-Type") == 0);

    /* tests that "Content-Length" is recognized */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Content-Length",
        14
    );
    V_ASSERT(handler_wsgi_context->_next_header == CONTENT_LENGTH);

    /* tests that "Cookie" is recognized */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Cookie",
        6
    );
    V_ASSERT(handler_wsgi_context->_next_header == COOKIE);

    /* tests that "Host" is recognized */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Host",
        4
    );
    V_ASSERT(handler_wsgi_context->_next_header == HOST);

    /* tests that an unknown header leaves the state as undefined */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "X-Custom",
        8
    );
    V_ASSERT(handler_wsgi_context->_next_header == UNDEFINED_HEADER);

    delete_handler_wsgi_context(handler_wsgi_context);
    delete_http_parser(http_parser);

    return NULL;
}

const char *test_handler_wsgi_header_value(void) {
    struct http_parser_t *http_parser;
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the HTTP parser and the handler WSGI context
    then wires them together */
    create_http_parser(&http_parser, TRUE);
    create_handler_wsgi_context(&handler_wsgi_context);
    http_parser->context = handler_wsgi_context;

    /* simulates parsing a Content-Type header */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Content-Type",
        12
    );
    header_value_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "text/html",
        9
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->content_type, "text/html") == 0);
    V_ASSERT(handler_wsgi_context->_content_type_string.length == 9);
    V_ASSERT(handler_wsgi_context->headers->count == 1);

    /* simulates parsing a Content-Length header */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Content-Length",
        14
    );
    header_value_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "42",
        2
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->content_length_, "42") == 0);
    V_ASSERT(handler_wsgi_context->_content_length_string.length == 2);
    V_ASSERT(handler_wsgi_context->headers->count == 2);

    /* simulates parsing a Host header with port */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Host",
        4
    );
    header_value_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "localhost:8080",
        14
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->host, "localhost:8080") == 0);
    V_ASSERT(strcmp((char *) handler_wsgi_context->server_name, "localhost") == 0);
    V_ASSERT(handler_wsgi_context->_server_name_string.length == 9);

    /* simulates parsing a Cookie header */
    header_field_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "Cookie",
        6
    );
    header_value_callback_handler_wsgi(
        http_parser,
        (unsigned char *) "session=abc123",
        14
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->cookie, "session=abc123") == 0);

    delete_handler_wsgi_context(handler_wsgi_context);
    delete_http_parser(http_parser);

    return NULL;
}

const char *test_wsgi_input_read_null(void) {
    PyObject *input;
    PyObject *result;
    char *buffer;
    Py_ssize_t buffer_size;

    /* creates a new WSGI input object with no underlying post data
    (simulating a GET request) */
    input = _new_wsgi_input(NULL, 0);
    V_ASSERT(input != NULL);

    /* calls the read method on the input object */
    result = wsgi_input_read(input, NULL);
    V_ASSERT(result != NULL);
    V_ASSERT(PyBytes_Check(result));

    /* verifies the result is an empty bytes object */
    PyBytes_AsStringAndSize(result, &buffer, &buffer_size);
    V_ASSERT(buffer_size == 0);

    Py_DECREF(result);
    Py_DECREF(input);

    return NULL;
}

const char *test_wsgi_input_read_data(void) {
    PyObject *input;
    PyObject *result;
    char *buffer;
    Py_ssize_t buffer_size;
    unsigned char post_data[] = "hello=world&foo=bar";

    /* creates a new WSGI input object with actual POST data */
    input = _new_wsgi_input(post_data, 19);
    V_ASSERT(input != NULL);

    /* calls read and verifies the full data is returned */
    result = wsgi_input_read(input, NULL);
    V_ASSERT(result != NULL);
    V_ASSERT(PyBytes_Check(result));
    PyBytes_AsStringAndSize(result, &buffer, &buffer_size);
    V_ASSERT(buffer_size == 19);
    V_ASSERT(memcmp(buffer, "hello=world&foo=bar", 19) == 0);
    Py_DECREF(result);

    /* calls read again and verifies nothing is returned
    (position has been advanced to the end) */
    result = wsgi_input_read(input, NULL);
    V_ASSERT(result != NULL);
    V_ASSERT(PyBytes_Check(result));
    PyBytes_AsStringAndSize(result, &buffer, &buffer_size);
    V_ASSERT(buffer_size == 0);
    Py_DECREF(result);

    Py_DECREF(input);

    return NULL;
}

const char *test_wsgi_app_lifecycle(void) {
    ERROR_CODE error;
    PyObject *module;
    PyObject *handler_function;
    const char *script_path;

    /* resolves the path to the test handler script, the test binary
    is expected to run from the build directory so the script path
    is passed as a compile-time define */
#ifdef VIRIATUM_TEST_SCRIPT_PATH
    script_path = VIRIATUM_TEST_SCRIPT_PATH;
#else
    script_path = "test_handler.py";
#endif

    /* loads the module from the test handler script using the same
    code path that the request handler uses (Py_CompileString +
    PyImport_ExecCodeModuleEx), the module name is unique to avoid
    collisions with previously loaded modules */
    module = NULL;
    error = _load_module_wsgi(&module, "wsgi_app_test_lifecycle", (char *) script_path);
    V_ASSERT(error == 0);
    V_ASSERT_M(module != NULL, "failed to load test_handler.py");

    /* verifies that the application attribute can be retrieved from
    the loaded module and that it refers a callable */
    handler_function = PyObject_GetAttrString(module, "application");
    V_ASSERT_M(handler_function != NULL, "application is not defined");
    V_ASSERT_M(PyCallable_Check(handler_function), "application is not a function");
    Py_DECREF(handler_function);

    Py_DECREF(module);

    return NULL;
}

const char *test_wsgi_app_persistence(void) {
    ERROR_CODE error;
    PyObject *module;
    PyObject *handler_function;
    const char *script_path;

#ifdef VIRIATUM_TEST_SCRIPT_PATH
    script_path = VIRIATUM_TEST_SCRIPT_PATH;
#else
    script_path = "test_handler.py";
#endif

    /* first load: execute the script and store the module reference */
    module = NULL;
    error = _load_module_wsgi(&module, "wsgi_app_test_persistence", (char *) script_path);
    V_ASSERT(error == 0);
    V_ASSERT_M(module != NULL, "failed to load test_handler.py (first)");

    /* simulate the non-reload path: do NOT re-execute the script,
    just verify the module is still accessible with a valid application
    function across multiple lookups */
    handler_function = PyObject_GetAttrString(module, "application");
    V_ASSERT_M(handler_function != NULL, "application lost on second access");
    V_ASSERT_M(PyCallable_Check(handler_function), "application not callable on second access");
    Py_DECREF(handler_function);

    /* simulate a third access to confirm persistence */
    handler_function = PyObject_GetAttrString(module, "application");
    V_ASSERT_M(handler_function != NULL, "application lost on third access");
    V_ASSERT_M(PyCallable_Check(handler_function), "application not callable on third access");
    Py_DECREF(handler_function);

    Py_DECREF(module);

    return NULL;
}

void exec_mod_wsgi_tests(struct test_case_t *test_case) {
    /* HTTP callback tests (no Python dependency) */
    V_RUN_TEST(test_handler_wsgi_context, test_case);
    V_RUN_TEST(test_handler_wsgi_url, test_case);
    V_RUN_TEST(test_handler_wsgi_header_field, test_case);
    V_RUN_TEST(test_handler_wsgi_header_value, test_case);

    /* Python extension tests */
    V_RUN_TEST(test_wsgi_input_read_null, test_case);
    V_RUN_TEST(test_wsgi_input_read_data, test_case);

    /* WSGI app loading tests */
    V_RUN_TEST(test_wsgi_app_lifecycle, test_case);
    V_RUN_TEST(test_wsgi_app_persistence, test_case);
}

ERROR_CODE run_mod_wsgi_tests(void) {
    ERROR_CODE return_value = run_test_case(exec_mod_wsgi_tests, "mod_wsgi_tests");
    RAISE_AGAIN(return_value);
}
