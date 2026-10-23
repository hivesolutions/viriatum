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
 * Tests that a script that runs to its end is answered with
 * the status, the fields and the output it produced, and that
 * the connection is kept alive once the response has left.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_php_response(void);

/**
 * Tests that the payload of a message is handed to the script
 * that serves it, the reading of the input reaching the very
 * bytes that travelled after the headers.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_php_post(void);

/**
 * Tests that a message that is not meant to be kept alive is
 * answered as such and that the connection is closed once the
 * response has left.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_php_close(void);

/**
 * Tests that a script that is not there is answered with an
 * error and that the interpreter is left able to serve the
 * message that comes after it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_php_missing(void);

/**
 * Tests that a path that names a directory rather than a script
 * is answered with an error and that the interpreter is left able
 * to serve the message that comes after it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_php_directory(void);

/**
 * Tests that a script that fails while it runs is answered with
 * an error and that the interpreter is left able to serve the
 * message that comes after it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_php_error(void);

/**
 * Executes the set of mod PHP tests in the current
 * test case.
 *
 * @param test_case The test case context for which
 * the mod PHP tests will be executed, should be able
 * to store some context information about the execution.
 */
void exec_mod_php_tests(struct test_case_t *test_case);

/**
 * Runs the set of mod PHP tests in the current
 * test case. This is the main entry point for the
 * mod PHP test case.
 */
ERROR_CODE run_mod_php_tests(void);
