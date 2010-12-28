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

int encodeBase64(unsigned char *data, size_t dataLength, unsigned char *result, size_t resultLength) {
    size_t resultIndex = 0;
    size_t x;
    unsigned long n = 0;
    int padCount = dataLength % 3;
    unsigned char n0, n1, n2, n3;

    /* increment over the length of the string, three characters at a time */
    for(x = 0; x < dataLength; x += 3) {
        /* these three 8-bit (ASCII) characters become one 24-bit number */
        n = data[x] << 16;

        if(x + 1 < dataLength) {
            n += data[x + 1] << 8;
        }

        if(x + 2 < dataLength) {
            n += data[x + 2];
        }

        /* separates the 24 bit number into four 6 bit numbers */
        n0 = (unsigned char) (n >> 18) & 63;
        n1 = (unsigned char) (n >> 12) & 63;
        n2 = (unsigned char) (n >> 6) & 63;
        n3 = (unsigned char) n & 63;

        /*
         * if we have one byte available, then its encoding is spread
         * out over two characters
         */
        if(resultIndex >= resultLength) {
            return 0;   /* indicate failure: buffer too small */
        }

        result[resultIndex++] = base64Characters[n0];

        if(resultIndex >= resultLength) {
            return 0;   /* indicate failure: buffer too small */
        }

        result[resultIndex++] = base64Characters[n1];

        /*
         * if we have only two bytes available, then their encoding is
         * spread out over three chars
         */
        if(x + 1 < dataLength) {
            if(resultIndex >= resultLength) {
                return 0;   /* indicate failure: buffer too small */
            }

            result[resultIndex++] = base64Characters[n2];
        }

        /*
         * if we have all three bytes available, then their encoding is spread
         * out over four characters
         */
        if(x + 2 < dataLength) {
            if(resultIndex >= resultLength) {
                return 0;   /* indicate failure: buffer too small */
            }

            result[resultIndex++] = base64Characters[n3];
        }
    }

	/*
	 * create and add padding that is required if we did not have a multiple of 3
     * number of characters available
     */
	if(padCount > 0) {
		for(; padCount < 3; padCount++) {
			if(resultIndex >= resultLength) {
				return 0;   /* indicate failure: buffer too small */
			}

			result[resultIndex++] = '=';
		}
	}

	if(resultIndex >= resultLength) {
		return 0;   /* indicate failure: buffer too small */
	}

	/* sets the final result value */
	result[resultIndex] = 0;

	/* returns one success */
	return 1;
}
