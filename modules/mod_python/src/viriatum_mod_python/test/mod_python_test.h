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
 * Tests that the handler WSGI context is properly
 * initialized with zeroed buffers and default values.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_wsgi_context(void);

/**
 * Tests that the URL callback correctly parses
 * URLs with and without query strings.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_wsgi_url(void);

/**
 * Tests that the header field callback correctly
 * identifies known headers (Content-Type, Content-Length,
 * Cookie, Host) and sets the next header state.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_wsgi_header_field(void);

/**
 * Tests that the header value callback correctly
 * stores values for the recognized header types.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_wsgi_header_value(void);

/**
 * Tests that wsgi_input_read returns an empty bytes
 * object when no POST data is available (GET request).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_wsgi_input_read_null(void);

/**
 * Tests that wsgi_input_read returns the full POST
 * data buffer and advances the position to the end.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_wsgi_input_read_data(void);

/**
 * Tests the WSGI application lifecycle: loading a Python
 * script through _load_module_python, and verifying that
 * the application attribute can be resolved from it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_wsgi_app_lifecycle(void);

/**
 * Tests that a loaded Python module remains accessible
 * across multiple lookups without reloading the file
 * (the reload=0 path).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_wsgi_app_persistence(void);

/**
 * Executes the set of mod Python tests in the current
 * test case.
 *
 * @param test_case The test case context for which
 * the mod Python tests will be executed, should be able
 * to store some context information about the execution.
 */
/**
 * Tests that the mod ASGI HTTP handler is created with the
 * default values and released without leaving anything behind.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_mod_python_asgi(void);

/**
 * Tests that the application is loaded out of the module that
 * the path of the handler names and that it is a callable.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_application(void);

/**
 * Tests that a path that no file sits under and a name that the
 * module does not carry are both refused rather than leaving the
 * handler with something that may not be served.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_application_missing(void);

/**
 * Tests that an attribute that is not a callable at all is refused
 * the way a missing one is, an application being a thing that may
 * be called and never merely a name that resolves.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_application_uncallable(void);

/**
 * Tests that the loading of a second application releases the one
 * that came before it rather than leaking it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_application_reload(void);

/**
 * Tests that the unloading leaves nothing set and that running it
 * a second time over the same handler is harmless.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_application_unload(void);

/**
 * Tests that a module is loaded out of a file and that it carries
 * the names the file defines.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_module_load(void);

/**
 * Tests that a file that is not there and a file that does not
 * compile both leave the module unset without raising, which is
 * what the loading of an application is told apart by.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_module_broken(void);

/**
 * Tests that the shape of a callable is told from the callable
 * itself, the single one of the more recent version apart from
 * the pair of the older and from the class that stands for it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_double_callable(void);

/**
 * Tests that a path is told to be absolute both by the separator
 * that opens it and by the drive letter that may open it instead,
 * a configuration naming the second being resolved against the
 * contents of the service and reaching nothing at all.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_absolute(void);

/**
 * Tests that the directory the application sits in is put ahead of
 * everything else on the path of the imports, so that a module
 * beside it is the one that is found for it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_path(void);

/**
 * Tests that the module is populated with the name, the version
 * and the operations that the service reaches it through.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_module_python_info(void);

/**
 * Tests that the reason of a failure is handed back through the
 * operation the service asks for one with.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_module_python_error(void);

/**
 * Tests that the values of the configuration reach the handler,
 * the path of the file among them, and that a section which is
 * not there leaves the values that stand in for it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_configuration(void);

/**
 * Tests that the operation run once per cycle is harmless while
 * no application has been loaded, which is the state a failed
 * loading leaves the module in.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_cycle(void);

/**
 * Tests the starting and the stopping of the module against a real
 * service, the handler of the interface reaching the service and the
 * application being loaded, started and taken down again.
 *
 * This one takes the interpreter down with it, so it is the last of
 * the suite to be run and nothing that holds a reference of the
 * interpreter may follow it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_module_python_lifecycle(void);

/**
 * Tests that a configuration naming no application at all leaves the
 * more recent interface unserved rather than failing the starting of
 * the module, which carries the older one on its own.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_start_unconfigured(void);

/**
 * Tests that a configuration naming an application that may not be
 * loaded fails the starting rather than leaving the module reporting
 * a success it did not have, and that the stopping of one that never
 * started is harmless.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_start_missing(void);

/**
 * Tests that the marker of an adaptation helper is read as absent
 * when the application carries none of it, rather than the error of
 * the interpreter being left behind for another call to find.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_marker(void);

void exec_mod_python_tests(struct test_case_t *test_case);

/**
 * Runs the set of mod Python tests in the current
 * test case. This is the main entry point for the
 * mod Python test case.
 */
ERROR_CODE run_mod_python_tests(void);
