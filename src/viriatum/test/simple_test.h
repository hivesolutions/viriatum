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
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_thread_pool(void);

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
 * Tests the parsing of a template file by the engine, together
 * with the file that is not there and the callback that fails.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_engine(void);

/**
 * Tests the parsing of a buffer by the engine, the events that
 * every kind of tag reports and the character that closes the
 * very end of it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_engine_buffer(void);

/**
 * Tests the creation of the cache of templates, every one of
 * its entries starting out empty.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache(void);

/**
 * Tests the acquiring of a template, the first time parsing it
 * and every time after that handing back the tree already parsed.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_acquire(void);

/**
 * Tests that a template that is not there is reported as an
 * error rather than as an entry that describes nothing.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_missing(void);

/**
 * Tests that a template written over in place is rendered as
 * it now stands and never as it used to be.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_changed(void);

/**
 * Tests two templates that fall on the very same entry of
 * the cache, each of them taking the entry over from the other.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_collision(void);

/**
 * Tests the clearing of the cache, which closes every file and
 * releases every tree and leaves the cache usable afterwards.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_clear(void);

/**
 * Tests that a path too long for an entry to carry is refused
 * rather than copied past the end of the entry.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_long(void);

/**
 * Tests an entry past the time it is trusted for, which is looked
 * at again through its path and renewed when nothing has changed.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_expired(void);

/**
 * Tests that a template replaced by another of the very same
 * length is rendered as the one now under the path once the
 * entry is looked at again.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_replaced(void);

/**
 * Tests that an entry left holding a descriptor which no
 * longer reaches anything answers with an error.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_cache_stale(void);

/**
 * Tests the template handler, the page that every one of the
 * tags builds, the template that is not there and the tags that
 * carry none of the parameters they need.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_handler(void);

/**
 * Tests the rendering of a page out of the cache of templates,
 * the tree being parsed once and rendered as many times as asked.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_handler_cache(void);

/**
 * Tests the pages held rendered by the cache of templates, one
 * under each key, handed over as they stand and gone with the
 * tree they were rendered out of.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_template_handler_page(void);

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
/**
 * Tests the counting of the size of a file, together
 * with the files that it is unable to count at all.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_count_file(void);

/**
 * Tests the telling of a directory from a file, together with
 * the path that is not there at all.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_is_directory_file(void);

/**
 * Tests the describing of the set of entries of a directory as
 * a number, one that moves with the set and not with the entries.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_fingerprint_directory_file(void);

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
 * Compares an element with another, useful
 * for the sorting tests.
 *
 * @param first The first value to be compared.
 * @param second The second value to be compared.
 * @return The result of the comparison.
 */
int _compare(void *first, void *second);

/**
 * Populates the provided suite with the table of entries
 * that describe the set of simple tests.
 *
 * @param suite The suite to be populated with the entries
 * of the simple tests and with their name.
 */
void create_simple_suite(struct test_suite_t *suite);

/**
 * Runs the set of simple tests in the current
 * test case. This is the main entry point for the
 * simple test case.
 *
 * @param options The options that control the selection of
 * the tests to be run and the reporting of the results, a
 * null value runs every one of them reporting to the
 * standard output alone.
 */
ERROR_CODE run_simple_tests(struct test_options_t *options);
