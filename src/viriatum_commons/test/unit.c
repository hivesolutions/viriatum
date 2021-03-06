/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "unit.h"

ERROR_CODE run_speed_test(char *name, test_function function, size_t iterations) {
    /* allocates the variables (long variables) that are going to be
    used to stores the values for the speed measuring and the float
    value to be used at the final type conversion */
    size_t index;
    long start_time;
    long end_time;
    float elapsed;
    float elapsed_f;

    /* defaults the iterations count value to the one value
    in case no valid has been provided */
    iterations = iterations == 0 ? 1 : iterations;

    /* prints a debug message at the beginning of the speed function
    execution process indicating the name of the function */
    PRINTF_F("Running '%s' for %ld times ...\n", name, (long int) iterations);

    /* retrieves the initial clock value for the execution of the
    function, it's going to be used to measure time at the end */
    start_time = clock();

    /* executes the function, the control flow may only be returned
    after a couple of seconds or more (assume worst case) */
    for(index = 0; index < iterations; index++) { function(); }

    /* retrieves the final clock for the program execution and ten
    calculates the elapsed time with the difference between the start
    time and the current time */
    end_time = clock();
    elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC * 1000.0f;

    /* calculates the elapsed time as a float value and then prints
    a message regarding the amount of time it took to execute */
    elapsed_f = (float) elapsed / 1000.0f;
    PRINTF_F("Executed '%s' in %.2f seconds\n", name, elapsed_f);

    /* returns a no error value indicating that nothing outside
    the normal values has occurred */
    RAISE_NO_ERROR;
}

ERROR_CODE run_test_case(test_case_function function, const char *name) {
    long start_time;
    long end_time;
    float elapsed;
    float elapsed_f;
    struct test_case_t test_case;
    test_case.total = 0;
    test_case.success = 0;
    test_case.failure = 0;
    test_case.echo = TRUE;
    V_PRINT_F("Running %s test case ...\n", name);
    start_time = clock();
    function(&test_case);
    end_time = clock();
    elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC * 1000.0f;
    elapsed_f = (float) elapsed / 1000.0f;
    V_PRINT_F(
        "Ran %d tests in %.2f seconds (%d ok, %d not ok)\n",
        test_case.total, elapsed_f, test_case.success, test_case.failure
    );
    RAISE_ERROR_S(test_case.failure > 0 ? 1 : 0);
}
