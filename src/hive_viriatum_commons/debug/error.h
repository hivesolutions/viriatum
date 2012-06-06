/*
 Hive Viriatum Commons
 Copyright (C) 2008 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef VIRIATUM_PLATFORM_WIN32
#define DEBUGGER __debugreak();
#else
#define DEBUGGER
#endif

#define EXCEPTION_ERROR_CODE 1
#define RUNTIME_EXCEPTION_ERROR_CODE 2

#define EMPTY_ERROR_MESSAGE "N/A"

#define ERROR_CODE unsigned int
#define RAISE_AGAIN(error_code) return error_code
#define RAISE_ERROR(error_code) set_last_error_message(NULL); return error_code
#define RAISE_ERROR_M(error_code, error_message) set_last_error_message(error_message); return error_code
#define RAISE_ERROR_S(error_code) return error_code
#define RAISE_NO_ERROR return 0
#define IS_ERROR_CODE(error_code) error_code != 0
#define GET_ERROR get_last_error_message_safe

VIRIATUM_EXTERNAL_PREFIX unsigned int last_error_code;
VIRIATUM_EXTERNAL_PREFIX unsigned char *last_error_message;

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
    /* in case the last error message is not set */
    if(last_error_message == NULL) {
        /* returns the empty error message */
        return (unsigned char *) EMPTY_ERROR_MESSAGE;
    }
    /* otherwise (normal behaviour */
    else {
        /* returns the last error message */
        return last_error_message;
    }
}

static __inline void set_last_error_message(unsigned char *error_message) {
    last_error_message = error_message;
}
