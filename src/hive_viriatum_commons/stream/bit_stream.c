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
 
 __author__    = Jo‹o Magalh‹es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
 */

#include "stdafx.h"

#include "bit_stream.h"

VIRIATUM_EXPORT_PREFIX void createBitStream(struct BitStream_t **bitStreamPointer, struct Stream_t *stream) {
    /* retrieves the bit stream size */
    size_t bitStreamSize = sizeof(struct BitStream_t);

    /* allocates space for the bit stream */
    struct BitStream_t *bitStream = (struct BitStream_t *) MALLOC(bitStreamSize);

    /* calculates the buffer size */
    size_t bufferSize = BIT_STREAM_BUFFER_SIZE * sizeof(unsigned char *);

    /* allocates the buffer for internal usage */
    bitStream->buffer = (unsigned char *) MALLOC(bufferSize);

    /* sets the (inner) stream reference */
    bitStream->stream = stream;

    /* sets the various internal structure values of the bit stream */
    bitStream->size = 0;
    bitStream->position = 0;
    bitStream->currentByteWrite = 0;
    bitStream->currentByteOffsetWrite = 0;
    bitStream->bitCounterWrite = 0;
    bitStream->byteCounterWrite = 0;

    /* sets the bit stream in the bit stream pointer */
    *bitStreamPointer = bitStream;
}

VIRIATUM_EXPORT_PREFIX void deleteBitStream(struct BitStream_t *bitStream) {
    /* releases the bit stream */
    FREE(bitStream);
}

VIRIATUM_EXPORT_PREFIX void openBitStream(struct BitStream_t *bitStream) {
    /* retrieves the (inner) stream from the bit stream */
    struct Stream_t *stream = bitStream->stream;

    /* opens the (inner) stream */
    stream->open(stream);
}

VIRIATUM_EXPORT_PREFIX void closeBitStream(struct BitStream_t *bitStream) {
    /* retrieves the (inner) stream from the bit stream */
    struct Stream_t *stream = bitStream->stream;

    /* flushes the bit stream */
    flushBitStream(bitStream);

    /* closes the (inner) stream */
    stream->close(stream);
}

VIRIATUM_EXPORT_PREFIX void readBitStream(struct BitStream_t *bitStream, unsigned char *buffer, size_t count, size_t *readCount) {
}

VIRIATUM_EXPORT_PREFIX void writeBitStream(struct BitStream_t *bitStream, unsigned char *buffer, size_t size) {
}

VIRIATUM_EXPORT_PREFIX void writeByteBitStream(struct BitStream_t *bitStream, unsigned char byte, unsigned char size) {
    /* calculates the number of available bits (count) and
    then used it to calculate the number of extra bits */
    unsigned char availableBitsCount = BIT_STREAM_ITEM_SIZE - bitStream->currentByteOffsetWrite;
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
    bitStream->currentByteWrite <<= size;

    /* adds the valid bits of the current byte (bits excluding the extra ones)
    to the current byte (shifts the byte the number of extra bits) */
    bitStream->currentByteWrite |= byte >> extraBitsCount;

    /* increments the bit counter and offset by the size */
    bitStream->bitCounterWrite += size;
    bitStream->currentByteOffsetWrite += size;

    /* checks the stream for the need to write (flush) */
    flushWriteBitStream(bitStream);

    /* in case there are no extra bits to be written */
    if(extraBitsCount == 0) {
        /* returns immediately */
        return;
    }

    /* sets the current byte as the extra bits value creates
    a mask to mask the byte and "show" only the extra bits */
    bitStream->currentByteWrite |= byte & ((0x1 << extraBitsCount) - 1);

    /* increments the bit counter and offset by the number of extra bits */
    bitStream->bitCounterWrite += extraBitsCount;
    bitStream->currentByteOffsetWrite += extraBitsCount;
}

VIRIATUM_EXPORT_PREFIX void flushWriteBitStream(struct BitStream_t *bitStream) {
    /* in case the bit counter (for write) hasn't reached
    the limit no flush is required (no complete byte available) */
    if(bitStream->currentByteOffsetWrite != BIT_STREAM_ITEM_SIZE) {
        /* returns immediately */
        return;
    }

    /* sets the buffer stream value */
    bitStream->buffer[bitStream->byteCounterWrite]= bitStream->currentByteWrite;

    /* resets the various bit stream write
    byte oriented structures */
    bitStream->byteCounterWrite++;
    bitStream->currentByteOffsetWrite = 0;
    bitStream->currentByteWrite = 0;

    /* in case the stream (buffer) is full */
    if(bitStream->byteCounterWrite == BIT_STREAM_BUFFER_SIZE) {
        /* flushes the bit stream */
        flushBitStream(bitStream);
    }
}

VIRIATUM_EXPORT_PREFIX void flushBitStream(struct BitStream_t *bitStream) {
    /* retrieves the (inner) stream from the bit stream */
    struct Stream_t *stream = bitStream->stream;

    /* in case there is a (partial) byte waiting to be writen */
    if(bitStream->currentByteOffsetWrite > 0) {
        // sets the write buffer value
        bitStream->buffer[bitStream->byteCounterWrite] = bitStream->currentByteWrite << (BIT_STREAM_ITEM_SIZE - bitStream->currentByteOffsetWrite);

        /* resets the various bit stream write
        byte oriented structures */
        bitStream->byteCounterWrite++;
        bitStream->currentByteOffsetWrite = 0;
        bitStream->currentByteWrite = 0;
    }

    /* flushes the bit stream in the (internal) stream */
    stream->write(stream, bitStream->buffer, bitStream->byteCounterWrite);

    /* resets the byte counter write (the write buffer
    is also reset by interface) */
    bitStream->byteCounterWrite = 0;
}
