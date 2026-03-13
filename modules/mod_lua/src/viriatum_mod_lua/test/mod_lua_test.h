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

#pragma once

#include "../stdafx.h"
#include "../module/handler.h"
#include "../module/extension.h"

/**
 * Tests that the handler Lua context is properly
 * initialized with zeroed buffers and default values.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_lua_context(void);

/**
 * Tests that the URL callback correctly parses
 * URLs with and without query strings.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_lua_url(void);

/**
 * Tests that the header field callback correctly
 * identifies known headers (Content-Type, Content-Length,
 * Cookie, Host) and sets the next header state.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_lua_header_field(void);

/**
 * Tests that the header value callback correctly
 * stores values for the recognized header types.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_lua_header_value(void);

/**
 * Tests that lua_input_read returns an empty string
 * when no POST data is available (GET request).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_lua_input_read_null(void);

/**
 * Tests that lua_input_read returns the full POST
 * data buffer and advances the position to the end.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_lua_input_read_data(void);

/**
 * Tests the WSAPI lifecycle: loading a Lua script,
 * verifying the _wsapi_app global is stored, and
 * that the run function can be resolved from it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_wsapi_lifecycle(void);

/**
 * Tests that the _wsapi_app global persists across
 * multiple calls without reloading the script (the
 * file_dirty=0 path).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_wsapi_app_persistence(void);

/**
 * Executes the set of mod Lua tests in the current
 * test case.
 *
 * @param test_case The test case context for which
 * the mod Lua tests will be executed, should be able
 * to store some context information about the execution.
 */
void exec_mod_lua_tests(struct test_case_t *test_case);

/**
 * Runs the set of mod Lua tests in the current
 * test case. This is the main entry point for the
 * mod Lua test case.
 */
ERROR_CODE run_mod_lua_tests(void);
