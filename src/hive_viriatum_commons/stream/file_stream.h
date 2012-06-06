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

#include "../system/system.h"
#include "io_stream.h"

typedef struct file_stream_t {
    /**
     * The reference to the file object.
     */
    FILE *file;

    /**
     * The path to the file to be used
     * in the stream.
     */
    unsigned char *file_path;

    /**
     * The mode to be used in the opening
     * of the file.
     */
    unsigned char *mode;

    /**
     * The reference to the upper stream
     * structure.
     */
    struct stream_t *stream;
} file_stream;

VIRIATUM_EXPORT_PREFIX void create_file_stream(struct file_stream_t **file_stream_pointer, unsigned char *file_path, unsigned char *mode);
VIRIATUM_EXPORT_PREFIX void delete_file_stream(struct file_stream_t *file_stream);
VIRIATUM_EXPORT_PREFIX struct stream_t *get_stream_file_stream(struct file_stream_t *file_stream);
VIRIATUM_EXPORT_PREFIX void open_file_stream(struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX void close_file_stream(struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX size_t read_file_stream(struct stream_t *stream, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX size_t write_file_stream(struct stream_t *stream, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX void flush_file_stream(struct stream_t *stream);
