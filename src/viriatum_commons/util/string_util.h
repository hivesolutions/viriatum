/*
 Hive Viriatum Commons
 Copyright (c) 2008-2018 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
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

static __inline void lowercase(char *string_value) {
    while(*string_value != '\0') {
        if(*string_value == '_') { *string_value = '-'; }
        else { *string_value = tolower((unsigned char) *string_value); }
        string_value++;
    }
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

static __inline char *extension_path(char *string_value) {
    size_t index = strlen(string_value) - 1;
    while(string_value[index] != '.' && index != 0) { index--; }
    return &string_value[index];
}

static __inline char atob(char *string_value) {
    const char *_string_value;
    size_t index = 0;

    while(TRUE) {
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

    while(TRUE) {
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

    while(TRUE) {
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

    while(TRUE) {
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
    while(TRUE) {
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

    valid = TRUE;
    value = delta % 60;
    if(valid) {
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
    char *format = "";
    size_t value = 0;
    size_t value_i = 0;
    double value_f = 0.0f;
    size_t count = 0;
    char valid = FALSE;
    char is_float = TRUE;

    value = (size_t) ROUND((double) bytes / 1073741824.0f);
    if(value > 0) {
        if(value == 1) { format = "%.1f GByte"; }
        else if(value < 10) { format = "%.1f GBytes"; }
        else { format = "%d GBytes"; is_float = FALSE; }

        value_i = value;
        if(is_float) { value_f = (double) bytes / 1073741824.0f; }
        valid = TRUE;
    }

    value = (size_t) ROUND((double) bytes / 1048576.0f);
    if(!valid && value > 0) {
        if(value == 1) { format = "%.1f MByte"; }
        else if(value < 10) { format = "%.1f MBytes"; }
        else { format = "%d MBytes"; is_float = FALSE; }

        value_i = value;
        if(is_float) { value_f = (double) bytes / 1048576.0f; }
        valid = TRUE;
    }

    value = (size_t) ROUND((double) bytes / 1024.0f);
    if(!valid && value > 0) {
        if(value == 1) { format = "%.1f KByte"; }
        else if(value < 10) { format = "%.1f KBytes"; }
        else { format = "%d KBytes"; is_float = FALSE; }

        value_i = value;
        if(is_float) { value_f = (double) bytes / 1024.0f; }
        valid = TRUE;
    }

    value = bytes;
    if(!valid) {
        if(value == 1) { format = "%.1f Byte"; }
        else if(value < 10) { format = "%.1f Bytes"; }
        else { format = "%d Bytes"; is_float = FALSE; }

        value_i = value;
        if(is_float) { value_f = (double) bytes; }
        valid = TRUE;
    }

    if(!valid) { return count; }

    if(is_float) {
        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value_f
        );
    } else {
        count += SPRINTF(
            &buffer[count],
            size,
            format,
            value_i
        );
    }

    return count;
}
