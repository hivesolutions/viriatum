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

int decodeBase64(unsigned char *encodedBuffer, size_t encodedBufferLength, unsigned char **decodedBufferPointer, size_t *decodedBufferLengthPointer) {
    /* retrieves the padding count from the encoded buffer */
    unsigned int paddingCount = _getPaddingCount(encodedBuffer, encodedBufferLength);

    /* allocates the decoded buffer, and assigns the decoded buffer length */
    _allocateDecodedBuffer(encodedBufferLength, decodedBufferPointer, decodedBufferLengthPointer, paddingCount);

    /* decodes the buffer from base 64 */
    _decodeBase64(encodedBuffer, encodedBufferLength, *decodedBufferPointer, *decodedBufferLengthPointer, paddingCount);

    /* returns valid */
    return 0;
}

size_t calculateEncodedBufferLengthBase64(size_t bufferLength) {
    /* allocates the encoded buffer length */
    size_t encodedBufferLength;

    /* allocates the padding count */
    size_t paddingCount;

    /* calculates the padding count */
    paddingCount = (bufferLength % 3) ? 3 - (bufferLength % 3) : 0;

    /* calculates the encoded buffer length */
    encodedBufferLength = ((bufferLength + paddingCount) * 4 / 3);

    /* returns the encoded buffer length */
    return encodedBufferLength;
}

size_t calculateDecodedBufferLenghtBase64(size_t encodedBufferLength, size_t paddingCount) {
    /* allocates the decoded buffer length */
    size_t decodedBufferLength;

    /* calculates the decoded buffer length */
    decodedBufferLength = (encodedBufferLength / 4 * 3) - paddingCount;

    /* returns the decoded buffer length */
    return decodedBufferLength;
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
    unsigned int padCount = (unsigned int) bufferLength % 3;

    /* starts the encoded buffer index */
    encodedBufferIndex = 0;

    /* increment over the length of the buffer, three characters at a time */
    for(index = 0; index < bufferLength; index += 3) {
        /* starts creating the number with the first byte */
        number = buffer[index] << 16;

        /* in case there are two bytes (at least) available */
        if(index + 1 < bufferLength) {
            /* increments the number with the value of the second byte */
            number += buffer[index + 1] << 8;
        }

        /* in case there are three bytes (at least) available */
        if(index + 2 < bufferLength) {
            /* increments the number with the value of the third byte */
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
        /* iterates over the pad count */
        for(; padCount < 3; padCount++) {
            /* sets the padding character in the encoded buffer buffer */
            encodedBuffer[encodedBufferIndex++] = PADDING_BASE_64;
        }
    }

    /* returns one success */
    return 1;
}

int _decodeBase64(unsigned char *encodedBuffer, size_t encodedBufferLength, unsigned char *buffer, size_t bufferLength, size_t paddingCount) {
    /* allocates space for the the buffer index */
    size_t bufferIndex;

    /* allocates space for the the index */
    size_t index;

    /* allocates space for the long number */
    unsigned long number = 0;

    /* allocates space for the partial numbers */
    unsigned char number0, number1, number2;

    /* calculates the valid buffer length */
    size_t validBufferLength = bufferLength - paddingCount;

    /* starts the buffer index */
    bufferIndex = 0;

    /* increments over the length of the encoded buffer, four characters at a time */
    for(index = 0; index < encodedBufferLength; index += 4) {
        /* retrieves the number resulting from the concatenation of the 24 bits */
        number = (_lookupFastBase64(encodedBuffer[index]) << 18) + (_lookupFastBase64(encodedBuffer[index + 1]) << 12) +
                 (_lookupFastBase64(encodedBuffer[index + 2]) << 6) + _lookupFastBase64(encodedBuffer[index + 3]);

        /* retrieves the various bytes from the retrieved number */
        number0 = (unsigned char) (number >> 16) & 255;
        number1 = (unsigned char) (number >> 8) & 255;
        number2 = (unsigned char) number & 255;

        /* writes the first byte in the buffer */
        buffer[bufferIndex++] = number0;

        /* in case the buffer index is still valid */
        if(bufferIndex <= validBufferLength) {
            /* writes the second byte in the buffer */
            buffer[bufferIndex++] = number1;
        }

        /* in case the buffer index is still valid */
        if(bufferIndex <= validBufferLength) {
            /* writes the third byte in the buffer */
            buffer[bufferIndex++] = number2;
        }
    }

    /* returns one success */
    return 1;
}

int _allocateEncodedBuffer(size_t bufferLength, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer) {
    /* allocates the encoded buffer length */
    *encodedBufferLengthPointer = calculateEncodedBufferLengthBase64(bufferLength);

    /* allocates the encoded buffer */
    *encodedBufferPointer = (unsigned char *) MALLOC(*encodedBufferLengthPointer);

    /* returns valid */
    return 0;
}

int _allocateDecodedBuffer(size_t encodedBufferLength, unsigned char **decodedBufferPointer, size_t *decodedBufferLengthPointer, size_t paddingCount) {
    /* allocates the decoded buffer length */
    *decodedBufferLengthPointer = calculateDecodedBufferLenghtBase64(encodedBufferLength, paddingCount);

    /* allocates the decoded buffer */
    *decodedBufferPointer = (unsigned char *) MALLOC(*decodedBufferLengthPointer);

    /* returns valid */
    return 0;
}

unsigned int _getPaddingCount(unsigned char *encodedBuffer, size_t encodedBufferLength) {
    /* allocates the index */
    unsigned int index;

    /* allocates the current value */
    unsigned char currentValue;

    /* allocates the padding count */
    int paddingCount = 0;

    /* iterates over the encoded buffer */
    for(index = (unsigned int) encodedBufferLength - 1; index > 0 ; index--) {
        /* retireve the current value */
        currentValue = encodedBuffer[index];

        /* in case the current value is not a padding character */
        if(currentValue != PADDING_BASE_64) {
            /* breaks the loop */
            break;
        }

        /* increments the padding count */
        paddingCount++;
    }

    /* returns the padding count */
    return paddingCount;
}

unsigned char _lookupBase64(unsigned char value) {
    /* allocates space for the index */
    unsigned char index;

    /* allocates space for the current value */
    unsigned char currentValue;

    /* iterates over all the base 64 characters */
    for(index = 0; index < 64; index++) {
        /* retrieves the current value */
        currentValue = base64Characters[index];

        /* in case the current value and the value match */
        if(currentValue == value) {
            /* returns the index */
            return index;
        }
    }

    /* returns the default value */
    return 0;
}

unsigned char _lookupFastBase64(unsigned char value) {
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
