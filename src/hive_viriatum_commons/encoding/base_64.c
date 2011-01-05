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

const char base64Characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int encodeBase64(unsigned char *buffer, size_t bufferLength, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer) {
    /* allocates the encoded buffer, and assigns the encoded buffer length */
    _allocateEncodedBuffer(bufferLength, encodedBufferPointer, encodedBufferLengthPointer);

    /* encodes the buffer into base 64 */
    _encodeBase64(buffer, bufferLength, *encodedBufferPointer, *encodedBufferLengthPointer);

    /* returns valid */
    return 0;
}

size_t calculateEncodedBufferLengthBase64(size_t bufferLength) {
    /* allocates the encoded buffer length */
    size_t encodedBufferLength;

    /* calculates the encoded buffer length */
    encodedBufferLength = (bufferLength + 2 - ((bufferLength + 2) % 3)) / 3 * 4 ;

    /* returns the encoded buffer length */
    return encodedBufferLength;
}

int _encodeBase64(unsigned char *buffer, size_t bufferLength, unsigned char *encodedBuffer, size_t encodedBufferLength) {
    /* allocates space for the the encoded buffer index */
    size_t encodedBufferIndex;

    /* allocates space for the the index */
    size_t index;

    /* allocates space for the long number */
    unsigned long number = 0;

    /* allocates space for the partial numbers */
    unsigned char number0, number1, number2, number3;

    /* creates the pad count value */
    unsigned int padCount = bufferLength % 3;

    /* starts the encoded buffer index */
    encodedBufferIndex = 0;

    /* increment over the length of the buffer, three characters at a time */
    for(index = 0; index < bufferLength; index += 3) {
        /* starts creating the number with the first byte */
        number = buffer[index] << 16;

        /* in case there are two bytes (at least) available */
        if(index + 1 < bufferLength) {
            number += buffer[index + 1] << 8;
        }

        /* in case there are three bytes (at least) available */
        if(index + 2 < bufferLength) {
            number += buffer[index + 2];
        }

        /* separates the 24 bit number into four 6 bit numbers */
        number0 = (unsigned char) (number >> 18) & 63;
        number1 = (unsigned char) (number >> 12) & 63;
        number2 = (unsigned char) (number >> 6) & 63;
        number3 = (unsigned char) number & 63;

        /* sets the first two characters in the encoded buffer */
        encodedBuffer[encodedBufferIndex++] = base64Characters[number0];
        encodedBuffer[encodedBufferIndex++] = base64Characters[number1];

        /* in case there are two bytes (at least) available */
        if(index + 1 < bufferLength) {
            /* sets the third character in the encoded buffer */
            encodedBuffer[encodedBufferIndex++] = base64Characters[number2];
        }

        /* in case there are three bytes (at least) available */
        if(index + 2 < bufferLength) {
            /* sets the fourth character in the encoded buffer */
            encodedBuffer[encodedBufferIndex++] = base64Characters[number3];
        }
    }

    /* in case the padding count is valid */
    if(padCount > 0) {
        for(; padCount < 3; padCount++) {
            /* sets the padding character in the encoded buffer buffer */
            encodedBuffer[encodedBufferIndex++] = '=';
        }
    }

    /* returns one success */
    return 1;
}

int _decodeBase64(unsigned char *encodedBuffer, size_t encodedBufferLength, unsigned char *buffer, size_t bufferLength) {
    /* allocates space for the the buffer index */
    size_t bufferIndex;

    /* allocates space for the the index */
    size_t index;

    /* allocates space for the long number */
    unsigned long number = 0;

    /* allocates space for the partial numbers */
    unsigned char number0, number1, number2;

    /* creates the pad count value */
    unsigned int padCount = bufferLength % 3;

    /* starts the buffer index */
    bufferIndex = 0;

    /* increment over the length of the encoded buffer, four characters at a time */
    for(index = 0; index < encodedBufferLength; index += 4) {

        /* TENHO DE FAZER O MAPEAMENTE INVERSO DO MAPARA DE CARACTERES */

        number = (encodedBuffer[index] << 18) + (encodedBuffer[index + 1] << 12) +
                 (encodedBuffer[index + 2] << 6) + encodedBuffer[index + 3];

        number0 = (unsigned char) (number >> 16) & 255;
        number1 = (unsigned char) (number >> 8) & 255;
        number2 = (unsigned char) number & 255;
    }

    /* returns one success */
    return 1;
}

int _allocateEncodedBuffer(size_t bufferLength, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer) {
    /* allocates the encoded buffer length */
    *encodedBufferLengthPointer = calculateEncodedBufferLengthBase64(bufferLength);

    /* allocates the encoded buffer */
    *encodedBufferPointer = (unsigned char *) malloc(*encodedBufferLengthPointer);

    /* returns valid */
    return 0;
}
