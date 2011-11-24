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

#include "io_stream.h"

/**
 * The size in bits of an item of the
 * bit stream.
 * The currently used item is the byte (char).
 */
#define BIT_STREAM_ITEM_SIZE 8

/**
 * The size of the bit stream internal buffer.
 * The larger the size of this buffer the
 * more time the data takes to be automatically
 * flushed to the internal data stream.
 */
#define BIT_STREAM_BUFFER_SIZE 4096

/**
 * Structure describing the internal data
 * of a stream of bits.
 * This kind of stream allows a bit level
 * control over a buffer of bytes.
 */
typedef struct BitStream_t {
    /**
     * The internal stream used as the final
     * target for the stream.
     * This stream is updated in every flush.
     */
    struct Stream_t *stream;

    /**
     * The buffer containing the bit stream
     * contents in a byte oriented fashion.
     */
    unsigned char *buffer;

    /**
     * The size in bits of the current stream.
     */
    size_t size;

    /**
     * The position (offset) in bits inside the
     * current stream.
     */
    size_t position;

    /**
     * The current byte used in the writting
     * procedure.
     */
    unsigned char currentByteWrite;

    /**
     * The position of the "cursor" in the
     * current byte.
     * Used for the write mode in the current byte.
     */
    unsigned char currentByteOffsetWrite;

    /**
     * The counter (of bits) used for the writting
     * procedure in the current bit stream.
     */
    size_t bitCounterWrite;

    /**
     * The counter (of bytes) used for the writting
     * procedure in the current bit stream.
     */
    size_t byteCounterWrite;
} BitStream;

/**
 * Structure that describes a chunk (piece)
 * of bit data of a random size.
 * The data is stored in a byte stream a is
 * trunked (padded with zeros) to the bit
 * stream size.
 */
typedef struct BitChunk_t {
    /**
     * The byte buffer with the bit contents
     * of the chunk.
     */
    unsigned char *buffer;

    /**
     * The size of the bit buffer in bits.
     */
    size_t size;
} BitChunk;

VIRIATUM_EXPORT_PREFIX void createBitStream(struct BitStream_t **bitStreamPointer, struct Stream_t *stream);
VIRIATUM_EXPORT_PREFIX void deleteBitStream(struct BitStream_t *bitStream);
VIRIATUM_EXPORT_PREFIX void openBitStream(struct BitStream_t *bitStream);
VIRIATUM_EXPORT_PREFIX void closeBitStream(struct BitStream_t *bitStream);
VIRIATUM_EXPORT_PREFIX void readBitStream(struct BitStream_t *bitStream, unsigned char *buffer, size_t count, size_t *readCount);
VIRIATUM_EXPORT_PREFIX void writeBitStream(struct BitStream_t *bitStream, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX void writeByteBitStream(struct BitStream_t *bitStream, unsigned char byte, unsigned char size);
VIRIATUM_EXPORT_PREFIX void flushWriteBitStream(struct BitStream_t *bitStream);
VIRIATUM_EXPORT_PREFIX void flushBitStream(struct BitStream_t *bitStream);
