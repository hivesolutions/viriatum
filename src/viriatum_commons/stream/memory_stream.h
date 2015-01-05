/*
 Hive Viriatum Commons
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "io_stream.h"

/**
 * The size (in bytes) value that is going to
 * used for each of the memory allocation calls
 * for both the initial allocation and any other
 * re-allocation calls.
 */
#define SIZE_MEMORY_STREAM 4096

typedef struct memory_stream_t {
    /**
     * The internal buffer that is going to be
     * used for the storage of the data. This
     * would be used for both read and write
     * operations.
     */
    unsigned char *buffer;

    /**
     * The size in bytes of the allocated buffer,
     * this value may change whenever there's a
     * resize operation in the buffer to accommodate
     * more information.
     */
    size_t buffer_size;

    /**
     * The size (in bytes) of the current filled
     * buffer, the position value may never be
     * larger than the size of the filled buffer.
     */
    size_t size;

    /**
     * The current position of the caret inside
     * the buffer, this value would go down and
     * up in case there's a seek operation.
     */
    size_t position;

    /**
     * The reference to the upper stream
     * structure, to be used as refernece.
     */
    struct stream_t *stream;
} memory_stream;

VIRIATUM_EXPORT_PREFIX void create_memory_stream(struct memory_stream_t **memory_stream_pointer);
VIRIATUM_EXPORT_PREFIX void delete_memory_stream(struct memory_stream_t *memory_stream);
VIRIATUM_EXPORT_PREFIX struct stream_t *get_stream_memory_stream(struct memory_stream_t *memory_stream);
VIRIATUM_EXPORT_PREFIX void open_memory_stream(struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX void close_memory_stream(struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX size_t read_memory_stream(struct stream_t *stream, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX size_t write_memory_stream(struct stream_t *stream, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX void flush_memory_stream(struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX void seek_memory_stream(struct stream_t *stream, size_t target);
VIRIATUM_EXPORT_PREFIX size_t size_memory_stream(struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX size_t tell_memory_stream(struct stream_t *stream);
