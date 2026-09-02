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

#include "mod_python_test.h"

/**
 * The path to the script that the tests of the more recent interface
 * load an application out of, handed by the build so that a run from
 * anywhere finds it.
 */
#ifdef VIRIATUM_TEST_SCRIPT_ASGI_PATH
#define ASGI_TEST_SCRIPT_PATH VIRIATUM_TEST_SCRIPT_ASGI_PATH
#else
#define ASGI_TEST_SCRIPT_PATH "test_handler_asgi.py"
#endif

/**
 * The path that a file which does not compile is written to, so
 * that the refusing of one may be driven.
 */
#define ASGI_TEST_BROKEN_PATH "./viriatum_mod_python_broken.py"

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
    struct http_request_t *http_request;
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the HTTP request and the handler WSGI context
    then wires them together through the context pointer */
    create_http_request(&http_request);
    create_handler_wsgi_context(&handler_wsgi_context);
    http_request->context = handler_wsgi_context;

    /* tests that a normal URL is properly parsed,
    note that the file_name is normalized to the platform
    path separator while the url is kept as-is */
    error = url_callback_handler_wsgi(
        http_request,
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
        http_request,
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
    delete_http_request(http_request);

    return NULL;
}

const char *test_handler_wsgi_header_field(void) {
    struct http_request_t *http_request;
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the HTTP request and the handler WSGI context
    then wires them together */
    create_http_request(&http_request);
    create_handler_wsgi_context(&handler_wsgi_context);
    http_request->context = handler_wsgi_context;

    /* tests that "Content-Type" is recognized */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Content-Type",
        12
    );
    V_ASSERT(handler_wsgi_context->_next_header == CONTENT_TYPE);
    V_ASSERT(strcmp(handler_wsgi_context->header->name, "Content-Type") == 0);

    /* tests that "Content-Length" is recognized */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Content-Length",
        14
    );
    V_ASSERT(handler_wsgi_context->_next_header == CONTENT_LENGTH);

    /* tests that "Cookie" is recognized */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Cookie",
        6
    );
    V_ASSERT(handler_wsgi_context->_next_header == COOKIE);

    /* tests that "Host" is recognized */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Host",
        4
    );
    V_ASSERT(handler_wsgi_context->_next_header == HOST);

    /* tests that an unknown header leaves the state as undefined */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "X-Custom",
        8
    );
    V_ASSERT(handler_wsgi_context->_next_header == UNDEFINED_HEADER);

    delete_handler_wsgi_context(handler_wsgi_context);
    delete_http_request(http_request);

    return NULL;
}

const char *test_handler_wsgi_header_value(void) {
    struct http_request_t *http_request;
    struct handler_wsgi_context_t *handler_wsgi_context;

    /* creates the HTTP request and the handler WSGI context
    then wires them together */
    create_http_request(&http_request);
    create_handler_wsgi_context(&handler_wsgi_context);
    http_request->context = handler_wsgi_context;

    /* simulates parsing a Content-Type header */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Content-Type",
        12
    );
    header_value_callback_handler_wsgi(
        http_request,
        (unsigned char *) "text/html",
        9
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->content_type, "text/html") == 0);
    V_ASSERT(handler_wsgi_context->_content_type_string.length == 9);
    V_ASSERT(handler_wsgi_context->headers->count == 1);

    /* simulates parsing a Content-Length header */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Content-Length",
        14
    );
    header_value_callback_handler_wsgi(
        http_request,
        (unsigned char *) "42",
        2
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->content_length_, "42") == 0);
    V_ASSERT(handler_wsgi_context->_content_length_string.length == 2);
    V_ASSERT(handler_wsgi_context->headers->count == 2);

    /* simulates parsing a Host header with port */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Host",
        4
    );
    header_value_callback_handler_wsgi(
        http_request,
        (unsigned char *) "localhost:8080",
        14
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->host, "localhost:8080") == 0);
    V_ASSERT(strcmp((char *) handler_wsgi_context->server_name, "localhost") == 0);
    V_ASSERT(handler_wsgi_context->_server_name_string.length == 9);

    /* simulates parsing a Cookie header */
    header_field_callback_handler_wsgi(
        http_request,
        (unsigned char *) "Cookie",
        6
    );
    header_value_callback_handler_wsgi(
        http_request,
        (unsigned char *) "session=abc123",
        14
    );
    V_ASSERT(strcmp((char *) handler_wsgi_context->cookie, "session=abc123") == 0);

    delete_handler_wsgi_context(handler_wsgi_context);
    delete_http_request(http_request);

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
    error = _load_module_python(&module, "wsgi_app_test_lifecycle", (char *) script_path);
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
    error = _load_module_python(&module, "wsgi_app_test_persistence", (char *) script_path);
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

const char *test_mod_python_asgi(void) {
    /* allocates space for the handler that is going to be built
    and for the one of the upper layer it is set in */
    struct mod_python_asgi_t *mod_python_asgi;

    /* the handler starts with nothing loaded and with the values
    that stand in for a configuration that named none */
    create_mod_python_asgi(&mod_python_asgi);
    V_ASSERT_EQ_U(mod_python_asgi->file_path[0], '\0');
    V_ASSERT_EQ_S(mod_python_asgi->application_name, DEFAULT_APPLICATION_NAME);
    V_ASSERT_NULL(mod_python_asgi->module);
    V_ASSERT_NULL(mod_python_asgi->application);

    delete_mod_python_asgi(mod_python_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application(void) {
    /* allocates space for the handler that the application is
    loaded into and for the error the loading raises */
    struct mod_python_asgi_t *mod_python_asgi;
    ERROR_CODE error;

    create_mod_python_asgi(&mod_python_asgi);
    SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);

    /* the application named by the handler is taken out of the
    module and both of them are held, the module being what keeps
    the globals the application reads alive */
    error = load_application_asgi(mod_python_asgi);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(mod_python_asgi->module);
    V_ASSERT_NOT_NULL(mod_python_asgi->application);
    V_ASSERT(PyCallable_Check(mod_python_asgi->application));

    delete_mod_python_asgi(mod_python_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_missing(void) {
    /* allocates space for the handler that the loading is driven
    against and for the error it raises */
    struct mod_python_asgi_t *mod_python_asgi;
    ERROR_CODE error;

    /* a path that no file sits under is refused and leaves the
    handler with nothing set rather than half of something */
    create_mod_python_asgi(&mod_python_asgi);
    SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", "./viriatum_mod_python_absent.py");
    error = load_application_asgi(mod_python_asgi);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_python_asgi->module);
    V_ASSERT_NULL(mod_python_asgi->application);
    delete_mod_python_asgi(mod_python_asgi);

    /* a name the module does not carry is refused the same way,
    the module having loaded is not enough on its own */
    create_mod_python_asgi(&mod_python_asgi);
    SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);
    SPRINTF(mod_python_asgi->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", "absent");
    error = load_application_asgi(mod_python_asgi);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_python_asgi->module);
    V_ASSERT_NULL(mod_python_asgi->application);
    delete_mod_python_asgi(mod_python_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_uncallable(void) {
    /* allocates space for the handler that the loading is driven
    against and for the error it raises */
    struct mod_python_asgi_t *mod_python_asgi;
    ERROR_CODE error;

    /* a name that resolves to something that may not be called is
    not an application, the module carries a number under one */
    create_mod_python_asgi(&mod_python_asgi);
    SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);
    SPRINTF(mod_python_asgi->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", "number");
    error = load_application_asgi(mod_python_asgi);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_python_asgi->application);

    delete_mod_python_asgi(mod_python_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_reload(void) {
    /* allocates space for the handler and for the reference that
    the first of the two loadings left in it */
    struct mod_python_asgi_t *mod_python_asgi;
    PyObject *application;
    ERROR_CODE error;

    create_mod_python_asgi(&mod_python_asgi);
    SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);

    error = load_application_asgi(mod_python_asgi);
    V_ASSERT_EQ_U(error, 0);
    application = mod_python_asgi->application;
    Py_INCREF(application);

    /* the second of the loadings replaces what the first one left
    rather than writing over it, a reference that was written over
    would never be released and the count of it says so */
    error = load_application_asgi(mod_python_asgi);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(mod_python_asgi->application);
    V_ASSERT_EQ_I(Py_REFCNT(application), 1);
    Py_DECREF(application);

    delete_mod_python_asgi(mod_python_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_unload(void) {
    /* allocates space for the handler that the unloading is
    driven against and for the error the loading raises */
    struct mod_python_asgi_t *mod_python_asgi;
    ERROR_CODE error;

    create_mod_python_asgi(&mod_python_asgi);
    SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);
    error = load_application_asgi(mod_python_asgi);
    V_ASSERT_EQ_U(error, 0);

    /* the unloading leaves nothing set, so that the releasing of
    the very same reference a second time is never possible */
    unload_application_asgi(mod_python_asgi);
    V_ASSERT_NULL(mod_python_asgi->module);
    V_ASSERT_NULL(mod_python_asgi->application);

    /* and running it again over a handler that carries nothing is
    harmless, which is what the deleting of one relies on */
    unload_application_asgi(mod_python_asgi);
    V_ASSERT_NULL(mod_python_asgi->module);
    V_ASSERT_NULL(mod_python_asgi->application);

    delete_mod_python_asgi(mod_python_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_load(void) {
    /* allocates space for the module that is loaded and for the
    name that is taken out of it */
    PyObject *module;
    PyObject *value;
    ERROR_CODE error;

    /* the module is loaded out of the file and carries the names
    that the file defines, the application among them */
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_load", (char *) ASGI_TEST_SCRIPT_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(module);

    value = PyObject_GetAttrString(module, "application");
    V_ASSERT_NOT_NULL(value);
    V_ASSERT(PyCallable_Check(value));
    Py_DECREF(value);

    Py_DECREF(module);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_broken(void) {
    /* allocates space for the module that the loading writes and
    for the error it raises */
    PyObject *module;
    ERROR_CODE error;

    /* a file that is not there leaves the module unset without
    raising, the loading of an application is what tells that
    apart and reports it */
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_absent", (char *) "./viriatum_mod_python_absent.py");
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(module);

    /* a file that compiles and then raises while it runs is left
    the very same way, the module of it never coming to be */
    write_file(
        (char *) ASGI_TEST_BROKEN_PATH,
        (unsigned char *) "raise ValueError('boom')\n",
        25
    );
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_raising", (char *) ASGI_TEST_BROKEN_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(module);
    V_ASSERT_NULL(PyErr_Occurred());
    remove(ASGI_TEST_BROKEN_PATH);

    /* a file that does not compile is left the very same way, the
    error of the interpreter being cleared rather than carried out
    into a request that has nothing to do with it */
    write_file(
        (char *) ASGI_TEST_BROKEN_PATH,
        (unsigned char *) "def application(:\n",
        18
    );
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_broken", (char *) ASGI_TEST_BROKEN_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(module);
    V_ASSERT_NULL(PyErr_Occurred());
    remove(ASGI_TEST_BROKEN_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_double_callable(void) {
    /* allocates space for the module the shapes are taken out of,
    for each of them and for the flag that is decided */
    PyObject *module;
    PyObject *value;
    char double_callable;
    ERROR_CODE error;

    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_callable", (char *) ASGI_TEST_SCRIPT_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(module);

    /* the single callable of the more recent version is a coroutine
    function of its own and takes the three values in one call */
    value = PyObject_GetAttrString(module, "application");
    V_ASSERT_NOT_NULL(value);
    double_callable = double_callable_handler_asgi(value);
    V_ASSERT_EQ_U(double_callable, FALSE);
    Py_DECREF(value);

    /* the pair of the older takes the scope apart, it is not a
    coroutine function itself and so it is the double one */
    value = PyObject_GetAttrString(module, "legacy");
    V_ASSERT_NOT_NULL(value);
    double_callable = double_callable_handler_asgi(value);
    V_ASSERT_EQ_U(double_callable, TRUE);
    Py_DECREF(value);

    /* a class stands for the very same pair, the building of the
    instance taking the scope and the calling of it the callables */
    value = PyObject_GetAttrString(module, "Application");
    V_ASSERT_NOT_NULL(value);
    double_callable = double_callable_handler_asgi(value);
    V_ASSERT_EQ_U(double_callable, TRUE);
    Py_DECREF(value);

    /* an instance whose call is a coroutine one is the single shape,
    which the detector of the module never told apart at all */
    value = PyObject_GetAttrString(module, "instance");
    V_ASSERT_NOT_NULL(value);
    double_callable = double_callable_handler_asgi(value);
    V_ASSERT_EQ_U(double_callable, FALSE);
    Py_DECREF(value);

    /* and the markers of the adaptation helpers are read ahead of
    any shape, an application that carries one is taken at its word */
    value = PyObject_GetAttrString(module, "marked_single");
    V_ASSERT_NOT_NULL(value);
    V_ASSERT_EQ_U(has_marker_handler_asgi(value, "_asgi_single_callable"), TRUE);
    double_callable = double_callable_handler_asgi(value);
    V_ASSERT_EQ_U(double_callable, FALSE);
    Py_DECREF(value);

    value = PyObject_GetAttrString(module, "marked_double");
    V_ASSERT_NOT_NULL(value);
    double_callable = double_callable_handler_asgi(value);
    V_ASSERT_EQ_U(double_callable, TRUE);
    V_ASSERT_NULL(PyErr_Occurred());
    Py_DECREF(value);

    Py_DECREF(module);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_absolute(void) {
    /* a path that opens with a separator is absolute wherever it is
    read, which is the only shape the older module told apart */
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "/srv/app.py"), TRUE);
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "\\srv\\app.py"), TRUE);

    /* one that opens with a drive letter and a separator is absolute
    as well, a configuration naming one was being resolved against
    the contents of the service and reaching nothing at all */
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "C:\\srv\\app.py"), TRUE);
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "c:/srv/app.py"), TRUE);

    /* everything else is relative and is resolved against them, a
    letter that carries no separator after it among them */
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "app.py"), FALSE);
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "srv/app.py"), FALSE);
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) "C:app.py"), FALSE);
    V_ASSERT_EQ_U(_is_absolute_asgi((unsigned char *) ""), FALSE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_path(void) {
    /* allocates space for the list of the paths of the imports and
    for the one that is expected to open it */
    PyObject *path;
    PyObject *value;
    const char *first;

    /* the directory the application sits in is put ahead of
    everything else, so that a module beside it is the one found */
    _path_asgi_state((char *) "/srv/application/app.py");
    path = PySys_GetObject("path");
    V_ASSERT_NOT_NULL(path);
    value = PyList_GetItem(path, 0);
    V_ASSERT_NOT_NULL(value);
    first = PyUnicode_AsUTF8(value);
    V_ASSERT_EQ_S((char *) first, "/srv/application");

    /* a path that carries no directory at all leaves the working
    one at the front, which is where the application sits */
    _path_asgi_state((char *) "app.py");
    path = PySys_GetObject("path");
    value = PyList_GetItem(path, 0);
    first = PyUnicode_AsUTF8(value);
    V_ASSERT_EQ_S((char *) first, DEFAULT_BASE_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_module_python_info(void) {
    /* allocates space for the module that is populated by the
    operation that describes one */
    struct module_t module;

    /* the module carries the name and the version of the build
    together with the operations the service reaches it through */
    info_module_python(&module);
    V_ASSERT_EQ_S((char *) module.name, "viriatum_mod_python");
    V_ASSERT_EQ_S((char *) module.name_s, "asgi");
    V_ASSERT_EQ_U(module.type, MODULE_TYPE_HTTP_HANDLER);
    V_ASSERT_EQ_P(module.start, start_module_python);
    V_ASSERT_EQ_P(module.stop, stop_module_python);
    V_ASSERT_EQ_P(module.info, info_module_python);
    V_ASSERT_EQ_P(module.error, error_module_python);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_module_python_error(void) {
    /* allocates space for the message that the operation hands
    back, which is the reason of the last failure */
    unsigned char *message;

    /* the reason reaches the caller rather than being left for it
    to be read out of somewhere else */
    error_module_python(&message);
    V_ASSERT_EQ_P(message, get_last_error_message());

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_configuration(void) {
    /* allocates space for the handler the values reach, for the
    service that carries them and for the maps of it */
    struct mod_python_asgi_t *mod_python_asgi;
    struct service_t service;
    struct service_options_t options;
    struct sort_map_t *configuration;
    struct sort_map_t *section;

    /* a service that carries no configuration at all leaves the
    handler with the values that stand in for one */
    create_mod_python_asgi(&mod_python_asgi);
    service.configuration = NULL;
    _load_configuration_asgi(&service, mod_python_asgi);
    V_ASSERT_EQ_U(mod_python_asgi->file_path[0], '\0');
    delete_mod_python_asgi(mod_python_asgi);

    /* and so does one whose configuration carries no section of
    this module, the reading of it stopping there */
    create_sort_map(&configuration, 8);
    create_mod_python_asgi(&mod_python_asgi);
    service.configuration = configuration;
    _load_configuration_asgi(&service, mod_python_asgi);
    V_ASSERT_EQ_U(mod_python_asgi->file_path[0], '\0');
    delete_mod_python_asgi(mod_python_asgi);

    /* a path that is absolute reaches the handler as it was
    written, together with the name of the application */
    create_sort_map(&section, 8);
    set_value_string_sort_map(section, (unsigned char *) "asgi_script_path", (void *) "/tmp/app.py");
    set_value_string_sort_map(section, (unsigned char *) "asgi_application", (void *) "other");
    set_value_string_sort_map(configuration, (unsigned char *) "mod_python", (void *) section);

    create_mod_python_asgi(&mod_python_asgi);
    _load_configuration_asgi(&service, mod_python_asgi);
    V_ASSERT_EQ_S(mod_python_asgi->file_path, "/tmp/app.py");
    V_ASSERT_EQ_S(mod_python_asgi->application_name, "other");
    delete_mod_python_asgi(mod_python_asgi);

    /* one that is relative is resolved against the contents of the
    service, so that a configuration never has to name the tree */
    set_value_string_sort_map(section, (unsigned char *) "asgi_script_path", (void *) "app.py");
    SPRINTF((char *) options.contents_path, VIRIATUM_MAX_PATH_SIZE, "%s", "/srv");
    service.options = &options;

    create_mod_python_asgi(&mod_python_asgi);
    _load_configuration_asgi(&service, mod_python_asgi);
    V_ASSERT_EQ_S(mod_python_asgi->file_path, "/srv" VIRIATUM_PATH_SEPARATOR "app.py");
    delete_mod_python_asgi(mod_python_asgi);

    delete_sort_map(section);
    delete_sort_map(configuration);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_cycle(void) {
    /* allocates space for the service the operation is run
    against, which it only ever reaches the polling of */
    struct service_t service;
    ERROR_CODE error;

    /* the operation is harmless while no module has been started,
    which is the state a loading that failed leaves behind, and it
    never reaches the polling of the service in that case */
    _mod_python_module = NULL;
    error = cycle_module_python(&service);
    V_ASSERT_EQ_U(error, 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_start_unconfigured(void) {
    /* allocates space for the service the starting is driven against
    and for the module that carries the state of it */
    struct service_t service;
    struct mod_python_module_t mod_python_module;
    struct sort_map_t *configuration;
    ERROR_CODE error;

    /* a service whose configuration names no application at all
    leaves the interface unserved and the starting untroubled */
    memset(&mod_python_module, 0, sizeof(struct mod_python_module_t));
    create_sort_map(&configuration, 8);
    service.configuration = configuration;
    error = _start_asgi_module(&service, &mod_python_module);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(mod_python_module.mod_python_asgi);
    V_ASSERT_NULL(mod_python_module.loop_python);

    /* and the stopping of one that never started is harmless, which
    is what the stopping of the module relies on */
    error = _stop_asgi_module(&service, &mod_python_module);
    V_ASSERT_EQ_U(error, 0);

    delete_sort_map(configuration);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_start_missing(void) {
    /* allocates space for the service the starting is driven against
    and for the module that carries the state of it */
    struct service_t service;
    struct mod_python_module_t mod_python_module;
    struct sort_map_t *configuration;
    struct sort_map_t *section;
    ERROR_CODE error;

    /* a configuration naming an application that no file carries
    fails the starting, a module that reported a success would be
    left registered with nothing at all behind it */
    memset(&mod_python_module, 0, sizeof(struct mod_python_module_t));
    create_sort_map(&configuration, 8);
    create_sort_map(&section, 8);
    set_value_string_sort_map(section, (unsigned char *) "asgi_script_path", (void *) "/viriatum_mod_python_absent.py");
    set_value_string_sort_map(configuration, (unsigned char *) "mod_python", (void *) section);
    service.configuration = configuration;

    error = _start_asgi_module(&service, &mod_python_module);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_python_module.loop_python);

    delete_sort_map(section);
    delete_sort_map(configuration);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_marker(void) {
    /* allocates space for the module the applications are taken out
    of and for the one that is read for a marker */
    PyObject *module;
    PyObject *value;
    ERROR_CODE error;

    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_marker", (char *) ASGI_TEST_SCRIPT_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(module);

    /* an application carrying none of the markers is read as absent
    and leaves no error of the interpreter behind it, one that did
    would be found by whatever call happened to come next */
    value = PyObject_GetAttrString(module, "application");
    V_ASSERT_NOT_NULL(value);
    V_ASSERT_EQ_U(has_marker_handler_asgi(value, "_asgi_single_callable"), FALSE);
    V_ASSERT_EQ_U(has_marker_handler_asgi(value, "_asgi_double_callable"), FALSE);
    V_ASSERT_NULL(PyErr_Occurred());
    Py_DECREF(value);

    /* and one that carries it is read as carrying it, whatever the
    shape of the callable underneath happens to be */
    value = PyObject_GetAttrString(module, "marked_double");
    V_ASSERT_NOT_NULL(value);
    V_ASSERT_EQ_U(has_marker_handler_asgi(value, "_asgi_double_callable"), TRUE);
    Py_DECREF(value);

    Py_DECREF(module);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_module_python_lifecycle(void) {
    /* allocates space for the service the module is started against,
    for the environment that carries it and for the module itself */
    struct service_t *service;
    struct environment_t environment;
    struct module_t module;
    struct http_handler_t *http_handler;
    struct mod_python_module_t *mod_python_module;
    struct sort_map_t *configuration;
    struct sort_map_t *section;

    /* builds the service and points its configuration at the script
    the suite loads an application out of, which is what the starting
    of the module reads the path out of */
    create_service(&service, (unsigned char *) "viriatum", (unsigned char *) "viriatum");

    /* loads the values that describe the build into the service, the
    module writes them into the one it exports to the interpreter and
    a service that carries none of them is never one that serves */
    load_specifications(service);
    create_sort_map(&configuration, 8);
    create_sort_map(&section, 8);
    set_value_string_sort_map(section, (unsigned char *) "asgi_script_path", (void *) ASGI_TEST_SCRIPT_PATH);
    set_value_string_sort_map(configuration, (unsigned char *) "mod_python", (void *) section);
    service->configuration = configuration;
    environment.name = (unsigned char *) "test";
    environment.service = service;

    /* starts the module, which brings the interpreter up, loads the
    application and hands the serving of it to the handler */
    start_module_python(&environment, &module);
    mod_python_module = (struct mod_python_module_t *) module.lower;
    V_ASSERT_NOT_NULL(mod_python_module);
    V_ASSERT_NOT_NULL(mod_python_module->mod_python_asgi);
    V_ASSERT_NOT_NULL(mod_python_module->mod_python_asgi->application);
    V_ASSERT_NOT_NULL(mod_python_module->loop_python);

    /* the handler of the interface reaches the service under the name
    that a configuration and the command line both name it by */
    http_handler = NULL;
    service->get_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    V_ASSERT_NOT_NULL(http_handler);

    /* the operation run once per cycle is taken by the module, which
    is what advances a task that no request is driving */
    V_ASSERT_EQ_P(service->on_cycle, cycle_module_python);

    /* and running it advances the loop rather than raising, the
    waiting of the polling being left at a value it may wait on */
    V_ASSERT_EQ_U(cycle_module_python(service), 0);

    /* stops the module, which takes the application down, gives the
    operation of the cycle back and releases everything it held */
    stop_module_python(&environment, &module);
    V_ASSERT_NULL(service->on_cycle);

    http_handler = NULL;
    service->get_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    V_ASSERT_NULL(http_handler);

    delete_sort_map(section);
    delete_sort_map(configuration);
    service->configuration = NULL;
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

void exec_mod_python_tests(struct test_case_t *test_case) {
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

    /* the state of the more recent of the two interfaces */
    V_RUN_TEST(test_mod_python_asgi, test_case);
    V_RUN_TEST(test_asgi_application, test_case);
    V_RUN_TEST(test_asgi_application_missing, test_case);
    V_RUN_TEST(test_asgi_application_uncallable, test_case);
    V_RUN_TEST(test_asgi_application_reload, test_case);
    V_RUN_TEST(test_asgi_application_unload, test_case);
    V_RUN_TEST(test_asgi_module_load, test_case);
    V_RUN_TEST(test_asgi_module_broken, test_case);
    V_RUN_TEST(test_asgi_double_callable, test_case);
    V_RUN_TEST(test_asgi_absolute, test_case);
    V_RUN_TEST(test_asgi_path, test_case);
    V_RUN_TEST(test_asgi_configuration, test_case);
    V_RUN_TEST(test_asgi_cycle, test_case);
    V_RUN_TEST(test_asgi_marker, test_case);
    V_RUN_TEST(test_asgi_start_unconfigured, test_case);
    V_RUN_TEST(test_asgi_start_missing, test_case);

    /* the starting and the stopping of the module, which takes the
    interpreter down with it and so closes the suite */
    V_RUN_TEST(test_module_python_lifecycle, test_case);
}

ERROR_CODE run_mod_python_tests(void) {
    ERROR_CODE return_value = run_test_case(exec_mod_python_tests, "mod_python_tests");
    RAISE_AGAIN(return_value);
}
