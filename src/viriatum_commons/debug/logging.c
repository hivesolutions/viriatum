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

#include "stdafx.h"

#include "logging.h"

int logging_use_color(void) {
#ifdef VIRIATUM_PLATFORM_WIN32
    /* on Windows, checks whether the virtual terminal processing mode
    is enabled, which is required for ANSI escape sequences to work */
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if(handle == INVALID_HANDLE_VALUE) { return 0; }
    if(!GetConsoleMode(handle, &mode)) { return 0; }
    return (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) ? 1 : 0;
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
    /* on Unix, checks whether stdout is connected to a terminal */
    return isatty(fileno(stdout)) ? 1 : 0;
#endif

    return 0;
}

void debug(const char *format, ...) {
    /* allocates the arguments list */
    va_list arguments_list;

    /* loads the arguments list */
    va_start(arguments_list, format);

    /* calls the print function */
    vprintf(format, arguments_list);
}
