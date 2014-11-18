/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2014 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2014 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "speed_test.h"

void run_speed_test(char *name, test_function function, size_t iterations) {
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
}

ERROR_CODE run_speed_tests() {
    run_speed_test("test_linked_list", test_linked_list, 1000000);
    run_speed_test("test_linked_list_stress", test_linked_list_stress, 1);
    run_speed_test("test_linked_list_big", test_linked_list_big, 1);
    RAISE_NO_ERROR;
}
