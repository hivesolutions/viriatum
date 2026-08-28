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

#pragma once

#include "simple_test.h"

/**
 * Populates the provided suite with the table of entries
 * that describe the set of speed tests.
 *
 * @param suite The suite to be populated with the entries
 * of the speed tests and with their name.
 */
void create_speed_suite(struct test_suite_t *suite);

/**
 * Starts the various test that measure performance
 * for the current viriatum infra-structure, the results
 * are printed in the current standard output file.
 *
 * @param options The options that control the selection of
 * the tests to be run and the reporting of the results, a
 * null value runs every one of them reporting to the
 * standard output alone.
 */
ERROR_CODE run_speed_tests(struct test_options_t *options);
