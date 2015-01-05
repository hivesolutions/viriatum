/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
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
