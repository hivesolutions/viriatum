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
#include "hash_map.h"
#include "linked_list.h"

typedef struct SortMap_t {
    struct HashMap_t *hashMap;
    struct LinkedList_t *keyList;
    struct LinkedList_t *keyStringList;
} SortMap;

VIRIATUM_EXPORT_PREFIX void createSortMap(struct SortMap_t **sortMapPointer, size_t initialSize);
VIRIATUM_EXPORT_PREFIX void deleteSortMap(struct SortMap_t *sortMap);
VIRIATUM_EXPORT_PREFIX void setValueSortMap(struct SortMap_t *sortMap, size_t key, unsigned char *keyString, void *value);
VIRIATUM_EXPORT_PREFIX void setValueStringSortMap(struct SortMap_t *sortMap, unsigned char *keyString, void *value);
VIRIATUM_EXPORT_PREFIX void getValueSortMap(struct SortMap_t *sortMap, size_t key, unsigned char *keyString, void **valuePointer);
VIRIATUM_EXPORT_PREFIX void getValueStringSortMap(struct SortMap_t *sortMap, unsigned char *keyString, void **valuePointer);
VIRIATUM_EXPORT_PREFIX void createIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t **iteratorPointer);
VIRIATUM_EXPORT_PREFIX void createElementIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t **iteratorPointer);
VIRIATUM_EXPORT_PREFIX void deleteIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void resetIteratorSortMap(struct SortMap_t *sortMap, struct Iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void getNextIteratorSortMap(struct Iterator_t *iterator, void **nextPointer);
VIRIATUM_EXPORT_PREFIX void getNextElementIteratorSortMap(struct Iterator_t *iterator, void **nextPointer);
