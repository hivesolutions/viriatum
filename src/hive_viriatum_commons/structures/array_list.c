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

#include "array_list.h"

void create_array_list(struct array_list_t **array_list_pointer, size_t element_size, size_t initial_size) {
    /* retrieves the array list size */
    size_t array_list_size = sizeof(struct array_list_t);

    /* allocates space for the array list */
    struct array_list_t *array_list = (struct array_list_t *) MALLOC(array_list_size);

    /* in case the initial size is not set (zero) */
    if((void *) initial_size == 0) {
        /* sets the default initial size value */
        initial_size = DEFAULT_ARRAY_LIST_SIZE;
    }

    /* initializes the array list size */
    array_list->size = 0;

    /* sets the array list element size */
    array_list->element_size = element_size;

    /* sets the elements buffer size */
    array_list->elements_buffer_size = initial_size;

    /* allocates space for the elements buffer */
    array_list->elements_buffer = (void **) MALLOC(element_size * initial_size);

    /* sets the array list in the array list pointer */
    *array_list_pointer = array_list;
}

void delete_array_list(struct array_list_t *array_list) {
    /* releases the array list elements buffer */
    FREE(array_list->elements_buffer);

    /* releases the array list */
    FREE(array_list);
}

void set_array_list(struct array_list_t *array_list, size_t index, void *element) {
    /*  calculates the base index value */
    size_t base_index = array_list->element_size * index;

    /* retrieves the base address value */
    void *base_address = &array_list->elements_buffer[base_index];

    /* copies the element to the array list
    elements buffer, using the element size */
    memcpy(base_address, element, array_list->element_size);
}

void get_array_list(struct array_list_t *array_list, size_t index, void **element_pointer) {
    /*  calculates the base index value */
    size_t base_index = array_list->element_size * index;

    /* retrieves the element from the element buffer */
    void *element = &array_list->elements_buffer[base_index];

    /* sets the element in the element pointer */
    *element_pointer = element;
}
