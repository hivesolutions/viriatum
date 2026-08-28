/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "hpack.h"

/**
 * The largest value that an integer of the prefix based
 * representation is allowed to take, an encoding that goes
 * beyond it is refused instead of being wrapped around into
 * a small value.
 */
#define HPACK_MAX_INTEGER 0x0fffffff

/**
 * The static table of the header fields defined by the appendix A of
 * RFC 7541, the index of an entry on the wire is its position in this
 * table incremented by one.
 * The tables that follow it describe the canonical Huffman code of the
 * appendix B, the codes and their lengths are used by the encoder and
 * the symbols sorted by length together with the first code and the
 * first index of each length are used by the decoder.
 */
static const struct hpack_header_t hpack_static_table[HPACK_STATIC_SIZE] = {
    { (unsigned char *) ":authority", 10, (unsigned char *) "", 0 },
    { (unsigned char *) ":method", 7, (unsigned char *) "GET", 3 },
    { (unsigned char *) ":method", 7, (unsigned char *) "POST", 4 },
    { (unsigned char *) ":path", 5, (unsigned char *) "/", 1 },
    { (unsigned char *) ":path", 5, (unsigned char *) "/index.html", 11 },
    { (unsigned char *) ":scheme", 7, (unsigned char *) "http", 4 },
    { (unsigned char *) ":scheme", 7, (unsigned char *) "https", 5 },
    { (unsigned char *) ":status", 7, (unsigned char *) "200", 3 },
    { (unsigned char *) ":status", 7, (unsigned char *) "204", 3 },
    { (unsigned char *) ":status", 7, (unsigned char *) "206", 3 },
    { (unsigned char *) ":status", 7, (unsigned char *) "304", 3 },
    { (unsigned char *) ":status", 7, (unsigned char *) "400", 3 },
    { (unsigned char *) ":status", 7, (unsigned char *) "404", 3 },
    { (unsigned char *) ":status", 7, (unsigned char *) "500", 3 },
    { (unsigned char *) "accept-charset", 14, (unsigned char *) "", 0 },
    { (unsigned char *) "accept-encoding", 15, (unsigned char *) "gzip, deflate", 13 },
    { (unsigned char *) "accept-language", 15, (unsigned char *) "", 0 },
    { (unsigned char *) "accept-ranges", 13, (unsigned char *) "", 0 },
    { (unsigned char *) "accept", 6, (unsigned char *) "", 0 },
    { (unsigned char *) "access-control-allow-origin", 27, (unsigned char *) "", 0 },
    { (unsigned char *) "age", 3, (unsigned char *) "", 0 },
    { (unsigned char *) "allow", 5, (unsigned char *) "", 0 },
    { (unsigned char *) "authorization", 13, (unsigned char *) "", 0 },
    { (unsigned char *) "cache-control", 13, (unsigned char *) "", 0 },
    { (unsigned char *) "content-disposition", 19, (unsigned char *) "", 0 },
    { (unsigned char *) "content-encoding", 16, (unsigned char *) "", 0 },
    { (unsigned char *) "content-language", 16, (unsigned char *) "", 0 },
    { (unsigned char *) "content-length", 14, (unsigned char *) "", 0 },
    { (unsigned char *) "content-location", 16, (unsigned char *) "", 0 },
    { (unsigned char *) "content-range", 13, (unsigned char *) "", 0 },
    { (unsigned char *) "content-type", 12, (unsigned char *) "", 0 },
    { (unsigned char *) "cookie", 6, (unsigned char *) "", 0 },
    { (unsigned char *) "date", 4, (unsigned char *) "", 0 },
    { (unsigned char *) "etag", 4, (unsigned char *) "", 0 },
    { (unsigned char *) "expect", 6, (unsigned char *) "", 0 },
    { (unsigned char *) "expires", 7, (unsigned char *) "", 0 },
    { (unsigned char *) "from", 4, (unsigned char *) "", 0 },
    { (unsigned char *) "host", 4, (unsigned char *) "", 0 },
    { (unsigned char *) "if-match", 8, (unsigned char *) "", 0 },
    { (unsigned char *) "if-modified-since", 17, (unsigned char *) "", 0 },
    { (unsigned char *) "if-none-match", 13, (unsigned char *) "", 0 },
    { (unsigned char *) "if-range", 8, (unsigned char *) "", 0 },
    { (unsigned char *) "if-unmodified-since", 19, (unsigned char *) "", 0 },
    { (unsigned char *) "last-modified", 13, (unsigned char *) "", 0 },
    { (unsigned char *) "link", 4, (unsigned char *) "", 0 },
    { (unsigned char *) "location", 8, (unsigned char *) "", 0 },
    { (unsigned char *) "max-forwards", 12, (unsigned char *) "", 0 },
    { (unsigned char *) "proxy-authenticate", 18, (unsigned char *) "", 0 },
    { (unsigned char *) "proxy-authorization", 19, (unsigned char *) "", 0 },
    { (unsigned char *) "range", 5, (unsigned char *) "", 0 },
    { (unsigned char *) "referer", 7, (unsigned char *) "", 0 },
    { (unsigned char *) "refresh", 7, (unsigned char *) "", 0 },
    { (unsigned char *) "retry-after", 11, (unsigned char *) "", 0 },
    { (unsigned char *) "server", 6, (unsigned char *) "", 0 },
    { (unsigned char *) "set-cookie", 10, (unsigned char *) "", 0 },
    { (unsigned char *) "strict-transport-security", 25, (unsigned char *) "", 0 },
    { (unsigned char *) "transfer-encoding", 17, (unsigned char *) "", 0 },
    { (unsigned char *) "user-agent", 10, (unsigned char *) "", 0 },
    { (unsigned char *) "vary", 4, (unsigned char *) "", 0 },
    { (unsigned char *) "via", 3, (unsigned char *) "", 0 },
    { (unsigned char *) "www-authenticate", 16, (unsigned char *) "", 0 }
};

static const unsigned int hpack_huffman_codes[257] = {
    0x00001ff8, 0x007fffd8, 0x0fffffe2, 0x0fffffe3, 0x0fffffe4, 0x0fffffe5,
    0x0fffffe6, 0x0fffffe7, 0x0fffffe8, 0x00ffffea, 0x3ffffffc, 0x0fffffe9,
    0x0fffffea, 0x3ffffffd, 0x0fffffeb, 0x0fffffec, 0x0fffffed, 0x0fffffee,
    0x0fffffef, 0x0ffffff0, 0x0ffffff1, 0x0ffffff2, 0x3ffffffe, 0x0ffffff3,
    0x0ffffff4, 0x0ffffff5, 0x0ffffff6, 0x0ffffff7, 0x0ffffff8, 0x0ffffff9,
    0x0ffffffa, 0x0ffffffb, 0x00000014, 0x000003f8, 0x000003f9, 0x00000ffa,
    0x00001ff9, 0x00000015, 0x000000f8, 0x000007fa, 0x000003fa, 0x000003fb,
    0x000000f9, 0x000007fb, 0x000000fa, 0x00000016, 0x00000017, 0x00000018,
    0x00000000, 0x00000001, 0x00000002, 0x00000019, 0x0000001a, 0x0000001b,
    0x0000001c, 0x0000001d, 0x0000001e, 0x0000001f, 0x0000005c, 0x000000fb,
    0x00007ffc, 0x00000020, 0x00000ffb, 0x000003fc, 0x00001ffa, 0x00000021,
    0x0000005d, 0x0000005e, 0x0000005f, 0x00000060, 0x00000061, 0x00000062,
    0x00000063, 0x00000064, 0x00000065, 0x00000066, 0x00000067, 0x00000068,
    0x00000069, 0x0000006a, 0x0000006b, 0x0000006c, 0x0000006d, 0x0000006e,
    0x0000006f, 0x00000070, 0x00000071, 0x00000072, 0x000000fc, 0x00000073,
    0x000000fd, 0x00001ffb, 0x0007fff0, 0x00001ffc, 0x00003ffc, 0x00000022,
    0x00007ffd, 0x00000003, 0x00000023, 0x00000004, 0x00000024, 0x00000005,
    0x00000025, 0x00000026, 0x00000027, 0x00000006, 0x00000074, 0x00000075,
    0x00000028, 0x00000029, 0x0000002a, 0x00000007, 0x0000002b, 0x00000076,
    0x0000002c, 0x00000008, 0x00000009, 0x0000002d, 0x00000077, 0x00000078,
    0x00000079, 0x0000007a, 0x0000007b, 0x00007ffe, 0x000007fc, 0x00003ffd,
    0x00001ffd, 0x0ffffffc, 0x000fffe6, 0x003fffd2, 0x000fffe7, 0x000fffe8,
    0x003fffd3, 0x003fffd4, 0x003fffd5, 0x007fffd9, 0x003fffd6, 0x007fffda,
    0x007fffdb, 0x007fffdc, 0x007fffdd, 0x007fffde, 0x00ffffeb, 0x007fffdf,
    0x00ffffec, 0x00ffffed, 0x003fffd7, 0x007fffe0, 0x00ffffee, 0x007fffe1,
    0x007fffe2, 0x007fffe3, 0x007fffe4, 0x001fffdc, 0x003fffd8, 0x007fffe5,
    0x003fffd9, 0x007fffe6, 0x007fffe7, 0x00ffffef, 0x003fffda, 0x001fffdd,
    0x000fffe9, 0x003fffdb, 0x003fffdc, 0x007fffe8, 0x007fffe9, 0x001fffde,
    0x007fffea, 0x003fffdd, 0x003fffde, 0x00fffff0, 0x001fffdf, 0x003fffdf,
    0x007fffeb, 0x007fffec, 0x001fffe0, 0x001fffe1, 0x003fffe0, 0x001fffe2,
    0x007fffed, 0x003fffe1, 0x007fffee, 0x007fffef, 0x000fffea, 0x003fffe2,
    0x003fffe3, 0x003fffe4, 0x007ffff0, 0x003fffe5, 0x003fffe6, 0x007ffff1,
    0x03ffffe0, 0x03ffffe1, 0x000fffeb, 0x0007fff1, 0x003fffe7, 0x007ffff2,
    0x003fffe8, 0x01ffffec, 0x03ffffe2, 0x03ffffe3, 0x03ffffe4, 0x07ffffde,
    0x07ffffdf, 0x03ffffe5, 0x00fffff1, 0x01ffffed, 0x0007fff2, 0x001fffe3,
    0x03ffffe6, 0x07ffffe0, 0x07ffffe1, 0x03ffffe7, 0x07ffffe2, 0x00fffff2,
    0x001fffe4, 0x001fffe5, 0x03ffffe8, 0x03ffffe9, 0x0ffffffd, 0x07ffffe3,
    0x07ffffe4, 0x07ffffe5, 0x000fffec, 0x00fffff3, 0x000fffed, 0x001fffe6,
    0x003fffe9, 0x001fffe7, 0x001fffe8, 0x007ffff3, 0x003fffea, 0x003fffeb,
    0x01ffffee, 0x01ffffef, 0x00fffff4, 0x00fffff5, 0x03ffffea, 0x007ffff4,
    0x03ffffeb, 0x07ffffe6, 0x03ffffec, 0x03ffffed, 0x07ffffe7, 0x07ffffe8,
    0x07ffffe9, 0x07ffffea, 0x07ffffeb, 0x0ffffffe, 0x07ffffec, 0x07ffffed,
    0x07ffffee, 0x07ffffef, 0x07fffff0, 0x03ffffee, 0x3fffffff
};

static const unsigned char hpack_huffman_lengths[257] = {
    13, 23, 28, 28, 28, 28, 28, 28, 28, 24, 30, 28, 28, 30, 28, 28,
    28, 28, 28, 28, 28, 28, 30, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    6, 10, 10, 12, 13, 6, 8, 11, 10, 10, 8, 11, 8, 6, 6, 6,
    5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 8, 15, 6, 12, 10,
    13, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 8, 7, 8, 13, 19, 13, 14, 6,
    15, 5, 6, 5, 6, 5, 6, 6, 6, 5, 7, 7, 6, 6, 6, 5,
    6, 7, 6, 5, 5, 6, 7, 7, 7, 7, 7, 15, 11, 14, 13, 28,
    20, 22, 20, 20, 22, 22, 22, 23, 22, 23, 23, 23, 23, 23, 24, 23,
    24, 24, 22, 23, 24, 23, 23, 23, 23, 21, 22, 23, 22, 23, 23, 24,
    22, 21, 20, 22, 22, 23, 23, 21, 23, 22, 22, 24, 21, 22, 23, 23,
    21, 21, 22, 21, 23, 22, 23, 23, 20, 22, 22, 22, 23, 22, 22, 23,
    26, 26, 20, 19, 22, 23, 22, 25, 26, 26, 26, 27, 27, 26, 24, 25,
    19, 21, 26, 27, 27, 26, 27, 24, 21, 21, 26, 26, 28, 27, 27, 27,
    20, 24, 20, 21, 22, 21, 21, 23, 22, 22, 25, 25, 24, 24, 26, 23,
    26, 27, 26, 26, 27, 27, 27, 27, 27, 28, 27, 27, 27, 27, 27, 26,
    30
};

static const unsigned short hpack_huffman_symbols[257] = {
    48, 49, 50, 97, 99, 101, 105, 111, 115, 116, 32, 37,
    45, 46, 47, 51, 52, 53, 54, 55, 56, 57, 61, 65,
    95, 98, 100, 102, 103, 104, 108, 109, 110, 112, 114, 117,
    58, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 89,
    106, 107, 113, 118, 119, 120, 121, 122, 38, 42, 44, 59,
    88, 90, 33, 34, 40, 41, 63, 39, 43, 124, 35, 62,
    0, 36, 64, 91, 93, 126, 94, 125, 60, 96, 123, 92,
    195, 208, 128, 130, 131, 162, 184, 194, 224, 226, 153, 161,
    167, 172, 176, 177, 179, 209, 216, 217, 227, 229, 230, 129,
    132, 133, 134, 136, 146, 154, 156, 160, 163, 164, 169, 170,
    173, 178, 181, 185, 186, 187, 189, 190, 196, 198, 228, 232,
    233, 1, 135, 137, 138, 139, 140, 141, 143, 147, 149, 150,
    151, 152, 155, 157, 158, 165, 166, 168, 174, 175, 180, 182,
    183, 188, 191, 197, 231, 239, 9, 142, 144, 145, 148, 159,
    171, 206, 215, 225, 236, 237, 199, 207, 234, 235, 192, 193,
    200, 201, 202, 205, 210, 213, 218, 219, 238, 240, 242, 243,
    255, 203, 204, 211, 212, 214, 221, 222, 223, 241, 244, 245,
    246, 247, 248, 250, 251, 252, 253, 254, 2, 3, 4, 5,
    6, 7, 8, 11, 12, 14, 15, 16, 17, 18, 19, 20,
    21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 127, 220,
    249, 10, 13, 22, 256
};

static const unsigned short hpack_huffman_counts[31] = {
    0, 0, 0, 0, 0, 10, 26, 32, 6, 0, 5, 3, 2, 6, 2, 3,
    0, 0, 0, 3, 8, 13, 26, 29, 12, 4, 15, 19, 29, 0, 4
};

static const unsigned int hpack_huffman_first_code[31] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000014, 0x0000005c,
    0x000000f8, 0x00000000, 0x000003f8, 0x000007fa, 0x00000ffa, 0x00001ff8, 0x00003ffc, 0x00007ffc,
    0x00000000, 0x00000000, 0x00000000, 0x0007fff0, 0x000fffe6, 0x001fffdc, 0x003fffd2, 0x007fffd8,
    0x00ffffea, 0x01ffffec, 0x03ffffe0, 0x07ffffde, 0x0fffffe2, 0x00000000, 0x3ffffffc
};

static const unsigned short hpack_huffman_first_index[31] = {
    0, 0, 0, 0, 0, 0, 10, 36, 68, 74, 74, 79, 82, 84, 90, 92,
    95, 95, 95, 95, 98, 106, 119, 145, 174, 186, 190, 205, 224, 253, 253
};

/**
 * Releases the oldest entry of the provided dynamic table,
 * the caller is the one responsible for making sure that the
 * table is not empty.
 *
 * @param hpack_table The HPACK table to be evicted from.
 */
static void _evict_hpack_table(struct hpack_table_t *hpack_table) {
    /* retrieves the position of the oldest entry, it sits at the head
    of the ring deducted of the number of entries that follow it */
    size_t position = (hpack_table->head + HPACK_MAX_ENTRIES - (hpack_table->count - 1)) % HPACK_MAX_ENTRIES;
    struct hpack_entry_t *hpack_entry = &hpack_table->entries[position];

    /* releases both the name and the value of the entry, they are
    owned by it and so no one else is referencing them */
    FREE(hpack_entry->name);
    FREE(hpack_entry->value);
    hpack_entry->name = NULL;
    hpack_entry->value = NULL;

    /* accounts the eviction in both the size and the number of the
    entries that the table is holding */
    hpack_table->size -= hpack_entry->size;
    hpack_table->count--;
}

ERROR_CODE create_hpack_table(struct hpack_table_t **hpack_table_pointer) {
    /* retrieves the HPACK table size */
    size_t hpack_table_size = sizeof(struct hpack_table_t);

    /* allocates space for the HPACK table */
    struct hpack_table_t *hpack_table = (struct hpack_table_t *) MALLOC(hpack_table_size);

    /* sets the HPACK table attributes, the head is placed at the end
    of the ring so that the first insertion lands at its start */
    hpack_table->head = HPACK_MAX_ENTRIES - 1;
    hpack_table->count = 0;
    hpack_table->size = 0;
    hpack_table->max_size = HPACK_TABLE_SIZE;

    /* sets the HPACK table in the HPACK table pointer */
    *hpack_table_pointer = hpack_table;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_hpack_table(struct hpack_table_t *hpack_table) {
    /* evicts every one of the entries that the table is still
    holding, this releases the memory that each one of them owns */
    while(hpack_table->count > 0) { _evict_hpack_table(hpack_table); }

    /* releases the HPACK table */
    FREE(hpack_table);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE resize_hpack_table(struct hpack_table_t *hpack_table, size_t max_size) {
    /* refuses a size above the one that has been advertised to the
    peer, the peer is never allowed to grow the table beyond it */
    if(max_size > HPACK_TABLE_SIZE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Dynamic table size above the advertised one"
        );
    }

    /* sets the new maximum size and then evicts the oldest entries
    until the table fits in it */
    hpack_table->max_size = max_size;
    while(hpack_table->count > 0 && hpack_table->size > hpack_table->max_size) {
        _evict_hpack_table(hpack_table);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE insert_hpack_table(struct hpack_table_t *hpack_table, struct hpack_header_t *hpack_header) {
    /* allocates space for the entry that is going to receive the
    provided header field */
    struct hpack_entry_t *hpack_entry;

    /* calculates the size of the entry as accounted by the
    specification, the constant overhead included */
    size_t size = hpack_header->name_size + hpack_header->value_size + HPACK_ENTRY_OVERHEAD;

    /* evicts the oldest entries until the new one fits in the table */
    while(hpack_table->count > 0 && hpack_table->size + size > hpack_table->max_size) {
        _evict_hpack_table(hpack_table);
    }

    /* an entry that is larger than the complete table empties it and
    is then not inserted, which is the behaviour that the
    specification requires and not an error condition */
    if(size > hpack_table->max_size) { RAISE_NO_ERROR; }

    /* advances the head of the ring and takes the entry that sits at
    the new position, it is the one that becomes the newest */
    hpack_table->head = (hpack_table->head + 1) % HPACK_MAX_ENTRIES;
    hpack_entry = &hpack_table->entries[hpack_table->head];

    /* copies both the name and the value into memory owned by the
    entry, the buffers of the caller are transient */
    hpack_entry->name = (unsigned char *) MALLOC(hpack_header->name_size + 1);
    memcpy(hpack_entry->name, hpack_header->name, hpack_header->name_size);
    hpack_entry->name[hpack_header->name_size] = '\0';
    hpack_entry->name_size = hpack_header->name_size;
    hpack_entry->value = (unsigned char *) MALLOC(hpack_header->value_size + 1);
    memcpy(hpack_entry->value, hpack_header->value, hpack_header->value_size);
    hpack_entry->value[hpack_header->value_size] = '\0';
    hpack_entry->value_size = hpack_header->value_size;
    hpack_entry->size = size;

    /* accounts the insertion in both the size and the number of the
    entries that the table is holding */
    hpack_table->count++;
    hpack_table->size += size;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE get_hpack_table(struct hpack_table_t *hpack_table, size_t index, struct hpack_header_t *hpack_header) {
    /* allocates space for both the position in the ring and for the
    entry that is going to be found at it */
    size_t position;
    struct hpack_entry_t *hpack_entry;

    /* the index is based at one, so the value zero never refers to a
    field and is reserved for the literal representations */
    if(index == 0) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Invalid zero index in the header block"
        );
    }

    /* the lower part of the address space is the static table, whose
    entries are constant and shared by every connection */
    if(index <= HPACK_STATIC_SIZE) {
        *hpack_header = hpack_static_table[index - 1];
        RAISE_NO_ERROR;
    }

    /* moves the index into the address space of the dynamic table and
    then verifies that it refers to an entry that exists */
    index -= HPACK_STATIC_SIZE;
    if(index > hpack_table->count) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Index above the size of the dynamic table"
        );
    }

    /* retrieves the entry, the index one of the dynamic table refers
    to the newest entry, which is the one sitting at the head */
    position = (hpack_table->head + HPACK_MAX_ENTRIES - (index - 1)) % HPACK_MAX_ENTRIES;
    hpack_entry = &hpack_table->entries[position];

    /* populates the provided structure with the references that the
    entry holds, no copy takes place at this point */
    hpack_header->name = hpack_entry->name;
    hpack_header->name_size = hpack_entry->name_size;
    hpack_header->value = hpack_entry->value;
    hpack_header->value_size = hpack_entry->value_size;

    /* raises no error */
    RAISE_NO_ERROR;
}

char find_hpack_table(struct hpack_table_t *hpack_table, struct hpack_header_t *hpack_header, size_t *index) {
    /* allocates space for the iteration over both of the tables and
    for the entries that are visited along it */
    size_t position;
    size_t offset;
    const struct hpack_header_t *entry;
    struct hpack_entry_t *hpack_entry;

    /* unsets the index so that the absence of a match is reported by
    the value zero, which never refers to a field */
    *index = 0;

    /* walks the static table first, the size of the name is compared
    before its contents as it discards almost every candidate */
    for(position = 0; position < HPACK_STATIC_SIZE; position++) {
        entry = &hpack_static_table[position];
        if(entry->name_size != hpack_header->name_size) { continue; }
        if(memcmp(entry->name, hpack_header->name, entry->name_size) != 0) { continue; }
        if(*index == 0) { *index = position + 1; }
        if(entry->value_size != hpack_header->value_size) { continue; }
        if(memcmp(entry->value, hpack_header->value, entry->value_size) != 0) { continue; }
        *index = position + 1;
        return TRUE;
    }

    /* walks the dynamic table from the newest entry to the oldest
    one, which is the order the indexes follow */
    for(position = 1; position <= hpack_table->count; position++) {
        offset = (hpack_table->head + HPACK_MAX_ENTRIES - (position - 1)) % HPACK_MAX_ENTRIES;
        hpack_entry = &hpack_table->entries[offset];
        if(hpack_entry->name_size != hpack_header->name_size) { continue; }
        if(memcmp(hpack_entry->name, hpack_header->name, hpack_entry->name_size) != 0) { continue; }
        if(*index == 0) { *index = HPACK_STATIC_SIZE + position; }
        if(hpack_entry->value_size != hpack_header->value_size) { continue; }
        if(memcmp(hpack_entry->value, hpack_header->value, hpack_entry->value_size) != 0) { continue; }
        *index = HPACK_STATIC_SIZE + position;
        return TRUE;
    }

    /* returns false as only the name may have been matched, the index
    tells the caller if even that has happened */
    return FALSE;
}

ERROR_CODE decode_integer_hpack(const unsigned char *data, size_t data_size, size_t *offset, unsigned char prefix, size_t *value) {
    /* allocates space for the position in the buffer, for the value
    being accumulated and for the byte being visited */
    size_t position = *offset;
    size_t result;
    size_t shift = 0;
    unsigned char byte;

    /* calculates the mask of the prefix, a value that saturates it
    signals that the continuation bytes follow */
    unsigned int mask = (1 << prefix) - 1;

    /* verifies that the prefix byte itself is present */
    if(position >= data_size) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Truncated integer in the header block"
        );
    }

    /* takes the value out of the prefix and moves past the byte */
    result = data[position] & mask;
    position++;

    /* in case the prefix is not saturated the value is complete and
    none of the continuation bytes is present */
    if(result < mask) {
        *offset = position;
        *value = result;
        RAISE_NO_ERROR;
    }

    /* accumulates the continuation bytes, each one of them carries
    seven bits of the value and one bit of continuation */
    while(TRUE) {
        if(position >= data_size) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Truncated integer in the header block"
            );
        }

        /* refuses an encoding that carries more continuation bytes
        than the accepted range of the value is able to hold, a peer
        is otherwise able to wrap the value around */
        if(shift > 21) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Overflowing integer in the header block"
            );
        }

        byte = data[position];
        position++;

        result += (size_t) (byte & 0x7f) << shift;
        shift += 7;

        if(result > HPACK_MAX_INTEGER) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Overflowing integer in the header block"
            );
        }

        if(!(byte & 0x80)) { break; }
    }

    /* updates both the position and the value with the result of the
    complete decoding operation */
    *offset = position;
    *value = result;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_integer_hpack(unsigned char *buffer, size_t buffer_size, size_t *offset, unsigned char prefix, unsigned char flags, size_t value) {
    /* allocates space for the position in the buffer and calculates
    the mask of the prefix */
    size_t position = *offset;
    unsigned int mask = (1 << prefix) - 1;

    /* verifies that there's space for the prefix byte itself */
    if(position >= buffer_size) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "No space for the integer in the buffer"
        );
    }

    /* in case the value fits in the prefix it is written together
    with the flags and no continuation byte is required */
    if(value < mask) {
        buffer[position] = (unsigned char) (flags | value);
        position++;
    } else {
        /* saturates the prefix and then writes the remainder of the
        value in the continuation bytes that follow it */
        buffer[position] = (unsigned char) (flags | mask);
        position++;
        value -= mask;

        while(value >= 128) {
            if(position >= buffer_size) {
                RAISE_ERROR_M(
                    RUNTIME_EXCEPTION_ERROR_CODE,
                    (unsigned char *) "No space for the integer in the buffer"
                );
            }
            buffer[position] = (unsigned char) ((value & 0x7f) | 0x80);
            position++;
            value >>= 7;
        }

        if(position >= buffer_size) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "No space for the integer in the buffer"
            );
        }
        buffer[position] = (unsigned char) value;
        position++;
    }

    /* updates the position with the one that follows the integer */
    *offset = position;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_huffman_hpack(const unsigned char *data, size_t data_size, unsigned char *buffer, size_t buffer_size, size_t *result_size) {
    /* allocates space for the iteration over the coded bytes and for
    the code that is being accumulated out of them */
    size_t index;
    size_t offset = 0;
    int bit;
    unsigned char byte;
    unsigned int code = 0;
    unsigned int bits = 0;
    unsigned int first;
    unsigned short symbol;

    /* walks every one of the coded bytes, the code is canonical and
    so a symbol is recognised as soon as the accumulated value falls
    inside the range of the codes of the current length */
    for(index = 0; index < data_size; index++) {
        byte = data[index];

        for(bit = 7; bit >= 0; bit--) {
            code = (code << 1) | ((byte >> bit) & 0x01);
            bits++;

            /* the accumulated code has gone past the longest code of
            the table, so the encoding is not a valid one */
            if(bits > HPACK_HUFFMAN_MAX_BITS) {
                RAISE_ERROR_M(
                    RUNTIME_EXCEPTION_ERROR_CODE,
                    (unsigned char *) "Invalid Huffman code in the header block"
                );
            }

            /* no symbol is encoded with the current number of bits,
            so more of them have to be accumulated */
            if(hpack_huffman_counts[bits] == 0) { continue; }

            first = hpack_huffman_first_code[bits];
            if(code < first || code - first >= hpack_huffman_counts[bits]) { continue; }

            symbol = hpack_huffman_symbols[hpack_huffman_first_index[bits] + (code - first)];

            /* the end of string symbol is never allowed to appear in
            the coded data, only its prefix is, as the padding */
            if(symbol == HPACK_HUFFMAN_EOS) {
                RAISE_ERROR_M(
                    RUNTIME_EXCEPTION_ERROR_CODE,
                    (unsigned char *) "End of string symbol in the header block"
                );
            }

            if(offset >= buffer_size) {
                RAISE_ERROR_M(
                    RUNTIME_EXCEPTION_ERROR_CODE,
                    (unsigned char *) "Decoded string above the accepted size"
                );
            }

            buffer[offset] = (unsigned char) symbol;
            offset++;

            code = 0;
            bits = 0;
        }
    }

    /* the bits that remain are the padding, they must be fewer than a
    byte and must be the most significant bits of the end of string
    symbol, which are all set */
    if(bits > 7) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Padding above a byte in the header block"
        );
    }
    if(bits > 0 && code != ((unsigned int) 1 << bits) - 1) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Invalid padding in the header block"
        );
    }

    /* updates the size with the one of the decoded buffer */
    *result_size = offset;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_huffman_hpack(const unsigned char *data, size_t data_size, unsigned char *buffer, size_t buffer_size, size_t *result_size) {
    /* allocates space for the iteration over the bytes to be coded
    and for the accumulator that gathers the bits of their codes */
    size_t index;
    size_t offset = 0;
    unsigned int code;
    unsigned int length;
    unsigned int take;
    unsigned int accumulator = 0;
    unsigned int count = 0;

    /* walks every one of the bytes, pushing the bits of its code into
    the accumulator and flushing it one byte at a time */
    for(index = 0; index < data_size; index++) {
        code = hpack_huffman_codes[data[index]];
        length = hpack_huffman_lengths[data[index]];

        /* pushes the code in at most two steps so that the
        accumulator never has to hold more than thirty two bits */
        while(length > 0) {
            take = 32 - count;
            if(take > length) { take = length; }

            accumulator = (accumulator << take) | ((code >> (length - take)) & (((unsigned int) 1 << take) - 1));
            count += take;
            length -= take;

            while(count >= 8) {
                if(offset >= buffer_size) {
                    RAISE_ERROR_M(
                        RUNTIME_EXCEPTION_ERROR_CODE,
                        (unsigned char *) "No space for the coded string in the buffer"
                    );
                }
                count -= 8;
                buffer[offset] = (unsigned char) (accumulator >> count);
                offset++;
            }
        }
    }

    /* pads the last byte with the most significant bits of the end of
    string symbol, which are all set */
    if(count > 0) {
        if(offset >= buffer_size) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "No space for the coded string in the buffer"
            );
        }
        accumulator = (accumulator << (8 - count)) | (((unsigned int) 1 << (8 - count)) - 1);
        buffer[offset] = (unsigned char) accumulator;
        offset++;
    }

    /* updates the size with the one of the coded buffer */
    *result_size = offset;

    /* raises no error */
    RAISE_NO_ERROR;
}

size_t size_huffman_hpack(const unsigned char *data, size_t data_size) {
    /* allocates space for the iteration over the bytes and for the
    accumulated number of bits of their codes */
    size_t index;
    size_t bits = 0;

    /* accumulates the length of the code of every one of the bytes */
    for(index = 0; index < data_size; index++) {
        bits += hpack_huffman_lengths[data[index]];
    }

    /* returns the number of bytes required to hold the bits, the last
    one of them is padded and so it counts as a complete byte */
    return (bits + 7) / 8;
}

ERROR_CODE decode_string_hpack(const unsigned char *data, size_t data_size, size_t *offset, unsigned char *buffer, size_t buffer_size, size_t *string_size) {
    /* allocates space for the position in the buffer, for the length
    of the string and for the error code of the operations */
    size_t position = *offset;
    size_t length;
    char huffman;
    ERROR_CODE return_value;

    /* verifies that the length byte itself is present and then reads
    the flag that tells if the string is coded */
    if(position >= data_size) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Truncated string in the header block"
        );
    }
    huffman = data[position] & 0x80 ? TRUE : FALSE;

    /* decodes the length of the string, it sits in the seven bits
    that follow the flag */
    return_value = decode_integer_hpack(data, data_size, &position, 7, &length);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* verifies that the complete string is contained in the buffer,
    the subtraction avoids the overflow of the addition */
    if(length > data_size - position) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Truncated string in the header block"
        );
    }

    if(huffman) {
        return_value = decode_huffman_hpack(&data[position], length, buffer, buffer_size, string_size);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    } else {
        if(length > buffer_size) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "String above the accepted size"
            );
        }
        memcpy(buffer, &data[position], length);
        *string_size = length;
    }

    /* updates the position with the one that follows the string */
    *offset = position + length;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_string_hpack(unsigned char *buffer, size_t buffer_size, size_t *offset, const unsigned char *value, size_t value_size) {
    /* allocates space for the size of the coded form and for the
    error code of the operations */
    size_t huffman_size = size_huffman_hpack(value, value_size);
    size_t result_size;
    ERROR_CODE return_value;

    /* uses the coded form only when it is the shorter of the two
    representations, as the specification recommends */
    if(huffman_size < value_size) {
        return_value = encode_integer_hpack(buffer, buffer_size, offset, 7, 0x80, huffman_size);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
        return_value = encode_huffman_hpack(value, value_size, &buffer[*offset], buffer_size - *offset, &result_size);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
        *offset += result_size;
    } else {
        return_value = encode_integer_hpack(buffer, buffer_size, offset, 7, 0x00, value_size);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
        if(value_size > buffer_size - *offset) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "No space for the string in the buffer"
            );
        }
        memcpy(&buffer[*offset], value, value_size);
        *offset += value_size;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_hpack(struct hpack_table_t *hpack_table, const unsigned char *data, size_t data_size, hpack_callback callback, void *parameters) {
    /* allocates space for the position in the block, for the index of
    the representation and for the error code of the operations */
    size_t offset = 0;
    size_t index;
    size_t max_size;
    size_t list_size = 0;
    unsigned char byte;
    unsigned char prefix;
    char indexing;
    ERROR_CODE return_value;

    /* allocates space for the field being decoded and for the one
    that an index refers to */
    struct hpack_header_t hpack_header;
    struct hpack_header_t indexed;

    /* allocates the buffers that receive the literal parts of a
    field, they are the ones handed to the callback */
    unsigned char name[HPACK_MAX_NAME_SIZE];
    unsigned char value[HPACK_MAX_VALUE_SIZE];

    /* walks the block one representation at a time, the leading bits
    of the first byte are the ones that identify it */
    while(offset < data_size) {
        byte = data[offset];

        /* the indexed header field representation, both the name and
        the value are taken from one of the tables */
        if(byte & 0x80) {
            return_value = decode_integer_hpack(data, data_size, &offset, 7, &index);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            return_value = get_hpack_table(hpack_table, index, &hpack_header);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
        }
        /* the dynamic table size update, it carries no field at all
        and only changes the maximum size of the table */
        else if((byte & 0xe0) == 0x20) {
            return_value = decode_integer_hpack(data, data_size, &offset, 5, &max_size);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            return_value = resize_hpack_table(hpack_table, max_size);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            continue;
        }
        /* one of the literal representations, they differ on the size
        of the prefix and on whether the field joins the table, the
        never indexed one is handled as the one without indexing as
        this end never re-encodes what it has received */
        else {
            indexing = byte & 0x40 ? TRUE : FALSE;
            prefix = indexing ? 6 : 4;

            return_value = decode_integer_hpack(data, data_size, &offset, prefix, &index);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

            /* an index of zero means that the name is carried as a
            literal, otherwise it refers to one of the tables */
            if(index == 0) {
                return_value = decode_string_hpack(data, data_size, &offset, name, sizeof(name), &hpack_header.name_size);
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            } else {
                return_value = get_hpack_table(hpack_table, index, &indexed);
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

                /* copies the name out of the table, the insertion of
                the field is able to evict the very entry that it has
                been taken from */
                if(indexed.name_size > sizeof(name)) {
                    RAISE_ERROR_M(
                        RUNTIME_EXCEPTION_ERROR_CODE,
                        (unsigned char *) "Name above the accepted size"
                    );
                }
                memcpy(name, indexed.name, indexed.name_size);
                hpack_header.name_size = indexed.name_size;
            }

            hpack_header.name = name;

            return_value = decode_string_hpack(data, data_size, &offset, value, sizeof(value), &hpack_header.value_size);
            if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            hpack_header.value = value;

            if(indexing) {
                return_value = insert_hpack_table(hpack_table, &hpack_header);
                if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
            }
        }

        /* accounts the field against the maximum size of the header
        list, this is the guard against the expansion of a small block
        into a very large list of fields */
        list_size += hpack_header.name_size + hpack_header.value_size + HPACK_ENTRY_OVERHEAD;
        if(list_size > HPACK_MAX_HEADER_LIST_SIZE) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Header list above the accepted size"
            );
        }

        /* hands the field to the callback, an error returned from it
        aborts the decoding of the remainder of the block */
        return_value = callback(parameters, &hpack_header);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_hpack(struct hpack_table_t *hpack_table, unsigned char *buffer, size_t buffer_size, size_t *offset, struct hpack_header_t *hpack_header, char indexing) {
    /* allocates space for the index of the field in the tables and
    for the error code of the operations */
    size_t index;
    char complete;
    ERROR_CODE return_value;

    /* searches the tables for the field, a complete match allows the
    field to be carried by a single indexed representation */
    complete = find_hpack_table(hpack_table, hpack_header, &index);
    if(complete) {
        return_value = encode_integer_hpack(buffer, buffer_size, offset, 7, 0x80, index);
        RAISE_AGAIN(return_value);
    }

    /* writes the representation itself, the index found is the one of
    the name and is zero when not even that has matched */
    if(indexing) {
        return_value = encode_integer_hpack(buffer, buffer_size, offset, 6, 0x40, index);
    } else {
        return_value = encode_integer_hpack(buffer, buffer_size, offset, 4, 0x00, index);
    }
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* writes the name as a literal only when it is not addressable
    through one of the tables */
    if(index == 0) {
        return_value = encode_string_hpack(buffer, buffer_size, offset, hpack_header->name, hpack_header->name_size);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    }

    return_value = encode_string_hpack(buffer, buffer_size, offset, hpack_header->value, hpack_header->value_size);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* joins the field to the table so that the next occurrence of it
    is carried by a single indexed representation */
    if(indexing) {
        return_value = insert_hpack_table(hpack_table, hpack_header);
        if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}
