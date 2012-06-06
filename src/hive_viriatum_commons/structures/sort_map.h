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

#pragma once

#include "iterator.h"
#include "hash_map.h"
#include "linked_list.h"

typedef struct sort_map_t {
    struct hash_map_t *hash_map;
    struct linked_list_t *key_list;
    struct linked_list_t *key_string_list;
} sort_map;

VIRIATUM_EXPORT_PREFIX void create_sort_map(struct sort_map_t **sort_map_pointer, size_t initial_size);
VIRIATUM_EXPORT_PREFIX void delete_sort_map(struct sort_map_t *sort_map);
VIRIATUM_EXPORT_PREFIX void set_value_sort_map(struct sort_map_t *sort_map, size_t key, unsigned char *key_string, void *value);
VIRIATUM_EXPORT_PREFIX void set_value_string_sort_map(struct sort_map_t *sort_map, unsigned char *key_string, void *value);
VIRIATUM_EXPORT_PREFIX void get_value_sort_map(struct sort_map_t *sort_map, size_t key, unsigned char *key_string, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void get_value_string_sort_map(struct sort_map_t *sort_map, unsigned char *key_string, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void create_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t **iterator_pointer);
VIRIATUM_EXPORT_PREFIX void create_element_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t **iterator_pointer);
VIRIATUM_EXPORT_PREFIX void delete_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void reset_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void get_next_iterator_sort_map(struct iterator_t *iterator, void **next_pointer);
VIRIATUM_EXPORT_PREFIX void get_next_element_iterator_sort_map(struct iterator_t *iterator, void **next_pointer);
