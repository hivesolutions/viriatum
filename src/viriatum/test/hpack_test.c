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

#include "hpack_test.h"

#ifdef VIRIATUM_HTTP2

/**
 * Counts the fields that reach it and gathers nothing at all, so
 * that a test of the limits of the decoding is bounded by those
 * rather than by the room of the collector.
 *
 * @param parameters The counter to be incremented.
 * @param hpack_header The header field that has been decoded.
 * @return The resulting error code.
 */
static ERROR_CODE _count_hpack_test(void *parameters, struct hpack_header_t *hpack_header) {
    (*(size_t *) parameters)++;
    RAISE_NO_ERROR;
}

ERROR_CODE collect_hpack_test(void *parameters, struct hpack_header_t *hpack_header) {
    /* retrieves the collector out of the opaque parameters and then
    verifies that there's still room for one more field */
    struct hpack_collector_t *collector = (struct hpack_collector_t *) parameters;

    if(collector->count >= HPACK_TEST_MAX_HEADERS) { RAISE_ERROR_S(RUNTIME_EXCEPTION_ERROR_CODE); }
    if(hpack_header->name_size >= HPACK_TEST_MAX_NAME) { RAISE_ERROR_S(RUNTIME_EXCEPTION_ERROR_CODE); }
    if(hpack_header->value_size >= HPACK_TEST_MAX_VALUE) { RAISE_ERROR_S(RUNTIME_EXCEPTION_ERROR_CODE); }

    /* copies both the name and the value into the collector, the
    decoding hands them over without a termination */
    memcpy(collector->names[collector->count], hpack_header->name, hpack_header->name_size);
    collector->names[collector->count][hpack_header->name_size] = '\0';
    memcpy(collector->values[collector->count], hpack_header->value, hpack_header->value_size);
    collector->values[collector->count][hpack_header->value_size] = '\0';
    collector->count++;

    /* raises no error */
    RAISE_NO_ERROR;
}

const char *test_hpack_table(void) {
    /* allocates space for the dynamic table and for the field that
    the retrieval operations are going to populate */
    struct hpack_table_t *hpack_table;
    struct hpack_header_t hpack_header;
    struct hpack_header_t inserted;
    ERROR_CODE error;

    /* creates the dynamic table and verifies that it starts empty
    and sized at the value advertised to the peer */
    create_hpack_table(&hpack_table);
    V_ASSERT_EQ_U(hpack_table->count, 0);
    V_ASSERT_EQ_U(hpack_table->size, 0);
    V_ASSERT_EQ_U(hpack_table->max_size, HPACK_TABLE_SIZE);

    /* the index one of the combined space is the first entry of the
    static table, which carries the authority pseudo header */
    error = get_hpack_table(hpack_table, 1, &hpack_header);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_MEM(hpack_header.name, ":authority", 10);
    V_ASSERT_EQ_U(hpack_header.name_size, 10);
    V_ASSERT_EQ_U(hpack_header.value_size, 0);

    /* the index two carries the method pseudo header together with
    the value that the specification pairs it with */
    error = get_hpack_table(hpack_table, 2, &hpack_header);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_MEM(hpack_header.name, ":method", 7);
    V_ASSERT_MEM(hpack_header.value, "GET", 3);

    /* the last index of the static table is the one that closes it,
    the specification defines exactly sixty one entries */
    error = get_hpack_table(hpack_table, HPACK_STATIC_SIZE, &hpack_header);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_MEM(hpack_header.name, "www-authenticate", 16);

    /* the index zero never refers to a field, it is the value that
    the literal representations use to signal a literal name */
    error = get_hpack_table(hpack_table, 0, &hpack_header);
    V_ASSERT(IS_ERROR_CODE(error));

    /* an index above the static table with an empty dynamic one has
    nothing to refer to and is refused */
    error = get_hpack_table(hpack_table, HPACK_STATIC_SIZE + 1, &hpack_header);
    V_ASSERT(IS_ERROR_CODE(error));

    /* inserts a field in the dynamic table and verifies that it is
    accounted with the overhead that the specification defines */
    inserted.name = (unsigned char *) "custom-key";
    inserted.name_size = 10;
    inserted.value = (unsigned char *) "custom-value";
    inserted.value_size = 12;
    error = insert_hpack_table(hpack_table, &inserted);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(hpack_table->count, 1);
    V_ASSERT_EQ_U(hpack_table->size, 10 + 12 + HPACK_ENTRY_OVERHEAD);

    /* the first index of the dynamic table refers to the entry that
    has just been inserted, as it is the newest one */
    error = get_hpack_table(hpack_table, HPACK_STATIC_SIZE + 1, &hpack_header);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_MEM(hpack_header.name, "custom-key", 10);
    V_ASSERT_MEM(hpack_header.value, "custom-value", 12);

    /* deletes the dynamic table, the entries it still holds are
    released together with it */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_table_resize(void) {
    /* allocates space for the dynamic table and for the field that
    is going to be inserted into it */
    struct hpack_table_t *hpack_table;
    struct hpack_header_t inserted;
    ERROR_CODE error;

    /* creates the dynamic table and inserts two entries of a known
    size so that the eviction may be observed */
    create_hpack_table(&hpack_table);
    inserted.name = (unsigned char *) "first";
    inserted.name_size = 5;
    inserted.value = (unsigned char *) "value";
    inserted.value_size = 5;
    insert_hpack_table(hpack_table, &inserted);
    inserted.name = (unsigned char *) "second";
    inserted.name_size = 6;
    insert_hpack_table(hpack_table, &inserted);
    V_ASSERT_EQ_U(hpack_table->count, 2);
    V_ASSERT_EQ_U(hpack_table->size, (5 + 5 + 32) + (6 + 5 + 32));

    /* reduces the table so that only the newest entry fits, the
    oldest one is the one that has to be evicted */
    error = resize_hpack_table(hpack_table, 45);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(hpack_table->count, 1);
    V_ASSERT_EQ_U(hpack_table->size, 6 + 5 + 32);

    /* reduces the table to nothing, every one of the entries has to
    be evicted for it to fit */
    error = resize_hpack_table(hpack_table, 0);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(hpack_table->count, 0);
    V_ASSERT_EQ_U(hpack_table->size, 0);

    /* a size above the one advertised to the peer is refused, the
    peer is never allowed to grow the table beyond it */
    error = resize_hpack_table(hpack_table, HPACK_TABLE_SIZE + 1);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(hpack_table->max_size, 0);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_table_insert(void) {
    /* allocates space for the dynamic table, for the field being
    inserted and for the buffer of an oversized value */
    struct hpack_table_t *hpack_table;
    struct hpack_header_t inserted;
    struct hpack_header_t hpack_header;
    unsigned char value[HPACK_TABLE_SIZE];
    size_t index;

    /* creates the dynamic table and fills it with entries of the
    smallest possible size, which is the constant overhead */
    create_hpack_table(&hpack_table);
    inserted.name = (unsigned char *) "";
    inserted.name_size = 0;
    inserted.value = (unsigned char *) "";
    inserted.value_size = 0;
    for(index = 0; index < HPACK_MAX_ENTRIES; index++) {
        insert_hpack_table(hpack_table, &inserted);
    }
    V_ASSERT_EQ_U(hpack_table->count, HPACK_MAX_ENTRIES);
    V_ASSERT_EQ_U(hpack_table->size, HPACK_TABLE_SIZE);

    /* one more insertion has to evict the oldest entry, the number
    of entries is bounded by the size of the table */
    insert_hpack_table(hpack_table, &inserted);
    V_ASSERT_EQ_U(hpack_table->count, HPACK_MAX_ENTRIES);
    V_ASSERT_EQ_U(hpack_table->size, HPACK_TABLE_SIZE);

    /* an entry larger than the complete table empties it and is then
    not inserted, which is the behaviour of the specification */
    memset(value, 'a', sizeof(value));
    inserted.name = (unsigned char *) "name";
    inserted.name_size = 4;
    inserted.value = value;
    inserted.value_size = sizeof(value);
    insert_hpack_table(hpack_table, &inserted);
    V_ASSERT_EQ_U(hpack_table->count, 0);
    V_ASSERT_EQ_U(hpack_table->size, 0);

    /* the emptied table has nothing to refer to, so the first index
    of the dynamic space is once again refused */
    V_ASSERT(IS_ERROR_CODE(get_hpack_table(hpack_table, HPACK_STATIC_SIZE + 1, &hpack_header)));

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_table_find(void) {
    /* allocates space for the dynamic table, for the field being
    searched for and for the index that the search reports */
    struct hpack_table_t *hpack_table;
    struct hpack_header_t hpack_header;
    size_t index;
    char complete;

    /* creates the dynamic table, the search starts by covering only
    the static part of the address space */
    create_hpack_table(&hpack_table);

    /* a field that matches both the name and the value of a static
    entry is reported as a complete match */
    hpack_header.name = (unsigned char *) ":method";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) "GET";
    hpack_header.value_size = 3;
    complete = find_hpack_table(hpack_table, &hpack_header, &index);
    V_ASSERT(complete == TRUE);
    V_ASSERT_EQ_U(index, 2);

    /* a field whose value differs is reported as a match of the name
    alone, the index is the one of the first entry carrying it */
    hpack_header.value = (unsigned char *) "PUT";
    complete = find_hpack_table(hpack_table, &hpack_header, &index);
    V_ASSERT(complete == FALSE);
    V_ASSERT_EQ_U(index, 2);

    /* a field whose name is unknown matches nothing at all and the
    index is left at the value that refers to no entry */
    hpack_header.name = (unsigned char *) "custom-key";
    hpack_header.name_size = 10;
    hpack_header.value = (unsigned char *) "custom-value";
    hpack_header.value_size = 12;
    complete = find_hpack_table(hpack_table, &hpack_header, &index);
    V_ASSERT(complete == FALSE);
    V_ASSERT_EQ_U(index, 0);

    /* inserting the field makes it addressable, the dynamic part of
    the space starts right after the static one */
    insert_hpack_table(hpack_table, &hpack_header);
    complete = find_hpack_table(hpack_table, &hpack_header, &index);
    V_ASSERT(complete == TRUE);
    V_ASSERT_EQ_U(index, HPACK_STATIC_SIZE + 1);

    /* a value that differs from the one held by the dynamic entry is
    once again reported as a match of the name alone */
    hpack_header.value = (unsigned char *) "other-value";
    hpack_header.value_size = 11;
    complete = find_hpack_table(hpack_table, &hpack_header, &index);
    V_ASSERT(complete == FALSE);
    V_ASSERT_EQ_U(index, HPACK_STATIC_SIZE + 1);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_integer(void) {
    /* allocates space for the buffer of the encoding, for the
    position in it and for the decoded value */
    unsigned char buffer[16];
    size_t offset;
    size_t value;
    ERROR_CODE error;

    /* the example C.1.1 of the specification, the value ten fits in
    a prefix of five bits and so no continuation follows it */
    offset = 0;
    error = encode_integer_hpack(buffer, sizeof(buffer), &offset, 5, 0x00, 10);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(offset, 1);
    V_ASSERT_EQ_U(buffer[0], 0x0a);

    offset = 0;
    error = decode_integer_hpack(buffer, 1, &offset, 5, &value);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(value, 10);
    V_ASSERT_EQ_U(offset, 1);

    /* the example C.1.2 of the specification, the value saturates
    the prefix and is carried by two continuation bytes */
    offset = 0;
    error = encode_integer_hpack(buffer, sizeof(buffer), &offset, 5, 0x00, 1337);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(offset, 3);
    V_ASSERT_EQ_U(buffer[0], 0x1f);
    V_ASSERT_EQ_U(buffer[1], 0x9a);
    V_ASSERT_EQ_U(buffer[2], 0x0a);

    offset = 0;
    error = decode_integer_hpack(buffer, 3, &offset, 5, &value);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(value, 1337);
    V_ASSERT_EQ_U(offset, 3);

    /* the example C.1.3 of the specification, the value starts at an
    octet boundary and so the prefix takes the complete byte */
    offset = 0;
    error = encode_integer_hpack(buffer, sizeof(buffer), &offset, 8, 0x00, 42);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(offset, 1);
    V_ASSERT_EQ_U(buffer[0], 0x2a);

    offset = 0;
    error = decode_integer_hpack(buffer, 1, &offset, 8, &value);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(value, 42);

    /* the bits that sit above the prefix are preserved, they are the
    ones identifying the representation being written */
    offset = 0;
    error = encode_integer_hpack(buffer, sizeof(buffer), &offset, 7, 0x80, 2);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(buffer[0], 0x82);

    /* a saturated prefix with no continuation byte behind it is a
    truncated encoding and is refused */
    buffer[0] = 0x1f;
    offset = 0;
    error = decode_integer_hpack(buffer, 1, &offset, 5, &value);
    V_ASSERT(IS_ERROR_CODE(error));

    /* an empty buffer carries no prefix byte at all */
    offset = 0;
    error = decode_integer_hpack(buffer, 0, &offset, 5, &value);
    V_ASSERT(IS_ERROR_CODE(error));

    /* an encoding whose continuation would take the value beyond the
    accepted range is refused instead of being wrapped around */
    buffer[0] = 0x1f;
    buffer[1] = 0xff;
    buffer[2] = 0xff;
    buffer[3] = 0xff;
    buffer[4] = 0xff;
    buffer[5] = 0xff;
    buffer[6] = 0x7f;
    offset = 0;
    error = decode_integer_hpack(buffer, 7, &offset, 5, &value);
    V_ASSERT(IS_ERROR_CODE(error));

    /* an encoding padded with continuation bytes that carry no value
    at all is refused too, a peer is otherwise able to make a small
    integer consume an unbounded number of bytes */
    buffer[0] = 0x1f;
    buffer[1] = 0x80;
    buffer[2] = 0x80;
    buffer[3] = 0x80;
    buffer[4] = 0x80;
    buffer[5] = 0x80;
    buffer[6] = 0x00;
    offset = 0;
    error = decode_integer_hpack(buffer, 7, &offset, 5, &value);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a buffer that has no room for the encoding is refused rather
    than being written past its end */
    offset = 0;
    error = encode_integer_hpack(buffer, 1, &offset, 5, 0x00, 1337);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a buffer that takes the continuation but not the byte that
    closes the value is refused just the same */
    offset = 0;
    error = encode_integer_hpack(buffer, 2, &offset, 5, 0x00, 1337);
    V_ASSERT(IS_ERROR_CODE(error));

    offset = 0;
    error = encode_integer_hpack(buffer, 0, &offset, 5, 0x00, 1);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a value that needs more than one continuation byte runs out of
    room while the continuation is still being written */
    offset = 0;
    error = encode_integer_hpack(buffer, 2, &offset, 5, 0x00, 16415);
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_string(void) {
    /* allocates space for the buffers of the encoding and of the
    decoding together with the sizes that they report */
    unsigned char buffer[64];
    unsigned char result[64];
    unsigned char raw[16];
    size_t offset;
    size_t result_size;
    ERROR_CODE error;

    /* a string whose coded form is not shorter than the raw one is
    carried raw, the leading bit of the length says so */
    offset = 0;
    error = encode_string_hpack(buffer, sizeof(buffer), &offset, (unsigned char *) "ab", 2);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(buffer[0] & 0x80, 0x00);
    V_ASSERT_EQ_U(buffer[0] & 0x7f, 2);

    offset = 0;
    error = decode_string_hpack(buffer, 3, &offset, result, sizeof(result), &result_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(result_size, 2);
    V_ASSERT_MEM(result, "ab", 2);

    /* a string whose coded form is shorter is carried coded, the
    authority of the vectors of the specification is such a case */
    offset = 0;
    error = encode_string_hpack(buffer, sizeof(buffer), &offset, (unsigned char *) "www.example.com", 15);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(buffer[0], 0x8c);
    V_ASSERT_EQ_U(offset, 13);

    offset = 0;
    error = decode_string_hpack(buffer, 13, &offset, result, sizeof(result), &result_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(result_size, 15);
    V_ASSERT_MEM(result, "www.example.com", 15);
    V_ASSERT_EQ_U(offset, 13);

    /* a raw string built by hand is decoded as it stands, this is
    the form the appendix C.2 of the specification describes */
    raw[0] = 0x0a;
    memcpy(&raw[1], "custom-key", 10);
    offset = 0;
    error = decode_string_hpack(raw, 11, &offset, result, sizeof(result), &result_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(result_size, 10);
    V_ASSERT_MEM(result, "custom-key", 10);

    /* a length that goes beyond the buffer that carries it is a
    truncated representation and is refused */
    raw[0] = 0x0a;
    offset = 0;
    error = decode_string_hpack(raw, 5, &offset, result, sizeof(result), &result_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a raw string that does not fit in the buffer of the caller is
    refused before any of it is copied */
    raw[0] = 0x0a;
    offset = 0;
    error = decode_string_hpack(raw, 11, &offset, result, 4, &result_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* an empty buffer carries no length byte at all */
    offset = 0;
    error = decode_string_hpack(raw, 0, &offset, result, sizeof(result), &result_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a buffer that has no room for the string is refused in both of
    the representations that the encoding is able to pick */
    offset = 0;
    error = encode_string_hpack(buffer, 2, &offset, (unsigned char *) "ab", 2);
    V_ASSERT(IS_ERROR_CODE(error));

    offset = 0;
    error = encode_string_hpack(buffer, 4, &offset, (unsigned char *) "www.example.com", 15);
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_decode_request(void) {
    /* allocates space for the dynamic table and for the collector
    that gathers the fields out of each one of the blocks */
    struct hpack_table_t *hpack_table;
    struct hpack_collector_t collector;
    ERROR_CODE error;

    /* the three request blocks of the appendix C.3 of the
    specification, they carry no coded string at all */
    unsigned char first[] = {
        0x82, 0x86, 0x84, 0x41, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65,
        0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d
    };
    unsigned char second[] = {
        0x82, 0x86, 0x84, 0xbe, 0x58, 0x08, 0x6e, 0x6f, 0x2d, 0x63,
        0x61, 0x63, 0x68, 0x65
    };
    unsigned char third[] = {
        0x82, 0x87, 0x85, 0xbf, 0x40, 0x0a, 0x63, 0x75, 0x73, 0x74,
        0x6f, 0x6d, 0x2d, 0x6b, 0x65, 0x79, 0x0c, 0x63, 0x75, 0x73,
        0x74, 0x6f, 0x6d, 0x2d, 0x76, 0x61, 0x6c, 0x75, 0x65
    };

    /* creates the dynamic table, the three blocks are decoded in
    sequence over the very same table */
    create_hpack_table(&hpack_table);

    /* the first block carries the authority as a literal that joins
    the table, everything else is already indexed */
    collector.count = 0;
    error = decode_hpack(hpack_table, first, sizeof(first), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 4);
    V_ASSERT_EQ_S(collector.names[0], ":method");
    V_ASSERT_EQ_S(collector.values[0], "GET");
    V_ASSERT_EQ_S(collector.names[1], ":scheme");
    V_ASSERT_EQ_S(collector.values[1], "http");
    V_ASSERT_EQ_S(collector.names[2], ":path");
    V_ASSERT_EQ_S(collector.values[2], "/");
    V_ASSERT_EQ_S(collector.names[3], ":authority");
    V_ASSERT_EQ_S(collector.values[3], "www.example.com");
    V_ASSERT_EQ_U(hpack_table->count, 1);
    V_ASSERT_EQ_U(hpack_table->size, 57);

    /* the second block refers to the authority through the entry
    that the first one has added to the table */
    collector.count = 0;
    error = decode_hpack(hpack_table, second, sizeof(second), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 5);
    V_ASSERT_EQ_S(collector.names[3], ":authority");
    V_ASSERT_EQ_S(collector.values[3], "www.example.com");
    V_ASSERT_EQ_S(collector.names[4], "cache-control");
    V_ASSERT_EQ_S(collector.values[4], "no-cache");
    V_ASSERT_EQ_U(hpack_table->count, 2);
    V_ASSERT_EQ_U(hpack_table->size, 110);

    /* the third block carries a field whose name is a literal too,
    exercising the representation with a literal name */
    collector.count = 0;
    error = decode_hpack(hpack_table, third, sizeof(third), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 5);
    V_ASSERT_EQ_S(collector.names[1], ":scheme");
    V_ASSERT_EQ_S(collector.values[1], "https");
    V_ASSERT_EQ_S(collector.names[2], ":path");
    V_ASSERT_EQ_S(collector.values[2], "/index.html");
    V_ASSERT_EQ_S(collector.names[4], "custom-key");
    V_ASSERT_EQ_S(collector.values[4], "custom-value");
    V_ASSERT_EQ_U(hpack_table->count, 3);
    V_ASSERT_EQ_U(hpack_table->size, 164);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_decode_request_huffman(void) {
    /* allocates space for the dynamic table and for the collector
    that gathers the fields out of each one of the blocks */
    struct hpack_table_t *hpack_table;
    struct hpack_collector_t collector;
    ERROR_CODE error;

    /* the three request blocks of the appendix C.4 of the
    specification, the literals of these ones are coded */
    unsigned char first[] = {
        0x82, 0x86, 0x84, 0x41, 0x8c, 0xf1, 0xe3, 0xc2, 0xe5, 0xf2,
        0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff
    };
    unsigned char second[] = {
        0x82, 0x86, 0x84, 0xbe, 0x58, 0x86, 0xa8, 0xeb, 0x10, 0x64,
        0x9c, 0xbf
    };
    unsigned char third[] = {
        0x82, 0x87, 0x85, 0xbf, 0x40, 0x88, 0x25, 0xa8, 0x49, 0xe9,
        0x5b, 0xa9, 0x7d, 0x7f, 0x89, 0x25, 0xa8, 0x49, 0xe9, 0x5b,
        0xb8, 0xe8, 0xb4, 0xbf
    };

    /* creates the dynamic table, the three blocks are decoded in
    sequence over the very same table */
    create_hpack_table(&hpack_table);

    /* the coded authority decodes to the very same value as the raw
    one of the previous vector */
    collector.count = 0;
    error = decode_hpack(hpack_table, first, sizeof(first), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 4);
    V_ASSERT_EQ_S(collector.names[3], ":authority");
    V_ASSERT_EQ_S(collector.values[3], "www.example.com");
    V_ASSERT_EQ_U(hpack_table->size, 57);

    collector.count = 0;
    error = decode_hpack(hpack_table, second, sizeof(second), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 5);
    V_ASSERT_EQ_S(collector.names[4], "cache-control");
    V_ASSERT_EQ_S(collector.values[4], "no-cache");
    V_ASSERT_EQ_U(hpack_table->size, 110);

    collector.count = 0;
    error = decode_hpack(hpack_table, third, sizeof(third), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 5);
    V_ASSERT_EQ_S(collector.names[4], "custom-key");
    V_ASSERT_EQ_S(collector.values[4], "custom-value");
    V_ASSERT_EQ_U(hpack_table->size, 164);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_decode_response(void) {
    /* allocates space for the dynamic table and for the collector
    that gathers the fields out of each one of the blocks */
    struct hpack_table_t *hpack_table;
    struct hpack_collector_t collector;
    ERROR_CODE error;

    /* the three response blocks of the appendix C.6 of the
    specification, they are decoded against a table reduced to two
    hundred and fifty six bytes so that the eviction takes place */
    unsigned char first[] = {
        0x48, 0x82, 0x64, 0x02, 0x58, 0x85, 0xae, 0xc3, 0x77, 0x1a,
        0x4b, 0x61, 0x96, 0xd0, 0x7a, 0xbe, 0x94, 0x10, 0x54, 0xd4,
        0x44, 0xa8, 0x20, 0x05, 0x95, 0x04, 0x0b, 0x81, 0x66, 0xe0,
        0x82, 0xa6, 0x2d, 0x1b, 0xff, 0x6e, 0x91, 0x9d, 0x29, 0xad,
        0x17, 0x18, 0x63, 0xc7, 0x8f, 0x0b, 0x97, 0xc8, 0xe9, 0xae,
        0x82, 0xae, 0x43, 0xd3
    };
    unsigned char second[] = {
        0x48, 0x83, 0x64, 0x0e, 0xff, 0xc1, 0xc0, 0xbf
    };
    unsigned char third[] = {
        0x88, 0xc1, 0x61, 0x96, 0xd0, 0x7a, 0xbe, 0x94, 0x10, 0x54,
        0xd4, 0x44, 0xa8, 0x20, 0x05, 0x95, 0x04, 0x0b, 0x81, 0x66,
        0xe0, 0x84, 0xa6, 0x2d, 0x1b, 0xff, 0xc0, 0x5a, 0x83, 0x9b,
        0xd9, 0xab, 0x77, 0xad, 0x94, 0xe7, 0x82, 0x1d, 0xd7, 0xf2,
        0xe6, 0xc7, 0xb3, 0x35, 0xdf, 0xdf, 0xcd, 0x5b, 0x39, 0x60,
        0xd5, 0xaf, 0x27, 0x08, 0x7f, 0x36, 0x72, 0xc1, 0xab, 0x27,
        0x0f, 0xb5, 0x29, 0x1f, 0x95, 0x87, 0x31, 0x60, 0x65, 0xc0,
        0x03, 0xed, 0x4e, 0xe5, 0xb1, 0x06, 0x3d, 0x50, 0x07
    };

    /* creates the dynamic table and reduces it so that the eviction
    of the oldest entries becomes observable */
    create_hpack_table(&hpack_table);
    resize_hpack_table(hpack_table, 256);

    /* the first block adds four entries to the table, which is
    exactly what the reduced size is able to hold */
    collector.count = 0;
    error = decode_hpack(hpack_table, first, sizeof(first), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 4);
    V_ASSERT_EQ_S(collector.names[0], ":status");
    V_ASSERT_EQ_S(collector.values[0], "302");
    V_ASSERT_EQ_S(collector.names[1], "cache-control");
    V_ASSERT_EQ_S(collector.values[1], "private");
    V_ASSERT_EQ_S(collector.names[2], "date");
    V_ASSERT_EQ_S(collector.values[2], "Mon, 21 Oct 2013 20:13:21 GMT");
    V_ASSERT_EQ_S(collector.names[3], "location");
    V_ASSERT_EQ_S(collector.values[3], "https://www.example.com");
    V_ASSERT_EQ_U(hpack_table->count, 4);
    V_ASSERT_EQ_U(hpack_table->size, 222);

    /* the second block changes only the status, the entry it adds
    evicts the previous status and is of the very same size, so the
    table ends up holding as much as it did before */
    collector.count = 0;
    error = decode_hpack(hpack_table, second, sizeof(second), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 4);
    V_ASSERT_EQ_S(collector.values[0], "307");
    V_ASSERT_EQ_S(collector.values[3], "https://www.example.com");
    V_ASSERT_EQ_U(hpack_table->count, 4);
    V_ASSERT_EQ_U(hpack_table->size, 222);

    /* the third block carries a body and evicts every one of the
    entries that no longer fits the reduced table */
    collector.count = 0;
    error = decode_hpack(hpack_table, third, sizeof(third), collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 6);
    V_ASSERT_EQ_S(collector.values[0], "200");
    V_ASSERT_EQ_S(collector.values[2], "Mon, 21 Oct 2013 20:13:22 GMT");
    V_ASSERT_EQ_S(collector.names[4], "content-encoding");
    V_ASSERT_EQ_S(collector.values[4], "gzip");
    V_ASSERT_EQ_S(collector.names[5], "set-cookie");
    V_ASSERT_EQ_U(hpack_table->count, 3);
    V_ASSERT_EQ_U(hpack_table->size, 215);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_decode_errors(void) {
    /* allocates space for the dynamic table, for the collector and
    for the blocks that are expected to be refused */
    struct hpack_table_t *hpack_table;
    struct hpack_collector_t collector;
    unsigned char block[8];
    ERROR_CODE error;

    /* creates the dynamic table, every one of the blocks is decoded
    against a table that starts empty */
    create_hpack_table(&hpack_table);

    /* an indexed representation carrying the index zero refers to no
    field at all and is refused */
    block[0] = 0x80;
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 1, collect_hpack_test, (void *) &collector);
    V_ASSERT(IS_ERROR_CODE(error));

    /* an indexed representation above the size of both tables has
    nothing to refer to and is refused */
    block[0] = 0xff;
    block[1] = 0x00;
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 2, collect_hpack_test, (void *) &collector);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a literal representation whose value is missing altogether is
    a truncated block and is refused */
    block[0] = 0x40;
    block[1] = 0x02;
    block[2] = 'a';
    block[3] = 'b';
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 4, collect_hpack_test, (void *) &collector);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a size update of exactly the advertised value is accepted, the
    limit of the specification is an inclusive one */
    block[0] = 0x3f;
    block[1] = 0xe1;
    block[2] = 0x1f;
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 3, collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(hpack_table->max_size, HPACK_TABLE_SIZE);

    /* a size update of one byte above it is refused, the table is
    never allowed to grow beyond what the peer has been told */
    block[0] = 0x3f;
    block[1] = 0xe2;
    block[2] = 0x1f;
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 3, collect_hpack_test, (void *) &collector);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a size update inside the advertised range is accepted and
    changes the maximum size of the table */
    block[0] = 0x3f;
    block[1] = 0x00;
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 2, collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(hpack_table->max_size, 31);
    V_ASSERT_EQ_U(collector.count, 0);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_decode_limits(void) {
    /* allocates space for the dynamic table, for the collector and
    for the blocks that exercise each one of the limits */
    struct hpack_table_t *hpack_table;
    struct hpack_collector_t collector;
    struct hpack_header_t inserted;
    unsigned char block[HPACK_MAX_HEADER_LIST_SIZE / HPACK_ENTRY_OVERHEAD];
    unsigned char name[HPACK_MAX_NAME_SIZE + 16];
    size_t index;
    size_t count;
    ERROR_CODE error;

    /* creates the dynamic table, the blocks that follow are decoded
    against a table that starts empty */
    create_hpack_table(&hpack_table);

    /* a block made only of indexed representations expands into a
    header list far larger than the block itself, which is the
    compression bomb that the limit of the list guards against, the
    fields are only counted so that nothing else bounds the block */
    for(index = 0; index < sizeof(block); index++) { block[index] = 0x82; }
    count = 0;
    error = decode_hpack(hpack_table, block, sizeof(block), _count_hpack_test, (void *) &count);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT(count > HPACK_TEST_MAX_HEADERS);

    /* an entry whose name is larger than the buffer that receives it
    is refused when a representation refers to it, the name has to be
    copied out of the table before the field joins it */
    memset(name, 'a', sizeof(name));
    inserted.name = name;
    inserted.name_size = sizeof(name);
    inserted.value = (unsigned char *) "";
    inserted.value_size = 0;
    insert_hpack_table(hpack_table, &inserted);
    V_ASSERT_EQ_U(hpack_table->count, 1);

    block[0] = 0x40 | (HPACK_STATIC_SIZE + 1);
    block[1] = 0x00;
    collector.count = 0;
    error = decode_hpack(hpack_table, block, 2, collect_hpack_test, (void *) &collector);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a size update is only ever allowed at the very start of a
    block, the one that follows a field is a decoding error */
    block[0] = 0x82;
    block[1] = 0x20;
    count = 0;
    error = decode_hpack(hpack_table, block, 2, _count_hpack_test, (void *) &count);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_U(count, 1);

    /* the very same update at the start of the block is accepted, it
    is the place the specification puts it at */
    block[0] = 0x20;
    block[1] = 0x82;
    count = 0;
    error = decode_hpack(hpack_table, block, 2, _count_hpack_test, (void *) &count);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(count, 1);
    V_ASSERT_EQ_U(hpack_table->max_size, 0);

    /* deletes the dynamic table */
    delete_hpack_table(hpack_table);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_encode(void) {
    /* allocates space for the tables of the two ends, for the buffer
    of the encoding and for the collector of the decoding */
    struct hpack_table_t *encoder;
    struct hpack_table_t *decoder;
    struct hpack_collector_t collector;
    struct hpack_header_t hpack_header;
    unsigned char buffer[256];
    size_t offset;
    ERROR_CODE error;

    /* creates the table of each one of the ends, what one of them
    encodes the other one has to decode into the same fields */
    create_hpack_table(&encoder);
    create_hpack_table(&decoder);

    /* a field that is already in the static table is carried by a
    single indexed representation */
    hpack_header.name = (unsigned char *) ":status";
    hpack_header.name_size = 7;
    hpack_header.value = (unsigned char *) "200";
    hpack_header.value_size = 3;
    offset = 0;
    error = encode_hpack(encoder, buffer, sizeof(buffer), &offset, &hpack_header, FALSE);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(offset, 1);
    V_ASSERT_EQ_U(buffer[0], 0x88);

    /* a field whose name alone is in the static table carries the
    index of the name and the value as a literal */
    hpack_header.name = (unsigned char *) "server";
    hpack_header.name_size = 6;
    hpack_header.value = (unsigned char *) "viriatum";
    hpack_header.value_size = 8;
    error = encode_hpack(encoder, buffer, sizeof(buffer), &offset, &hpack_header, TRUE);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(encoder->count, 1);

    /* a field that is unknown to both of the tables carries both the
    name and the value as literals */
    hpack_header.name = (unsigned char *) "x-viriatum";
    hpack_header.name_size = 10;
    hpack_header.value = (unsigned char *) "on";
    hpack_header.value_size = 2;
    error = encode_hpack(encoder, buffer, sizeof(buffer), &offset, &hpack_header, TRUE);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(encoder->count, 2);

    /* the block that has been produced decodes into the very same
    fields on the other end of the connection */
    collector.count = 0;
    error = decode_hpack(decoder, buffer, offset, collect_hpack_test, (void *) &collector);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 3);
    V_ASSERT_EQ_S(collector.names[0], ":status");
    V_ASSERT_EQ_S(collector.values[0], "200");
    V_ASSERT_EQ_S(collector.names[1], "server");
    V_ASSERT_EQ_S(collector.values[1], "viriatum");
    V_ASSERT_EQ_S(collector.names[2], "x-viriatum");
    V_ASSERT_EQ_S(collector.values[2], "on");

    /* both of the tables have followed the very same path and so
    they hold the same number of entries */
    V_ASSERT_EQ_U(decoder->count, encoder->count);

    /* the field that has joined the table on the previous encoding
    is now carried by a single indexed representation */
    hpack_header.name = (unsigned char *) "x-viriatum";
    hpack_header.name_size = 10;
    hpack_header.value = (unsigned char *) "on";
    hpack_header.value_size = 2;
    offset = 0;
    error = encode_hpack(encoder, buffer, sizeof(buffer), &offset, &hpack_header, FALSE);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(offset, 1);

    /* a buffer that has no room for the field is refused rather than
    being written past its end */
    offset = 0;
    error = encode_hpack(encoder, buffer, 1, &offset, &hpack_header, FALSE);
    V_ASSERT_EQ_U(error, 0);

    hpack_header.name = (unsigned char *) "x-other";
    hpack_header.name_size = 7;
    offset = 0;
    error = encode_hpack(encoder, buffer, 2, &offset, &hpack_header, FALSE);
    V_ASSERT(IS_ERROR_CODE(error));

    /* deletes the tables of both of the ends */
    delete_hpack_table(encoder);
    delete_hpack_table(decoder);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_huffman(void) {
    /* allocates space for the buffers of the coding and of the
    decoding together with the sizes that they report */
    unsigned char buffer[64];
    unsigned char result[64];
    size_t result_size;
    size_t decoded_size;
    ERROR_CODE error;

    /* the authority of the vectors of the specification codes into
    twelve bytes, three fewer than the raw form */
    V_ASSERT_EQ_U(size_huffman_hpack((unsigned char *) "www.example.com", 15), 12);

    error = encode_huffman_hpack((unsigned char *) "www.example.com", 15, buffer, sizeof(buffer), &result_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(result_size, 12);
    V_ASSERT_EQ_U(buffer[0], 0xf1);
    V_ASSERT_EQ_U(buffer[11], 0xff);

    error = decode_huffman_hpack(buffer, result_size, result, sizeof(result), &decoded_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(decoded_size, 15);
    V_ASSERT_MEM(result, "www.example.com", 15);

    /* the cache control value of the vectors codes into six bytes,
    exercising a different set of code lengths */
    error = encode_huffman_hpack((unsigned char *) "no-cache", 8, buffer, sizeof(buffer), &result_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(result_size, 6);
    V_ASSERT_EQ_U(buffer[0], 0xa8);

    error = decode_huffman_hpack(buffer, result_size, result, sizeof(result), &decoded_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(decoded_size, 8);
    V_ASSERT_MEM(result, "no-cache", 8);

    /* an empty buffer codes into nothing at all, there are no bits
    to be padded into a trailing byte */
    error = encode_huffman_hpack((unsigned char *) "", 0, buffer, sizeof(buffer), &result_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(result_size, 0);

    error = decode_huffman_hpack(buffer, 0, result, sizeof(result), &decoded_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(decoded_size, 0);

    /* a byte that is coded with one of the longest codes of the
    table survives the round trip just as the short ones do */
    error = encode_huffman_hpack((unsigned char *) "\x00\x0a\x16", 3, buffer, sizeof(buffer), &result_size);
    V_ASSERT_EQ_U(error, 0);
    error = decode_huffman_hpack(buffer, result_size, result, sizeof(result), &decoded_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(decoded_size, 3);
    V_ASSERT_MEM(result, "\x00\x0a\x16", 3);

    /* a buffer that has no room for the coded form is refused rather
    than being written past its end */
    error = encode_huffman_hpack((unsigned char *) "www.example.com", 15, buffer, 4, &result_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a buffer that holds every one of the coded bytes but has no
    room left for the padded one is refused in the same way */
    error = encode_huffman_hpack((unsigned char *) "www.example.com", 15, buffer, 11, &result_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a buffer that has no room for the decoded form is refused in
    the very same way */
    error = encode_huffman_hpack((unsigned char *) "www.example.com", 15, buffer, sizeof(buffer), &result_size);
    V_ASSERT_EQ_U(error, 0);
    error = decode_huffman_hpack(buffer, result_size, result, 4, &decoded_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hpack_huffman_errors(void) {
    /* allocates space for the coded buffer being refused and for the
    one that receives the decoding */
    unsigned char buffer[8];
    unsigned char result[16];
    size_t decoded_size;
    ERROR_CODE error;

    /* a padding whose bits are not the most significant ones of the
    end of string symbol is refused, they must all be set */
    buffer[0] = 0x00;
    error = decode_huffman_hpack(buffer, 1, result, sizeof(result), &decoded_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a padding that takes a complete byte is refused, it means that
    a symbol has been left unfinished */
    buffer[0] = 0xff;
    error = decode_huffman_hpack(buffer, 1, result, sizeof(result), &decoded_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a symbol that completes and leaves a padding whose bits are not
    all set is refused just the same */
    buffer[0] = 0xff;
    buffer[1] = 0xf0;
    error = decode_huffman_hpack(buffer, 2, result, sizeof(result), &decoded_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* the end of string symbol is never allowed to appear in the
    coded data, only the prefix of it is, as the padding */
    buffer[0] = 0xff;
    buffer[1] = 0xff;
    buffer[2] = 0xff;
    buffer[3] = 0xff;
    error = decode_huffman_hpack(buffer, 4, result, sizeof(result), &decoded_size);
    V_ASSERT(IS_ERROR_CODE(error));

    /* a valid padding of the smallest code of the table is accepted,
    which is the counterpart of the first of the refusals */
    buffer[0] = 0x07;
    error = decode_huffman_hpack(buffer, 1, result, sizeof(result), &decoded_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(decoded_size, 1);
    V_ASSERT_EQ_U(result[0], '0');

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

#endif
