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

void create_linked_list(struct linked_list_t **linked_list_pointer) {
    /* retrieves the linked list size */
    size_t linked_list_size = sizeof(struct linked_list_t);

    /* allocates space for the linked list */
    struct linked_list_t *linked_list = (struct linked_list_t *) MALLOC(linked_list_size);

    /* initializes the linked list size */
    linked_list->size = 0;

    /* sets the first and last elements of the linked list to null */
    linked_list->first = NULL;
    linked_list->last = NULL;

    /* sets the linked list in the linked list pointer */
    *linked_list_pointer = linked_list;
}

void delete_linked_list(struct linked_list_t *linked_list) {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the next node */
    struct linked_list_node_t *next_node;

    /* sets the initial iteration node */
    struct linked_list_node_t *current_node = linked_list->first;

    /* iterates over the index value */
    for(index = 0; index < linked_list->size; index++) {
        /* retrieves the next node */
        next_node = current_node->next;

        /* deletes the linked list node */
        delete_linked_list_node(current_node);

        /* sets the current node as the next node */
        current_node = next_node;
    }

    /* releases the linked list */
    FREE(linked_list);
}

void create_linked_list_node(struct linked_list_node_t **linked_list_node_pointer) {
    /* retrieves the linked list node size */
    size_t linked_list_node_size = sizeof(struct linked_list_node_t);

    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node = (struct linked_list_node_t *) MALLOC(linked_list_node_size);

    /* initializes the linked list node value */
    linked_list_node->value = NULL;

    /* sets the next and previous elements of the linked list node to null */
    linked_list_node->next = NULL;
    linked_list_node->previous = NULL;

    /* sets the linked list node in the linked list node pointer */
    *linked_list_node_pointer = linked_list_node;
}

void delete_linked_list_node(struct linked_list_node_t *linked_list_node) {
    /* releases the linked list node */
    FREE(linked_list_node);
}

void clear_linked_list(struct linked_list_t *linked_list) {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the next node */
    struct linked_list_node_t *next_node;

    /* sets the initial iteration node */
    struct linked_list_node_t *current_node = linked_list->first;

    /* iterates over the index value */
    for(index = 0; index < linked_list->size; index++) {
        /* retrieves the next node */
        next_node = current_node->next;

        /* deletes the linked list node */
        delete_linked_list_node(current_node);

        /* sets the current node as the next node */
        current_node = next_node;
    }

    /* resets the linked list size */
    linked_list->size = 0;
}

void append_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t *linked_list_node) {
    /* in case the linked list is empty */
    if(linked_list->size == 0) {
        /* sets the linked list node as the first node */
        linked_list->first = linked_list_node;
    } else {
        /* sets the next element of the last element as
        the new linked list node */
        linked_list->last->next = linked_list_node;

        /* sets the previous element of the linked list
        node as the last one of the linked list */
        linked_list_node->previous = linked_list->last;
    }

    /* sets the linked list node as the last node */
    linked_list->last = linked_list_node;

    /* increments the linked list size */
    linked_list->size++;
}

void append_value_linked_list(struct linked_list_t *linked_list, void *value) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* creates the linked list node */
    create_linked_list_node(&linked_list_node);

    /* sets the value in the linked list node */
    linked_list_node->value = value;

    /* appends the linked list node to the linked list */
    append_linked_list(linked_list, linked_list_node);
}

void remove_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t *linked_list_node, char delete_node) {
    /* allocates space for the previous node */
    struct linked_list_node_t *previous_node;

    /* allocates space for the next node */
    struct linked_list_node_t *next_node;

    /* in case the linked list node is invalid */
    if(linked_list_node == NULL) {
        /* returns immediately */
        return;
    }

    /* retrieves the previous node */
    previous_node = linked_list_node->previous;

    /* retrieves the next node */
    next_node = linked_list_node->next;

    /* in case the previous node is valid */
    if(previous_node != NULL) {
        /* sets the next as the next node */
        previous_node->next = next_node;
    }

    /* in case the next node is valid */
    if(next_node != NULL) {
        /* sets the previous as the previous node */
        next_node->previous = previous_node;
    }

    /* in case the element to be removed is the last */
    if(next_node == NULL) {
        /* sets the last node as the previous node */
        linked_list->last = previous_node;
    }
    /* in case the element to be removed is the last minus one */
    else if(next_node != NULL && next_node->next == NULL) {
        /* sets the last node as the next node */
        linked_list->last = next_node;
    }

    /* in case the element to be removed is the first */
    if(previous_node == NULL) {
        /* sets the last node as the previous node */
        linked_list->first = next_node;
    }

    /* decrements the linked list size */
    linked_list->size--;

    /* in case the delete node flag is set */
    if(delete_node) {
        /* deletes the linked list node */
        delete_linked_list_node(linked_list_node);
    }
}

void remove_value_linked_list(struct linked_list_t *linked_list, void *value, char delete_node) {
    /* allocates space for the index */
    size_t index;

    /* allocates space for the target node */
    struct linked_list_node_t *target_node = NULL;

    /* allocates space for the current node */
    struct linked_list_node_t *current_node = NULL;

    /* sets the initial iteration node */
    current_node = linked_list->first;

    /* iterates over the index value */
    for(index = 0; index < linked_list->size; index++) {
        /* in case the current node value is the same
        as the target value */
        if(current_node->value == value) {
            /* sets the target node as the current node */
            target_node = current_node;

            /* breaks the loop */
            break;
        }

        /* sets the current node as the next node */
        current_node = current_node->next;
    }

    /* in case no node is found for the value */
    if(target_node == NULL) {
        /* returns immediately */
        return;
    }

    /* removes the target node from the linked list */
    remove_linked_list(linked_list, target_node, delete_node);
}

void remove_index_linked_list(struct linked_list_t *linked_list, size_t index, char delete_node) {
    /* allocates space for the index */
    size_t _index;

    /* allocates space for the current node */
    struct linked_list_node_t *current_node;

    /* sets the initial iteration node */
    current_node = linked_list->first;

    /* iterates over the index value */
    for(_index = 0; _index < index; _index++) {
        /* sets the current node as the next node */
        current_node = current_node->next;
    }

    /* removes the current node from the linked list */
    remove_linked_list(linked_list, current_node, delete_node);
}

void get_linked_list(struct linked_list_t *linked_list, size_t index, struct linked_list_node_t **linked_list_node_pointer) {
    /* allocates space for the index */
    size_t _index = 0;

    /* allocates space for the current node */
    struct linked_list_node_t *current_node = NULL;

    /* sets the initial iteration node */
    current_node = linked_list->first;

    /* iterates over the index value */
    for(_index = 0; _index < index; _index++) {
        /* sets the current node as the next node */
        current_node = current_node->next;
    }

    /* sets the current node in the linked list node pointer */
    *linked_list_node_pointer = current_node;
}

void get_value_linked_list(struct linked_list_t *linked_list, size_t index, void **value_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* retrieves the linked list node for the index */
    get_linked_list(linked_list, index, &linked_list_node);

    /* sets the linked list node value in the value pointer */
    *value_pointer = linked_list_node->value;
}

void pop_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* retrieves the initial linked list node */
    get_linked_list(linked_list, 0, &linked_list_node);

    /* removes the first linked list node from the linked list */
    remove_linked_list(linked_list, linked_list_node, 0);

    /* sets the linked list node in the linked list node pointer */
    *linked_list_node_pointer = linked_list_node;
}

void pop_top_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* retrieves the final linked list node, note that
    there is a validation on the size of the linked list */
    get_linked_list(linked_list, linked_list->size > 0 ? linked_list->size - 1 : 0, &linked_list_node);

    /* removes the last linked list node from the linked list */
    remove_linked_list(linked_list, linked_list_node, 0);

    /* sets the linked list node in the linked list node pointer */
    *linked_list_node_pointer = linked_list_node;
}

void pop_value_linked_list(struct linked_list_t *linked_list, void **value_pointer, char delete_node) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* pops the linked list node */
    pop_linked_list(linked_list, &linked_list_node);

    /* in case the linked list node is invalid */
    if(linked_list_node == NULL) {
        /* sets the null valie in the value pointer */
        *value_pointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *value_pointer = linked_list_node->value;
    }

    /* in case the linked list node is valid
    and the delete node flag is set */
    if(linked_list_node != NULL && delete_node) {
        /* deletes the linked list node */
        delete_linked_list_node(linked_list_node);
    }
}

void pop_top_value_linked_list(struct linked_list_t *linked_list, void **value_pointer, char delete_node) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* pops top the linked list node */
    pop_top_linked_list(linked_list, &linked_list_node);

    /* in case the linked list node is invalid */
    if(linked_list_node == NULL) {
        /* sets the null valie in the value pointer */
        *value_pointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *value_pointer = linked_list_node->value;
    }

    /* in case the linked list node is valid
    and the delete node flag is set */
    if(linked_list_node != NULL && delete_node) {
        /* deletes the linked list node */
        delete_linked_list_node(linked_list_node);
    }
}

void peek_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* retrieves the initial linked list node */
    get_linked_list(linked_list, 0, &linked_list_node);

    /* sets the linked list node in the linked list node pointer */
    *linked_list_node_pointer = linked_list_node;
}

void peek_top_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* retrieves the initial linked list node, note that
    there is a validation on the size of the linked list */
    get_linked_list(linked_list, linked_list->size > 0 ? linked_list->size - 1 : 0, &linked_list_node);

    /* sets the linked list node in the linked list node pointer */
    *linked_list_node_pointer = linked_list_node;
}

void peek_value_linked_list(struct linked_list_t *linked_list, void **value_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* peeks the linked list node */
    peek_linked_list(linked_list, &linked_list_node);

    /* in case the linked list node is invalid */
    if(linked_list_node == NULL) {
        /* sets the null valie in the value pointer */
        *value_pointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *value_pointer = linked_list_node->value;
    }
}

void peek_top_value_linked_list(struct linked_list_t *linked_list, void **value_pointer) {
    /* allocates space for the linked list node */
    struct linked_list_node_t *linked_list_node;

    /* peeks the top linked list node */
    peek_top_linked_list(linked_list, &linked_list_node);

    /* in case the linked list node is invalid */
    if(linked_list_node == NULL) {
        /* sets the null valie in the value pointer */
        *value_pointer = NULL;
    } else {
        /* sets the linked list node value in the value pointer */
        *value_pointer = linked_list_node->value;
    }
}

void create_iterator_linked_list(struct linked_list_t *linked_list, struct iterator_t **iterator_pointer) {
    /* allocates the iterator */
    struct iterator_t *iterator;

    /* creates the iterator */
    create_iterator(&iterator);

    /* sets the linked list in the structure */
    iterator->structure = (void *) linked_list;

    /* sets the get next function in the iterator */
    iterator->get_next_function = get_next_iterator_linked_list;

    /* resets the iterator */
    reset_iterator_linked_list(linked_list, iterator);

    /* sets the iterator in the iterator pointer */
    *iterator_pointer = iterator;
}

void delete_iterator_linked_list(struct linked_list_t *linked_list, struct iterator_t *iterator) {
    /* deletes the iterator */
    delete_iterator(iterator);
}

void reset_iterator_linked_list(struct linked_list_t *linked_list, struct iterator_t *iterator) {
    /* sets the iterator parameters as the first item of the linked list */
    iterator->parameters = (void *) linked_list->first;
}

void get_next_iterator_linked_list(struct iterator_t *iterator, void **next_pointer) {
    /* retrieves the current node from the iterator parameters */
    struct linked_list_node_t *current_node = (struct linked_list_node_t *) iterator->parameters;

    /* allocates the next node */
    struct linked_list_node_t *next_node;

    /* allocates the next */
    void *next;

    /* in case the current node is null */
    if(current_node == NULL) {
        /* sets the next node as null */
        next_node = NULL;

        /* sets the next as null */
        next = NULL;
    } else {
        /* retrieves the next node from the current node */
        next_node = current_node->next;

        /* sets the next as the current node value */
        next = current_node->value;
    }

    /* sets the next node in the iterator parameters */
    iterator->parameters = (void *) next_node;

    /* sets the next in the next pointer */
    *next_pointer = next;
}

void to_sequence_linked_list(struct linked_list_t *linked_list, void ***sequence_pointer) {
    /* allocates space for the next and current node node,
    sets the current node to the linked list first node */
    struct linked_list_node_t *next_node;
    struct linked_list_node_t *current_node = linked_list->first;

    /* allocates space for the iteration index accumulator */
    size_t index = 0;

    /* retrieves the void pointer size */
    size_t void_pointer_size = sizeof(void *);

    /* allocates memory space for the sequence of values */
    void **sequence = (void **) MALLOC(linked_list->size * void_pointer_size);

    /* iterates over the sequence to create the sequence
    with the linked list nodes value */
    for(index = 0; index < linked_list->size; index++) {
        /* retrieves the next node */
        next_node = current_node->next;

        /* sets the current sequence value with
        the current node value */
        sequence[index] = current_node->value;

        /* sets the current node as the next node */
        current_node = next_node;
    }

    /* sets the sequence "pointed" by the sequence pointer
    as the "newly" created sequence */
    *sequence_pointer = sequence;
}

void sort_linked_list(struct linked_list_t *linked_list, comparator cmp) {
    /* allocats space for the index accumulator to be
    used durring the sequence iteration */
    size_t index;

    /* allocates space for the linear sequence of values to be
    created for the sorting algorithm execution and then retrieves
    the target size for it */
    void **sequence;
    size_t sequence_size = linked_list->size;

    /* allocates space for the next and current node node,
    sets the current node to the linked list first node */
    struct linked_list_node_t *next_node;
    struct linked_list_node_t *current_node = linked_list->first;

    /* converts the linked list into a linear sequence of values
    then uses the sequence for sorting execution */
    to_sequence_linked_list(linked_list, &sequence);
    sort_quicksort(sequence, 0, sequence_size, cmp);

    /* iterates over the sequence to update the proper
    elements in the linked list (reuses current nodes) */
    for(index = 0; index < sequence_size; index++) {
        /* retrieves the next node */
        next_node = current_node->next;

        /* updates the current node value with the
        current value in the sequence */
        current_node->value = sequence[index];

        /* sets the current node as the next node */
        current_node = next_node;
    }

    /* releases the created sequence */
    FREE(sequence);
}
