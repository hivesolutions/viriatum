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

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "stdafx.h"

#include "linked_list.h"

void createLinkedList(struct LinkedList_t **linkedListPointer) {
    /* retrieves the linked list size */
    size_t linkedListSize = sizeof(struct LinkedList_t);

    /* allocates space for the linked list */
    struct LinkedList_t *linkedList = (struct LinkedList_t *) malloc(linkedListSize);

    /* initializes the linked list size */
    linkedList->size = 0;

    /* sets the first and last elements of the linked list to null */
    linkedList->first = NULL;
    linkedList->last = NULL;

    /* sets the linked list in the linked list pointer */
    *linkedListPointer = linkedList;
}

void deleteLinkedList(struct LinkedList_t *linkedList) {
    /* releases the linked list */
    free(linkedList);
}

void createLinkedListNode(struct LinkedListNode_t **linkedListNodePointer) {
    /* retrieves the linked list node size */
    size_t linkedListNodeSize = sizeof(struct LinkedListNode_t);

    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode = (struct LinkedListNode_t *) malloc(linkedListNodeSize);

    /* initializes the linked list node value */
    linkedListNode->value = NULL;

    /* sets the next and previous elements of the linked list node to null */
    linkedListNode->next = NULL;
    linkedListNode->previous = NULL;

    /* sets the linked list node in the linked list node pointer */
    *linkedListNodePointer = linkedListNode;
}

void appendLinkedList(struct LinkedList_t *linkedList, void *value) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* creates the linked list node */
    createLinkedListNode(&linkedListNode);

    /* sets the value in the linked list node */
    linkedListNode->value = value;

    /* in case the linked list is empty */
    if(linkedList->size == 0) {
        linkedList->first = linkedListNode;
        linkedList->last = linkedListNode;
    }
}
