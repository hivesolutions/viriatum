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
    /* releases the elements buffer */
    FREE(hashMap->elementsBuffer);

    /* releases the hash map */
    FREE(hashMap);
}

void setValueHashMap(struct HashMap_t *hashMap, size_t key, void *value) {
    /* allocates space for the hash map element to be used */
    struct HashMapElement_t *element;

    /* allocates space for the index used for element
    access (computed modulus hash) */
    size_t index;

    /* in case the current hash map size "overflows"
    the maximum size (a resizing is required) */
    if(hashMap->size >= hashMap->maximumSize) {
        /* resizes the hash map to an appropriate
        size to avoid collisions */
        _resizeHashMap(hashMap);
    }

    /* calculates the index using the modulus */
    index = key % hashMap->elementsBufferSize;

    /* iterates continously (to get an empty space
    in the hash map), this conforms with the open
    addressing strategy for hash maps */
    while(1) {
        /* retrieves the base address value */
        element = &hashMap->elementsBuffer[index];

        /* in case the element is empty or the
        key is the same (overwrite) */
        if(element->used == 0 || element->key == key) {
            /* breaks the loop */
            break;
        }

        /* in case the index value is "normal"
        and sane (normal case) */
        if(index < hashMap->elementsBufferSize - 1) {
            /* increment the index value */
            index++;
        }
        /* otherwise the hash map overflows (need
        to reset the counter value) */
        else {
            /* resets the index value (overflow) */
            index = 0;
        }
    }

    /* sets the element fields */
    element->value = value;
    element->key = key;
    element->used = 1;

    /* increments the hash map size */
    hashMap->size++;
}
void setValueStringHashMap(struct HashMap_t *hashMap, unsigned char *keyString, void *value) {
    /* calculates the key (hash) value from the key string
    and uses it to set the value in the hash map */
    size_t key = _calculateStringHashMap(keyString);
    setValueHashMap(hashMap, key, value);
}

void getHashMap(struct HashMap_t *hashMap, size_t key, struct HashMapElement_t **elementPointer) {
    /* allocates space for the hash map element to be used */
    struct HashMapElement_t *element;

    /* calculates the index using the modulus */
    size_t index = key % hashMap->elementsBufferSize;

    /* iterates continously (to retrieve the appropriate
    element in the hash map), this conforms with the open
    addressing strategy for hash maps */
    while(1) {
        /* retrieves the base address value */
        element = &hashMap->elementsBuffer[index];

        /* in case the element is not used, the element
        search is over (element was not found) */
        if(element->used == 0) {
            /* breaks the loop */
            break;
        }

        /* in case the element key is the same as the
        requested (element found) */
        if(element->key == key) {
            /* breaks the loop */
            break;
        }

        /* in case the index value is "normal"
        and sane (normal case) */
        if(index < hashMap->elementsBufferSize - 1) {
            /* increment the index value */
            index++;
        }
        /* otherwise the hash map overflows (need
        to reset the counter value) */
        else {
            /* resets the index value (overflow) */
            index = 0;
        }
    }

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

void _resizeHashMap(struct HashMap_t *hashMap) {
    /* allocates space for the index */
    size_t index = 0;

    /* allocates space for the hash map element to be used
    for the copy of the elements data */
    struct HashMapElement_t *element;

    /* allocates space and copies the "old" elements
    buffer and size (backup of elements buffer) */
    struct HashMapElement_t *elementsBuffer = hashMap->elementsBuffer;
    size_t elementsBufferSize = hashMap->elementsBufferSize;

    /* resets the hash map size to zero (no elements inserted) */
    hashMap->size = 0;

    /* increments the elements buffer size by the default
    resize factor and creates the new memory buffer for them */
    hashMap->elementsBufferSize *= DEFAULT_RESIZE_FACTOR;
    hashMap->elementsBuffer = (struct HashMapElement_t *) MALLOC(hashMap->elementSize * hashMap->elementsBufferSize);

    /* re-calculates the maximum size that elements buffer may
    assume before resizing */
    hashMap->maximumSize = (size_t) ((double) hashMap->elementsBufferSize * DEFAULT_MAXIMUM_LOAD_FACTOR);

    /* resets the elements buffer value */
    memset(hashMap->elementsBuffer, 0, hashMap->elementSize * hashMap->elementsBufferSize);

    /* iterates over all the "old" elements to copy
    and set them in the new hash map buffer */
    for(index = 0; index < elementsBufferSize; index++) {
        /* retrieves the current element structure from
        the elements buffer to set the element key and value
        in the "newly" resized hash map */
        element = &elementsBuffer[index];

        /* in case the element is not "used", there's
        no need to set the value in the hash map*/
        if(element->used == 0) {
            /* continues the loop */
            continue;
        }

        /* sets the value (key and value) in the hash map (copy step) */
        setValueHashMap(hashMap, element->key, element->value);
    }
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

    /* checks for aditional abnormal key value */
    if(key == -1) {
        /* removes the error code value */
        key = -2;
    }

    /* returns the (calculated) key */
    return key;
}
