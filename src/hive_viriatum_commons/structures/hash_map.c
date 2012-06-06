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

#include "hash_map.h"

void create_hash_map(struct hash_map_t **hash_map_pointer, size_t initial_size) {
    /* retrieves the hash map size */
    size_t hash_map_size = sizeof(struct hash_map_t);

    /* retrives the hash map element size */
    size_t hash_map_element_size = sizeof(struct hash_map_element_t);

    /* allocates space for the hash map */
    struct hash_map_t *hash_map = (struct hash_map_t *) MALLOC(hash_map_size);

    /* in case the initial size is not set (zero) */
    if((void *) initial_size == 0) {
        /* sets the default initial size value */
        initial_size = DEFAULT_HASH_MAP_SIZE;
    }

    /* initializes the hash map size */
    hash_map->size = 0;

    /* sets the maximum size of the hash map */
    hash_map->maximum_size = (size_t) ((double) initial_size * DEFAULT_MAXIMUM_LOAD_FACTOR);

    /* sets the hash map element size */
    hash_map->element_size = hash_map_element_size;

    /* sets the elements buffer size */
    hash_map->elements_buffer_size = initial_size;

    /* allocates space for the elements buffer */
    hash_map->elements_buffer = (struct hash_map_element_t *) MALLOC(hash_map_element_size * initial_size);

    /* resets the elements buffer value */
    memset(hash_map->elements_buffer, 0, hash_map_element_size * initial_size);

    /* sets the hash map in the hash map pointer */
    *hash_map_pointer = hash_map;
}

void delete_hash_map(struct hash_map_t *hash_map) {
    /* allocates space for the index accumulator to be used
    for the iteration and for the temporary element */
    size_t index;
    struct hash_map_element_t *element;

    /* iterates over all the elements in the hash map to
    release the memory present in the buffers */
    for(index = 0; index < hash_map->elements_buffer_size; index++) {
        /* retrieves the base address value */
        element = &hash_map->elements_buffer[index];

        /* in case the element is not used or in case
        the key string is not set no need to release
        the allocated memory (no memory used) */
        if(element->used == 0) { continue; }
        if(element->key_string == NULL) { continue; }

        /* releases the key string buffer, to avoid any
        possible memory leak */
        FREE(element->key_string);
    }

    /* releases the elements buffer */
    FREE(hash_map->elements_buffer);

    /* releases the hash map */
    FREE(hash_map);
}

void set_value_hash_map(struct hash_map_t *hash_map, size_t key, unsigned char *key_string, void *value) {
    /* allocates space for the hash map element to be used */
    struct hash_map_element_t *element;

    /* allocates space for the index used for element
    access (computed modulus hash) and for the size of
    the key string to be set in the element */
    size_t index;
    size_t key_string_size;

    /* in case the current hash map size "overflows"
    the maximum size (a resizing is required) */
    if(hash_map->size >= hash_map->maximum_size) {
        /* resizes the hash map to an appropriate
        size to avoid collisions */
        _resize_hash_map(hash_map);
    }

    /* calculates the index using the modulus */
    index = key % hash_map->elements_buffer_size;

    /* iterates continously (to get an empty space
    in the hash map), this conforms with the open
    addressing strategy for hash maps */
    while(1) {
        /* retrieves the base address value */
        element = &hash_map->elements_buffer[index];

        /* in case the element is empty or the
        key is the same (overwrite) */
        if(element->used == 0 || element->key == key) {
            /* breaks the loop */
            break;
        }

        /* in case the index value is "normal"
        and sane (normal case) */
        if(index < hash_map->elements_buffer_size - 1) {
            /* increment the index value */
            index++;
        }
        /* otherwise the hash map overflows (need
        to reset the counter value) */
        else {
            /* resets the index value (overflow) */
            index = 0;
        }
    }

    /* in case the key string is already defined in the
    element (must release it properly) unsets it */
    if(element->key_string != NULL) {
        /* releases the key string memory (avoids memory leak)
        and then unsets the key string reference in the element */
        FREE(element->key_string);
        element->key_string = NULL;
    }

    /* sets the element fields */
    element->value = value;
    element->key = key;
    element->used = 1;

    /* in case the key string is defined must copy
    the string information into the element */
    if(key_string != NULL) {
        /* allocates the required memory for the key string
        and then copies the key string into the element */
        key_string_size = strlen((char *) key_string);
        element->key_string = (unsigned char *) MALLOC(key_string_size + 1);
        memcpy(element->key_string, key_string, key_string_size + 1);
    }

    /* increments the hash map size */
    hash_map->size++;
}

void set_value_string_hash_map(struct hash_map_t *hash_map, unsigned char *key_string, void *value) {
    /* calculates the key (hash) value from the key string
    and uses it to set the value in the hash map */
    size_t key = _calculate_string_hash_map(key_string);
    set_value_hash_map(hash_map, key, key_string, value);
}

void get_hash_map(struct hash_map_t *hash_map, size_t key, unsigned char *key_string, struct hash_map_element_t **element_pointer) {
    /* allocates space for the hash map element to be used */
    struct hash_map_element_t *element;

    /* calculates the index using the modulus */
    size_t index = key % hash_map->elements_buffer_size;

    /* iterates continously (to retrieve the appropriate
    element in the hash map), this conforms with the open
    addressing strategy for hash maps */
    while(1) {
        /* retrieves the base address value */
        element = &hash_map->elements_buffer[index];

        /* in case the element is not used, the element
        search is over (element was not found) */
        if(element->used == 0) {
            /* breaks the loop */
            break;
        }

        /* in case the element key is the same as the
        requested (element found) note that an extra
        verification is done to make sure that in case
        a string key is specified it also matches */
        if(element->key == key && (key_string == NULL || strcmp((char *) element->key_string, (char *) key_string) == 0)) {
            /* breaks the loop */
            break;
        }

        /* in case the index value is "normal"
        and sane (normal case) */
        if(index < hash_map->elements_buffer_size - 1) {
            /* increment the index value */
            index++;
        }
        /* otherwise the hash map overflows (need
        to reset the counter value) */
        else {
            /* resets the index value (overflow) */
            index = 0;
        }
    }

    /* sets the element in the element pointer */
    *element_pointer = element;
}

void get_value_hash_map(struct hash_map_t *hash_map, size_t key, unsigned char *key_string, void **value_pointer) {
    /* allocates space for the element */
    struct hash_map_element_t *element;

    /* retrieves the hash map element for the key */
    get_hash_map(hash_map, key, key_string, &element);

    /* sets the element value in the value pointer */
    *value_pointer = element->value;
}

void get_value_string_hash_map(struct hash_map_t *hash_map, unsigned char *key_string, void **value_pointer) {
    /* calculates the key (hash) value from the key string
    and uses it to retrieve the value from the hash map */
    size_t key = _calculate_string_hash_map(key_string);
    get_value_hash_map(hash_map, key, key_string, value_pointer);
}

void create_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t **iterator_pointer) {
    /* allocates the iterator */
    struct iterator_t *iterator;

    /* creates the iterator */
    create_iterator(&iterator);

    /* sets the hash map in the structure */
    iterator->structure = (void *) hash_map;

    /* sets the get next function in the iterator */
    iterator->get_next_function = get_next_iterator_hash_map;

    /* resets the iterator */
    reset_iterator_hash_map(hash_map, iterator);

    /* sets the iterator in the iterator pointer */
    *iterator_pointer = iterator;
}

void create_element_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t **iterator_pointer) {
    /* allocates the iterator */
    struct iterator_t *iterator;

    /* creates the iterator */
    create_iterator(&iterator);

    /* sets the hash map in the structure */
    iterator->structure = (void *) hash_map;

    /* sets the get next function in the iterator */
    iterator->get_next_function = get_next_element_iterator_hash_map;

    /* resets the iterator */
    reset_iterator_hash_map(hash_map, iterator);

    /* sets the iterator in the iterator pointer */
    *iterator_pointer = iterator;
}

void delete_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t *iterator) {
    /* deletes the iterator */
    delete_iterator(iterator);
}

void reset_iterator_hash_map(struct hash_map_t *hash_map, struct iterator_t *iterator) {
    /* sets the iterator parameters as the initial index of
    the elements buffer in the hash map */
    iterator->parameters = 0;
}

void get_next_iterator_hash_map(struct iterator_t *iterator, void **next_pointer) {
    /* retrieves the hash map associated with the iterator */
    struct hash_map_t *hash_map = (struct hash_map_t *) iterator->structure;

    /* retrieves the current index offset in the elements buffer  */
    size_t current_index = (size_t) iterator->parameters;

    /* allocates space for the hash map element to be used */
    struct hash_map_element_t *element;

    /* allocates space for the next value */
    void *next;

    /* iterates continuously */
    while(1) {
        /* in case the current index excedes the elements
        buffer size (it's the end of iteration) */
        if(current_index >= hash_map->elements_buffer_size) {
            /* sets the next element as null (end of iteration) */
            next = NULL;

            /* breaks the cycle (nothing more to process) */
            break;
        }

        /* retrieves the current element from the elements
        buffer */
        element = &hash_map->elements_buffer[current_index];

        /* increments the current index value */
        current_index++;

        /* in case the current element is used it is ready
        to be retrieved as next value in iteration */
        if(element->used == 1) {
            /* sets the next value in iteration as
            the element key (next value in iteration) */
            next = (void *) &element->key;

            /* breaks the cycle (found the value) */
            break;
        }
    }

    /* sets the current index in the iterator parameters */
    iterator->parameters = (void *) current_index;

    /* sets the next in the next pointer */
    *next_pointer = next;
}

void get_next_element_iterator_hash_map(struct iterator_t *iterator, void **next_pointer) {
    /* retrieves the hash map associated with the iterator */
    struct hash_map_t *hash_map = (struct hash_map_t *) iterator->structure;

    /* retrieves the current index offset in the elements buffer  */
    size_t current_index = (size_t) iterator->parameters;

    /* allocates space for the hash map element to be used */
    struct hash_map_element_t *element;

    /* allocates space for the next value */
    void *next;

    /* iterates continuously */
    while(1) {
        /* in case the current index excedes the elements
        buffer size (it's the end of iteration) */
        if(current_index >= hash_map->elements_buffer_size) {
            /* sets the next element as null (end of iteration) */
            next = NULL;

            /* breaks the cycle (nothing more to process) */
            break;
        }

        /* retrieves the current element from the elements
        buffer */
        element = &hash_map->elements_buffer[current_index];

        /* increments the current index value */
        current_index++;

        /* in case the current element is used it is ready
        to be retrieved as next value in iteration */
        if(element->used == 1) {
            /* sets the next value in iteration as
            the element (next value in iteration) */
            next = (void *) element;

            /* breaks the cycle (found the value) */
            break;
        }
    }

    /* sets the current index in the iterator parameters */
    iterator->parameters = (void *) current_index;

    /* sets the next in the next pointer */
    *next_pointer = next;
}

void _resize_hash_map(struct hash_map_t *hash_map) {
    /* allocates space for the index */
    size_t index = 0;

    /* allocates space for the hash map element to be used
    for the copy of the elements data */
    struct hash_map_element_t *element;

    /* allocates space and copies the "old" elements
    buffer and size (backup of elements buffer) */
    struct hash_map_element_t *elements_buffer = hash_map->elements_buffer;
    size_t elements_buffer_size = hash_map->elements_buffer_size;

    /* resets the hash map size to zero (no elements inserted) */
    hash_map->size = 0;

    /* increments the elements buffer size by the default
    resize factor and creates the new memory buffer for them */
    hash_map->elements_buffer_size *= DEFAULT_RESIZE_FACTOR;
    hash_map->elements_buffer = (struct hash_map_element_t *) MALLOC(hash_map->element_size * hash_map->elements_buffer_size);

    /* re-calculates the maximum size that elements buffer may
    assume before resizing */
    hash_map->maximum_size = (size_t) ((double) hash_map->elements_buffer_size * DEFAULT_MAXIMUM_LOAD_FACTOR);

    /* resets the elements buffer value */
    memset(hash_map->elements_buffer, 0, hash_map->element_size * hash_map->elements_buffer_size);

    /* iterates over all the "old" elements to copy
    and set them in the new hash map buffer */
    for(index = 0; index < elements_buffer_size; index++) {
        /* retrieves the current element structure from
        the elements buffer to set the element key and value
        in the "newly" resized hash map */
        element = &elements_buffer[index];

        /* in case the element is not "used", there's
        no need to set the value in the hash map*/
        if(element->used == 0) {
            /* continues the loop */
            continue;
        }

        /* sets the value (key and value) in the hash map (copy step) */
        set_value_hash_map(hash_map, element->key, element->key_string, element->value);
    }

    /* releases the memory used by the "old" elements
    buffer (avoids possible memory leak) */
    FREE(elements_buffer);
}

size_t _calculate_string_hash_map(unsigned char *key_string) {
    /* creates the value to hold the current character
    in iteration */
    unsigned char current;

    /* creates the value to hold the current key value
    and the current index accumulator */
    size_t key = 0;
    unsigned int index = 0;

    /* in case the key string is invalid or in
    case it's empty */
    if(key_string == NULL || key_string[0] == '\0') {
        /* returns the key immediately as zero */
        return 0;
    }

    /* calculates the initial key value and
    increments the initial index value */
    key = key_string[0] << 7;
    index++;

    /* iterates continuously for (integer) key calculation */
    while(1) {
        /* retrievs the current character from the
        the key string value */
        current = key_string[index];

        /* in case the current value is end of string */
        if(current == '\0') {
            /* breaks the loop */
            break;
        }

        /* re-calculates the key value based uppon the current
        character value in loop */
        key = (1000003 * key) ^ current;

        /* increments the index value (iteration) */
        index++;
    }

    /* closes the key value calculation */
    key = key ^ index;

    /* checks for aditional abnormal key value */
    if(key == -1) {
        /* removes the error code value */
        key = -2;
    }

    /* returns the (calculated) key */
    return key;
}
