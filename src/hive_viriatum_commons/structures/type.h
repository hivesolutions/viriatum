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

#include "../debug/debug.h"
#include "../system/system.h"
#include "hash_map.h"
#include "sort_map.h"
#include "linked_list.h"

/**
 * Union defining a type with all it's
 * possible values, this union approach is
 * inspired by many other dynamic approaches.
 * This union is supposed to be used together
 * with the typ structure.
 */
typedef union type_value_t {
    /**
     * The storage space used for the
     * integer value representation.
     */
    int value_int;

    /**
     * The storage space used for the
     * float value representation.
     */
    float value_float;

    /**
     * The storage space used for the
     * string value representation, this
     * representation is done via a buffer.
     */
    char *value_string;

    /**
     * The storage space used for the
     * hash map (dictionary) value
     * representation.
     */
    struct hash_map_t *value_map;

    /**
     * The storage space used for the
     * sort map (ordered dictionary) value
     * representation.
     */
    struct sort_map_t *value_sort_map;

    /**
     * The storage space used for the
     * linked list (sequence) value
     * representation.
     */
    struct linked_list_t *value_list;

    /**
     * Used to store any other value
     * described as a type.
     * This value must be casted uppon
     * usage.
     */
    void *value_pointer;
} type_value;

/**
 * Enumeration describing the various types
 * (of types) that can be used in the type
 * structure.
 * This types represent both the basic data
 * types and the "extended" ones.
 */
typedef enum type_e {
    INTEGER_TYPE = 1,
    FLOAT_TYPE,
    STRING_TYPE,
    MAP_TYPE,
    SORT_MAP_TYPE,
    LIST_TYPE,
    VOID_TYPE
} _type;

/**
 * Structure that defines a "portable" type
 * that can be interpreted and correctly
 * converted at runtime.
 * This kind of type is inspired by the data
 * structures used in dynamic languages.
 */
typedef struct type_t {
    /**
     * The type of the current type,
     * (eg: integer, float, string).
     */
    enum type_e type;

    /**
     * The value or a pointer to the
     * value of the current type.
     * It's important to interpret the type
     * value before acessing the correct
     * value in the union.
     */
    union type_value_t value;
} type;

VIRIATUM_EXPORT_PREFIX void create_type(struct type_t **type_pointer, enum type_e _type);
VIRIATUM_EXPORT_PREFIX void delete_type(struct type_t *type);
VIRIATUM_EXPORT_PREFIX ERROR_CODE free_type(struct type_t *type);
VIRIATUM_EXPORT_PREFIX ERROR_CODE to_string_type(struct type_t *type, unsigned char **buffer_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE print_type(struct type_t *type);
VIRIATUM_EXPORT_PREFIX struct type_t integer_type(int value);
VIRIATUM_EXPORT_PREFIX struct type_t float_type(float value);
VIRIATUM_EXPORT_PREFIX struct type_t string_type(char *value);
VIRIATUM_EXPORT_PREFIX struct type_t map_type(struct hash_map_t *value);
VIRIATUM_EXPORT_PREFIX struct type_t sort_map_type(struct sort_map_t *value);
VIRIATUM_EXPORT_PREFIX struct type_t list_type(struct linked_list_t *value);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _size_type(struct type_t *type, size_t *size);
