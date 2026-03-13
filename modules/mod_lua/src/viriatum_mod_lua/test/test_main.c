/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "../stdafx.h"

#include "mod_lua_test.h"

int main(int argc, char **argv) {
    ERROR_CODE return_value;

    V_PRINT_F(
        "viriatum_mod_lua test runner (%s, %s)\n",
        VIRIATUM_COMPILATION_DATE,
        VIRIATUM_COMPILATION_TIME
    );

    return_value = run_mod_lua_tests();

    return IS_ERROR_CODE(return_value) ? 1 : 0;
}
