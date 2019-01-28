/*
 Hive Viriatum Commons
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "percent.h"

ERROR_CODE encode_percent(unsigned char *buffer, size_t length, unsigned char *_buffer, size_t *length_pointer) {
    /* allocates space for the byte value to be read in
    each iteration and allocates space for the index accumulator
    value for the iteration process */
    unsigned char byte;
    size_t index;

    /* creates a "backup" of the original buffer position
    provided by the call to be used in the final calculus
    of the size of the buffer */
    unsigned char *__buffer = _buffer;

    /* iterates over the range of the provided buffer length
    in order to encoded the string parts */
    for(index = 0; index < length; index++) {
        /* retrieves the current iteration byte from the buffer,
        this is the byte to be encoded */
        byte = buffer[index];

        /* in case the current byte is valid URL value (no need
        to encode it) sets the byte "directly" */
        if(isalphanum(byte) || byte == '-' || byte == '_' || byte == '.' || byte == '~') {
            /* sets the current byte (directly) in the buffer pointer */
            *__buffer++ = byte;
        }
        /* in case it's a space character it must be encoded as
        the plus character */
        else if(byte == ' ') {
            /* sets the plus character in the buffer pointer */
            *__buffer++ = '+';
        }
        /* otherwise it's a special character and must be encoded in
        the hexadecimal form */
        else  {
            /* sets the current buffer values with the hexadecimal
            encoded characters */
            *__buffer++ = '%';
            *__buffer++ = to_hex(byte >> 4);
            *__buffer++ = to_hex(byte & 15);
        }
    }

    /* closes the encoded buffer with the null value */
    *__buffer = '\0';

    /* calculates the length of the "populated" output buffer
    using the diference between the "original" buffer pointer
    and the current pointer */
    *length_pointer = __buffer - _buffer;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_percent(unsigned char *buffer, size_t length, unsigned char *_buffer,  size_t *length_pointer) {
    /* allocates the pointer to be used durring the iteration
    process for the URL decoding */
    unsigned char *pointer;

    /* creates a "backup" of the original buffer position
    provided by the call to be used in the final calculus
    of the size of the buffer */
    unsigned char *__buffer = _buffer;

    /* iterates over the range of the provided buffer length
    in order to decode the string parts */
    for(pointer = buffer; (size_t) (pointer - buffer) < length; pointer++) {
        /* in case the current pointer value refers a value
        that is the start character for a special value */
        if(*pointer == '%') {
            if(pointer[1] && pointer[2]) {
                /* converts the character using the hexadecimal value
                from it and then increments the current buffer pointer
                by a value of two (buffer advancing) */
                *__buffer++ = from_hex(pointer[1]) << 4 | from_hex(pointer[2]);
                pointer += 2;
            } else {
                /* raises an error */
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem decoding URL string");
            }
        }
        /* in case the current pointer value refers the plus
        value refering a space character */
        else if (*pointer == '+') {
            /* sets the space chracter in the buffer */
            *__buffer++ = ' ';
        }
        /* otherwise the character is considered to be "normal"
        and is writen as it is */
        else {
            /* sets the current pointer value "directly" in the
            buffer (direct value assignment) */
            *__buffer++ = *pointer;
        }
    }

    /* closes the decoded buffer with the null value */
    *__buffer = '\0';

    /* calculates the length of the "populated" output buffer
    using the diference between the "original" buffer pointer
    and the current pointer */
    *length_pointer = __buffer - _buffer;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_encode(unsigned char *buffer, size_t length, unsigned char **buffer_pointer, size_t *length_pointer) {
    unsigned char *_buffer = MALLOC(length * 3 + 1);
    *buffer_pointer = _buffer;
    return encode_percent(buffer, length, _buffer, length_pointer);
}

ERROR_CODE url_decode(unsigned char *buffer, size_t length, unsigned char **buffer_pointer, size_t *length_pointer) {
    unsigned char *_buffer = MALLOC(length + 1);
    *buffer_pointer = _buffer;
    return decode_percent(buffer, length, _buffer, length_pointer);
}
