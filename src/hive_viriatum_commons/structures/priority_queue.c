/*
 Hive Viriatum Commons
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "priority_queue.h"

void create_priority_queue(struct priority_queue_t **priority_queue_pointer, comparator cmp) {
    size_t priority_queue_size = sizeof(struct priority_queue_t);
    struct priority_queue_t *priority_queue = (struct priority_queue_t *) MALLOC(priority_queue_size);

    priority_queue->cmp = cmp;
    create_linked_list(&priority_queue->list);

    *priority_queue_pointer = priority_queue;
}

void delete_priority_queue(struct priority_queue_t *priority_queue) {
    delete_linked_list(priority_queue->list);
    FREE(priority_queue);
}

void push_priority_queue(struct priority_queue_t *priority_queue, void *value) {
    struct iterator_t *iterator;
    void *entry;
    int result;
    size_t index = 0;

    create_iterator_linked_list(priority_queue->list, &iterator);
    while(TRUE) {
        get_next_iterator(iterator, (void **) &entry);
        if(entry == NULL) { break; }

        result = priority_queue->cmp(value, entry);
        if(result < 0) { break; }

        index++;
    }

    delete_iterator_linked_list(priority_queue->list, iterator);
    append_index_value_linked_list(priority_queue->list, value, index);
}

void pop_priority_queue(struct priority_queue_t *priority_queue, void **value_pointer) {
    pop_value_linked_list(priority_queue->list, value_pointer, TRUE);
}
