/*
 Hive Viriatum Commons
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../debug/debug.h"

VIRIATUM_EXPORT_PREFIX ERROR_CODE encode_percent(unsigned char *buffer, size_t length, unsigned char **buffer_pointer, size_t *length_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE decode_percent(unsigned char *buffer, size_t length, unsigned char **buffer_pointer,  size_t *length_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE url_encode(unsigned char *buffer, size_t length, unsigned char **buffer_pointer, size_t *length_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE url_decode(unsigned char *buffer, size_t length, unsigned char **buffer_pointer, size_t *length_pointer);

static __inline char from_hex(unsigned char byte) {
    return isdigit(byte) ? byte - '0' : tolower(byte) - 'a' + 10;
}

static __inline char to_hex(unsigned char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

static __inline char isalphanum(unsigned char byte) {
    if((byte >= '0' && byte <= '9') || (byte >= 'a' && byte <= 'z') ||
       (byte >= 'A' && byte <= 'Z')) { return 1; }

    return 0;
}
