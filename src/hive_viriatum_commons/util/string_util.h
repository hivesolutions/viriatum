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

static __inline unsigned char *base_string_value(unsigned char *string_value) {
    /* allocates the index counter */
    unsigned int index;

    /* allocates the current character value */
    unsigned int current_character;

    /* retrieves the string value length */
    unsigned int string_value_length = (unsigned int) strlen((char *) string_value);

    /* iterates from back throught the string value */
    for(index = string_value_length; index > 0; index--) {
        /* retrieves the current character */
        current_character = string_value[index];

        /* in case the current character is a slash or a back-slash */
        if(current_character == '\\' || current_character == '/') {
            /* increments the index in order to set the apropriate
            pointer value (need to point to the next chracter after the slash) */
            index += 1;

            /* breaks the loop */
            break;
        }
    }

    /* returns the "new" string value pointer */
    return string_value + index;
}

static __inline int ends_with_string(unsigned char *string_value, unsigned char *end_value) {
    /* allocates space for the index value to be used in iteration */
    size_t index;

    /* retrieves the string size for both the (base) string
    value and the end value to be verified */
    size_t string_value_size = strlen((char *) string_value);
    size_t end_value_size = strlen((char *) end_value);

    /* in case the string to compare the end value is bigger
    than the base string value to be verified (impossible to
    end with the string) */
    if(end_value_size > string_value_size) {
        /* returns immediately false */
        return 0;
    }

    /* iterates over the end value string to compare
    the various character values */
    for(index = 0; index < end_value_size; index++) {
        /* in case the current end values does not match the equivalent
        value in the base string value */
        if(end_value[index] != string_value[string_value_size - end_value_size + index]) {
            /* returns in error (failed match) */
            return 0;
        }
    }

    /* return in success (complete match) */
    return 1;
}

static __inline void uppercase(char *string_value) {
    while(*string_value != '\0') {
        if(*string_value == '-') { *string_value = '_'; }
        else { *string_value = toupper((unsigned char) *string_value); }
        string_value++;
    }
}

static __inline void normalize_path(char *string_value) {
    while(*string_value != '\0') {
        if(*string_value == '\\') { *string_value = '/'; }
        string_value++;
    }
}

static __inline void split(char *string_value, char *buffer, size_t size_e, char token) {
    /* allocates space for the temporary token to be used to
    store the current token in iteration */
    char _token;

    /* allocates space fo the size count the index counter
    and the auxiliary (start) index counter */
    size_t size;
    size_t index = 0;
    size_t _index = 0;

    /* iterates continuousuly over the string value to correctly
    parse the string value and split it */
    while(1) {
        /* retrieves the current token character to be evaluated
        for conditional expression execution */
        _token = string_value[index];

        /* in case the current token matches the one to be used
        as split, must split the string */
        if(_token == token) {
            /* calculates the size of the string from the difference
            of index and then copies the string from one memory
            buffer to the other over the calculated size */
            size = index - _index;
            memcpy(buffer, &string_value[_index], size);
            buffer[size] = '\0';

            /* increments the buffer position with the size of one
            buffer element and then re-sets the auxiliary buffer value */
            buffer += size_e;
            _index = index + 1;
        }
        /* otherwise in case the current token isthe end of string
        the current string must be copied as the last trunk */
        else if(_token == '\0') {
            /* calculates the size of the string from the difference
            of index and then copies the string from one memory
            buffer to the other over the calculated size */
            size = index - _index;
            memcpy(buffer, &string_value[_index], size);
            buffer[size] = '\0';

            /* breaks the current loop (no more tokens to be split) */
            break;
        }

        /* increments the current index value by one (buffer
        iteration increment) */
        index++;
    }
}
