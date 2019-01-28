/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "simple_test.h"

/**
 * Executes the set of speed tests in the current
 * test case.
 *
 * @param test_case The test case context for which
 * the speed tests will be executed, should be able
 * to store some context information about the execution.
 */
void exec_speed_tests(struct test_case_t *test_case);

/**
 * Starts the various test that measure performance
 * for the current viriatum infra-structure, the results
 * are printed in the current standard output file.
 */
ERROR_CODE run_speed_tests();
