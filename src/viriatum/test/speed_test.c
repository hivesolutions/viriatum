/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "speed_test.h"

void exec_speed_tests(struct test_case_t *test_case) {
    V_RUN_SPEED(test_linked_list, 1000000, test_case);
    V_RUN_SPEED(test_linked_list_stress, 1, test_case);
    V_RUN_SPEED(test_linked_list_big, 1, test_case);
}

ERROR_CODE run_speed_tests() {
    ERROR_CODE return_value = run_test_case(exec_speed_tests, "speed_tests");
    RAISE_AGAIN(return_value);
}
