/*
 Hive Viriatum Commons
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

/**
 * Structure defining a buffer object
 * that is a simple sequence of bytes in
 * memory with an associated size.
 */
typedef struct buffer_t {
    /**
     * The memory pointer to the initial
     * position of the buffer.
     */
    void *pointer;

    /**
     * The size (in bytes) of the memory
     * buffer.
     * The final pointer position is the
     * base pointer plus this size.
     */
    size_t size;
} buffer;

VIRIATUM_EXPORT_PREFIX void create_buffer(struct buffer_t **buffer_pointer, size_t initial_size);
VIRIATUM_EXPORT_PREFIX void delete_buffer(struct buffer_t *buffer);

/**
 * Converts the given buffer into a string representation
 * of it (memory copy of it).
 * The allocated string must be released by the caller
 * function as it becomes the owner of it.
 *
 * @param buffer The buffer object to be converted into the
 * string representation.
 * @return The string representation of the given buffer, this
 * object must be released by the caller function (owner).
 */
VIRIATUM_EXPORT_PREFIX char *to_string_buffer(struct buffer_t *buffer);

static __inline char equals_buffer(struct buffer_t *buffer, struct buffer_t *target) {
    return buffer->size == target->size
        && memcmp(buffer->pointer, target->pointer, buffer->size) == 0;
}
