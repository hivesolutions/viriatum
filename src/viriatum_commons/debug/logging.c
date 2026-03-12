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

#ifdef VIRIATUM_PLATFORM_WIN32
/* ENABLE_VIRTUAL_TERMINAL_PROCESSING is only defined in the
Windows 10 SDK and later, so guard against older SDKs */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

int logging_use_color(void) {
    /* caches the detection result so that the terminal
    check runs only once during the process lifetime */
    static int cached = -1;
    if(cached >= 0) { return cached; }

#ifdef VIRIATUM_PLATFORM_WIN32
    {
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if(handle == INVALID_HANDLE_VALUE) { cached = 0; return 0; }
        if(!GetConsoleMode(handle, &mode)) { cached = 0; return 0; }
        cached = (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) ? 1 : 0;
        return cached;
    }
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
    {
        const char *term = getenv("TERM");
        if(!isatty(fileno(stdout))) { cached = 0; return 0; }
        if(term == NULL || strcmp(term, "dumb") == 0) { cached = 0; return 0; }
        cached = 1;
        return cached;
    }
#endif

    cached = 0;
    return 0;
}

void logging_print_date(void) {
    /* allocates the time value and the broken-down local time
    structure used to format the current date and time */
    time_t time_value;
    struct tm *local_time_value;
    char date_buffer[20];

    /* retrieves the current time and converts it to local
    time so that it can be formatted as a date string */
    time(&time_value);
    local_time_value = localtime(&time_value);
    strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d %H:%M:%S", local_time_value);

    /* prints the date string surrounded by colour escapes when the
    terminal supports it, or as plain text otherwise */
    if(logging_use_color()) {
        printf(V_COLOR_DATE "%s" V_COLOR_RESET " ", date_buffer);
    } else {
        printf("%s ", date_buffer);
    }
}

void debug(const char *format, ...) {
    /* allocates the arguments list */
    va_list arguments_list;

    /* loads the arguments list */
    va_start(arguments_list, format);

    /* calls the print function */
    vprintf(format, arguments_list);
}
