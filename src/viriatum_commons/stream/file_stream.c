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

#include "stdafx.h"

#include "file_stream.h"

void create_file_stream(
    struct file_stream_t **file_stream_pointer,
    unsigned char *file_path,
    unsigned char *mode
) {
    /* retrieves the file stream size */
    size_t file_stream_size = sizeof(struct file_stream_t);

    /* allocates space for the file stream */
    struct file_stream_t *file_stream = (struct file_stream_t *) MALLOC(file_stream_size);

    /* creates the stream (structure) and sets the
    current file stream as the lower substrate */
    create_stream(&file_stream->stream);
    file_stream->stream->lower = (void *) file_stream;

    /* sets the file parameters in the file stream including
    the initialization of the file object with invalid value */
    file_stream->file = NULL;
    file_stream->file_path = file_path;
    file_stream->mode = mode;

    /* sets the various functions of the stream */
    file_stream->stream->open = open_file_stream;
    file_stream->stream->close = close_file_stream;
    file_stream->stream->read = read_file_stream;
    file_stream->stream->write = write_file_stream;
    file_stream->stream->flush = flush_file_stream;
    file_stream->stream->seek = seek_file_stream;
    file_stream->stream->size = size_file_stream;
    file_stream->stream->tell = tell_file_stream;

    /* sets the file stream in the file stream pointer */
    *file_stream_pointer = file_stream;
}

void delete_file_stream(
    struct file_stream_t *file_stream
) {
    /* closes the file reference in case the file is
    currently open then deletes the underlying stream reference
    and then releases the current structure */
    if(file_stream->file) { fclose(file_stream->file); }
    delete_stream(file_stream->stream);
    FREE(file_stream);
}

struct stream_t *get_stream_file_stream(struct file_stream_t *file_stream) {
    return file_stream->stream;
}

void open_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lower substrate) and
    then uses the structure to open the associated file object */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    FOPEN(
        &file_stream->file,
        (char *) file_stream->file_path,
        (char *) file_stream->mode
    );
}

void close_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lower substrate)
    and uses the structure reference to close the currently opened file */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    fclose(file_stream->file);
    file_stream->file = NULL;
}

size_t read_file_stream(struct stream_t *stream, unsigned char *buffer, size_t size) {
    /* retrieves the file stream from the stream (as the lowee substrate)
    and reads the amount of data requested from the currently open file */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    return fread(buffer, sizeof(unsigned char), size, file_stream->file);
}

size_t write_file_stream(struct stream_t *stream, unsigned char *buffer, size_t size) {
    /* retrieves the file stream from the stream (as the lower substrate)
    and then writes the provided data to the underlying file reference */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    return fwrite(buffer, sizeof(unsigned char), size, file_stream->file);
}

void flush_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lower substrate)
    and flushes the current contents to the underlying file object */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    fflush(file_stream->file);
}

void seek_file_stream(struct stream_t *stream, size_t target) {
    /* retrieves the file stream from the stream (as the lower substrate)
    and then seeks the target object into the desired position */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    fseek(file_stream->file, target, SEEK_SET);
}

size_t size_file_stream(struct stream_t *stream) {
    /* allocates the space for the variable that is going
    to be used to store the final size value */
    size_t size;

    /* retrieves the current index position so that it's
    possible to restore it after the measure of the file
    size, this is required otherwise corruption would occur */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    size_t current = (size_t) ftell(file_stream->file);

    /* seeks the file to the end of the tream and measures
    the current position as the size of it, then restores the
    current position back to the file pointer and returns the
    size value to the caller function (result value) */
    fseek(file_stream->file, 0, SEEK_END);
    size = (size_t) ftell(file_stream->file);
    fseek(file_stream->file, current, SEEK_SET);
    return size;
}

size_t tell_file_stream(struct stream_t *stream) {
    /* retrieves the file stream from the stream (as the lower substrate)
    and then suses it to "tell" the current byte position in the stream */
    struct file_stream_t *file_stream = (struct file_stream_t *) stream->lower;
    return (size_t) ftell(file_stream->file);
}
