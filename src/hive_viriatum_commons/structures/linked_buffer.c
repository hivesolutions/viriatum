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
    unsigned char *bufferValue;

    /* iterates continuously for release list
    cleanup (buffer value memory release) */
    while(1) {
        /* pops a node from the release list */
        popValueLinkedList(linkedBuffer->releaseList, (void **) &bufferValue, 1);

        /* in case the value is invalid (empty list) */
        if(bufferValue == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the buffer value */
        FREE(bufferValue);
    }

    /* deletes the list of release buffers from the linked buffer */
    deleteLinkedList(linkedBuffer->releaseList);

    /* deletes the list of buffers from the linked buffer */
	deleteLinkedList(linkedBuffer->bufferList);

    /* releases the linked buffer */
    FREE(linkedBuffer);
}