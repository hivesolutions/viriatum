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

#include "mod_lua_test.h"

const char *test_handler_lua_context(void) {
    /* allocates space for the handler Lua context
    structure to be used in the test */
    struct handler_lua_context_t *handler_lua_context;

    /* creates the handler Lua context and verifies
    that the default values are properly initialized */
    create_handler_lua_context(&handler_lua_context);

    V_ASSERT(handler_lua_context->url[0] == '\0');
    V_ASSERT(handler_lua_context->file_name[0] == '\0');
    V_ASSERT(handler_lua_context->query[0] == '\0');
    V_ASSERT(handler_lua_context->prefix_name[0] == '\0');
    V_ASSERT(handler_lua_context->file_path[0] == '\0');
    V_ASSERT(handler_lua_context->content_type[0] == '\0');
    V_ASSERT(handler_lua_context->content_length_[0] == '\0');
    V_ASSERT(handler_lua_context->cookie[0] == '\0');
    V_ASSERT(handler_lua_context->host[0] == '\0');
    V_ASSERT(handler_lua_context->server_name[0] == '\0');
    V_ASSERT(handler_lua_context->header == NULL);
    V_ASSERT(handler_lua_context->flags == 0);
    V_ASSERT(handler_lua_context->_next_header == UNDEFINED_HEADER);
    V_ASSERT(handler_lua_context->_url_string.length == 0);
    V_ASSERT(handler_lua_context->_file_name_string.length == 0);
    V_ASSERT(handler_lua_context->_query_string.length == 0);
    V_ASSERT(handler_lua_context->_prefix_name_string.length == 0);

    /* deletes the handler Lua context */
    delete_handler_lua_context(handler_lua_context);

    return NULL;
}

const char *test_handler_lua_url(void) {
    ERROR_CODE error;
    struct http_parser_t *http_parser;
    struct handler_lua_context_t *handler_lua_context;

    /* creates the HTTP parser and the handler Lua context
    then wires them together through the context pointer */
    create_http_parser(&http_parser, TRUE);
    create_handler_lua_context(&handler_lua_context);
    http_parser->context = handler_lua_context;

    /* tests that a normal URL is properly parsed,
    note that the file_name is normalized to the platform
    path separator while the url is kept as-is */
    error = url_callback_handler_lua(
        http_parser,
        (unsigned char *) "/index.html",
        11
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) handler_lua_context->url, "/index.html") == 0);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp((char *) handler_lua_context->file_name, "\\index.html") == 0);
#else
    V_ASSERT(strcmp((char *) handler_lua_context->file_name, "/index.html") == 0);
#endif
    V_ASSERT(handler_lua_context->query[0] == '\0');

    /* tests that a URL with query string has the query
    parameters properly separated from the path */
    error = url_callback_handler_lua(
        http_parser,
        (unsigned char *) "/page?id=1&name=test",
        20
    );
    V_ASSERT(error == 0);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp((char *) handler_lua_context->file_name, "\\page") == 0);
#else
    V_ASSERT(strcmp((char *) handler_lua_context->file_name, "/page") == 0);
#endif
    V_ASSERT(strcmp((char *) handler_lua_context->query, "id=1&name=test") == 0);
    V_ASSERT(strcmp((char *) handler_lua_context->url, "/page?id=1&name=test") == 0);

    /* tests that the prefix name is cleared */
    V_ASSERT(handler_lua_context->prefix_name[0] == '\0');

    /* tests the string length tracking */
    V_ASSERT(handler_lua_context->_file_name_string.length == 5);
    V_ASSERT(handler_lua_context->_query_string.length == 14);

    delete_handler_lua_context(handler_lua_context);
    delete_http_parser(http_parser);

    return NULL;
}

const char *test_handler_lua_header_field(void) {
    struct http_parser_t *http_parser;
    struct handler_lua_context_t *handler_lua_context;
    struct http_headers_t headers;

    /* creates the HTTP parser and the handler Lua context
    then wires them together */
    create_http_parser(&http_parser, TRUE);
    create_handler_lua_context(&handler_lua_context);
    http_parser->context = handler_lua_context;

    /* initializes the headers structure and wires it into
    the handler context (normally done by set_handler_lua) */
    headers.count = 0;
    handler_lua_context->headers = &headers;

    /* tests that "Content-Type" is recognized */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Content-Type",
        12
    );
    V_ASSERT(handler_lua_context->_next_header == CONTENT_TYPE);
    V_ASSERT(strcmp(handler_lua_context->header->name, "Content-Type") == 0);

    /* tests that "Content-Length" is recognized */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Content-Length",
        14
    );
    V_ASSERT(handler_lua_context->_next_header == CONTENT_LENGTH);

    /* tests that "Cookie" is recognized */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Cookie",
        6
    );
    V_ASSERT(handler_lua_context->_next_header == COOKIE);

    /* tests that "Host" is recognized */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Host",
        4
    );
    V_ASSERT(handler_lua_context->_next_header == HOST);

    /* tests that an unknown header leaves the state as undefined */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "X-Custom",
        8
    );
    V_ASSERT(handler_lua_context->_next_header == UNDEFINED_HEADER);

    delete_handler_lua_context(handler_lua_context);
    delete_http_parser(http_parser);

    return NULL;
}

const char *test_handler_lua_header_value(void) {
    struct http_parser_t *http_parser;
    struct handler_lua_context_t *handler_lua_context;
    struct http_headers_t headers;

    /* creates the HTTP parser and the handler Lua context
    then wires them together */
    create_http_parser(&http_parser, TRUE);
    create_handler_lua_context(&handler_lua_context);
    http_parser->context = handler_lua_context;

    /* initializes the headers structure */
    headers.count = 0;
    handler_lua_context->headers = &headers;

    /* simulates parsing a Content-Type header */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Content-Type",
        12
    );
    header_value_callback_handler_lua(
        http_parser,
        (unsigned char *) "text/html",
        9
    );
    V_ASSERT(strcmp((char *) handler_lua_context->content_type, "text/html") == 0);
    V_ASSERT(handler_lua_context->_content_type_string.length == 9);
    V_ASSERT(headers.count == 1);

    /* simulates parsing a Content-Length header */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Content-Length",
        14
    );
    header_value_callback_handler_lua(
        http_parser,
        (unsigned char *) "42",
        2
    );
    V_ASSERT(strcmp((char *) handler_lua_context->content_length_, "42") == 0);
    V_ASSERT(handler_lua_context->_content_length_string.length == 2);
    V_ASSERT(headers.count == 2);

    /* simulates parsing a Host header with port */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Host",
        4
    );
    header_value_callback_handler_lua(
        http_parser,
        (unsigned char *) "localhost:8080",
        14
    );
    V_ASSERT(strcmp((char *) handler_lua_context->host, "localhost:8080") == 0);
    V_ASSERT(strcmp((char *) handler_lua_context->server_name, "localhost") == 0);
    V_ASSERT(handler_lua_context->_server_name_string.length == 9);

    /* simulates parsing a Cookie header */
    header_field_callback_handler_lua(
        http_parser,
        (unsigned char *) "Cookie",
        6
    );
    header_value_callback_handler_lua(
        http_parser,
        (unsigned char *) "session=abc123",
        14
    );
    V_ASSERT(strcmp((char *) handler_lua_context->cookie, "session=abc123") == 0);

    delete_handler_lua_context(handler_lua_context);
    delete_http_parser(http_parser);

    return NULL;
}

const char *test_lua_input_read_null(void) {
    lua_State *lua_state;
    const char *result;
    size_t result_size;

    /* creates a Lua state and registers the input metatable */
    lua_state = luaL_newstate();
    luaL_newmetatable(lua_state, VIRIATUM_LUA_INPUT_NAME);
    lua_pushcfunction(lua_state, lua_input_read);
    lua_setfield(lua_state, -2, "read");
    lua_pushvalue(lua_state, -1);
    lua_setfield(lua_state, -2, "__index");
    lua_pop(lua_state, 1);

    /* pushes a NULL post_data input (simulating a GET request) */
    lua_input_push(lua_state, NULL, 0);

    /* calls the read method on the input object */
    lua_getfield(lua_state, -1, "read");
    lua_pushvalue(lua_state, -2);
    lua_call(lua_state, 1, 1);

    /* verifies the result is an empty string */
    result = lua_tolstring(lua_state, -1, &result_size);
    V_ASSERT(result != NULL);
    V_ASSERT(result_size == 0);

    lua_close(lua_state);

    return NULL;
}

const char *test_lua_input_read_data(void) {
    lua_State *lua_state;
    const char *result;
    size_t result_size;
    unsigned char post_data[] = "hello=world&foo=bar";

    /* creates a Lua state and registers the input metatable */
    lua_state = luaL_newstate();
    luaL_newmetatable(lua_state, VIRIATUM_LUA_INPUT_NAME);
    lua_pushcfunction(lua_state, lua_input_read);
    lua_setfield(lua_state, -2, "read");
    lua_pushvalue(lua_state, -1);
    lua_setfield(lua_state, -2, "__index");
    lua_pop(lua_state, 1);

    /* pushes input with actual POST data */
    lua_input_push(lua_state, post_data, 19);

    /* calls read and verifies the full data is returned */
    lua_getfield(lua_state, -1, "read");
    lua_pushvalue(lua_state, -2);
    lua_call(lua_state, 1, 1);

    result = lua_tolstring(lua_state, -1, &result_size);
    V_ASSERT(result != NULL);
    V_ASSERT(result_size == 19);
    V_ASSERT(memcmp(result, "hello=world&foo=bar", 19) == 0);
    lua_pop(lua_state, 1);

    /* calls read again and verifies nothing is returned
    (position has been advanced to the end) */
    lua_getfield(lua_state, -1, "read");
    lua_pushvalue(lua_state, -2);
    lua_call(lua_state, 1, 1);

    result = lua_tolstring(lua_state, -1, &result_size);
    V_ASSERT(result != NULL);
    V_ASSERT(result_size == 0);

    lua_close(lua_state);

    return NULL;
}

const char *test_wsapi_lifecycle(void) {
    lua_State *lua_state;
    int result_code;
    const char *script_path;

    /* creates a fresh Lua state and opens the standard libraries
    so that the test script has access to coroutine, string, etc. */
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    /* resolves the path to the test handler script, the test binary
    is expected to run from the build directory so the script path
    is passed as a compile-time define */
#ifdef VIRIATUM_TEST_SCRIPT_PATH
    script_path = VIRIATUM_TEST_SCRIPT_PATH;
#else
    script_path = "test_handler.lua";
#endif

    /* loads and runs the script, the return value should be a table */
    result_code = luaL_dofile(lua_state, script_path);
    V_ASSERT_M(result_code == 0, "failed to load test_handler.lua");
    V_ASSERT_M(lua_istable(lua_state, -1), "script did not return a table");

    /* stores the returned table as _wsapi_app (mirroring handler.c logic) */
    lua_setglobal(lua_state, "_wsapi_app");

    /* verifies that the run function can be retrieved from _wsapi_app */
    lua_getglobal(lua_state, "_wsapi_app");
    V_ASSERT_M(!lua_isnil(lua_state, -1), "_wsapi_app is nil after store");
    lua_getfield(lua_state, -1, "run");
    V_ASSERT_M(lua_isfunction(lua_state, -1), "run is not a function");
    lua_pop(lua_state, 2);

    lua_close(lua_state);

    return NULL;
}

const char *test_wsapi_app_persistence(void) {
    lua_State *lua_state;
    int result_code;
    const char *script_path;

    /* creates a fresh Lua state and opens the standard libraries */
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

#ifdef VIRIATUM_TEST_SCRIPT_PATH
    script_path = VIRIATUM_TEST_SCRIPT_PATH;
#else
    script_path = "test_handler.lua";
#endif

    /* first load: execute script and store module table */
    result_code = luaL_dofile(lua_state, script_path);
    V_ASSERT_M(result_code == 0, "failed to load test_handler.lua (first)");
    V_ASSERT_M(lua_istable(lua_state, -1), "script did not return a table (first)");
    lua_setglobal(lua_state, "_wsapi_app");

    /* simulate the non-dirty path: do NOT re-execute the script,
    just verify _wsapi_app is still accessible with a valid run function
    (this is the scenario that triggered the alternating failure bug) */
    lua_getglobal(lua_state, "_wsapi_app");
    V_ASSERT_M(!lua_isnil(lua_state, -1), "_wsapi_app lost on second access");
    lua_getfield(lua_state, -1, "run");
    V_ASSERT_M(lua_isfunction(lua_state, -1), "run lost on second access");
    lua_pop(lua_state, 2);

    /* simulate a third access to confirm persistence */
    lua_getglobal(lua_state, "_wsapi_app");
    V_ASSERT_M(!lua_isnil(lua_state, -1), "_wsapi_app lost on third access");
    lua_getfield(lua_state, -1, "run");
    V_ASSERT_M(lua_isfunction(lua_state, -1), "run lost on third access");
    lua_pop(lua_state, 2);

    lua_close(lua_state);

    return NULL;
}

void exec_mod_lua_tests(struct test_case_t *test_case) {
    /* HTTP callback tests (no Lua dependency) */
    V_RUN_TEST(test_handler_lua_context, test_case);
    V_RUN_TEST(test_handler_lua_url, test_case);
    V_RUN_TEST(test_handler_lua_header_field, test_case);
    V_RUN_TEST(test_handler_lua_header_value, test_case);

    /* Lua extension tests */
    V_RUN_TEST(test_lua_input_read_null, test_case);
    V_RUN_TEST(test_lua_input_read_data, test_case);

    /* WSAPI integration tests */
    V_RUN_TEST(test_wsapi_lifecycle, test_case);
    V_RUN_TEST(test_wsapi_app_persistence, test_case);
}

ERROR_CODE run_mod_lua_tests(void) {
    ERROR_CODE return_value = run_test_case(exec_mod_lua_tests, "mod_lua_tests");
    RAISE_AGAIN(return_value);
}
