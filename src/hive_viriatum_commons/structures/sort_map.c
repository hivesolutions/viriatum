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

#include "sort_map.h"

void create_sort_map(struct sort_map_t **sort_map_pointer, size_t initial_size) {
    /* retrieves the sort map size */
    size_t sort_map_size = sizeof(struct sort_map_t);

    /* allocates space for the sort map */
    struct sort_map_t *sort_map = (struct sort_map_t *) MALLOC(sort_map_size);

    /* creates the internal structures for internal
    control of the items */
    create_hash_map(&sort_map->hash_map, initial_size);
    create_linked_list(&sort_map->key_list);
    create_linked_list(&sort_map->key_string_list);

    /* sets the sort map in the sort map pointer */
    *sort_map_pointer = sort_map;
}

void delete_sort_map(struct sort_map_t *sort_map) {
    /* deletes the internal structures */
    delete_linked_list(sort_map->key_string_list);
    delete_linked_list(sort_map->key_list);
    delete_hash_map(sort_map->hash_map);

    /* releases the sort map */
    FREE(sort_map);
}

void set_value_sort_map(struct sort_map_t *sort_map, size_t key, unsigned char *key_string, void *value) {
    struct hash_map_element_t *element;

    set_value_hash_map(sort_map->hash_map, key, key_string, value);
    get_hash_map(sort_map->hash_map, key, key_string, &element);
    append_value_linked_list(sort_map->key_list, (void *) key);
    append_value_linked_list(sort_map->key_string_list, (void *) element->key_string);
}

void set_value_string_sort_map(struct sort_map_t *sort_map, unsigned char *key_string, void *value) {
    /* calculates the key (hash) value from the key string
    and uses it to set the value in the sort map */
    size_t key = _calculate_string_hash_map(key_string);
    set_value_sort_map(sort_map, key, key_string, value);
}

void get_value_sort_map(struct sort_map_t *sort_map, size_t key, unsigned char *key_string, void **value_pointer) {
    get_value_hash_map(sort_map->hash_map, key, key_string, value_pointer);
}

void get_value_string_sort_map(struct sort_map_t *sort_map, unsigned char *key_string, void **value_pointer) {
    get_value_string_hash_map(sort_map->hash_map, key_string, value_pointer);
}

void create_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t **iterator_pointer) {
    /* allocates the iterator */
    struct iterator_t *iterator;

    /* creates the iterator */
    create_iterator(&iterator);

    /* sets the sort map in the structure */
    iterator->structure = (void *) sort_map;

    /* sets the get next function in the iterator */
    iterator->get_next_function = get_next_iterator_sort_map;

    /* resets the iterator */
    reset_iterator_sort_map(sort_map, iterator);

    /* sets the iterator in the iterator pointer */
    *iterator_pointer = iterator;
}

void create_element_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t **iterator_pointer) {
    /* allocates the iterator */
    struct iterator_t *iterator;

    /* creates the iterator */
    create_iterator(&iterator);

    /* sets the sort map in the structure */
    iterator->structure = (void *) sort_map;

    /* sets the get next function in the iterator */
    iterator->get_next_function = get_next_element_iterator_sort_map;

    /* resets the iterator */
    reset_iterator_sort_map(sort_map, iterator);

    /* sets the iterator in the iterator pointer */
    *iterator_pointer = iterator;
}

void delete_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t *iterator) {
    /* deletes the iterator */
    delete_iterator(iterator);
}

void reset_iterator_sort_map(struct sort_map_t *sort_map, struct iterator_t *iterator) {
    /* sets the iterator parameters as the initial index of
    the elements buffer in the sort map */
    iterator->parameters = 0;
}

void get_next_iterator_sort_map(struct iterator_t *iterator, void **next_pointer) {
    /* retrieves the sort map associated with the iterator */
    struct sort_map_t *sort_map = (struct sort_map_t *) iterator->structure;

    /* retrieves the current index offset in the elements buffer  */
    size_t current_index = (size_t) iterator->parameters;

    /* allocates space for the hash map element to be used */
    struct hash_map_element_t *element;

    /* allocates space for the next value */
    void *next;

    /* allocates space for both the key value and the key
    string value to be used for value retrieval */
    size_t key;
    unsigned char *key_string;

    /* in case the current index excedes the elements
    in the key list (it's the end of iteration) */
    if(current_index >= sort_map->key_list->size) {
        /* sets the next element as null (end of iteration) */
        next = NULL;
    }
    /* otherwise there's still space for retrieval
    of more elements */
    else {
        /* retrieves both the key and the key string values
        to be used to retrieve the hash element */
        get_value_linked_list(sort_map->key_list, current_index, (void **) &key);
        get_value_linked_list(sort_map->key_string_list, current_index, (void **) &key_string);

        /* retrieves the current element from the elements
        buffer */
        get_hash_map(sort_map->hash_map, key, key_string, &element);

        /* sets the next value in iteration as
        the element key (next value in iteration) */
        next = (void *) &element->key;

        /* increments the current index value */
        current_index++;
    }

    /* sets the current index in the iterator parameters */
    iterator->parameters = (void *) current_index;

    /* sets the next in the next pointer */
    *next_pointer = next;
}

void get_next_element_iterator_sort_map(struct iterator_t *iterator, void **next_pointer) {
    /* retrieves the sort map associated with the iterator */
    struct sort_map_t *sort_map = (struct sort_map_t *) iterator->structure;

    /* retrieves the current index offset in the elements buffer  */
    size_t current_index = (size_t) iterator->parameters;

    /* allocates space for the hash map element to be used */
    struct hash_map_element_t *element;

    /* allocates space for the next value */
    void *next;

    /* allocates space for both the key value and the key
    string value to be used for value retrieval */
    size_t key;
    unsigned char *key_string;

    /* in case the current index excedes the elements
    in the key list (it's the end of iteration) */
    if(current_index >= sort_map->key_list->size) {
        /* sets the next element as null (end of iteration) */
        next = NULL;
    }
    /* otherwise there's still space for retrieval
    of more elements */
    else {
        /* retrieves both the key and the key string values
        to be used to retrieve the hash element */
        get_value_linked_list(sort_map->key_list, current_index, (void **) &key);
        get_value_linked_list(sort_map->key_string_list, current_index, (void **) &key_string);

        /* retrieves the current element from the elements
        buffer */
        get_hash_map(sort_map->hash_map, key, key_string, &element);

        /* sets the next value in iteration as
        the element (next value in iteration) */
        next = (void *) element;

        /* increments the current index value */
        current_index++;
    }

    /* sets the current index in the iterator parameters */
    iterator->parameters = (void *) current_index;

    /* sets the next in the next pointer */
    *next_pointer = next;
}
