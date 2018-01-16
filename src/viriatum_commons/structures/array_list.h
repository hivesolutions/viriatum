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

#pragma once

/**
 * The default size to be used in a newly
 * created array list.
 */
#define DEFAULT_ARRAY_LIST_SIZE 256

typedef struct array_list_t {
    size_t size;
    size_t element_size;
    size_t elements_buffer_size;
    void **elements_buffer;
} array_list;

VIRIATUM_EXPORT_PREFIX void create_array_list(struct array_list_t **array_list_pointer, size_t value_size, size_t initial_size);
VIRIATUM_EXPORT_PREFIX void delete_array_list(struct array_list_t *array_list);
VIRIATUM_EXPORT_PREFIX void set_array_list(struct array_list_t *array_list, size_t index, void *element);
VIRIATUM_EXPORT_PREFIX void get_array_list(struct array_list_t *array_list, size_t index, void **element_pointer);
