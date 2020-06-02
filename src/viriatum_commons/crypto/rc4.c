/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "rc4.h"

void create_rc4(char *key, struct rc4_t **rc4_pointer) {
    /* starts the various internal values to be used
    for the construction of the rc4 structure */
    size_t i;
    unsigned char _x;
    unsigned char x = 0;
    size_t key_length = strlen(key);

    /* retrieves the size of the structure to be created
    and uses the size to allocate the required space */
    size_t rc4_size = sizeof(struct rc4_t);
    struct rc4_t *rc4 = (struct rc4_t *) MALLOC(rc4_size);

    /* sets the base element for the rc4 structure
    including the x and y references and the key */
    rc4->x = 0;
    rc4->y = 0;
    rc4->key = key;

    /* iterates over the range of the box to populate
    it with the initial index value and then computes
    the initial box values according to specification */
    for(i = 0; i < 256; i++) { rc4->box[i] = (unsigned char) i; }
    for(i = 0; i < 256; i++) {
        x = (x + rc4->box[i] + key[i % key_length]) % 256;
        _x = rc4->box[x];
        rc4->box[x] = rc4->box[i];
        rc4->box[i] = _x;
    }

    /* updates the rc4 pointer value with the created
    structure (value return) */
    *rc4_pointer = rc4;
}

void delete_rc4(struct rc4_t *rc4) {
    /* releases the memory associated with
    the rc4 structure (avoids memory leaks) */
    FREE(rc4);
}

void cipher_rc4(struct rc4_t *rc4, char *data, size_t size, char *data_c) {
    /* allocates the space for the various variables
    to be used in this encoding "round" */
    size_t i;
    unsigned char _x;
    unsigned char byte;
    unsigned char k;
    unsigned char ciph;

    /* iterates over the size of the data to be encrypted
    each of the iterations is considered a round of rc4 */
    for(i = 0; i < size; i++) {
        /* retrieves the current byte in iteration for
        encryption */
        byte = data[i];

        rc4->x = (rc4->x + 1) % 256;
        rc4->y = (rc4->y + rc4->box[rc4->x]) % 256;

        _x = rc4->box[rc4->x];
        rc4->box[rc4->x] = rc4->box[rc4->y];
        rc4->box[rc4->y] = _x;

        k = rc4->box[(rc4->box[rc4->x] + rc4->box[rc4->y]) % 256];

        /* runs the xor operation fot the current byte and the
        pseudo randomly generated key byte, then sets the result
        in the associated encrypted data buffer index */
        ciph = byte ^ k;
        data_c[i] = ciph;
    }
}

void crypt_rc4(char *message, char *key, char **result_pointer) {
    /* allocates space for both the resulting buffer
    pointer and for the rc4 structure to be used */
    char *result;
    struct rc4_t *rc4;
    size_t message_length = strlen(message);

    /* allocates the required space for the resulting
    buffer (the same as the message) and then creates the
    rc4 structure to be used and run the encryption function
    for the message range, then delete the rc4 structure */
    result = (char *) MALLOC(message_length);
    create_rc4(key, &rc4);
    cipher_rc4(rc4, message, message_length, result);
    delete_rc4(rc4);

    /* sets the resulting buffer pointer as the result
    of the current function indirect setting */
    *result_pointer = result;
}
