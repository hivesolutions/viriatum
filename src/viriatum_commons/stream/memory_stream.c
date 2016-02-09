/*
 Hive Viriatum Commons
 Copyright (c) 2008-2016 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "memory_stream.h"

void create_memory_stream(struct memory_stream_t **memory_stream_pointer) {
    /* calculates the size of the memory stream structure and
    then uses this value to allocate  */
    size_t memory_stream_size = sizeof(struct memory_stream_t);
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) MALLOC(memory_stream_size);

    /* creates the stream (structure) and sets the
    current memory stream as the lower substrate */
    create_stream(&memory_stream->stream);
    memory_stream->stream->lower = (void *) memory_stream;

    /* sets the memory parameters in the memory stream, the values
    are set with the original (unset) values */
    memory_stream->buffer = NULL;
    memory_stream->buffer_size = 0;
    memory_stream->size = 0;
    memory_stream->position = 0;

    /* sets the various functions of the stream */
    memory_stream->stream->open = open_memory_stream;
    memory_stream->stream->close = close_memory_stream;
    memory_stream->stream->read = read_memory_stream;
    memory_stream->stream->write = write_memory_stream;
    memory_stream->stream->flush = flush_memory_stream;
    memory_stream->stream->seek = seek_memory_stream;
    memory_stream->stream->size = size_memory_stream;
    memory_stream->stream->tell = tell_memory_stream;

    /* sets the memory stream in the memory stream pointer */
    *memory_stream_pointer = memory_stream;
}

void delete_memory_stream(struct memory_stream_t *memory_stream) {
    /* in case the internal buffer is defined its memory
    must be released and then releases the memory of the
    proper memory stream structure (avoid memory leaks) */
    if(memory_stream->buffer) { FREE(memory_stream->buffer); }
    FREE(memory_stream);
}

struct stream_t *get_stream_memory_stream(struct memory_stream_t *memory_stream) {
    return memory_stream->stream;
}

void open_memory_stream(struct stream_t *stream) {
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    size_t buffer_size = SIZE_MEMORY_STREAM * sizeof(unsigned char);
    memory_stream->buffer = (unsigned char *) MALLOC(buffer_size);
    memory_stream->buffer_size = SIZE_MEMORY_STREAM;
    memory_stream->size = 0;
    memory_stream->position = 0;
}

void close_memory_stream(struct stream_t *stream) {
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    if(memory_stream->buffer) { FREE(memory_stream->buffer); }
    memory_stream->buffer = NULL;
    memory_stream->buffer_size = 0;
    memory_stream->size = 0;
    memory_stream->position = 0;
}

size_t read_memory_stream(struct stream_t *stream, unsigned char *buffer, size_t size) {
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    size_t remaining = memory_stream->size - memory_stream->position;
    size_t count = size > remaining ? remaining : size;
    unsigned char *pointer = &memory_stream->buffer[memory_stream->position];

    memcpy(buffer, pointer, count);
    memory_stream->position += count;

    return count;
}

size_t write_memory_stream(struct stream_t *stream, unsigned char *buffer, size_t size) {
    size_t extra;
    size_t allocation;
    unsigned char *pointer;
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    size_t remaining = memory_stream->buffer_size - memory_stream->position;

    if(size > remaining) {
        extra = size - remaining;
        allocation = extra > SIZE_MEMORY_STREAM ? extra : SIZE_MEMORY_STREAM;
        memory_stream->buffer_size += allocation;
        memory_stream->buffer =    REALLOC(
            memory_stream->buffer,
            memory_stream->buffer_size
        );
    }

    pointer = &memory_stream->buffer[memory_stream->position];
    memcpy(pointer, buffer, size);
    memory_stream->position += size;
    memory_stream->size += size;

    return size;
}

void flush_memory_stream(struct stream_t *stream) {
}

void seek_memory_stream(struct stream_t *stream, size_t target) {
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    memory_stream->position = target;
}

size_t size_memory_stream(struct stream_t *stream) {
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    return memory_stream->size;
}

size_t tell_memory_stream(struct stream_t *stream) {
    struct memory_stream_t *memory_stream =\
        (struct memory_stream_t *) stream->lower;
    return memory_stream->position;
}
