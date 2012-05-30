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

#include "sort_map.h"

void createSortMap(struct SortMap_t **sortMapPointer, size_t initialSize) {
    /* retrieves the sort map size */
    size_t sortMapSize = sizeof(struct SortMap_t);

    /* allocates space for the sort map */
    struct SortMap_t *sortMap = (struct SortMap_t *) MALLOC(sortMapSize);

	/* creates the internal structures for internal
	control of the items */
	createHashMap(&sortMap->hashMap, initialSize);
	createLinkedList(&sortMap->keyList);
	createLinkedList(&sortMap->keyStringList);

    /* sets the sort map in the sort map pointer */
    *sortMapPointer = sortMap;
}

void deleteSortMap(struct SortMap_t *sortMap) {
	/* deletes the internal structures */
	deleteLinkedList(sortMap->keyStringList);
	deleteLinkedList(sortMap->keyList);
	deleteHashMap(sortMap->hashMap);

    /* releases the sort map */
    FREE(sortMap);
}

void setValueSortMap(struct SortMap_t *sortMap, size_t key, unsigned char *keyString, void *value) {
	struct HashMapElement_t *element;

	setValueHashMap(sortMap->hashMap, key, keyString, value);
	getHashMap(sortMap->hashMap, key, keyString, &element);
	appendValueLinkedList(sortMap->keyList, (void *) key);
	appendValueLinkedList(sortMap->keyStringList, (void *) element->keyString);
}

void setValueStringSortMap(struct SortMap_t *sortMap, unsigned char *keyString, void *value) {
    /* calculates the key (hash) value from the key string
    and uses it to set the value in the sort map */
    size_t key = _calculateStringHashMap(keyString);
    setValueSortMap(sortMap, key, keyString, value);
}

void getValueSortMap(struct SortMap_t *sortMap, size_t key, unsigned char *keyString, void **valuePointer) {
	getValueHashMap(sortMap->hashMap, key, keyString, valuePointer);
}

void getValueStringSortMap(struct SortMap_t *sortMap, unsigned char *keyString, void **valuePointer) {
    getValueStringHashMap(sortMap->hashMap, keyString, valuePointer);
}

void createIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t **iteratorPointer) {
    /* allocates the iterator */
    struct Iterator_t *iterator;

    /* creates the iterator */
    createIterator(&iterator);

    /* sets the sort map in the structure */
    iterator->structure = (void *) sortMap;

    /* sets the get next function in the iterator */
    iterator->getNextFunction = getNextIteratorSortMap;

    /* resets the iterator */
    resetIteratorSortMap(sortMap, iterator);

    /* sets the iterator in the iterator pointer */
    *iteratorPointer = iterator;
}

void createElementIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t **iteratorPointer) {
    /* allocates the iterator */
    struct Iterator_t *iterator;

    /* creates the iterator */
    createIterator(&iterator);

    /* sets the sort map in the structure */
    iterator->structure = (void *) sortMap;

    /* sets the get next function in the iterator */
    iterator->getNextFunction = getNextElementIteratorSortMap;

    /* resets the iterator */
    resetIteratorSortMap(sortMap, iterator);

    /* sets the iterator in the iterator pointer */
    *iteratorPointer = iterator;
}

void deleteIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t *iterator) {
    /* deletes the iterator */
    deleteIterator(iterator);
}

void resetIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t *iterator) {
    /* sets the iterator parameters as the initial index of
    the elements buffer in the sort map */
    iterator->parameters = 0;
}

void getNextIteratorSortMap(struct Iterator_t *iterator, void **nextPointer) {
    /* retrieves the sort map associated with the iterator */
    struct SortMap_t *sortMap = (struct SortMap_t *) iterator->structure;

    /* retrieves the current index offset in the elements buffer  */
    size_t currentIndex = (size_t) iterator->parameters;

    /* allocates space for the hash map element to be used */
    struct HashMapElement_t *element;

    /* allocates space for the next value */
    void *next;

	/* allocates space for both the key value and the key
	string value to be used for value retrieval */
	size_t key;
	unsigned char *keyString;

    /* in case the current index excedes the elements
    in the key list (it's the end of iteration) */
	if(currentIndex >= sortMap->keyList->size) {
        /* sets the next element as null (end of iteration) */
        next = NULL;
	}
	/* otherwise there's still space for retrieval
	of more elements */
	else {
        /* retrieves the */
		getValueLinkedList(sortMap->keyList, currentIndex, (void **) &key);
		getValueLinkedList(sortMap->keyStringList, currentIndex, (void **) &keyString);

		/* retrieves the current element from the elements
		buffer */
		getHashMap(sortMap->hashMap, key, keyString, &element);

		/* sets the next value in iteration as
		the element key (next value in iteration) */
		next = (void *) &element->key;

		/* increments the current index value */
		currentIndex++;
	}

    /* sets the current index in the iterator parameters */
    iterator->parameters = (void *) currentIndex;

    /* sets the next in the next pointer */
    *nextPointer = next;
}

void getNextElementIteratorSortMap(struct Iterator_t *iterator, void **nextPointer) {
    /* retrieves the sort map associated with the iterator */
    struct SortMap_t *sortMap = (struct SortMap_t *) iterator->structure;

    /* retrieves the current index offset in the elements buffer  */
    size_t currentIndex = (size_t) iterator->parameters;

    /* allocates space for the hash map element to be used */
    struct HashMapElement_t *element;

    /* allocates space for the next value */
    void *next;

	/* allocates space for both the key value and the key
	string value to be used for value retrieval */
	size_t key;
	unsigned char *keyString;

    /* in case the current index excedes the elements
    in the key list (it's the end of iteration) */
	if(currentIndex >= sortMap->keyList->size) {
        /* sets the next element as null (end of iteration) */
        next = NULL;
	}
	/* otherwise there's still space for retrieval
	of more elements */
	else {
        /* retrieves the */
		getValueLinkedList(sortMap->keyList, currentIndex, (void **) &key);
		getValueLinkedList(sortMap->keyStringList, currentIndex, (void **) &keyString);

		/* retrieves the current element from the elements
		buffer */
		getHashMap(sortMap->hashMap, key, keyString, &element);

		/* sets the next value in iteration as
		the element (next value in iteration) */
		next = (void *) element;

		/* increments the current index value */
		currentIndex++;
	}

    /* sets the current index in the iterator parameters */
    iterator->parameters = (void *) currentIndex;

    /* sets the next in the next pointer */
    *nextPointer = next;
}