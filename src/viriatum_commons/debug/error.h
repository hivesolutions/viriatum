/*
 Hive Viriatum Commons
 Copyright (c) 2008-2016 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../system/system_util.h"
#include "logging.h"

#ifdef VIRIATUM_PLATFORM_WIN32
#define DEBUGGER __debugreak();
#else
#define DEBUGGER
#endif

#define D_ERROR_CODE 1
#define EXCEPTION_ERROR_CODE 2
#define RUNTIME_EXCEPTION_ERROR_CODE 3

#define ERROR_BUFFER_SIZE 4096

#define EMPTY_ERROR_MESSAGE "N/A"

#define ERROR_CODE unsigned int
#define RAISE_AGAIN(error_code) return error_code
#define RAISE_ERROR(error_code) unset_last_error_message(); return error_code
#define RAISE_ERROR_M(error_code, error_message) set_last_error_message(error_message); return error_code
#define RAISE_ERROR_F(error_code, error_message, ...) set_last_error_message_f(error_message, __VA_ARGS__); return error_code
#define RAISE_ERROR_S(error_code) return error_code
#define RAISE_NO_ERROR return 0
#define CATCH_ERROR V_WARNING_F("%s\n", get_last_error_message_safe()); unset_last_error_message()
#define CATCH_ERROR_R CATCH_ERROR; return 0
#define RESET_ERROR unset_last_error_message()
#define IS_ERROR_CODE(error_code) (error_code != 0)
#define GET_ERROR get_last_error_message_safe

VIRIATUM_EXTERNAL_PREFIX unsigned int last_error_code;
VIRIATUM_EXTERNAL_PREFIX unsigned char *last_error_message;
VIRIATUM_EXTERNAL_PREFIX unsigned char last_error_message_b[ERROR_BUFFER_SIZE];

/**
 * Retrieves the last (current) error code available.
 * This method uses the global variable last error code
 * to retrieve its value.
 *
 * @return The last (current) error code available.
 */
static __inline unsigned int get_last_error_code() {
    return last_error_code;
}

static __inline void set_last_error_code(ERROR_CODE error_code) {
    last_error_code = error_code;
}

static __inline unsigned char *get_last_error_message() {
    return last_error_message;
}

static __inline unsigned char *get_last_error_message_safe() {
    /* in case the last error message is not set
    returns the empty error message */
    if(last_error_message == NULL) {    
        return (unsigned char *) EMPTY_ERROR_MESSAGE;
    }
    /* otherwise (normal behaviour, returns the last
    error message value as a string */
    else {
        return last_error_message;
    }
}

static __inline void set_last_error_message(unsigned char *error_message) {
    memcpy(
        (char *) last_error_message_b,
        (char *) error_message,
        strlen((char *) error_message) + 1
    );
    last_error_message = last_error_message_b;
}

static __inline void set_last_error_message_f(unsigned char *error_message, ...) {
    va_list args;
    va_start(args, error_message);
    VSPRINTF(
        (char *) last_error_message_b,
        ERROR_BUFFER_SIZE,
        (char *) error_message,
        args
    );
    va_end(args);
    last_error_message = last_error_message_b;
}

static __inline void unset_last_error_message() {
    last_error_message = NULL;
}
