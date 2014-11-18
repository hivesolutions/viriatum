/*
 Hive Viriatum Commons
 Copyright (C) 2008-2014 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2014 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../debug/debug.h"

#define V_ASSERT(test, message) do { if(!(test)) { return message; } } while (0)
#define V_RUN_TEST(test, test_case, echo) do {\
    const char *message;\
    if(echo == TRUE) { V_PRINT_F("%s ... ", #test); }\
    message = test();\
    test_case->total++;\
    if(message == NULL) {\
        if(echo == TRUE) {\
            V_PRINT("ok\n");\
        }\
        test_case->success++;\
    } else {\
        if(echo == TRUE) {\
            V_PRINT("not ok\n");\
            V_PRINT_F("%s\n", message);\
        }\
        test_case->failure++;\
    }\
} while (0)

typedef struct test_case_t {
    unsigned int total;
    unsigned int success;
    unsigned int failure;
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

ERROR_CODE run_test_case(test_case_function function, const char *name);
