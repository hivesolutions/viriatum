/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../memory/memory.h"
#include "../sorting/sorting.h"
#include "iterator.h"

typedef struct linked_list_t {
    size_t size;
    struct linked_list_node_t *first;
    struct linked_list_node_t *last;
} linked_list;

typedef struct linked_list_node_t {
    void *value;
    struct linked_list_node_t *next;
    struct linked_list_node_t *previous;
} linked_list_node;

/**
 * Constructor of the linked list.
 *
 * @param linked_list_pointer The pointer to the linked list to be constructed.
 */
VIRIATUM_EXPORT_PREFIX void create_linked_list(struct linked_list_t **linked_list_pointer);

/**
 * Destructor of the linked list.
 *
 * @param linked_list The linked list to be destroyed.
 */
VIRIATUM_EXPORT_PREFIX void delete_linked_list(struct linked_list_t *linked_list);
VIRIATUM_EXPORT_PREFIX void create_linked_list_node(struct linked_list_node_t **linked_list_node_pointer);
VIRIATUM_EXPORT_PREFIX void delete_linked_list_node(struct linked_list_node_t *linked_list_node);
VIRIATUM_EXPORT_PREFIX void clear_linked_list(struct linked_list_t *linked_list);
VIRIATUM_EXPORT_PREFIX void append_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t *linked_list_node);
VIRIATUM_EXPORT_PREFIX void append_front_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t *linked_list_node);
VIRIATUM_EXPORT_PREFIX void append_index_linked_list(
    struct linked_list_t *linked_list,
    struct linked_list_node_t *linked_list_node,
    size_t index
);
VIRIATUM_EXPORT_PREFIX void append_value_linked_list(struct linked_list_t *linked_list, void *value);
VIRIATUM_EXPORT_PREFIX void append_front_value_linked_list(struct linked_list_t *linked_list, void *value);
VIRIATUM_EXPORT_PREFIX void append_index_value_linked_list(
    struct linked_list_t *linked_list,
    void *value,
    size_t index
);
VIRIATUM_EXPORT_PREFIX void remove_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t *linked_list_node, char delete_node);
VIRIATUM_EXPORT_PREFIX void remove_value_linked_list(struct linked_list_t *linked_list, void *value, char delete_node);
VIRIATUM_EXPORT_PREFIX void remove_index_linked_list(struct linked_list_t *linked_list, size_t index, char delete_node);
VIRIATUM_EXPORT_PREFIX void get_linked_list(struct linked_list_t *linked_list, size_t index, struct linked_list_node_t **linked_list_node_pointer);
VIRIATUM_EXPORT_PREFIX void get_value_linked_list(struct linked_list_t *linked_list, size_t index, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void peek_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer);
VIRIATUM_EXPORT_PREFIX void peek_top_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer);
VIRIATUM_EXPORT_PREFIX void peek_value_linked_list(struct linked_list_t *linked_list, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void peek_top_value_linked_list(struct linked_list_t *linked_list, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void pop_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer);
VIRIATUM_EXPORT_PREFIX void pop_top_linked_list(struct linked_list_t *linked_list, struct linked_list_node_t **linked_list_node_pointer);
VIRIATUM_EXPORT_PREFIX void pop_value_linked_list(struct linked_list_t *linked_list, void **value_pointer, char delete_node);
VIRIATUM_EXPORT_PREFIX void pop_top_value_linked_list(struct linked_list_t *linked_list, void **value_pointer, char delete_node);
VIRIATUM_EXPORT_PREFIX void create_iterator_linked_list(struct linked_list_t *linked_list, struct iterator_t **iterator_pointer);
VIRIATUM_EXPORT_PREFIX void delete_iterator_linked_list(struct linked_list_t *linked_list, struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void reset_iterator_linked_list(struct linked_list_t *linked_list, struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void get_next_iterator_linked_list(struct iterator_t *iterator, void **next_pointer);
VIRIATUM_EXPORT_PREFIX void to_sequence_linked_list(struct linked_list_t *linked_list, void ***sequence_pointer);
VIRIATUM_EXPORT_PREFIX void sort_linked_list(struct linked_list_t *linked_list, comparator cmp);
