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
    struct LinkedList_t *linkedList = (struct LinkedList_t *) MALLOC(linkedListSize);

    /* initializes the linked list size */
    linkedList->size = 0;

    /* sets the first and last elements of the linked list to null */
    linkedList->first = NULL;
    linkedList->last = NULL;

    /* sets the linked list in the linked list pointer */
    *linkedListPointer = linkedList;
}

void deleteLinkedList(struct LinkedList_t *linkedList) {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the next node */
    struct LinkedListNode_t *nextNode;

    /* sets the initial iteration node */
    struct LinkedListNode_t *currentNode = linkedList->first;

    /* iterates over the index value */
    for(index = 0; index < linkedList->size; index++) {
        /* retrieves the next node */
        nextNode = currentNode->next;

        /* deletes the linked list node */
        deleteLinkedListNode(currentNode);

        /* sets the current node as the next node */
        currentNode = nextNode;
    }

    /* releases the linked list */
    FREE(linkedList);
}

void createLinkedListNode(struct LinkedListNode_t **linkedListNodePointer) {
    /* retrieves the linked list node size */
    size_t linkedListNodeSize = sizeof(struct LinkedListNode_t);

    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode = (struct LinkedListNode_t *) MALLOC(linkedListNodeSize);

    /* initializes the linked list node value */
    linkedListNode->value = NULL;

    /* sets the next and previous elements of the linked list node to null */
    linkedListNode->next = NULL;
    linkedListNode->previous = NULL;

    /* sets the linked list node in the linked list node pointer */
    *linkedListNodePointer = linkedListNode;
}

void deleteLinkedListNode(struct LinkedListNode_t *linkedListNode) {
    /* releases the linked list node */
    FREE(linkedListNode);
}

void clearLinkedList(struct LinkedList_t *linkedList) {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the next node */
    struct LinkedListNode_t *nextNode;

    /* sets the initial iteration node */
    struct LinkedListNode_t *currentNode = linkedList->first;

    /* iterates over the index value */
    for(index = 0; index < linkedList->size; index++) {
        /* retrieves the next node */
        nextNode = currentNode->next;

        /* deletes the linked list node */
        deleteLinkedListNode(currentNode);

        /* sets the current node as the next node */
        currentNode = nextNode;
    }

    /* resets the linked list size */
    linkedList->size = 0;
}

void appendLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t *linkedListNode) {
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

void appendValueLinkedList(struct LinkedList_t *linkedList, void *value) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* creates the linked list node */
    createLinkedListNode(&linkedListNode);

    /* sets the value in the linked list node */
    linkedListNode->value = value;

    /* appends the linked list node to the linked list */
    appendLinkedList(linkedList, linkedListNode);
}

void removeLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t *linkedListNode, char deleteNode) {
    /* allocates space for the previous node */
    struct LinkedListNode_t *previousNode;

    /* allocates space for the next node */
    struct LinkedListNode_t *nextNode;

    /* in case the linked list node is invalid */
    if(linkedListNode == NULL) {
        /* returns immediately */
        return;
    }

    /* retrieves the previous node */
    previousNode = linkedListNode->previous;

    /* retrieves the next node */
    nextNode = linkedListNode->next;

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
    if(nextNode == NULL) {
        /* sets the last node as the previous node */
        linkedList->last = previousNode;
    }
    /* in case the element to be removed is the last minus one */
    else if(nextNode != NULL && nextNode->next == NULL) {
        /* sets the last node as the next node */
        linkedList->last = nextNode;
    }

    /* in case the element to be removed is the first */
    if(previousNode == NULL) {
        /* sets the last node as the previous node */
        linkedList->first = nextNode;
    }

    /* decrements the linked list size */
    linkedList->size--;

    /* in case the delete node flag is set */
    if(deleteNode) {
        /* deletes the linked list node */
        deleteLinkedListNode(linkedListNode);
    }
}

void removeValueLinkedList(struct LinkedList_t *linkedList, void *value, char deleteNode) {
    /* allocates space for the index */
    size_t index;

    /* allocates space for the target node */
    struct LinkedListNode_t *targetNode = NULL;

    /* allocates space for the current node */
    struct LinkedListNode_t *currentNode = NULL;

    /* sets the initial iteration node */
    currentNode = linkedList->first;

    /* iterates over the index value */
    for(index = 0; index < linkedList->size; index++) {
        /* in case the current node value is the same
        as the target value */
        if(currentNode->value == value) {
            /* sets the target node as the current node */
            targetNode = currentNode;

            /* breaks the loop */
            break;
        }

        /* sets the current node as the next node */
        currentNode = currentNode->next;
    }

    /* in case no node is found for the value */
    if(targetNode == NULL) {
        /* returns immediately */
        return;
    }

    /* removes the target node from the linked list */
    removeLinkedList(linkedList, targetNode, deleteNode);
}

void removeIndexLinkedList(struct LinkedList_t *linkedList, size_t index, char deleteNode) {
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

    /* removes the current node from the linked list */
    removeLinkedList(linkedList, currentNode, deleteNode);
}

void getLinkedList(struct LinkedList_t *linkedList, size_t index, struct LinkedListNode_t **linkedListNodePointer) {
    /* allocates space for the index */
    size_t _index = 0;

    /* allocates space for the current node */
    struct LinkedListNode_t *currentNode;

    /* sets the initial iteration node */
    currentNode = linkedList->first;

    /* iterates over the index value */
    for(_index = 0; _index < index; _index++) {
        /* sets the current node as the next node */
        currentNode = currentNode->next;
    }

    /* sets the current node in the linked list node pointer */
    *linkedListNodePointer = currentNode;
}

void getValueLinkedList(struct LinkedList_t *linkedList, size_t index, void **valuePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* retrieves the linked list node for the index */
    getLinkedList(linkedList, index, &linkedListNode);

    /* sets the linked list node value in the value pointer */
    *valuePointer = linkedListNode->value;
}

void popLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t **linkedListNodePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* retrieves the initial linked list node */
    getLinkedList(linkedList, 0, &linkedListNode);

    /* removes the first linked list node from the linked list */
    removeLinkedList(linkedList, linkedListNode, 0);

    /* sets the linked list node in the linked list node pointer */
    *linkedListNodePointer = linkedListNode;
}

void popTopLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t **linkedListNodePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* retrieves the final linked list node */
    getLinkedList(linkedList, linkedList->size - 1, &linkedListNode);

    /* removes the last linked list node from the linked list */
    removeLinkedList(linkedList, linkedListNode, 0);

    /* sets the linked list node in the linked list node pointer */
    *linkedListNodePointer = linkedListNode;
}

void popValueLinkedList(struct LinkedList_t *linkedList, void **valuePointer, char deleteNode) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* pops the linked list node */
    popLinkedList(linkedList, &linkedListNode);

    /* in case the linked list node is invalid */
    if(linkedListNode == NULL) {
        /* sets the null valie in the value pointer */
        *valuePointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *valuePointer = linkedListNode->value;
    }

    /* in case the linked list node is valid
    and the delete node flag is set */
    if(linkedListNode != NULL && deleteNode) {
        /* deletes the linked list node */
        deleteLinkedListNode(linkedListNode);
    }
}

void popTopValueLinkedList(struct LinkedList_t *linkedList, void **valuePointer, char deleteNode) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* pops top the linked list node */
    popTopLinkedList(linkedList, &linkedListNode);

    /* in case the linked list node is invalid */
    if(linkedListNode == NULL) {
        /* sets the null valie in the value pointer */
        *valuePointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *valuePointer = linkedListNode->value;
    }

    /* in case the linked list node is valid
    and the delete node flag is set */
    if(linkedListNode != NULL && deleteNode) {
        /* deletes the linked list node */
        deleteLinkedListNode(linkedListNode);
    }
}

void peekLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t **linkedListNodePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* retrieves the initial linked list node */
    getLinkedList(linkedList, 0, &linkedListNode);

    /* sets the linked list node in the linked list node pointer */
    *linkedListNodePointer = linkedListNode;
}

void peekTopLinkedList(struct LinkedList_t *linkedList, struct LinkedListNode_t **linkedListNodePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* retrieves the initial linked list node */
    getLinkedList(linkedList, linkedList->size - 1, &linkedListNode);

    /* sets the linked list node in the linked list node pointer */
    *linkedListNodePointer = linkedListNode;
}

void peekValueLinkedList(struct LinkedList_t *linkedList, void **valuePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* peeks the linked list node */
    peekLinkedList(linkedList, &linkedListNode);

    /* in case the linked list node is invalid */
    if(linkedListNode == NULL) {
        /* sets the null valie in the value pointer */
        *valuePointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *valuePointer = linkedListNode->value;
    }
}

void peekTopValueLinkedList(struct LinkedList_t *linkedList, void **valuePointer) {
    /* allocates space for the linked list node */
    struct LinkedListNode_t *linkedListNode;

    /* peeks the top linked list node */
    peekLinkedList(linkedList, &linkedListNode);

    /* in case the linked list node is invalid */
    if(linkedListNode == NULL) {
        /* sets the null valie in the value pointer */
        *valuePointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *valuePointer = linkedListNode->value;
    }
}

void createIteratorLinkedList(struct LinkedList_t *linkedList, struct Iterator_t **iteratorPointer) {
    /* allocates the iterator */
    struct Iterator_t *iterator;

    /* creates the iterator */
    createIterator(&iterator);

    /* sets the linked list in the structure */
    iterator->structure = (void *) linkedList;

    /* sets the get next function in the iterator */
    iterator->getNextFunction = getNextIteratorLinkedList;

    /* resets the iterator */
    resetIteratorLinkedList(linkedList, iterator);

    /* sets the iterator in the iterator pointer */
    *iteratorPointer = iterator;
}

void deleteIteratorLinkedList(struct LinkedList_t *linkedList, struct Iterator_t *iterator) {
    /* deletes the iterator */
    deleteIterator(iterator);
}

void resetIteratorLinkedList(struct LinkedList_t *linkedList, struct Iterator_t *iterator) {
    /* sets the iterator parameters as the first item of the linked list */
    iterator->parameters = (void *) linkedList->first;
}

void getNextIteratorLinkedList(struct Iterator_t *iterator, void **nextPointer) {
    /* retrieves the current node from the iterator parameters */
    struct LinkedListNode_t *currentNode = (struct LinkedListNode_t *) iterator->parameters;

    /* allocates the next node */
    struct LinkedListNode_t *nextNode;

    /* allocates the next */
    void *next;

    /* in case the current node is null */
    if(currentNode == NULL) {
        /* sets the next node as null */
        nextNode = NULL;

        /* sets the next as null */
        next = NULL;
    } else {
        /* retrieves the next node from the current node */
        nextNode = currentNode->next;

        /* sets the next as the current node value */
        next = currentNode->value;
    }

    /* sets the next node in the iterator parameters */
    iterator->parameters = (void *) nextNode;

    /* sets the next in the next pointer */
    *nextPointer = next;
}

void toSequenceLinkedList(struct LinkedList_t *linkedList, void ***sequencePointer) {
    /* allocates space for the next and current node node,
    sets the current node to the linked list first node */
    struct LinkedListNode_t *nextNode;
    struct LinkedListNode_t *currentNode = linkedList->first;

    /* allocates space for the iteration index accumulator */
    size_t index = 0;

    /* retrieves the void pointer size */
    size_t voidPointerSize = sizeof(void *);

    /* allocates memory space for the sequence of values */
    void **sequence = (void **) MALLOC(linkedList->size * voidPointerSize);

    /* iterates over the sequence to create the sequence
    with the linked list nodes value */
    for(index = 0; index < linkedList->size; index++) {
        /* retrieves the next node */
        nextNode = currentNode->next;

        /* sets the current sequence value with
        the current node value */
        sequence[index] = currentNode->value;

        /* sets the current node as the next node */
        currentNode = nextNode;
    }

    /* sets the sequence "pointed" by the sequence pointer
    as the "newly" created sequence */
    *sequencePointer = sequence;
}

void sortLinkedList(struct LinkedList_t *linkedList, comparator cmp) {
    /* allocats space for the index accumulator to be
    used durring the sequence iteration */
    size_t index;

    /* allocates space for the linear sequence of values to be
    created for the sorting algorithm execution and then retrieves
    the target size for it */
    void **sequence;
    size_t sequenceSize = linkedList->size;

    /* allocates space for the next and current node node,
    sets the current node to the linked list first node */
    struct LinkedListNode_t *nextNode;
    struct LinkedListNode_t *currentNode = linkedList->first;

    /* converts the linked list into a linear sequence of values
    then uses the sequence for sorting execution */
    toSequenceLinkedList(linkedList, &sequence);
    sortQuicksort(sequence, 0, sequenceSize, cmp);

    /* iterates over the sequence to update the proper
    elements in the linked list (reuses current nodes) */
    for(index = 0; index < sequenceSize; index++) {
        /* retrieves the next node */
        nextNode = currentNode->next;

        /* updates the current node value with the
        current value in the sequence */
        currentNode->value = sequence[index];

        /* sets the current node as the next node */
        currentNode = nextNode;
    }

    /* releases the created sequence */
    FREE(sequence);
}
