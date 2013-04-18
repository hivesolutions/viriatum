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

#include "stdafx.h"

#include "bit_stream.h"

void create_bit_stream(struct bit_stream_t **bit_stream_pointer, struct stream_t *stream) {
    /* retrieves the bit stream size */
    size_t bit_stream_size = sizeof(struct bit_stream_t);

    /* allocates space for the bit stream */
    struct bit_stream_t *bit_stream = (struct bit_stream_t *) MALLOC(bit_stream_size);

    /* calculates the buffer size */
    size_t buffer_size = BIT_STREAM_BUFFER_SIZE * sizeof(unsigned char *);

    /* allocates the buffer for internal usage */
    bit_stream->buffer = (unsigned char *) MALLOC(buffer_size);

    /* sets the (inner) stream reference */
    bit_stream->stream = stream;

    /* sets the various internal structure values of the bit stream */
    bit_stream->size = 0;
    bit_stream->position = 0;
    bit_stream->current_byte_write = 0;
    bit_stream->current_byte_offset_write = 0;
    bit_stream->bit_counter_write = 0;
    bit_stream->byte_counter_write = 0;

    /* sets the bit stream in the bit stream pointer */
    *bit_stream_pointer = bit_stream;
}

void delete_bit_stream(struct bit_stream_t *bit_stream) {
    /* releases the bit stream */
    FREE(bit_stream);
}

void open_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct stream_t *stream = bit_stream->stream;

    /* opens the (inner) stream */
    stream->open(stream);
}

void close_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct stream_t *stream = bit_stream->stream;

    /* flushes the bit stream */
    flush_bit_stream(bit_stream);

    /* closes the (inner) stream */
    stream->close(stream);
}

void read_bit_stream(struct bit_stream_t *bit_stream, unsigned char *buffer, size_t count, size_t *read_count) {
}

void write_bit_stream(struct bit_stream_t *bit_stream, unsigned char *buffer, size_t size) {
}

void write_byte_bit_stream(struct bit_stream_t *bit_stream, unsigned char byte, unsigned char size) {
    /* calculates the number of available bits (count) and
    then used it to calculate the number of extra bits */
    unsigned char available_bits_count = BIT_STREAM_ITEM_SIZE - bit_stream->current_byte_offset_write;
    unsigned char extra_bits_count = size - available_bits_count;

    /* in case the number of extra bits is set the number
    of bits to be writen is the same as the available bits */
    if(size > available_bits_count) {
        /* sets the size as the available bits count */
        size = available_bits_count;
    }
    /* otherwise the number of extra bits is zero */
    else {
        /* resets the extra bits count */
        extra_bits_count = 0;
    }

    /* shifts the current byte value (by the write size) */
    bit_stream->current_byte_write <<= size;

    /* adds the valid bits of the current byte (bits excluding the extra ones)
    to the current byte (shifts the byte the number of extra bits) */
    bit_stream->current_byte_write |= byte >> extra_bits_count;

    /* increments the bit counter and offset by the size */
    bit_stream->bit_counter_write += size;
    bit_stream->current_byte_offset_write += size;

    /* checks the stream for the need to write (flush) */
    flush_write_bit_stream(bit_stream);

    /* in case there are no extra bits to be written
	must return immediately */
    if(extra_bits_count == 0) { return; }

    /* sets the current byte as the extra bits value creates
    a mask to mask the byte and "show" only the extra bits */
    bit_stream->current_byte_write |= byte & ((0x1 << extra_bits_count) - 1);

    /* increments the bit counter and offset by the number of extra bits */
    bit_stream->bit_counter_write += extra_bits_count;
    bit_stream->current_byte_offset_write += extra_bits_count;
}

void flush_write_bit_stream(struct bit_stream_t *bit_stream) {
    /* in case the bit counter (for write) hasn't reached
    the limit no flush is required (no complete byte available)
	and so must return immediately */
    if(bit_stream->current_byte_offset_write != BIT_STREAM_ITEM_SIZE) {
        return;
    }

    /* sets the buffer stream value */
    bit_stream->buffer[bit_stream->byte_counter_write] = bit_stream->current_byte_write;

    /* resets the various bit stream write
    byte oriented structures */
    bit_stream->byte_counter_write++;
    bit_stream->current_byte_offset_write = 0;
    bit_stream->current_byte_write = 0;

    /* in case the stream (buffer) is full */
    if(bit_stream->byte_counter_write == BIT_STREAM_BUFFER_SIZE) {
        /* flushes the bit stream */
        flush_bit_stream(bit_stream);
    }
}

void flush_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct stream_t *stream = bit_stream->stream;

    /* in case there is a (partial) byte waiting to be writen */
    if(bit_stream->current_byte_offset_write > 0) {
        /* sets the write buffer value */
        bit_stream->buffer[bit_stream->byte_counter_write] =\
			bit_stream->current_byte_write << (BIT_STREAM_ITEM_SIZE - bit_stream->current_byte_offset_write);

        /* resets the various bit stream write
        byte oriented structures */
        bit_stream->byte_counter_write++;
        bit_stream->current_byte_offset_write = 0;
        bit_stream->current_byte_write = 0;
    }

    /* flushes the bit stream in the (internal) stream */
    stream->write(stream, bit_stream->buffer, bit_stream->byte_counter_write);

    /* resets the byte counter write (the write buffer
    is also reset by interface) */
    bit_stream->byte_counter_write = 0;
}
