/*
 Hive Viriatum Commons
 Copyright (c) 2008-2016 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../sorting/sorting.h"
#include "linked_list.h"

typedef struct priority_queue_t {
    struct linked_list_t *list;
    comparator cmp;
    size_t size;
} priority_queue;

typedef struct queue_node_t {
    struct queue_node_t *next;
} queue_node;

VIRIATUM_EXPORT_PREFIX void create_priority_queue(struct priority_queue_t **priority_queue_pointer, comparator cmp);
VIRIATUM_EXPORT_PREFIX void delete_priority_queue(struct priority_queue_t *priority_queue);
VIRIATUM_EXPORT_PREFIX void push_priority_queue(struct priority_queue_t *priority_queue, void *value);
VIRIATUM_EXPORT_PREFIX void pop_priority_queue(struct priority_queue_t *priority_queue, void **value_pointer);
