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
        /* sets the linked list node as the first node */
        linkedList->first = linkedListNode;
    } else {
        /* sets the next element of the last element as
        the new linked list node */
        linkedList->last->next = linkedListNode;

        /* sets the previous element of the linked list
        node as the last one of the linked list */
        linkedListNode->previous = linkedList->last;
    }

    /* sets the linked list node as the last node */
    linkedList->last = linkedListNode;

    /* increments the linked list size */
    linkedList->size++;
}

void removeLinkedList(struct LinkedList_t *linkedList, size_t index) {
    /* allocates space for the index */
    size_t _index;

    /* allocates space for the current node */
    struct LinkedListNode_t *currentNode;

    /* allocates space for the previous node */
    struct LinkedListNode_t *previousNode;

	/* allocates space for the next node */
    struct LinkedListNode_t *nextNode;

    /* sets the initial iteration node */
    currentNode = linkedList->first;

	/* iterates over the index value */
    for(_index = 0; _index < index; _index++) {
        /* sets the current node as the next node */
        currentNode = currentNode->next;
    }

	/* retrieves the previous node */
	previousNode = currentNode->previous;

	/* retrieves the next node */
	nextNode = currentNode->next;

	/* in case the previous node is valid */
	if(previousNode != NULL) {
		/* sets the next as the next node */
		previousNode->next = nextNode;
	}

	/* in case the next node is valid */
	if(nextNode != NULL) {
		/* sets the previous as the previous node */
		nextNode->previous = previousNode;
	}

	/* in case the element to be removed is the last */
	if(index == linkedList->size - 1) {
		/* sets the last node as the previous node */
		linkedList->last = previousNode;
	}

	/* in case the element to be removed is the last minus one */
	if(index == linkedList->size - 2) {
		/* sets the last node as the next node */
		linkedList->last = nextNode;
	}

	/* in case the element to be removed is the first */
	if(index == 0) {
		/* sets the last node as the previous node */
		linkedList->first = nextNode;
	}

    /* decrements the linked list size */
    linkedList->size--;
}

void getLinkedList(struct LinkedList_t *linkedList, size_t index, void **valuePointer) {
    /* allocates space for the index */
    size_t _index;

    /* allocates space for the current node */
    struct LinkedListNode_t *currentNode;

    /* sets the initial iteration node */
    currentNode = linkedList->first;

    /* iterates over the index value */
    for(_index = 0; _index < index; _index++) {
        /* sets the current node as the next node */
        currentNode = currentNode->next;
    }

    /* sets the current node value in the value pointer */
    *valuePointer = currentNode->value;
}

void popLinkedList(struct LinkedList_t *linkedList, void **valuePointer) {
	/* retrieves the first element from the linked list */
	getLinkedList(linkedList, 0, valuePointer);

	/* removes the first element from the linked list */
	removeLinkedList(linkedList, 0);
}