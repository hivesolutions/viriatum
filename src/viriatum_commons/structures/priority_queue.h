/*
 Hive Viriatum Commons
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
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
