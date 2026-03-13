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

#ifndef VIRIATUM_NO_THREADS

/**
 * Test thread function to be used in the thread
 * pool test.
 *
 * @param arguments The pointer to the arguments to
 * the thread.
 * @return The result of the thread execution.
 */
int thread_pool_start_function_test(void *arguments);

/**
 * Tests the thread pool implementation.
 */
void test_thread_pool(void);

#endif

/**
 * Tests the linked list structure.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_linked_list(void);

/**
 * Runs a series of stress tests in the linked
 * list structured (used mainly for performance).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_linked_list_stress(void);

/**
 * Runs a series of big tests in the linked
 * list structured (used mainly for performance).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_linked_list_big(void);

/**
 * Tests the array list structure.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *est_array_list(void);

/**
 * Tests the hash map structure.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_hash_map(void);

/**
 * Tests the sort map structure.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_sort_map(void);

/**
 * Tests the priority queue structure.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_priority_queue(void);

/**
 * Tests the string buffer, used
 * to buffer string in memory for
 * fast writing.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_string_buffer(void);

/**
 * Tests the linked buffer, used
 * to buffer a series of buffers
 * in memory for fast writing.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_linked_buffer(void);

/**
 * Tests the base 64 encoding.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_base64(void);

/**
 * Tests the huffman encoding.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_huffman(void);

/**
 * Tests the bit stream.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_bit_stream(void);

/**
 * Tests the file stream.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_stream(void);

/**
 * Tests the (in-)memory stream.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_memory_stream(void);

/**
 * Tests the template handler.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_handler(void);

/**
 * Tests the quicksort (algorithm).
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_quicksort(void);

/**
 * Tests the quicksort (algorithm),
 * for the linked list implementation.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_quicksort_linked_list(void);

/**
 * Tests the md5 hash implementation
 * calculation.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_md5(void);

/**
 * Tests the sha1 hash implementation
 * calculation.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_sha1(void);

/**
 * Tests the crc32 hash implementation
 * calculation.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_crc_32(void);

/**
 * Tests the is_path_safe utility function
 * for path traversal detection.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_is_path_safe(void);

/**
 * Tests the normalize_path utility function
 * for platform specific path normalization.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_normalize_path(void);

/**
 * Tests the join_path_file utility function
 * for platform specific path joining.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_join_path_file(void);

/**
 * Tests the absolute_path_file utility function
 * for resolving relative paths into absolute paths.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_absolute_path_file(void);

/**
 * Compares an element with another, usefull
 * for the sorting tests.
 *
 * @param first The first value to be compared.
 * @param second The second value to be compared.
 * @return The result of the comparison.
 */
int _compare(void *first, void *second);

/**
 * Executes the set of simple tests in the current
 * test case.
 *
 * @param test_case The test case context for which
 * the simple tests will be executed, should be able
 * to store some context information about the execution.
 */
void exec_simple_tests(struct test_case_t *test_case);

/**
 * Runs the set of simple tests in the current
 * test case. This is the main entry point for the
 * simple test case.
 */
ERROR_CODE run_simple_tests(void);
