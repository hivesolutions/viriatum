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

#include "stdafx.h"

#include "unit.h"

ERROR_CODE run_test_case(test_case_function function, const char *name) {
    struct test_case_t test_case;
    test_case.total = 0;
    test_case.success = 0;
    test_case.failure = 0;
    V_PRINT_F("Executing %s ...\n", name);
    function(&test_case);
    V_PRINT_F(
        "Ran %d tests (%d success, %d failure)\n",
        test_case.total, test_case.success, test_case.failure
    );
    RAISE_ERROR_S(test_case.failure > 0 ? 1 : 0);
}
