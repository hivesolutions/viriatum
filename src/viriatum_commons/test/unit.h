/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../debug/debug.h"

#define V_ASSERT(test) V_ASSERT_M(test, #test)
#define V_ASSERT_M(test, message) do { if(!(test)) { return message; } } while (0)
#define V_RUN_TEST(test, test_case) do {\
    const char *message;\
    if(test_case->echo == TRUE) { V_PRINT_F("%s ... ", #test); PRINT_FLUSH(); }\
    message = test();\
    test_case->total++;\
    if(message == NULL) {\
        if(test_case->echo == TRUE) {\
            V_PRINT("ok\n");\
        }\
        test_case->success++;\
    } else {\
        if(test_case->echo == TRUE) {\
            V_PRINT("not ok\n");\
            V_PRINT_F("[%s:%d] %s\n", base_string_value((unsigned char *) __FILE__), __LINE__, message);\
        }\
        test_case->failure++;\
    }\
} while (0)
#define V_RUN_SPEED(test, count, test_case) do {\
    run_speed_test(#test, test, count);\
    test_case->total++;\
    test_case->success++;\
} while (0)

typedef struct test_case_t {
    unsigned int total;
    unsigned int success;
    unsigned int failure;
    unsigned char echo;
} test_case;

/**
 * General function used for testing purposes, this is the
 * simplest definition for a test function. This kind of
 * functions should return a description of the result.
 */
typedef const char *(*test_function) ();

/**
 * Definition of the general entry point for a function that
 * is resposible for a test case execution.
 *
 * This function should run the various test associated with
 * the test case and then populate the test case structure.
 */
typedef void (*test_case_function) (struct test_case_t *test_case);

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
VIRIATUM_EXPORT_PREFIX ERROR_CODE run_speed_test(char *name, test_function function, size_t iterations);

/**
 * Runs the test case defined by the provided function and
 * described throught the provided name.
 *
 * The test case execution will be verbose meaning that a
 * message output will be performed.
 *
 * @param function The function that is responsible for the
 * the execution of the various test functions.
 * @param name The name of the test case that is going to be
 * for some debug output.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE run_test_case(test_case_function function, const char *name);
