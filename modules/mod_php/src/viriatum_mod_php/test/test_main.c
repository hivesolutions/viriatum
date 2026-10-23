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

#include "mod_php_test.h"

int main(int argc, char **argv) {
    ERROR_CODE return_value;

    V_PRINT_F(
        "viriatum_mod_php test runner (%s, %s)\n",
        VIRIATUM_COMPILATION_DATE,
        VIRIATUM_COMPILATION_TIME
    );

    /* has the interpreter allocate its memory out of the system and keep
    track of every one of the allocations, so that the release of memory
    that a request no longer holds fails on the spot rather than quietly
    corrupting the heap that the requests that follow are served from */
#ifdef VIRIATUM_PLATFORM_WIN32
    _putenv("USE_ZEND_ALLOC=0");
    _putenv("USE_TRACKED_ALLOC=1");
#else
    setenv("USE_ZEND_ALLOC", "0", 1);
    setenv("USE_TRACKED_ALLOC", "1", 1);
#endif

    /* loads the PHP state, starting the interpreter together with
    the module of the server so that the messages of the tests are
    served by a fully initialized virtual machine */
    _load_php_state();

    return_value = run_mod_php_tests();

    /* unloads the PHP state, shutting the interpreter down and
    releasing all the resources associated with it */
    _unload_php_state();

    return IS_ERROR_CODE(return_value) ? 1 : 0;
}
