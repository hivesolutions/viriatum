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

#pragma once

static __inline unsigned char *baseStringValue(unsigned char *stringValue) {
    /* allocates the index counter */
    unsigned int index;

    /* allocates the current character value */
    unsigned int currentCharacter;

    /* retrieves the string value length */
    unsigned int stringValueLength = strlen(stringValue);

    /* iterates from back throught the string value */
    for(index = stringValueLength; index > 0; index--) {
        /* retrieves the current character */
        currentCharacter = stringValue[index];

        /* in case the current character is a slash or a back-slash */
        if(currentCharacter == '\\' || currentCharacter == '/') {
            /* increments the index in order to set the apropriate
            pointer value (need to point to the next chracter after the slash) */
            index += 1;

            /* breaks the loop */
            break;
        }
    }

    /* returns the "new" string value pointer */
    return stringValue + index;
}
