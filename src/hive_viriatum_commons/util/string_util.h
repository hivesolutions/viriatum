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

#include "../system/system.h"

#define TRIM_STRING(string_buffer) &string_buffer[1]

/**
 * Group of string values to be used as
 * "valid" (evaluating to true) in the
 * ascii to boolean converter (atob).
 */
static const char *atob_strings[4] = {
    "On",
    "True",
    "1",
    NULL
};

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

static __inline unsigned char *trim(unsigned char *string_value) {
    size_t length = strlen((char *) string_value);
    if(length < 3) { return string_value; }
    string_value[length - 1] = '\0';
    return &string_value[1];
}

static __inline unsigned char *untrim(unsigned char *string_value, char last) {
    size_t length = strlen((char *) string_value);
    if(length < 1) { return string_value; }
    string_value[length] = last;
    return string_value - 1;
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

static __inline char atob(char *string_value) {
    const char *_string_value;
    size_t index = 0;

    while(1) {
        _string_value = atob_strings[index];
        if(_string_value == NULL) { break; }
        if(strcmp(_string_value, string_value) == 0) { return TRUE; }
        index++;
    }

    return FALSE;
}

static __inline size_t atoa(char *string_value, char *buffer, size_t elements, size_t element_size) {
    size_t size;
    size_t count = 0;
    size_t index = 0;
    char *_base_value = string_value;

    while(1) {
        switch(*string_value) {
            case ' ':
            case '\0':
                size = string_value - _base_value;
                if(size + 1 > element_size) { return 0; }
                if(count + 1 > elements) { return 0; }

                memcpy(&buffer[index], _base_value, size);
                buffer[index + size] = '\0';

                count += element_size;
                _base_value = string_value + 1;
                count++;

                break;

            default:
                break;
        }

        if(*string_value == '\0') { break; }

        string_value++;
    }

    return count;
}

static __inline size_t trailing_size(char *buffer, size_t size) {
    char byte;
    char *_pointer = buffer + size - 1;

    while(1) {
        byte = *_pointer;
        if(byte != ' ') { break; }
        _pointer--;
        size--;
        if(size == 0) { break; }
    }

    return size;
}

static __inline size_t leading_offset(char *buffer, size_t size) {
    char byte;
    size_t _size = 0;
    char *_pointer = buffer;

    while(1) {
        byte = *_pointer;
        if(byte != ' ') { break; }
        _pointer++;
        _size++;
        if(_size == size) { break; }
    }

    return _size;
}

static __inline size_t split(char *string_value, char *buffer, size_t size_e, char token) {
    /* allocates space for the temporary token to be used to
    store the current token in iteration */
    char _token;

    /* allocates space fo the size count the index counter
    and the auxiliary (start) index counter */
    size_t size;
    size_t index = 0;
    size_t _index = 0;
    size_t count = 0;

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

            /* increments the count (number of parsed elements)
            because one more split occured */
            count++;
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

            /* increments the count (number of parsed elements)
            because one more split occured (final split) */
            count++;

            /* breaks the current loop (no more tokens to be split) */
            break;
        }

        /* increments the current index value by one (buffer
        iteration increment) */
        index++;
    }

    /* returns the count value, number of parsed elements
    from the provided string */
    return count;
}

static __inline size_t format_delta(char *buffer, size_t size, unsigned long long delta, size_t counts) {
    char *format;
    unsigned long long value;
    size_t count = 0;
    size_t counter = 0;
    char valid = FALSE;

    value = delta / 86400;
    if(value > 0) {
        if(value == 1) { format = "%d day "; }
        else { format = "%d days "; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        counter++;
        valid = TRUE;
    }

    if(counter == counts) {
        buffer[count - 1] = '\0';
        return count;
    }

    value = (delta % 86400) / 3600;
    if(valid || value > 0) {
        if(value == 1) { format = "%d hour "; }
        else { format = "%d hours "; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        counter++;
        valid = TRUE;
    }

    if(counter == counts) {
        buffer[count - 1] = '\0';
        return count;
    }

    value = (delta % 3600) / 60;
    if(valid || value > 0) {
        if(value == 1) { format = "%d minute "; }
        else { format = "%d minutes "; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        counter++;
        valid = TRUE;
    }

    if(counter == counts) {
        buffer[count - 1] = '\0';
        return count;
    }

    value = delta % 60;
    if(valid || value >= 0) {
        if(value == 1) { format = "%d second "; }
        else { format = "%d seconds "; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        counter++;
        valid = TRUE;
    }

    return count;
}

static __inline size_t format_bytes(char *buffer, size_t size, size_t bytes) {
    char *format;
    size_t count = 0;
    size_t value;

    value = bytes / 1073741824;
    if(value > 0) {
        if(value == 1) { format = "%d GByte"; }
        else { format = "%d GBytes"; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        return count;
    }

    value = bytes / 1048576;
    if(value > 0) {
        if(value == 1) { format = "%d MByte"; }
        else { format = "%d MBytes"; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        return count;
    }

    value = bytes / 1024;
    if(value > 0) {
        if(value == 1) { format = "%d KByte"; }
        else { format = "%d KBytes"; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        return count;
    }

    value = bytes;
    if(value >= 0) {
        if(value == 1) { format = "%d Byte"; }
        else { format = "%d Bytes"; }

        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value
        );
        return count;
    }

    return count;
}
