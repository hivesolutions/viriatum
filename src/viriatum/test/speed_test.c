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

#include "stdafx.h"

#include "speed_test.h"

/* the table that describes the speed tests, every one of the
entries carries the amount of iterations that is going to be
performed for it during the measurement */
static struct test_entry_t _speed_entries[] = {
    V_TEST_S(test_linked_list, "structures", 1000000),
    V_TEST_S(test_linked_list_stress, "structures", 1),
    V_TEST_S(test_linked_list_big, "structures", 1)
};

void create_speed_suite(struct test_suite_t *suite) {
    suite->name = "speed_tests";
    suite->entries = _speed_entries;
    suite->count = V_TEST_COUNT(_speed_entries);
    suite->setup = NULL;
    suite->teardown = NULL;
}

ERROR_CODE run_speed_tests(struct test_options_t *options) {
    struct test_suite_t suite;
    ERROR_CODE return_value;
    create_speed_suite(&suite);
    return_value = run_test_suite(&suite, options);
    RAISE_AGAIN(return_value);
}
