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

void createLinkedBuffer(struct LinkedBuffer_t **linkedBufferPointer) {
    /* retrieves the linked buffer size */
    size_t linkedBufferSize = sizeof(struct LinkedBuffer_t);

    /* allocates space for the linked buffer */
    struct LinkedBuffer_t *linkedBuffer = (struct LinkedBuffer_t *) MALLOC(linkedBufferSize);

    /* sets the default values in the linked buffer */
    linkedBuffer->bufferLength = 0;

    /* creates the list to hold the various buffers
    for the linked buffer runtime */
    createLinkedList(&linkedBuffer->bufferList);

    /* creates the list to hold the various buffers
    to have the memory released uppon destruction */
    createLinkedList(&linkedBuffer->releaseList);

    /* sets the linked buffer in the linked buffer pointer */
    *linkedBufferPointer = linkedBuffer;
}

void deleteLinkedBuffer(struct LinkedBuffer_t *linkedBuffer) {
    /* allocates space for the temporary buffer value */
    struct Buffer_t *buffer;

    /* iterates continuously for release list
    cleanup (buffer pointer memory release) */
    while(1) {
        /* pops a node from the release list */
        popValueLinkedList(linkedBuffer->releaseList, (void **) &buffer, 1);

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
        popValueLinkedList(linkedBuffer->bufferList, (void **) &buffer, 1);

        /* in case the value is invalid (empty list)
        need to break the cycle */
        if(buffer == NULL) { break; }

        /* deletes the buffer value */
        deleteBuffer(buffer);
    }

    /* deletes the list of release buffers from the linked buffer */
    deleteLinkedList(linkedBuffer->releaseList);

    /* deletes the list of buffers from the linked buffer */
    deleteLinkedList(linkedBuffer->bufferList);

    /* releases the linked buffer */
    FREE(linkedBuffer);
}

void appendLinkedBuffer(struct LinkedBuffer_t *linkedBuffer, void *pointer, size_t size, unsigned char release) {
    /* allocates space for the buffer structure to be used to
    hold the meta information on the buffer to be added */
    struct Buffer_t *buffer;

    /* creates the buffer structure and then populates it with
    the cirrect pointer and size */
    createBuffer(&buffer, 0);
    buffer->pointer = pointer;
    buffer->size = size;

    /* adds the buffer value to the list of buffers and then
    increments the (total) buffer length with the length of
    the current buffer value */
    appendValueLinkedList(linkedBuffer->bufferList, buffer);
    linkedBuffer->bufferLength += size;

    /* in case the release flag is set must add the buffer
    element to the release list (releases memory on destroy) */
    if(release) { appendValueLinkedList(linkedBuffer->releaseList, buffer); }
}

void joinLinkedBuffer(struct LinkedBuffer_t *linkedBuffer, unsigned char **bufferValuePointer) {
    /* allocates space for the iterator used for percolating
    the various (partial) buffer values */
    struct Iterator_t *bufferIterator;

    /* allocates space for the partial value this is the buffer
    structure used in every iteration for the buffer object */
    struct Buffer_t *partialValue;

    /* allocates space for the pointer and allocates the buffer to hold
    the complete "joined" buffer value */
    unsigned char *pointer;
    unsigned char *bufferValue = (unsigned char *) MALLOC(linkedBuffer->bufferLength);

    /* creates an iterator for the list of buffers, to go
    arround them joining the buffers into a single buffer */
    createIteratorLinkedList(linkedBuffer->bufferList, &bufferIterator);

    /* sets the "initial" pointer value to the "base" buffer value position */
    pointer = bufferValue;

    /* iterates continuously to process to "join" the
    various "partial" string values into a single buffer */
    while(1) {
        /* retrieves the next value from the buffer iterator
        as the partial value */
        getNextIterator(bufferIterator, (void **) &partialValue);

        /* in case the partial value is not set
        there are no more items to be retrieved from
        the iterator, breaks the loop */
        if(partialValue == NULL) { break; }

        /* uses the partial value pointer and size to copy the
        values to the target buffer pointer (fast copy operation) */
        memcpy(pointer, partialValue->pointer, partialValue->size);

        /* updates the pointer value with the size of the partial
        buffer value (buffer length) */
        pointer += partialValue->size;
    }

    /* sets the buffer value in the value "pointed" by
    the buffer value pointer */
    *bufferValuePointer = bufferValue;

    /* deletes the buffer iterator */
    deleteIteratorLinkedList(linkedBuffer->bufferList, bufferIterator);
}

void _appendLinkedBuffer(struct LinkedBuffer_t *linkedBuffer, void *pointer, size_t size) {
    /* adds the buffer value to the linked buffer */
    appendLinkedBuffer(linkedBuffer, pointer, size, 1);
}
