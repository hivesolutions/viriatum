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

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

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

VIRIATUM_EXPORT_PREFIX void createLinkedList(struct LinkedList_t **linkedListPointer);
VIRIATUM_EXPORT_PREFIX void deleteLinkedList(struct LinkedList_t *linkedList);
VIRIATUM_EXPORT_PREFIX void createLinkedListNode(struct LinkedListNode_t **linkedListNodePointer);
VIRIATUM_EXPORT_PREFIX void appendLinkedList(struct LinkedList_t *linkedList, void *value);
VIRIATUM_EXPORT_PREFIX void removeLinkedList(struct LinkedList_t *linkedList, size_t index);
VIRIATUM_EXPORT_PREFIX void getLinkedList(struct LinkedList_t *linkedList, size_t index, void **valuePointer);
VIRIATUM_EXPORT_PREFIX void popLinkedList(struct LinkedList_t *linkedList, void **valuePointer);
