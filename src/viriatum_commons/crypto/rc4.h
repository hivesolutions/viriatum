/*
 Hive Viriatum Commons
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

typedef struct rc4_t {
    char *key;
    unsigned char x;
    unsigned char y;
    unsigned char box[255];
} rc4;

VIRIATUM_EXPORT_PREFIX void create_rc4(char *key, struct rc4_t **rc4_pointer);
VIRIATUM_EXPORT_PREFIX void delete_rc4(struct rc4_t *rc4);
VIRIATUM_EXPORT_PREFIX void cipher_rc4(struct rc4_t *rc4, char *data, size_t size, char *data_c);
VIRIATUM_EXPORT_PREFIX void crypt_rc4(char *message, char *key, char **result_pointer);
