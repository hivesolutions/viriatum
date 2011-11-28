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

#include "iterator.h"
#include "linked_list.h"

/**
 * Structure holding internal information
 * for a buffer (in memory) of strings.
 * The buffer is linear and contains the various
 * partial string values.
 */
typedef struct StringBuffer_t {
    /**
     * The total length for the internal
     * string value.
     */
    size_t stringLength;

    /**
     * The list of strings that compose the
     * the complete string value.
     */
    struct LinkedList_t *stringList;
} StringBuffer;

/**
 * Constructor of the string buffer.
 *
 * @param linkedListPointer The pointer to the string buffer to be constructed.
 */
VIRIATUM_EXPORT_PREFIX void createStringBuffer(struct StringBuffer_t **stringBufferPointer);

/**
 * Destructor of the string buffer.
 *
 * @param linkedList The string buffer to be destroyed.
 */
VIRIATUM_EXPORT_PREFIX void deleteStringBuffer(struct StringBuffer_t *stringBuffer);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 *
 * @param stringBuffer The string buffer reference.
 * @param stringValue The string value to be added to the string buffer.
 */
VIRIATUM_EXPORT_PREFIX void appendStringBuffer(struct StringBuffer_t *stringBuffer, unsigned char *stringValue);

/**
 * "Joins" the internal buffer of strings creating the final
 * string that is returned from the given string value pointer.
 *
 * @param stringBuffer The string buffer reference.
 * @param stringValuePointer The pointer to the string value to
 * hold the "joined" string value.
 */
VIRIATUM_EXPORT_PREFIX void joinStringBuffer(struct StringBuffer_t *stringBuffer, unsigned char **stringValuePointer);
