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
#include "../module/entry.h"
#include "../module/handler.h"

/**
 * Tests that the mod ASGI HTTP handler is created with the
 * default values and released without leaving anything behind.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_mod_asgi_http_handler(void);

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
 * Tests that the module is populated with the name, the version
 * and the operations that the service reaches it through.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_module_info(void);

/**
 * Tests that the reason of a failure is handed back through the
 * operation the service asks for one with.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_asgi_module_error(void);

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
const char *test_asgi_module_lifecycle(void);

void exec_mod_asgi_tests(struct test_case_t *test_case);
ERROR_CODE run_mod_asgi_tests(void);
