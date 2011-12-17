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
 
 __author__    = Jo‹o Magalh‹es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "string_buffer.h"

void createStringBuffer(struct StringBuffer_t **stringBufferPointer) {
    /* retrieves the string buffer size */
    size_t stringBufferSize = sizeof(struct StringBuffer_t);

    /* allocates space for the string buffer */
    struct StringBuffer_t *stringBuffer = (struct StringBuffer_t *) MALLOC(stringBufferSize);

    /* sets the default values in the string buffer */
    stringBuffer->stringLength = 0;

    /* creates the list to hold the various strings
    for the string buffer runtime */
    createLinkedList(&stringBuffer->stringList);

    /* creates the list to hold the various strings
    to have the memory released uppon destruction */
    createLinkedList(&stringBuffer->releaseList);

    /* sets the string buffer in the string buffer pointer */
    *stringBufferPointer = stringBuffer;
}

void deleteStringBuffer(struct StringBuffer_t *stringBuffer) {
    /* allocates space for the temporary string value */
    unsigned char *stringValue;

    /* iterates continuously for release list
    cleanup (string value memory release) */
    while(1) {
        /* pops a node from the release list */
        popValueLinkedList(stringBuffer->releaseList, (void **) &stringValue, 1);

        /* in case the value is invalid (empty list) */
        if(stringValue == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the string value */
        FREE(stringValue);
    }

    /* deletes the list of release strings from the string buffer */
    deleteLinkedList(stringBuffer->releaseList);

    /* deletes the list of strings from the string buffer */
    deleteLinkedList(stringBuffer->stringList);

    /* releases the string buffer */
    FREE(stringBuffer);
}

void appendStringBuffer(struct StringBuffer_t *stringBuffer, unsigned char *stringValue) {
    /* retrieves the length of the string value */
    size_t stringValueLength = strlen((char *) stringValue);

    /* adds the string value to the lsit of strings and then
    increments the (total) string length with the length of
    the current string value */
    appendValueLinkedList(stringBuffer->stringList, stringValue);
    stringBuffer->stringLength += stringValueLength;
}

void joinStringBuffer(struct StringBuffer_t *stringBuffer, unsigned char **stringValuePointer) {
    /* allocates space for the iterator used for percolating
    the various (partial) string values */
    struct Iterator_t *stringIterator;

    /* allocates space for the partial value and the partial value length */
    unsigned char *partialValue;
    size_t partialValueLength;

    /* allocates space for the pointer and allocates the buffer to hold
    the complete "joined" string value */
    unsigned char *pointer;
    unsigned char *stringValue = (unsigned char *) MALLOC(stringBuffer->stringLength + 1);

    /* creates an iterator for the list of strings, to go
    arround them joining the strings into a single buffer */
    createIteratorLinkedList(stringBuffer->stringList, &stringIterator);

    /* sets the "initial" pointer value to the "base" string value position */
    pointer = stringValue;

    /* iterates continuously to process to "join" the
    various "partial" string values into a single buffer */
    while(1) {
        /* retrieves the next value from the string iterator
        as the partial value */
        getNextIterator(stringIterator, (void **) &partialValue);

        /* in case the partial value is not set
        there are no more items to be retrieved from
        the iterator */
        if(partialValue == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrieves the partial value length and uses
        it to copy the contents of the partial value to
        the buffer "pointed" by pointer */
        partialValueLength = strlen((char *) partialValue);
        memcpy(pointer, partialValue, partialValueLength);

        /* updates the pointer value with the length of the
        partial (string) value */
        pointer += partialValueLength;
    }

    /* "closes" the string value */
    stringValue[stringBuffer->stringLength] = '\0';

    /* sets the string value in the value "pointed" by
    the string value pointer */
    *stringValuePointer = stringValue;

    /* deletes the string iterator */
    deleteIteratorLinkedList(stringBuffer->stringList, stringIterator);
}

void _appendStringBuffer(struct StringBuffer_t *stringBuffer, unsigned char *stringValue) {
    /* adds the string value to the list of strings to have
    the memory released uppon string buffer release */
    appendValueLinkedList(stringBuffer->releaseList, stringValue);

    /* adds the string value to the string buffer */
    appendStringBuffer(stringBuffer, stringValue);
}
