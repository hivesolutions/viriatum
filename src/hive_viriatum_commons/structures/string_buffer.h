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

#include "string.h"
#include "iterator.h"
#include "linked_list.h"

/**
 * Structure holding internal information
 * for a buffer (in memory) of strings.
 * The buffer is linear and contains the various
 * partial string values.
 */
typedef struct string_buffer_t {
    /**
     * The total length for the internal
     * string value.
     */
    size_t string_length;

    /**
     * The list of strings that compose the
     * the complete string value.
     */
    struct linked_list_t *string_list;

    /**
     * The list of lengths for the strings
     * that compose the complete string value.
     */
    struct linked_list_t *length_list;

    /**
     * The list of strings to have the memory
     * released uppon destruction of the string
     * buffer.
     */
    struct linked_list_t *release_list;
} string_buffer;

/**
 * Constructor of the string buffer.
 *
 * @param linked_list_pointer The pointer to the string buffer to be constructed.
 */
VIRIATUM_EXPORT_PREFIX void create_string_buffer(struct string_buffer_t **linked_list_pointer);

/**
 * Destructor of the string buffer.
 *
 * @param string_buffer The string buffer to be destroyed.
 */
VIRIATUM_EXPORT_PREFIX void delete_string_buffer(struct string_buffer_t *string_buffer);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 * The length of the provided string is calculated using the "classic"
 * null terminated string.
 *
 * @param string_buffer The string buffer reference.
 * @param string_value The string value to be added to the string buffer.
 */
VIRIATUM_EXPORT_PREFIX void append_string_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 * The length of the string is provided so no calculation is done.
 *
 * @param string_buffer The string buffer reference.
 * @param string_value The string value to be added to the string buffer.
 * @param string_length The length of the string value to be added to the
 * string buffer.
 */
VIRIATUM_EXPORT_PREFIX void append_string_l_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value, size_t string_length);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 * The string structure is provided so no length calculation is done.
 *
 * @param string_buffer The string buffer reference.
 * @param string The string structure to be added to the string buffer.
 */
VIRIATUM_EXPORT_PREFIX void append_string_t_buffer(struct string_buffer_t *string_buffer, struct string_t *string);

/**
 * "Joins" the internal buffer of strings creating the final
 * string that is returned from the given string value pointer.
 *
 * @param string_buffer The string buffer reference.
 * @param string_value_pointer The pointer to the string value to
 * hold the "joined" string value.
 */
VIRIATUM_EXPORT_PREFIX void join_string_buffer(struct string_buffer_t *string_buffer, unsigned char **string_value_pointer);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 * The length of the provided string is calculated using the "classic"
 * null terminated string.
 * This method releases the memory of the added string value, uppon
 * releasing the string buffer memory.
 * Use this method carefully it may cause memory corruption.
 *
 * @param string_buffer The string buffer reference.
 * @param string_value The string value to be added to the string buffer.
 */
VIRIATUM_EXPORT_PREFIX void _append_string_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 * The length of the string is provided so no calculation is done.
 * This method releases the memory of the added string value, uppon
 * releasing the string buffer memory.
 * Use this method carefully it may cause memory corruption.
 *
 * @param string_buffer The string buffer reference.
 * @param string_value The string value to be added to the string buffer.
 * @param string_length The length of the string value to be added to the
 * string buffer.
 */
VIRIATUM_EXPORT_PREFIX void _append_string_l_buffer(struct string_buffer_t *string_buffer, unsigned char *string_value, size_t string_length);

/**
 * Appends (adds) a string value to the string buffer, this value
 * will be appended to the final "joined" string.
 * The string structure is provided so no length calculation is done.
 * This method releases the memory of the added string value, uppon
 * releasing the string buffer memory.
 * Use this method carefully it may cause memory corruption.
 *
 * @param string_buffer The string buffer reference.
 * @param string The string structure to be added to the string buffer.
 */
VIRIATUM_EXPORT_PREFIX void _append_string_t_buffer(struct string_buffer_t *string_buffer, struct string_t *string);
