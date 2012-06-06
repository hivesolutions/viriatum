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

/**
 * The default size to be used in a newly
 * constructed hash map.
 */
#define DEFAULT_HASH_MAP_SIZE 1024

/**
 * The default maximum load factor for which
 * a re-allocation of the map must be done.
 */
#define DEFAULT_MAXIMUM_LOAD_FACTOR 0.75

/**
 * The default factor to be used when resizing the
 * the hash map due to size overflow.
 */
#define DEFAULT_RESIZE_FACTOR 4

typedef struct hash_map_t {
    size_t size;
    size_t maximum_size;
    size_t element_size;
    size_t elements_buffer_size;
    struct hash_map_element_t *elements_buffer;
} hash_map;

typedef struct hash_map_element_t {
    void *value;
    unsigned int used;
    size_t key;
    unsigned char *key_string;
} hash_map_element;

VIRIATUM_EXPORT_PREFIX void create_hash_map(struct hash_map_t **hash_map_pointer, size_t initial_size);
VIRIATUM_EXPORT_PREFIX void delete_hash_map(struct hash_map_t *hash_map);
VIRIATUM_EXPORT_PREFIX void set_value_hash_map(struct hash_map_t *hash_map, size_t key, unsigned char *key_string, void *value);
VIRIATUM_EXPORT_PREFIX void set_value_string_hash_map(struct hash_map_t *hash_map, unsigned char *key_string, void *value);
VIRIATUM_EXPORT_PREFIX void get_hash_map(struct hash_map_t *hash_map, size_t key, unsigned char *key_string, struct hash_map_element_t **element_pointer);
VIRIATUM_EXPORT_PREFIX void get_value_hash_map(struct hash_map_t *hash_map, size_t key, unsigned char *key_string, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void get_value_string_hash_map(struct hash_map_t *hash_map, unsigned char *key_string, void **value_pointer);
VIRIATUM_EXPORT_PREFIX void create_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t **iterator_pointer);
VIRIATUM_EXPORT_PREFIX void create_element_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t **iterator_pointer);
VIRIATUM_EXPORT_PREFIX void delete_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void reset_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void get_next_iterator_hash_map(struct iterator_t *iterator, void **next_pointer);
VIRIATUM_EXPORT_PREFIX void get_next_element_iterator_hash_map(struct iterator_t *iterator, void **next_pointer);
VIRIATUM_EXPORT_PREFIX void _resize_hash_map(struct hash_map_t *hash_map);
VIRIATUM_EXPORT_PREFIX size_t _calculate_string_hash_map(unsigned char *key_string);
