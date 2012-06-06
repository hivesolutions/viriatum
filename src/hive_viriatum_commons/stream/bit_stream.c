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

#include "bit_stream.h"

VIRIATUM_EXPORT_PREFIX void create_bit_stream(struct BitStream_t **bitStreamPointer, struct Stream_t *stream) {
    /* retrieves the bit stream size */
    size_t bitStreamSize = sizeof(struct BitStream_t);

    /* allocates space for the bit stream */
    struct BitStream_t *bit_stream = (struct BitStream_t *) MALLOC(bitStreamSize);

    /* calculates the buffer size */
    size_t buffer_size = BIT_STREAM_BUFFER_SIZE * sizeof(unsigned char *);

    /* allocates the buffer for internal usage */
    bit_stream->buffer = (unsigned char *) MALLOC(buffer_size);

    /* sets the (inner) stream reference */
    bit_stream->stream = stream;

    /* sets the various internal structure values of the bit stream */
    bit_stream->size = 0;
    bit_stream->position = 0;
    bit_stream->currentByteWrite = 0;
    bit_stream->currentByteOffsetWrite = 0;
    bit_stream->bitCounterWrite = 0;
    bit_stream->byteCounterWrite = 0;

    /* sets the bit stream in the bit stream pointer */
    *bitStreamPointer = bit_stream;
}

VIRIATUM_EXPORT_PREFIX void delete_bit_stream(struct BitStream_t *bit_stream) {
    /* releases the bit stream */
    FREE(bit_stream);
}

VIRIATUM_EXPORT_PREFIX void open_bit_stream(struct BitStream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct Stream_t *stream = bit_stream->stream;

    /* opens the (inner) stream */
    stream->open(stream);
}

VIRIATUM_EXPORT_PREFIX void close_bit_stream(struct BitStream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct Stream_t *stream = bit_stream->stream;

    /* flushes the bit stream */
    flushBitStream(bit_stream);

    /* closes the (inner) stream */
    stream->close(stream);
}

VIRIATUM_EXPORT_PREFIX void readBitStream(struct BitStream_t *bit_stream, unsigned char *buffer, size_t count, size_t *readCount) {
}

VIRIATUM_EXPORT_PREFIX void writeBitStream(struct BitStream_t *bit_stream, unsigned char *buffer, size_t size) {
}

VIRIATUM_EXPORT_PREFIX void write_byte_bit_stream(struct BitStream_t *bit_stream, unsigned char byte, unsigned char size) {
    /* calculates the number of available bits (count) and
    then used it to calculate the number of extra bits */
    unsigned char availableBitsCount = BIT_STREAM_ITEM_SIZE - bit_stream->currentByteOffsetWrite;
    unsigned char extraBitsCount = size - availableBitsCount;

    /* in case the number of extra bits is set the number
    of bits to be writen is the same as the available bits */
    if(size > availableBitsCount) {
        /* sets the size as the available bits count */
        size = availableBitsCount;
    }
    /* otherwise the number of extra bits is zero */
    else {
        /* resets the extra bits count */
        extraBitsCount = 0;
    }

    /* shifts the current byte value (by the write size) */
    bit_stream->currentByteWrite <<= size;

    /* adds the valid bits of the current byte (bits excluding the extra ones)
    to the current byte (shifts the byte the number of extra bits) */
    bit_stream->currentByteWrite |= byte >> extraBitsCount;

    /* increments the bit counter and offset by the size */
    bit_stream->bitCounterWrite += size;
    bit_stream->currentByteOffsetWrite += size;

    /* checks the stream for the need to write (flush) */
    flushWriteBitStream(bit_stream);

    /* in case there are no extra bits to be written */
    if(extraBitsCount == 0) {
        /* returns immediately */
        return;
    }

    /* sets the current byte as the extra bits value creates
    a mask to mask the byte and "show" only the extra bits */
    bit_stream->currentByteWrite |= byte & ((0x1 << extraBitsCount) - 1);

    /* increments the bit counter and offset by the number of extra bits */
    bit_stream->bitCounterWrite += extraBitsCount;
    bit_stream->currentByteOffsetWrite += extraBitsCount;
}

VIRIATUM_EXPORT_PREFIX void flushWriteBitStream(struct BitStream_t *bit_stream) {
    /* in case the bit counter (for write) hasn't reached
    the limit no flush is required (no complete byte available) */
    if(bit_stream->currentByteOffsetWrite != BIT_STREAM_ITEM_SIZE) {
        /* returns immediately */
        return;
    }

    /* sets the buffer stream value */
    bit_stream->buffer[bit_stream->byteCounterWrite]= bit_stream->currentByteWrite;

    /* resets the various bit stream write
    byte oriented structures */
    bit_stream->byteCounterWrite++;
    bit_stream->currentByteOffsetWrite = 0;
    bit_stream->currentByteWrite = 0;

    /* in case the stream (buffer) is full */
    if(bit_stream->byteCounterWrite == BIT_STREAM_BUFFER_SIZE) {
        /* flushes the bit stream */
        flushBitStream(bit_stream);
    }
}

VIRIATUM_EXPORT_PREFIX void flushBitStream(struct BitStream_t *bit_stream) {
    /* retrieves the (inner) stream from the bit stream */
    struct Stream_t *stream = bit_stream->stream;

    /* in case there is a (partial) byte waiting to be writen */
    if(bit_stream->currentByteOffsetWrite > 0) {
        // sets the write buffer value
        bit_stream->buffer[bit_stream->byteCounterWrite] = bit_stream->currentByteWrite << (BIT_STREAM_ITEM_SIZE - bit_stream->currentByteOffsetWrite);

        /* resets the various bit stream write
        byte oriented structures */
        bit_stream->byteCounterWrite++;
        bit_stream->currentByteOffsetWrite = 0;
        bit_stream->currentByteWrite = 0;
    }

    /* flushes the bit stream in the (internal) stream */
    stream->write(stream, bit_stream->buffer, bit_stream->byteCounterWrite);

    /* resets the byte counter write (the write buffer
    is also reset by interface) */
    bit_stream->byteCounterWrite = 0;
}
