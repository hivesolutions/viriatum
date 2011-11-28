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

#include "hash_map.h"

void createHashMap(struct HashMap_t **hashMapPointer, size_t initialSize) {
    /* retrieves the hash map size */
    size_t hashMapSize = sizeof(struct HashMap_t);

    /* retrives the hash map element size */
    size_t hashMapElementSize = sizeof(struct HashMapElement_t);

    /* allocates space for the hash map */
    struct HashMap_t *hashMap = (struct HashMap_t *) MALLOC(hashMapSize);

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
    hashMap->elementsBuffer = (struct HashMapElement_t *) MALLOC(hashMapElementSize * initialSize);

    /* resets the elements buffer value */
    memset(hashMap->elementsBuffer, 0, hashMapElementSize * initialSize);

    /* sets the hash map in the hash map pointer */
    *hashMapPointer = hashMap;
}

void deleteHashMap(struct HashMap_t *hashMap) {
    /* releases the hash map */
    FREE(hashMap);
}

void setValueHashMap(struct HashMap_t *hashMap, size_t key, void *value) {
    /* calculates the index using the modulus */
    size_t index = key % hashMap->elementsBufferSize;

    /* retrieves the base address value */
    struct HashMapElement_t *element = &hashMap->elementsBuffer[index];

    /* sets the element fields */
    element->value = value;
    element->key = key;
    element->used = 1;
}

void setValueStringHashMap(struct HashMap_t *hashMap, unsigned char *keyString, void *value) {
    /* calculates the key (hash) value from the key string
    and uses it to set the value in the hash map */
    size_t key = _calculateStringHashMap(keyString);
    setValueHashMap(hashMap, key, value);
}

void getHashMap(struct HashMap_t *hashMap, size_t key, struct HashMapElement_t **elementPointer) {
    /* calculates the index using the modulus */
    size_t index = key % hashMap->elementsBufferSize;

    /* retrieves the base address value */
    struct HashMapElement_t *element = &hashMap->elementsBuffer[index];

    /* sets the element in the element pointer */
    *elementPointer = element;
}

void getValueHashMap(struct HashMap_t *hashMap, size_t key, void **valuePointer) {
    /* allocates space for the element */
    struct HashMapElement_t *element;

    /* retrieves the hash map element for the key */
    getHashMap(hashMap, key, &element);

    /* sets the element value in the value pointer */
    *valuePointer = element->value;
}

void getValueStringHashMap(struct HashMap_t *hashMap, unsigned char *keyString, void **valuePointer) {
    /* calculates the key (hash) value from the key string
    and uses it to retrieve the value from the hash map */
    size_t key = _calculateStringHashMap(keyString);
    getValueHashMap(hashMap, key, valuePointer);
}

size_t _calculateStringHashMap(unsigned char *keyString) {
    /* creates the value to hold the current character
    in iteration */
    unsigned char current;

    /* creates the value to hold the current key value
    and the current index accumulator */
    size_t key = 0;
    unsigned int index = 0;

    /* in case the key string is invalid or in
    case it's empty */
    if(keyString == NULL || keyString[0] == '\0') {
        /* returns the key immediately as zero */
        return 0;
    }

    /* calculates the initial key value and
    increments the initial index value */
    key = keyString[0] << 7;
    index++;

    /* iterates continuously for (integer) key calculation */
    while(1) {
        /* retrievs the current character from the
        the key string value */
        current = keyString[index];

        /* in case the current value is end of string */
        if(current == '\0') {
            /* breaks the loop */
            break;
        }

        /* re-calculates the key value based uppon the current
        character value in loop */
        key = (1000003 * key) ^ current;

        /* increments the index value (iteration) */
        index++;
    }

    /* closes the key value calculation */
    key = key ^ index;
    key == -1 ? key == -2 : key == key;

    /* returns the (calculated) key */
    return key;
}
