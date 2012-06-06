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

#include "string_buffer.h"

void create_string_buffer(struct string_buffer_t **string_buffer_pointer) {
    /* retrieves the string buffer size */
    size_t string_buffer_size = sizeof(struct string_buffer_t);

    /* allocates space for the string buffer */
    struct string_buffer_t *string_buffer = (struct string_buffer_t *) MALLOC(string_buffer_size);

    /* sets the default values in the string buffer */
    string_buffer->string_length = 0;

    /* creates the list to hold the various strings
    for the string buffer runtime */
    create_linked_list(&string_buffer->string_list);

    /* creates the list to hold the various strings
    lengths for the string in the string buffer runtime */
    create_linked_list(&string_buffer->length_list);

    /* creates the list to hold the various strings
    to have the memory released uppon destruction */
    create_linked_list(&string_buffer->release_list);

    /* sets the string buffer in the string buffer pointer */
    *string_buffer_pointer = string_buffer;
}

void delete_string_buffer(struct string_buffer_t *string_buffer) {
    /* allocates space for the temporary string value */
    unsigned char *string_value;

    /* iterates continuously for release list
    cleanup (string value memory release) */
    while(1) {
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

    /* deletes the list of lengths from the string buffer */
    delete_linked_list(string_buffer->length_list);

    /* deletes the list of strings from the string buffer */
    delete_linked_list(string_buffer->string_list);

    /* releases the string buffer */
    FREE(string_buffer);
}

void append_string_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value) {
    /* retrieves the length of the string value */
    size_t string_value_length = strlen((char *) string_value);

    /* adds the string value to the list of strings and then
    increments the (total) string length with the length of
    the current string value */
    append_value_linked_list(string_buffer->string_list, (void *) string_value);
    append_value_linked_list(string_buffer->length_list, (void *) string_value_length);
    string_buffer->string_length += string_value_length;
}

void append_string_l_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value, size_t string_length) {
    /* adds the string value to the list of strings and then
    increments the (total) string length with the length of
    the current string value */
    append_value_linked_list(string_buffer->string_list, (void *) string_value);
    append_value_linked_list(string_buffer->length_list, (void *) string_length);
    string_buffer->string_length += string_length;
}

void append_string_t_buffer(struct string_buffer_t *string_buffer, struct string_t *string) {
    /* adds the string structure value to the string buffer,
    using the length oriented append function */
    append_string_l_buffer(string_buffer, string->buffer, string->length);
}

void join_string_buffer(struct string_buffer_t *string_buffer, unsigned char **string_value_pointer) {
    /* allocates space for the iterators used for percolating
    the various (partial) string values (include string length) */
    struct iterator_t *string_iterator;
    struct iterator_t *length_iterator;

    /* allocates space for the partial value and the partial value length */
    unsigned char *partial_value;
    size_t partial_value_length;

    /* allocates space for the pointer and allocates the buffer to hold
    the complete "joined" string value */
    unsigned char *pointer;
    unsigned char *string_value = (unsigned char *) MALLOC(string_buffer->string_length + 1);

    /* creates the iterators for the list of strings, to go
    arround them joining the strings into a single buffer */
    create_iterator_linked_list(string_buffer->string_list, &string_iterator);
    create_iterator_linked_list(string_buffer->length_list, &length_iterator);

    /* sets the "initial" pointer value to the "base" string value position */
    pointer = string_value;

    /* iterates continuously to process to "join" the
    various "partial" string values into a single buffer */
    while(1) {
        /* retrieves the next value from the string iterator
        as the partial value */
        get_next_iterator(string_iterator, (void **) &partial_value);

        /* retrieves the next value from the length iterator
        as the partial value length (length of the string) */
        get_next_iterator(length_iterator, (void **) &partial_value_length);

        /* in case the partial value is not set
        there are no more items to be retrieved from
        the iterator, breaks the loop */
        if(partial_value == NULL) { break; }

        /* copoes the contents of the partial value to
        the buffer "pointed" by pointer, using the previously
        retrieved partial value length */
        memcpy(pointer, partial_value, partial_value_length);

        /* updates the pointer value with the length of the
        partial (string) value */
        pointer += partial_value_length;
    }

    /* "closes" the string value, usefull for usage as a "classic"
    null terminated string */
    string_value[string_buffer->string_length] = '\0';

    /* sets the string value in the value "pointed" by
    the string value pointer */
    *string_value_pointer = string_value;

    /* deletes both the length and the string iterators, in order
    to avoid possible memory leaks */
    delete_iterator_linked_list(string_buffer->length_list, length_iterator);
    delete_iterator_linked_list(string_buffer->string_list, string_iterator);
}

void _append_string_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value) {
    /* adds the string value to the list of strings to have
    the memory released uppon string buffer release */
    append_value_linked_list(string_buffer->release_list, string_value);

    /* adds the string value to the string buffer */
    append_string_buffer(string_buffer, string_value);
}

void _append_string_l_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value, size_t string_length) {
    /* adds the string value to the list of strings to have
    the memory released uppon string buffer release */
    append_value_linked_list(string_buffer->release_list, string_value);

    /* adds the string (with length) value to the string buffer */
    append_string_l_buffer(string_buffer, string_value, string_length);
}

void _append_string_t_buffer(struct string_buffer_t *string_buffer, struct string_t *string) {
    /* adds the string structure value to the string buffer,
    using the length oriented append function */
    _append_string_l_buffer(string_buffer, string->buffer, string->length);
}
