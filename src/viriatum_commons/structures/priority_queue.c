/*
 Hive Viriatum Commons
 Copyright (c) 2008-2018 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "priority_queue.h"

void create_priority_queue(struct priority_queue_t **priority_queue_pointer, comparator cmp) {
    /* retrieves the size in byte that are required for the priority
    queue and then uses this value to allocate the queue */
    size_t priority_queue_size = sizeof(struct priority_queue_t);
    struct priority_queue_t *priority_queue =\
        (struct priority_queue_t *) MALLOC(priority_queue_size);

    /* sets the default values (including the comparator) in the
    priority queue, note that the size is started at zero */
    priority_queue->cmp = cmp;
    priority_queue->size = 0;

    /* creates the underlying list that is going to be used as the
    backend for the queue as all the operations are going to use it */
    create_linked_list(&priority_queue->list);

    /* sets the priority queue in the provided priority queue pointer
    returning the created queue to the caller function */
    *priority_queue_pointer = priority_queue;
}

void delete_priority_queue(struct priority_queue_t *priority_queue) {
    /* deletes the linked list associated with the queue to avoid
    any king of memory leak and then releases the memory associated
    with the priority queue itself */
    delete_linked_list(priority_queue->list);
    FREE(priority_queue);
}

void push_priority_queue(struct priority_queue_t *priority_queue, void *value) {
    /* allocates the various variable that are going to be used
    in the percolation of the linked list to find the correct
    place to insert the new value in the list */
    int result;
    void *entry;
    struct iterator_t *iterator;

    /* creates and starts the index position of the value at zero
    as this is the fist position in the list (default one) */
    size_t index = 0;

    /* creates the iterator for the list associated with the priority
    queue and then iterates over it until no more items are left or the
    right place to insert the value has been found */
    create_iterator_linked_list(priority_queue->list, &iterator);
    while(TRUE) {
        /* tries to retrieve the next value from the iterator in case
        none is returned breaks the current loop as the end of list has
        been found and so the value must be inserted at the end of the list */
        get_next_iterator(iterator, (void **) &entry);
        if(entry == NULL) { break; }

        /* uses the current comparator function to evaluate if the current
        value is smaller that the current entry in case it is breaks the loop
        as this is the right place to insert the new value */
        result = priority_queue->cmp(value, entry);
        if(result < 0) { break; }

        /* increments the index with one more value as a new iteration loop
        step has just finished */
        index++;
    }

    /* deletes the pending iterator and adds the value to the underlying list
    at the index that was just found through iteration */
    delete_iterator_linked_list(priority_queue->list, iterator);
    append_index_value_linked_list(priority_queue->list, value, index);

    /* increments the size of the priority queue as one more element has
    just been inserted into the queue */
    priority_queue->size++;
}

void pop_priority_queue(struct priority_queue_t *priority_queue, void **value_pointer) {
    /* pops a value from the underlying list directly to the value pointer
    and then decrements the size of the current priority queue */
    pop_value_linked_list(priority_queue->list, value_pointer, TRUE);
    priority_queue->size--;
}
