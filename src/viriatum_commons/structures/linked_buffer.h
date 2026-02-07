/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "buffer.h"
#include "iterator.h"
#include "linked_list.h"

/**
 * Structure holding internal information
 * for a buffer (in memory) of buffers.
 * The buffer is linear and contains the various
 * partial buffer values.
 */
typedef struct linked_buffer_t {
    /**
     * The total length for the internal
     * buffers value.
     */
    size_t buffer_length;

    /**
     * The list of buffers that compose the
     * the complete buffer value.
     */
    struct linked_list_t *buffer_list;

    /**
     * The list of buffers to have the memory
     * released uppon destruction of the linked
     * buffer.
     */
    struct linked_list_t *release_list;
} linked_buffer;

VIRIATUM_EXPORT_PREFIX void create_linked_buffer(struct linked_buffer_t **linked_buffer_pointer);
VIRIATUM_EXPORT_PREFIX void delete_linked_buffer(struct linked_buffer_t *linked_buffer);
VIRIATUM_EXPORT_PREFIX void append_linked_buffer(struct linked_buffer_t *linked_buffer, void *pointer, size_t size, unsigned char release);
VIRIATUM_EXPORT_PREFIX void join_linked_buffer(struct linked_buffer_t *linked_buffer, unsigned char **buffer_value_pointer);
VIRIATUM_EXPORT_PREFIX void _append_linked_buffer(struct linked_buffer_t *linked_buffer, void *pointer, size_t size);
