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

#include "file_stream.h"

VIRIATUM_EXPORT_PREFIX void create_file_stream(struct file_stream_t **file_stream_pointer, unsigned char *file_path, unsigned char *mode) {
    /* retrieves the file stream size */
    size_t file_stream_size = sizeof(struct file_stream_t);

    /* allocates space for the file stream */
    struct file_stream_t *file_stream = (struct file_stream_t *) MALLOC(file_stream_size);

    /* creates the stream (structure) and sets the
    current file stream as the lower substrate */
    create_stream(&file_stream->stream);
    file_stream->stream->lower = (void *) file_stream;

    /* sets the file parameters in the file stream */
    file_stream->file_path = file_path;
    file_stream->mode = mode;

    /* sets the various functions of the stream */
    file_stream->stream->open = open_file_stream;
    file_stream->stream->close = close_file_stream;
    file_stream->stream->read = read_file_stream;
    file_stream->stream->write = write_file_stream;
    file_stream->stream->flush = flush_file_stream;

    /* sets the file stream in the file stream pointer */
    *file_stream_pointer = file_stream;
}

VIRIATUM_EXPORT_PREFIX void delete_file_stream(struct file_stream_t *file_stream) {
    /* closes the file reference */
    fclose(file_stream->file);

    /* deltes the stream (structure) */
    delete_stream(file_stream->stream);

    /* releases the file stream */
    FREE(file_stream);
}

VIRIATUM_EXPORT_PREFIX struct stream_t *get_stream_file_stream(struct file_stream_t *file_stream) {
    return file_stream->stream;
}

VIRIATUM_EXPORT_PREFIX void open_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;

    /* opens the file reference using the file path and the mode */
    FOPEN(&file_stream->file, (char *) file_stream->file_path, (char *) file_stream->mode);
}

VIRIATUM_EXPORT_PREFIX void close_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;

    /* closes the file reference */
    fclose(file_stream->file);
}

VIRIATUM_EXPORT_PREFIX size_t read_file_stream(struct stream_t *stream, unsigned char *buffer, size_t size) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;

    /* reads the given size of bytes into the buffer */
    return fread(buffer, sizeof(unsigned char), size, file_stream->file);
}

VIRIATUM_EXPORT_PREFIX size_t write_file_stream(struct stream_t *stream, unsigned char *buffer, size_t size) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;

    /* writes the given buffer into the file reference */
    return fwrite(buffer, sizeof(unsigned char), size, file_stream->file);
}

VIRIATUM_EXPORT_PREFIX void flush_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;

    /* flushes the file reference */
    fflush(file_stream->file);
}
