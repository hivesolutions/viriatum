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
