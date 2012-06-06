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

#include "linked_buffer.h"

void create_linked_buffer(struct linked_buffer_t **linked_buffer_pointer) {
    /* retrieves the linked buffer size */
    size_t linked_buffer_size = sizeof(struct linked_buffer_t);

    /* allocates space for the linked buffer */
    struct linked_buffer_t *linked_buffer = (struct linked_buffer_t *) MALLOC(linked_buffer_size);

    /* sets the default values in the linked buffer */
    linked_buffer->buffer_length = 0;

    /* creates the list to hold the various buffers
    for the linked buffer runtime */
    create_linked_list(&linked_buffer->buffer_list);

    /* creates the list to hold the various buffers
    to have the memory released uppon destruction */
    create_linked_list(&linked_buffer->release_list);

    /* sets the linked buffer in the linked buffer pointer */
    *linked_buffer_pointer = linked_buffer;
}

void delete_linked_buffer(struct linked_buffer_t *linked_buffer) {
    /* allocates space for the temporary buffer value */
    struct buffer_t *buffer;

    /* iterates continuously for release list
    cleanup (buffer pointer memory release) */
    while(1) {
        /* pops a node from the release list */
        pop_value_linked_list(linked_buffer->release_list, (void **) &buffer, 1);

        /* in case the value is invalid (empty list)
        need to break the cycle */
        if(buffer == NULL) { break; }

        /* releases the buffer pointer toi the buffer */
        FREE(buffer->pointer);
    }

    /* iterates continuously for release list
    cleanup (buffer value memory release) */
    while(1) {
        /* pops a node from the buffer list */
        pop_value_linked_list(linked_buffer->buffer_list, (void **) &buffer, 1);

        /* in case the value is invalid (empty list)
        need to break the cycle */
        if(buffer == NULL) { break; }

        /* deletes the buffer value */
        delete_buffer(buffer);
    }

    /* deletes the list of release buffers from the linked buffer */
    delete_linked_list(linked_buffer->release_list);

    /* deletes the list of buffers from the linked buffer */
    delete_linked_list(linked_buffer->buffer_list);

    /* releases the linked buffer */
    FREE(linked_buffer);
}

void append_linked_buffer(struct linked_buffer_t *linked_buffer, void *pointer, size_t size, unsigned char release) {
    /* allocates space for the buffer structure to be used to
    hold the meta information on the buffer to be added */
    struct buffer_t *buffer;

    /* creates the buffer structure and then populates it with
    the cirrect pointer and size */
    create_buffer(&buffer, 0);
    buffer->pointer = pointer;
    buffer->size = size;

    /* adds the buffer value to the list of buffers and then
    increments the (total) buffer length with the length of
    the current buffer value */
    append_value_linked_list(linked_buffer->buffer_list, buffer);
    linked_buffer->buffer_length += size;

    /* in case the release flag is set must add the buffer
    element to the release list (releases memory on destroy) */
    if(release) { append_value_linked_list(linked_buffer->release_list, buffer); }
}

void join_linked_buffer(struct linked_buffer_t *linked_buffer, unsigned char **buffer_value_pointer) {
    /* allocates space for the iterator used for percolating
    the various (partial) buffer values */
    struct iterator_t *buffer_iterator;

    /* allocates space for the partial value this is the buffer
    structure used in every iteration for the buffer object */
    struct buffer_t *partial_value;

    /* allocates space for the pointer and allocates the buffer to hold
    the complete "joined" buffer value */
    unsigned char *pointer;
    unsigned char *buffer_value = (unsigned char *) MALLOC(linked_buffer->buffer_length);

    /* creates an iterator for the list of buffers, to go
    arround them joining the buffers into a single buffer */
    create_iterator_linked_list(linked_buffer->buffer_list, &buffer_iterator);

    /* sets the "initial" pointer value to the "base" buffer value position */
    pointer = buffer_value;

    /* iterates continuously to process to "join" the
    various "partial" string values into a single buffer */
    while(1) {
        /* retrieves the next value from the buffer iterator
        as the partial value */
        get_next_iterator(buffer_iterator, (void **) &partial_value);

        /* in case the partial value is not set
        there are no more items to be retrieved from
        the iterator, breaks the loop */
        if(partial_value == NULL) { break; }

        /* uses the partial value pointer and size to copy the
        values to the target buffer pointer (fast copy operation) */
        memcpy(pointer, partial_value->pointer, partial_value->size);

        /* updates the pointer value with the size of the partial
        buffer value (buffer length) */
        pointer += partial_value->size;
    }

    /* sets the buffer value in the value "pointed" by
    the buffer value pointer */
    *buffer_value_pointer = buffer_value;

    /* deletes the buffer iterator */
    delete_iterator_linked_list(linked_buffer->buffer_list, buffer_iterator);
}

void _append_linked_buffer(struct linked_buffer_t *linked_buffer, void *pointer, size_t size) {
    /* adds the buffer value to the linked buffer */
    append_linked_buffer(linked_buffer, pointer, size, 1);
}
