/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "hash_map.h"

void createHashMap(struct HashMap_t **hashMapPointer, size_t initialSize) {
    /* retrieves the hash map size */
    size_t hashMapSize = sizeof(struct HashMap_t);

    /* retrives the hash map element size */
    size_t hashMapElementSize = sizeof(struct HashMapElement_t);

    /* allocates space for the hash map */
    struct HashMap_t *hashMap = (struct HashMap_t *) malloc(hashMapSize);

    /* in case the initial size is not set (zero) */
    if((void *) initialSize == 0) {
        /* sets the default initial size value */
        initialSize = DEFAULT_HASH_MAP_SIZE;
    }

    /* initializes the hash map size */
    hashMap->size = 0;

    /* sets the maximum size of the hash map */
    hashMap->maximumSize = (size_t) ((double) initialSize * DEFAULT_MAXIMUM_LOAD_FACTOR);

    /* sets the hash map element size */
    hashMap->elementSize = hashMapElementSize;

    /* sets the elements buffer size */
    hashMap->elementsBufferSize = initialSize;

    /* allocates space for the elements buffer */
    hashMap->elementsBuffer = (struct HashMapElement_t *) malloc(hashMapElementSize * initialSize);

    /* resets the elements buffer value */
    memset(hashMap->elementsBuffer, 0, hashMapElementSize * initialSize);

    /* sets the hash map in the hash map pointer */
    *hashMapPointer = hashMap;
}

void setHashMap(struct HashMap_t *hashMap, size_t key, void *value) {
    /* calculates the index using the modulus */
    size_t index = key % hashMap->elementsBufferSize;

    /* retrieves the base address value */
    struct HashMapElement_t *element = &hashMap->elementsBuffer[index];

    /* sets the element fields */
    element->value = value;
    element->key = key;
    element->used = 1;
}

void getHashMap(struct HashMap_t *hashMap, size_t key, void **valuePointer) {
    /* calculates the index using the modulus */
    size_t index = key % hashMap->elementsBufferSize;

    /* retrieves the base address value */
    struct HashMapElement_t *element = &hashMap->elementsBuffer[index];

    /* sets the element value in the value pointer */
    *valuePointer = element->value;
}
