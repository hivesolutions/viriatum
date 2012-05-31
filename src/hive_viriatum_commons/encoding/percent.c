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

#include "percent.h"

ERROR_CODE encodePercent(unsigned char *buffer, size_t length, unsigned char **bufferPointer, size_t *lengthPointer) {
	/* allocates space for the byte value to be read in
	each iteration and allocates space for the index accumulator
	value for the iteration process */
    unsigned char byte;
	size_t index;

	/* allocates space for the buffer to hold the encoded
	string (assumes worst case for size calculation) and
	then sets this buffer as the current buffer pointer */
    unsigned char *_buffer = MALLOC(length * 3 + 1);
    unsigned char *__buffer = _buffer;
    
	/* iterates over the range of the provided buffer length
	in order to encoded the string parts */
    for(index = 0; index < length; index++) {
		/* retrieves the current iteration byte from the buffer,
		this is the byte to be encoded */
        byte = buffer[index];

		/* in case the current byte is valid url value (no need
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
            *__buffer++ = toHex(byte >> 4);
            *__buffer++ = toHex(byte & 15);
        }
    }

	/* closes the encoded buffer with the null value */
    *__buffer = '\0';

	/* sets the current encoded buffer in the buffer pointer value
	and sets the apropriate length inthe length pointer value */
	*bufferPointer = _buffer;
    *lengthPointer = __buffer - _buffer;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decodePercent(unsigned char *buffer, size_t length, unsigned char **bufferPointer,  size_t *lengthPointer) {
	/* allocates the pointer to be used durring the iteration
	process for the url decoding */
    unsigned char *pointer;

	/* allocates space for the buffer to hold the encoded
	string (assumes worst case for size calculation) and
	then sets this buffer as the current buffer pointer */
    unsigned char *_bufffer = MALLOC(length + 1);
    unsigned char *__buffer = _bufffer;

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
                *__buffer++ = fromHex(pointer[1]) << 4 | fromHex(pointer[2]);
                pointer += 2;
			} else {
				/* raises an error */
				RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem decoding url string");
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

	/* sets the current encoded buffer in the buffer pointer value
	and sets the apropriate length inthe length pointer value */
	*bufferPointer = _bufffer;
    *lengthPointer = __buffer - _bufffer;

    /* raises no error */
    RAISE_NO_ERROR;
}


ERROR_CODE urlEncode(unsigned char *buffer, size_t length, unsigned char **bufferPointer, size_t *lengthPointer) {
	return encodePercent(buffer, length, bufferPointer, lengthPointer);
}

ERROR_CODE urlDecode(unsigned char *buffer, size_t length, unsigned char **bufferPointer, size_t *lengthPointer) {
	return decodePercent(buffer, length, bufferPointer, lengthPointer);
}
