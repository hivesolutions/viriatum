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

#pragma once

#include "../http/hpack.h"
#include "test_support.h"

/**
 * The maximum number of header fields that the collector of
 * the decoding tests is able to gather out of a block.
 */
#define HPACK_TEST_MAX_HEADERS 16

/**
 * The maximum sizes of the name and of the value that the
 * collector of the decoding tests is able to hold, the vectors
 * of the specification stay well inside both of them.
 */
#define HPACK_TEST_MAX_NAME 64
#define HPACK_TEST_MAX_VALUE 128

/**
 * Gathers the header fields that the decoding of a block
 * produces, so that they may be compared against the ones the
 * specification describes for the vector.
 */
typedef struct hpack_collector_t {
    /**
     * The names of the gathered fields, terminated so that they
     * may be compared as strings.
     */
    char names[HPACK_TEST_MAX_HEADERS][HPACK_TEST_MAX_NAME];

    /**
     * The values of the gathered fields, terminated so that they
     * may be compared as strings.
     */
    char values[HPACK_TEST_MAX_HEADERS][HPACK_TEST_MAX_VALUE];

    /**
     * The number of fields that have been gathered so far.
     */
    size_t count;
} hpack_collector_t;

/**
 * Gathers a single header field into the provided collector,
 * this is the callback handed to the decoding operation.
 *
 * @param parameters The collector to gather the field into.
 * @param hpack_header The header field that has been decoded.
 * @return The resulting error code.
 */
ERROR_CODE collect_hpack_test(void *parameters, struct hpack_header_t *hpack_header);

/**
 * Tests the life-cycle of the dynamic table together with the
 * retrieval of a field out of the combined address space of
 * the static and the dynamic tables.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_table(void);

/**
 * Tests that a change of the maximum size of the dynamic table
 * evicts as many of the oldest entries as required and that a
 * size above the advertised one is refused.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_table_resize(void);

/**
 * Tests the eviction of the dynamic table, both the one driven
 * by the insertion of a new entry and the emptying caused by an
 * entry larger than the table itself.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_table_insert(void);

/**
 * Tests the search of the tables, verifying that a match of
 * both the name and the value is preferred over one of the
 * name alone.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_table_find(void);

/**
 * Tests the prefix based representation of an integer against
 * the examples of the appendix C.1 of the specification, the
 * refusal of an overflowing encoding included.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_integer(void);

/**
 * Tests the representation of a string literal in both the raw
 * and the coded forms, including the refusal of one that does
 * not fit in the buffer of the caller.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_string(void);

/**
 * Tests the decoding of the request blocks of the appendix C.3
 * of the specification, which carry no coded string.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_decode_request(void);

/**
 * Tests the decoding of the request blocks of the appendix C.4
 * of the specification, which carry coded strings.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_decode_request_huffman(void);

/**
 * Tests the decoding of the response blocks of the appendix
 * C.6 of the specification, which exercise the eviction of the
 * dynamic table through a reduced maximum size.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_decode_response(void);

/**
 * Tests that a malformed block is refused, covering the invalid
 * index, the truncated representation and the header list that
 * goes beyond the accepted size.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_decode_errors(void);

/**
 * Tests the limits that guard the decoding, both the expansion of
 * a small block into a very large header list and the name that
 * does not fit in the buffer that receives it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_decode_limits(void);

/**
 * Tests the encoding of a header field, verifying that a field
 * already present in one of the tables is carried by a single
 * indexed representation and that a round trip is stable.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_encode(void);

/**
 * Tests the Huffman code against the values of the appendix B
 * of the specification, both the sizing and the round trip.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_huffman(void);

/**
 * Tests that an invalid coded buffer is refused, covering the
 * padding that is not a prefix of the end of string symbol, the
 * padding longer than a byte and the symbol itself.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hpack_huffman_errors(void);
