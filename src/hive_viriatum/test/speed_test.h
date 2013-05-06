/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2012 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "simple_test.h"

/**
 * General function used for testing purposes, this is the
 * simplest definition for a test function.
 */
typedef void (*test_function) ();

/**
 * Runs a single speed test and prints a series of messages
 * to the standard output according to the provided name for
 * the speed fucntion.
 *
 * @param name The name of the test function to be executed
 * and measured for time.
 * @param function Pointer to the function to be executed
 * and have its execution time meassured.
 * @param iterations The number of iterations to be executed
 * in the performance test in case this value is not provided
 * the value defaults to one.
 */
void run_speed_test(char *name, test_function function, size_t iterations);

/**
 * Starts the various test that measure performance
 * for the current viriatum infra-structure, the results
 * are printed in the current standard output file.
 */
void run_speed_tests();
