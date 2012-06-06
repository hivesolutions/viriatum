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

#include "type.h"

void create_type(struct type_t **type_pointer, enum type_e _type) {
    /* retrieves the type size */
    size_t type_size = sizeof(struct type_t);

    /* allocates space for the type */
    struct type_t *type = (struct type_t *) MALLOC(type_size);

    /* sets the type in the type */
    type->type = _type;

    /* sets the type in the type pointer */
    *type_pointer = type;
}

void delete_type(struct type_t *type) {
    /* releases the type */
    FREE(type);
}

ERROR_CODE free_type(struct type_t *type) {
    /* allocats space for the current type in iteration
    for the iterator and for the possible hash map element */
    struct type_t *current;
    struct iterator_t *iterator;
    struct hash_map_element_t *element;

    /* switches over the type's type in order to
    execute the proper free operation */
    switch(type->type) {
        case INTEGER_TYPE:
            /* breaks the switch */
            break;

        case FLOAT_TYPE:
            /* breaks the switch */
            break;

        case STRING_TYPE:
            FREE(type->value.value_string);

            /* breaks the switch */
            break;

        case LIST_TYPE:
            create_iterator_linked_list(type->value.value_list, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &current);
                if(current == NULL) { break; }
                free_type(current);
            }

            delete_iterator_linked_list(type->value.value_list, iterator);
            delete_linked_list(type->value.value_list);

            /* breaks the switch */
            break;

        case MAP_TYPE:
            create_element_iterator_hash_map(type->value.value_map, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                free_type((struct type_t *) element->value);
            }

            delete_iterator_hash_map(type->value.value_map, iterator);
            delete_hash_map(type->value.value_map);

            /* breaks the switch */
            break;

        case SORT_MAP_TYPE:
            create_element_iterator_sort_map(type->value.value_sort_map, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                free_type((struct type_t *) element->value);
            }

            delete_iterator_sort_map(type->value.value_sort_map, iterator);
            delete_sort_map(type->value.value_sort_map);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

    /* deltes the base type structure for the
    current type, this applies to all the types */
    delete_type(type);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE to_string_type(struct type_t *type, unsigned char **buffer_pointer) {
    /* allocates space for both the buffer reference and
    the value to hold the necessary buffer size */
    unsigned char *buffer;
    size_t buffer_size;

    /* retrieves the necessary buffer size for the string
    representation of the given type, then uses the buffer size
    to allocate an appropriate buffer */
    _size_type(type, &buffer_size);
    buffer = (unsigned char *) MALLOC(buffer_size);

    /* switches over the type's type in order to
    execute the proper type conversion */
    switch(type->type) {
        case INTEGER_TYPE:
            /* converts the integer value using the string
            conversion function for integer values */
            SPRINTF((char *) buffer, buffer_size, "%d", type->value.value_int);

            /* breaks the switch */
            break;

        case FLOAT_TYPE:
            /* converts the float value using the string
            conversion function for float values */
            SPRINTF((char *) buffer, buffer_size, "%f", type->value.value_float);

            /* breaks the switch */
            break;

        case STRING_TYPE:
            /* copies the memory buffer of the value in string
            representation to the buffer */
            memcpy(buffer, type->value.value_string, strlen(type->value.value_string) + 1);

            /* breaks the switch */
            break;

        default:
            /* copies the undefined value to the buffer (no conversion
            is possible) */
            memcpy(buffer, "undefined", 10);

            /* breaks the switch */
            break;
    }

    /* sets the buffer pointer reference to the
    value in the buffer */
    *buffer_pointer = buffer;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE print_type(struct type_t *type) {
    /* allocates space for the current type for the
    type to handler the key values for map for the
    possible iterator, for the hash map element and
    for the is first (loop) flag */
    struct type_t *current;
    struct type_t key;
    struct iterator_t *iterator;
    struct hash_map_element_t *element;
    unsigned char is_first = 1;

    /* switches over the type's type in order to
    execute the proper print operation */
    switch(type->type) {
        case INTEGER_TYPE:
            PRINTF_F("%d", type->value.value_int);

            /* breaks the switch */
            break;

        case FLOAT_TYPE:
            PRINTF_F("%f", type->value.value_float);

            /* breaks the switch */
            break;

        case STRING_TYPE:
            PRINTF_F("'%s'", type->value.value_string);

            /* breaks the switch */
            break;

        case LIST_TYPE:
            PRINTF("[");

            create_iterator_linked_list(type->value.value_list, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &current);
                if(current == NULL) { break; }
                if(is_first == 0) { PRINTF(", "); };
                print_type(current);
                is_first = 0;
            }

            delete_iterator_linked_list(type->value.value_list, iterator);

            PRINTF("]");

            /* breaks the switch */
            break;

        case MAP_TYPE:
            PRINTF("{");

            create_element_iterator_hash_map(type->value.value_map, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                if(is_first == 0) { PRINTF(", "); };
                key = string_type((char *) element->key_string);
                print_type(&key);
                PRINTF(" : ");
                print_type((struct type_t *) element->value);
                is_first = 0;
            }

            delete_iterator_hash_map(type->value.value_map, iterator);

            PRINTF("}");

            /* breaks the switch */
            break;

        case SORT_MAP_TYPE:
            PRINTF("{");

            create_element_iterator_sort_map(type->value.value_sort_map, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                if(is_first == 0) { PRINTF(", "); };
                key = string_type((char *) element->key_string);
                print_type(&key);
                PRINTF(" : ");
                print_type((struct type_t *) element->value);
                is_first = 0;
            }

            delete_iterator_sort_map(type->value.value_sort_map, iterator);

            PRINTF("}");

            /* breaks the switch */
            break;


        default:
            PRINTF("undefined");

            /* breaks the switch */
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

struct type_t integer_type(int value) {
    /* allocates space for the type */
    struct type_t type;

    /* sets the type's type and value */
    type.type = INTEGER_TYPE;
    type.value.value_int = value;

    /* returns the type */
    return type;
}

struct type_t float_type(float value) {
    /* allocates space for the type */
    struct type_t type;

    /* sets the type's type and value */
    type.type = FLOAT_TYPE;
    type.value.value_float = value;

    /* returns the type */
    return type;
}

struct type_t string_type(char *value) {
    /* allocates space for the type */
    struct type_t type;

    /* sets the type's type and value */
    type.type = STRING_TYPE;
    type.value.value_string = value;

    return type;
}

struct type_t map_type(struct hash_map_t *value) {
    /* allocates space for the type */
    struct type_t type;

    /* sets the type's type and value */
    type.type = MAP_TYPE;
    type.value.value_map = value;

    /* returns the type */
    return type;
}

struct type_t sort_map_type(struct sort_map_t *value) {
    /* allocates space for the type */
    struct type_t type;

    /* sets the type's type and value */
    type.type = SORT_MAP_TYPE;
    type.value.value_sort_map = value;

    /* returns the type */
    return type;
}

struct type_t list_type(struct linked_list_t *value) {
    /* allocates space for the type */
    struct type_t type;

    /* sets the type's type and value */
    type.type = LIST_TYPE;
    type.value.value_list = value;

    /* returns the type */
    return type;
}

ERROR_CODE _size_type(struct type_t *type, size_t *size) {
    /* switches over the type's type in order to
    execute the proper type conversion */
    switch(type->type) {
        case INTEGER_TYPE:
            /* sets the size considered the maximum for an integer
            value representation */
            *size = 64;

            /* breaks the switch */
            break;

        case FLOAT_TYPE:
            /* sets the size considered the maximum for a float
            value representation */
            *size = 128;

            /* breaks the switch */
            break;

        case STRING_TYPE:
            /* sets the size to the length of the value string
            plus one */
            *size = strlen(type->value.value_string) + 1;

            /* breaks the switch */
            break;

        default:
            /* sets size the default value size */
            *size = 10;

            /* breaks the switch */
            break;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
