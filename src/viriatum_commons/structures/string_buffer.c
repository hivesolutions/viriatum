/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "string_buffer.h"

void create_string_buffer(struct string_buffer_t **string_buffer_pointer) {
    /* retrieves the string buffer size */
    size_t string_buffer_size = sizeof(struct string_buffer_t);

    /* allocates space for the string buffer */
    struct string_buffer_t *string_buffer = (struct string_buffer_t *) MALLOC(string_buffer_size);

    /* sets the default values in the string buffer */
    string_buffer->string_length = 0;
    string_buffer->count = 0;
    string_buffer->capacity = DEFAULT_STRING_BUFFER_SIZE;

    /* allocates the buffers to hold the various strings and
    their lengths for the string buffer runtime, sized for a
    few of them to begin with and grown as they fill up */
    string_buffer->strings = (unsigned char **) MALLOC(sizeof(unsigned char *) * string_buffer->capacity);
    string_buffer->lengths = (size_t *) MALLOC(sizeof(size_t) * string_buffer->capacity);

    /* creates the list to hold the various strings
    to have the memory released upon destruction */
    create_linked_list(&string_buffer->release_list);

    /* sets the string buffer in the string buffer pointer */
    *string_buffer_pointer = string_buffer;
}

void delete_string_buffer(struct string_buffer_t *string_buffer) {
    /* allocates space for the temporary string value */
    unsigned char *string_value;

    /* iterates continuously for release list
    cleanup (string value memory release) */
    while(TRUE) {
        /* pops a node from the release list */
        pop_value_linked_list(string_buffer->release_list, (void **) &string_value, 1);

        /* in case the value is invalid (empty list)
        need to break the cycle */
        if(string_value == NULL) { break; }

        /* deletes the string value */
        FREE(string_value);
    }

    /* deletes the list of release strings from the string buffer */
    delete_linked_list(string_buffer->release_list);

    /* releases the buffers of lengths and of strings from the
    string buffer, the strings themselves belong to whoever added
    them unless they were handed over for release above */
    FREE(string_buffer->lengths);
    FREE(string_buffer->strings);

    /* releases the string buffer */
    FREE(string_buffer);
}

void append_string_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value) {
    /* retrieves the length of the string value */
    size_t string_value_length = strlen((char *) string_value);

    /* adds the string value to the string buffer, using
    the length oriented append function */
    append_string_l_buffer(string_buffer, string_value, string_value_length);
}

void append_string_l_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value, size_t string_length) {
    /* in case the buffers are full they are grown to twice their
    size, the strings of a page are many and the growing of the
    buffers a handful of times costs a fraction of what a node of
    a list for every one of the strings did */
    if(string_buffer->count == string_buffer->capacity) {
        string_buffer->capacity *= 2;
        string_buffer->strings = (unsigned char **) REALLOC(
            (void *) string_buffer->strings,
            sizeof(unsigned char *) * string_buffer->capacity
        );
        string_buffer->lengths = (size_t *) REALLOC(
            (void *) string_buffer->lengths,
            sizeof(size_t) * string_buffer->capacity
        );
    }

    /* adds the string value and its length to the buffers and then
    increments the (total) string length with the length of the
    current string value */
    string_buffer->strings[string_buffer->count] = string_value;
    string_buffer->lengths[string_buffer->count] = string_length;
    string_buffer->count++;
    string_buffer->string_length += string_length;
}

void append_string_t_buffer(struct string_buffer_t *string_buffer, struct string_t *string) {
    /* adds the string structure value to the string buffer,
    using the length oriented append function */
    append_string_l_buffer(string_buffer, string->buffer, string->length);
}

void join_string_buffer(struct string_buffer_t *string_buffer, unsigned char **string_value_pointer) {
    /* allocates space for the index used for percolating
    the various (partial) string values */
    size_t index;

    /* allocates space for the pointer and allocates the buffer to hold
    the complete "joined" string value */
    unsigned char *pointer;
    unsigned char *string_value = (unsigned char *) MALLOC(string_buffer->string_length + 1);

    /* sets the "initial" pointer value to the "base" string value position */
    pointer = string_value;

    /* iterates over the various "partial" string values to
    "join" them into a single buffer */
    for(index = 0; index < string_buffer->count; index++) {
        /* copies the contents of the partial value to the buffer
        "pointed" by pointer, using the length stored beside it */
        memcpy(pointer, string_buffer->strings[index], string_buffer->lengths[index]);

        /* updates the pointer value with the length of the
        partial (string) value */
        pointer += string_buffer->lengths[index];
    }

    /* "closes" the string value, useful for usage as a "classic"
    null terminated string */
    string_value[string_buffer->string_length] = '\0';

    /* sets the string value in the value "pointed" by
    the string value pointer */
    *string_value_pointer = string_value;
}

void _append_string_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value) {
    /* adds the string value to the list of strings to have
    the memory released upon string buffer release */
    append_value_linked_list(string_buffer->release_list, string_value);

    /* adds the string value to the string buffer */
    append_string_buffer(string_buffer, string_value);
}

void _append_string_l_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value, size_t string_length) {
    /* adds the string value to the list of strings to have
    the memory released upon string buffer release */
    append_value_linked_list(string_buffer->release_list, string_value);

    /* adds the string (with length) value to the string buffer */
    append_string_l_buffer(string_buffer, string_value, string_length);
}

void _append_string_t_buffer(struct string_buffer_t *string_buffer, struct string_t *string) {
    /* adds the string structure value to the string buffer,
    using the length oriented append function */
    _append_string_l_buffer(string_buffer, string->buffer, string->length);
}
