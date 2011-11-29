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

#include "../sorting/sorting.h"
#include "iterator.h"

typedef struct LinkedList_t {
    size_t size;
    struct LinkedListNode_t *first;
    struct LinkedListNode_t *last;
} LinkedList;

typedef struct LinkedListNode_t {
    void *value;
    struct LinkedListNode_t *next;
    struct LinkedListNode_t *previous;
} LinkedListNode;

/**
 * Constructor of the linked list.
 *
 * @param linkedListPointer The pointer to the linked list to be constructed.
 */
VIRIATUM_EXPORT_PREFIX void createLinkedList(struct LinkedList_t **linkedListPointer);

/**
 * Destructor of the linked list.
 *
 * @param linkedList The linked list to be destroyed.
 */
VIRIATUM_EXPORT_PREFIX void deleteLinkedList(struct LinkedList_t *linkedList);
VIRIATUM_EXPORT_PREFIX void createLinkedListNode(struct LinkedListNode_t **linkedListNodePointer);
VIRIATUM_EXPORT_PREFIX void deleteLinkedListNode(struct LinkedListNode_t *linkedListNode);
VIRIATUM_EXPORT_PREFIX void clearLinkedList(struct LinkedList_t *linkedList);
VIRIATUM_EXPORT_PREFIX void appendLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t *linkedListNode);
VIRIATUM_EXPORT_PREFIX void appendValueLinkedList(struct LinkedList_t *linkedList, void *value);
VIRIATUM_EXPORT_PREFIX void removeLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t *linkedListNode, char deleteNode);
VIRIATUM_EXPORT_PREFIX void removeValueLinkedList(struct LinkedList_t *linkedList, void *value, char deleteNode);
VIRIATUM_EXPORT_PREFIX void removeIndexLinkedList(struct LinkedList_t *linkedList, size_t index, char deleteNode);
VIRIATUM_EXPORT_PREFIX void getLinkedList(struct LinkedList_t *linkedList, size_t index, struct LinkedListNode_t **linkedListNodePointer);
VIRIATUM_EXPORT_PREFIX void getValueLinkedList(struct LinkedList_t *linkedList, size_t index, void **valuePointer);
VIRIATUM_EXPORT_PREFIX void peekLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t **linkedListNodePointer);
VIRIATUM_EXPORT_PREFIX void peekValueLinkedList(struct LinkedList_t *linkedList, void **valuePointer);
VIRIATUM_EXPORT_PREFIX void popLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t **linkedListNodePointer);
VIRIATUM_EXPORT_PREFIX void popValueLinkedList(struct LinkedList_t *linkedList, void **valuePointer, char deleteNode);
VIRIATUM_EXPORT_PREFIX void createIteratorLinkedList(struct LinkedList_t *linkedList, struct Iterator_t **iteratorPointer);
VIRIATUM_EXPORT_PREFIX void deleteIteratorLinkedList(struct LinkedList_t *linkedList, struct Iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void resetIteratorLinkedList(struct LinkedList_t *linkedList, struct Iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void getNextIteratorLinkedList(struct Iterator_t *iterator, void **nextPointer);
VIRIATUM_EXPORT_PREFIX void toSequenceLinkedList(struct LinkedList_t *linkedList, void ***sequencePointer);
VIRIATUM_EXPORT_PREFIX void sortLinkedList(struct LinkedList_t *linkedList, comparator cmp);
