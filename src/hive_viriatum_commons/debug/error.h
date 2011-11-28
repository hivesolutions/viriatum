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

#define EXCEPTION_ERROR_CODE 1
#define RUNTIME_EXCEPTION_ERROR_CODE 2

#define EMPTY_ERROR_MESSAGE "N/A"

#define ERROR_CODE unsigned int
#define RAISE_AGAIN(errorCode) return errorCode
#define RAISE_ERROR(errorCode) setLastErrorMessage(NULL); return errorCode
#define RAISE_ERROR_M(errorCode, errorMessage) setLastErrorMessage(errorMessage); return errorCode
#define RAISE_ERROR_S(errorCode) return errorCode
#define RAISE_NO_ERROR return 0
#define IS_ERROR_CODE(errorCode) errorCode != 0
#define GET_ERROR getLastErrorMessageSafe

VIRIATUM_EXPORT_PREFIX unsigned int lastErrorCode;
VIRIATUM_EXPORT_PREFIX unsigned char *lastErrorMessage;

/**
 * Retrieves the last (current) error code available.
 * This method uses the global variable last error code
 * to retrieve its value.
 *
 * @return The last (current) error code available.
 */
static __inline unsigned int getLastErrorCode() {
    return lastErrorCode;
}

static __inline void setLastErrorCode(ERROR_CODE errorCode) {
    lastErrorCode = errorCode;
}

static __inline unsigned char *getLastErrorMessage() {
    return lastErrorMessage;
}

static __inline unsigned char *getLastErrorMessageSafe() {
    /* in case the last error message is not set */
    if(lastErrorMessage == NULL) {
        /* returns the empty error message */
        return (unsigned char *) EMPTY_ERROR_MESSAGE;
    }
    /* otherwise (normal behaviour */
    else {
        /* returns the last error message */
        return lastErrorMessage;
    }
}

static __inline void setLastErrorMessage(unsigned char *errorMessage) {
    lastErrorMessage = errorMessage;
}
