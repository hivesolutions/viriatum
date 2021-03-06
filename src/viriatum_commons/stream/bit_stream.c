/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "bit_stream.h"

static unsigned char masks[9] = {
    0x00,
    0x01,
    0x03,
    0x07,
    0x0f,
    0x1f,
    0x3f,
    0x7f,
    0xff
};

void create_bit_stream(struct bit_stream_t **bit_stream_pointer, struct stream_t *stream) {
    /* retrieves the bit stream size and uses it to
    allocate the "new" bit stream structure */
    size_t bit_stream_size = sizeof(struct bit_stream_t);
    struct bit_stream_t *bit_stream = (struct bit_stream_t *) MALLOC(bit_stream_size);

    /* calculates the buffer size using the pre-defined
    default value and then uses the value to allocate the
    buffer that is going to be used */
    size_t buffer_size = BIT_STREAM_BUFFER_SIZE * sizeof(unsigned char *);
    bit_stream->buffer = (unsigned char *) MALLOC(buffer_size);

    /* sets the (inner) stream reference, this is the refernece
    is going to be used in the flushing and writing of data */
    bit_stream->stream = stream;

    /* sets the various internal structure values of the bit stream */
    bit_stream->size = 0;
    bit_stream->position = 0;
    bit_stream->current_byte_read = 0;
    bit_stream->current_byte_write = 0;
    bit_stream->current_byte_offset_read = 0;
    bit_stream->current_byte_offset_write = 0;
    bit_stream->bit_counter_read = 0;
    bit_stream->bit_counter_write = 0;
    bit_stream->byte_counter_read = 0;
    bit_stream->byte_current_read = 0;
    bit_stream->byte_counter_write = 0;

    /* sets the bit stream in the bit stream pointer */
    *bit_stream_pointer = bit_stream;
}

void delete_bit_stream(struct bit_stream_t *bit_stream) {
    /* releases the bit stream */
    FREE(bit_stream);
}

void open_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream
    and opens it because it's going to be used in the
    operation for both read and write */
    struct stream_t *stream = bit_stream->stream;
    stream->open(stream);
}

void close_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream
    and then flushes the bit stream so that there's no
    more pending data and closes the stream to avoid any
    extra operation to be performed in the stream */
    struct stream_t *stream = bit_stream->stream;
    _write_bit_stream(bit_stream);
    stream->close(stream);
}

void read_bit_stream(
    struct bit_stream_t *bit_stream,
    unsigned char *buffer,
    size_t count,
    size_t *read_count
) {
}

void write_bit_stream(
    struct bit_stream_t *bit_stream,
    unsigned char *buffer,
    size_t size
) {
}

void seek_bit_stream(
    struct bit_stream_t *bit_stream,
    long long size
) {
    /* allocates space for the varioables that are going to be
    used in case a stream seek operation is required */
    size_t position;
    size_t diff_read;
    size_t total_read;
    struct stream_t *stream;

    /* allocates space for temporary variables that are going to be
    uses in the computation of the byte counting and updateing flags */
    unsigned char need_read;
    unsigned char offset_first;
    long long byte_count;

    /* calculates the amount of available bits in the current
    byte and then uses the value to calculate the remaining ammoutn
    of bits outside the current byte in the seek */
    long long available = BIT_STREAM_ITEM_SIZE - bit_stream->current_byte_offset_read;
    long long remaining = size - available;

    /* in case the amount of bits to be seek is zero returns immediately
    as there's nothing to be seeked for that situation */
    if(size == 0) { return; }

    /* decrements the current vounter for bits that were read
    by the amount of bits that are going to be "seeked" */
    bit_stream->bit_counter_read -= (size_t) size;

    /* in case the size is smaller or the same as the number
    of bits alredy read the situation is simple as the same
    byte is going to be used and only the offset is required
    to be changed (for the current byte) */
    if(remaining < 0) {
        bit_stream->current_byte_offset_read += (unsigned char) size;
    }
    /* otherwise the oepration is complex as it spans multiple
    bytes, it may or may not require reading from the underlying
    stream but it will always require changing the current byte */
    else {
        /* calulates the offset to the firt byte (target byte) with
        the modulus of the remaining bts and the size of the item */
        offset_first = remaining % BIT_STREAM_ITEM_SIZE;

        /* calculates the amount of bytes that are required by
        deviding the remaining value by eight (done using shifts)
        and adding one extra byte (the one currently in iteration) */
        byte_count = (remaining >> 3) + 1;

        /* in case the offset in the first byte is zero an extra
        shifting operation is required as the byte count is reduced
        and the offset in the first byte is repositioned at the begining
        of the byte as the carret is positioned at the begining */
        if(offset_first == 0) {
            byte_count--;
            offset_first = BIT_STREAM_ITEM_SIZE;
        }

        /* verifies if the current seek operation required reading from
        the stream, this is required when the amount of bytes to be
        skipped back is greater than the amount of bytes that have been
        already read from the current read buffer */
        need_read = byte_count >= bit_stream->byte_current_read;

        /* in case there's a need to read from the underlying stram the
        operation should reset and move the buffer and reset the counters */
        if(need_read) {
            /* calculates the size of the current read buffer by adding the
            current byte (index) to the counter and then calculates the diff
            to the initial position of the stream by calculating the diference
            between the current index position and the number of bytes to be
            set back plus one (this is the distance to the initial byte in
            the last read operation) */
            total_read = bit_stream->byte_current_read + bit_stream->byte_counter_read;
            diff_read = bit_stream->byte_current_read - (size_t) byte_count + 1;

            /* retrieves the reference to the underlying stream and uses it to
            gather its current position and recalculates the new position using
            the calculated diff then uses this value to update the stream position */
            stream = bit_stream->stream;
            position = stream->tell(stream);
            position -= total_read + diff_read;
            stream->seek(stream, position);

            /* reads the default (pre-defined) value of bytes from the stream and updated
            the counter and the current index values accordingly taking already into account
            that a new value is going to be read from the buffer */
            bit_stream->byte_counter_read = stream->read(
                stream,
                bit_stream->buffer,
                BIT_STREAM_BUFFER_SIZE
            ) - 1;
            bit_stream->byte_current_read = 1;
        }
        /* otherwise the only the read buffer is moved back without having to
        perform a (slow) read operation in the stream */
        else {
            /* decrements the current byte for read by the number of bytes
            and increments the counter by the same amount (buffer move) */
            bit_stream->byte_current_read -= (unsigned char) byte_count;
            bit_stream->byte_counter_read += (unsigned char) byte_count;
        }

        /* reads the new current byte value from the buffer with the current
        index minus one (previous byte) and then updates the offset inside the
        current byte by the calculated offset in the first byte of the seek */
        bit_stream->current_byte_read = bit_stream->buffer[bit_stream->byte_current_read - 1];
        bit_stream->current_byte_offset_read = offset_first;
    }
}

void read_word_bit_stream(
    struct bit_stream_t *bit_stream,
    unsigned short *word,
    unsigned char size
) {
    unsigned char upper = 0;
    unsigned char lower = 0;
    short upper_s = size - BIT_STREAM_ITEM_SIZE;
    short lower_s = size > BIT_STREAM_ITEM_SIZE ? BIT_STREAM_ITEM_SIZE : size;

    if(upper_s > 0) { read_byte_bit_stream(bit_stream, &upper, (unsigned char) upper_s); };
    if(lower_s > 0) { read_byte_bit_stream(bit_stream, &lower, (unsigned char) lower_s); };

    *word = lower + (upper << BIT_STREAM_ITEM_SIZE);
}

void read_byte_bit_stream(
    struct bit_stream_t *bit_stream,
    unsigned char *byte,
    unsigned char size
) {
    /* allocates space for the various variable that are going to
    be used for the reading of the byte operations */
    unsigned char available_bits_count;
    unsigned char extra_bits_count;
    register unsigned char shift_v;

    /* in case the requested size is zero this is a special case
    and so the byte is set to zero and the control flow is returned
    immediately to the caller function */
    if(size == 0) { *byte = 0; return; }

    /* runs the initial read flush operation in the bit stream, because
    there may be some pending read operations pending from previous
    operations (without this data corruption may occur) */
    flush_read_bit_stream(bit_stream);

    /* sets the amount of available bits inside the current byte
    as the current offset (counts from the right) and then calculates
    the amount of extra bits outsie the current byte using the value */
    available_bits_count = bit_stream->current_byte_offset_read;
    extra_bits_count = size - available_bits_count;

    /* in case the number of bits to be read is greater
    than the available number of bits for the current byte
    need to append some extra bits */
    if(size > available_bits_count) { size = available_bits_count; }

    /* otherwise the number of extra bits is zero, the value
    must then be reset to the invalid (zero value) */
    else { extra_bits_count = 0; }

    /* calculates the amount of bits that are going to be used in
    the shift operation of the current byte and then uses it to move
    the byte to the right and masks the left part of it using a pre-defined
    set of masks selected according to the current bit size */
    shift_v = bit_stream->current_byte_offset_read - size;
    *byte = (bit_stream->current_byte_read >> shift_v) & masks[size];

    /* increments the global bit counter and the current byte offset by
    the size of the currently written bits */
    bit_stream->bit_counter_read += size;
    bit_stream->current_byte_offset_read -= size;

    /* in case there are no extra bits to be read must
    return immediately nothing remaining to be done */
    if(extra_bits_count == 0) { return; }

    /* runs the read flush operation as it may be required to read a new
    byte from the pseudo-read stream in order to be able to process the
    extra bits that are going to be read */
    flush_read_bit_stream(bit_stream);

    /* calculates the shift that is going to be done in the byte and
    then shifts the current byte by the already read bits and adds the
    extra bits to it using the "just" calculated shift amount (value) */
    shift_v = bit_stream->current_byte_offset_read - extra_bits_count;
    *byte <<= extra_bits_count;
    *byte |= (bit_stream->current_byte_read >> shift_v);

    /* increments the bit counter and offset by the number of extra bits
    that have just been read from the new byte and then dcrements the offset
    in the current byte by the same amount (extra read bits) */
    bit_stream->bit_counter_read += extra_bits_count;
    bit_stream->current_byte_offset_read -= extra_bits_count;
}

void write_word_bit_stream(
    struct bit_stream_t *bit_stream,
    unsigned short word,
    unsigned char size
) {
    short upper_s = size - BIT_STREAM_ITEM_SIZE;
    short lower_s = size > BIT_STREAM_ITEM_SIZE ? BIT_STREAM_ITEM_SIZE : size;
    unsigned char upper = (unsigned char) (word >> 8);
    unsigned char lower = (unsigned char) (word & 0x00ff);

    if(upper_s > 0) { write_byte_bit_stream(bit_stream, upper, (unsigned char) upper_s); }
    if(lower_s > 0) { write_byte_bit_stream(bit_stream, lower, (unsigned char) lower_s); }
}

void write_byte_bit_stream(
    struct bit_stream_t *bit_stream,
    unsigned char byte,
    unsigned char size
) {
    /* calculates the number of available bits (count) and
    then uses it to calculate the number of extra bits */
    unsigned char available_bits_count = BIT_STREAM_ITEM_SIZE -\
        bit_stream->current_byte_offset_write;
    unsigned char extra_bits_count = size - available_bits_count;

    /* in case the requested size is zero this is a special case
    and the control flow is immediately returned to the caller */
    if(size == 0) { return; }

    /* in case the number of bits to be written is greater
    than the available number of bits for the current byte
    need to append some extra bits */
    if(size > available_bits_count) { size = available_bits_count; }

    /* otherwise the number of extra bits is zero, the value
    must then be reset to the invalid (zero value) */
    else { extra_bits_count = 0; }

    /* shifts the current byte value (by the write size) */
    bit_stream->current_byte_write <<= size;

    /* adds the valid bits of the current byte (bits excluding the extra ones)
    to the current byte (shifts the byte the number of extra bits) */
    bit_stream->current_byte_write |= byte >> extra_bits_count;

    /* increments the global bit counter and the current byte offset by
    the size of the currently written bits */
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

void flush_read_bit_stream(struct bit_stream_t *bit_stream) {
    /* in case the bit counter (for read) hasn't reached
    the limit no flush is required (no complete byte available)
    and so must return immediately */
    if(bit_stream->current_byte_offset_read != 0) {
        return;
    }

    /* in case the stream (buffer) is empty, need to read from
    the bit stream so that the buffer gets filled again */
    if(bit_stream->byte_counter_read == 0) {
        _read_bit_stream(bit_stream);
    }

    /* retrieves the current byte to be read from the buffer
    associated with the underlying bit stream (read operation) */
    bit_stream->current_byte_read = bit_stream->buffer[bit_stream->byte_current_read];

    /* runs the update operations on the byte counter (number
    of bytes pending in the buffer) and the current byte index
    that is going to be used in the reading from the buffer */
    bit_stream->byte_counter_read--;
    bit_stream->byte_current_read++;

    /* resets the value of the current byte offset to the default
    value, keep in mind that this value is inverted for performance
    reasons and so it starts in the highest value (from the right) */
    bit_stream->current_byte_offset_read = BIT_STREAM_ITEM_SIZE;
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

    /* in case the stream (buffer) is full, need
    to flush the bit stream writing it to the
    underlying stream element */
    if(bit_stream->byte_counter_write == BIT_STREAM_BUFFER_SIZE) {
        _write_bit_stream(bit_stream);
    }
}

void _read_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream
    to be used in the proper read operation */
    struct stream_t *stream = bit_stream->stream;

    /* flushes the bit stream in the (internal) stream
    so that the contents get read from the stream */
    bit_stream->byte_counter_read = stream->read(
        stream,
        bit_stream->buffer,
        BIT_STREAM_BUFFER_SIZE
    );
    bit_stream->byte_current_read = 0;
}

void _write_bit_stream(struct bit_stream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct stream_t *stream = bit_stream->stream;

    /* in case there is a (partial) byte waiting to be writen */
    if(bit_stream->current_byte_offset_write > 0) {
        /* sets the write buffer value */
        bit_stream->buffer[bit_stream->byte_counter_write] =\
            bit_stream->current_byte_write <<\
            (BIT_STREAM_ITEM_SIZE - bit_stream->current_byte_offset_write);

        /* resets the various bit stream write
        byte oriented structures */
        bit_stream->byte_counter_write++;
        bit_stream->current_byte_offset_write = 0;
        bit_stream->current_byte_write = 0;
    }

    /* flushes the bit stream in the (internal) stream
    so that the contents get written in the stream */
    stream->write(
        stream,
        bit_stream->buffer,
        bit_stream->byte_counter_write
    );

    /* resets the byte counter write (the write buffer
    is also reset by interface) */
    bit_stream->byte_counter_write = 0;
}
