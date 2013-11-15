/*
 Hive Viriatum Commons
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

static __inline unsigned char *base_string_value_s(unsigned char *string_value) {
    /* allicates the index counter the value that is going
    to store the current item in iteration and the length
    of the string from which the base name will be retrieved */
    unsigned int index;
    unsigned int current_character;
    unsigned int string_value_length = (unsigned int) strlen((char *) string_value);

    /* iterates from back throught the string value to try to find
    the first slash separator inside the string */
    for(index = string_value_length; index > 0; index--) {
        /* retrieves the current character and in cas the value
        is either a slash or a back slash breaks the loop (target found) */
        current_character = string_value[index];
        if(current_character == '\\' || current_character == '/') {
            /* increments the index in order to set the apropriate
            pointer value (need to point to the next chracter after
            the slash) and then breaks the loop */
            index += 1;
            break;
        }
    }

    /* returns the "new" string value pointer, that should be
    pointing to the base string value */
    return string_value + index;
}
