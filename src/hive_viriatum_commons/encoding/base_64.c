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

#include "stdafx.h"

#include "base_64.h"

const char base64_characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

ERROR_CODE encode_base64(unsigned char *buffer, size_t buffer_length, unsigned char **encoded_buffer_pointer, size_t *encoded_buffer_length_pointer) {
    /* allocates the encoded buffer, and assigns the encoded buffer length */
    _allocate_encoded_buffer(buffer_length, encoded_buffer_pointer, encoded_buffer_length_pointer);

    /* encodes the buffer into base 64 */
    _encode_base64(buffer, buffer_length, *encoded_buffer_pointer, *encoded_buffer_length_pointer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_base64(unsigned char *encoded_buffer, size_t encoded_buffer_length, unsigned char **decoded_buffer_pointer, size_t *decoded_buffer_length_pointer) {
    /* retrieves the padding count from the encoded buffer */
    unsigned int padding_count = _get_padding_count(encoded_buffer, encoded_buffer_length);

    /* allocates the decoded buffer, and assigns the decoded buffer length */
    _allocate_decoded_buffer(encoded_buffer_length, decoded_buffer_pointer, decoded_buffer_length_pointer, padding_count);

    /* decodes the buffer from base 64 */
    _decode_base64(encoded_buffer, encoded_buffer_length, *decoded_buffer_pointer, *decoded_buffer_length_pointer, padding_count);

    /* raises no error */
    RAISE_NO_ERROR;
}

size_t calculate_encoded_buffer_length_base64(size_t buffer_length) {
    /* allocates the encoded buffer length */
    size_t encoded_buffer_length;

    /* allocates the padding count */
    size_t padding_count;

    /* calculates the padding count */
    padding_count = (buffer_length % 3) ? 3 - (buffer_length % 3) : 0;

    /* calculates the encoded buffer length */
    encoded_buffer_length = ((buffer_length + padding_count) * 4 / 3);

    /* returns the encoded buffer length */
    return encoded_buffer_length;
}

size_t calculate_decoded_buffer_length_base64(size_t encoded_buffer_length, size_t padding_count) {
    /* allocates the decoded buffer length */
    size_t decoded_buffer_length;

    /* calculates the decoded buffer length */
    decoded_buffer_length = (encoded_buffer_length / 4 * 3) - padding_count;

    /* returns the decoded buffer length */
    return decoded_buffer_length;
}

ERROR_CODE _encode_base64(unsigned char *buffer, size_t buffer_length, unsigned char *encoded_buffer, size_t encoded_buffer_length) {
    /* allocates space for the the encoded buffer index */
    size_t encoded_buffer_index;

    /* allocates space for the the index */
    size_t index;

    /* allocates space for the long number */
    unsigned long number = 0;

    /* allocates space for the partial numbers */
    unsigned char number0, number1, number2, number3;

    /* creates the pad count value */
    unsigned int pad_count = (unsigned int) buffer_length % 3;

    /* starts the encoded buffer index */
    encoded_buffer_index = 0;

    /* increment over the length of the buffer, three characters at a time */
    for(index = 0; index < buffer_length; index += 3) {
        /* starts creating the number with the first byte */
        number = buffer[index] << 16;

        /* in case there are two bytes (at least) available */
        if(index + 1 < buffer_length) {
            /* increments the number with the value of the second byte */
            number += buffer[index + 1] << 8;
        }

        /* in case there are three bytes (at least) available */
        if(index + 2 < buffer_length) {
            /* increments the number with the value of the third byte */
            number += buffer[index + 2];
        }

        /* separates the 24 bit number into four 6 bit numbers */
        number0 = (unsigned char) (number >> 18) & 63;
        number1 = (unsigned char) (number >> 12) & 63;
        number2 = (unsigned char) (number >> 6) & 63;
        number3 = (unsigned char) number & 63;

        /* sets the first two characters in the encoded buffer */
        encoded_buffer[encoded_buffer_index++] = base64_characters[number0];
        encoded_buffer[encoded_buffer_index++] = base64_characters[number1];

        /* in case there are two bytes (at least) available */
        if(index + 1 < buffer_length) {
            /* sets the third character in the encoded buffer */
            encoded_buffer[encoded_buffer_index++] = base64_characters[number2];
        }

        /* in case there are three bytes (at least) available */
        if(index + 2 < buffer_length) {
            /* sets the fourth character in the encoded buffer */
            encoded_buffer[encoded_buffer_index++] = base64_characters[number3];
        }
    }

    /* in case the padding count is valid */
    if(pad_count > 0) {
        /* iterates over the pad count */
        for(; pad_count < 3; pad_count++) {
            /* sets the padding character in the encoded buffer buffer */
            encoded_buffer[encoded_buffer_index++] = PADDING_BASE_64;
        }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _decode_base64(unsigned char *encoded_buffer, size_t encoded_buffer_length, unsigned char *buffer, size_t buffer_length, size_t padding_count) {
    /* allocates space for the the buffer index */
    size_t buffer_index;

    /* allocates space for the the index */
    size_t index;

    /* allocates space for the long number */
    unsigned long number = 0;

    /* allocates space for the partial numbers */
    unsigned char number0, number1, number2;

    /* calculates the valid buffer length */
    size_t valid_buffer_length = buffer_length - padding_count;

    /* starts the buffer index */
    buffer_index = 0;

    /* increments over the length of the encoded buffer, four characters at a time */
    for(index = 0; index < encoded_buffer_length; index += 4) {
        /* retrieves the number resulting from the concatenation of the 24 bits */
        number = (_lookup_fast_base64(encoded_buffer[index]) << 18) + (_lookup_fast_base64(encoded_buffer[index + 1]) << 12) +
                 (_lookup_fast_base64(encoded_buffer[index + 2]) << 6) + _lookup_fast_base64(encoded_buffer[index + 3]);

        /* retrieves the various bytes from the retrieved number */
        number0 = (unsigned char) (number >> 16) & 255;
        number1 = (unsigned char) (number >> 8) & 255;
        number2 = (unsigned char) number & 255;

        /* writes the first byte in the buffer */
        buffer[buffer_index++] = number0;

        /* in case the buffer index is still valid */
        if(buffer_index <= valid_buffer_length) {
            /* writes the second byte in the buffer */
            buffer[buffer_index++] = number1;
        }

        /* in case the buffer index is still valid */
        if(buffer_index <= valid_buffer_length) {
            /* writes the third byte in the buffer */
            buffer[buffer_index++] = number2;
        }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _allocate_encoded_buffer(size_t buffer_length, unsigned char **encoded_buffer_pointer, size_t *encoded_buffer_length_pointer) {
    /* allocates the encoded buffer length */
    *encoded_buffer_length_pointer = calculate_encoded_buffer_length_base64(buffer_length);

    /* allocates the encoded buffer */
    *encoded_buffer_pointer = (unsigned char *) MALLOC(*encoded_buffer_length_pointer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _allocate_decoded_buffer(size_t encoded_buffer_length, unsigned char **decoded_buffer_pointer, size_t *decoded_buffer_length_pointer, size_t padding_count) {
    /* allocates the decoded buffer length */
    *decoded_buffer_length_pointer = calculate_decoded_buffer_length_base64(encoded_buffer_length, padding_count);

    /* allocates the decoded buffer */
    *decoded_buffer_pointer = (unsigned char *) MALLOC(*decoded_buffer_length_pointer);

    /* raises no error */
    RAISE_NO_ERROR;
}

unsigned int _get_padding_count(unsigned char *encoded_buffer, size_t encoded_buffer_length) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current value */
    unsigned char current_value;

    /* allocates the padding count */
    int padding_count = 0;

    /* iterates over the encoded buffer */
    for(index = (unsigned int) encoded_buffer_length - 1; index > 0 ; index--) {
        /* retireve the current value */
        current_value = encoded_buffer[index];

        /* in case the current value is not a padding character */
        if(current_value != PADDING_BASE_64) {
            /* breaks the loop */
            break;
        }

        /* increments the padding count */
        padding_count++;
    }

    /* returns the padding count */
    return padding_count;
}

unsigned char _lookup_base64(unsigned char value) {
    /* allocates space for the index */
    unsigned char index;

    /* allocates space for the current value */
    unsigned char current_value;

    /* iterates over all the base 64 characters */
    for(index = 0; index < 64; index++) {
        /* retrieves the current value */
        current_value = base64_characters[index];

        /* in case the current value and the value match */
        if(current_value == value) {
            /* returns the index */
            return index;
        }
    }

    /* returns the default value */
    return 0;
}

unsigned char _lookup_fast_base64(unsigned char value) {
    /* in case it's a capital letter */
    if(value >= 'A' && value <= 'Z') {
        return value - 65;
    }
    /* in case it's a non capital letter */
    else if(value >= 'a' && value <= 'z') {
        return value - 71;
    }
    /* in case it's a number */
    else if(value >= '0' && value <= '9') {
        return value + 4;
    }
    /* in case it's the plus sign */
    else if(value == '+') {
        return 62;
    }
    /* in case it's the slash sign */
    else if(value == '/') {
        return 63;
    }

    /* returns the default value */
    return 0;
}
