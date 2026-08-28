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

/**
 * Tests the formatting of the message of a failed
 * assertion, including the reuse of the buffer.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_format_message(void);

/**
 * Tests the assertions that report the values that
 * took part in the comparison that failed.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_assert_values(void);

/**
 * Tests the matching of the name of a test against
 * the pattern of a filter, wildcards included.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_match_name(void);

/**
 * Tests the matching of the tags of a test against
 * the tags that have been selected for a run.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_match_tags(void);

/**
 * Tests the selection of a complete entry, both the
 * name and the tags have to match for it to be run.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_match_entry(void);

/**
 * Tests the default values of the options that control
 * the selection and the reporting of a run.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_options(void);

/**
 * Tests the running of a suite, covering the fixtures,
 * the parametrized cases and every one of the statuses
 * an entry may end its execution under.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_run_suite(void);

/**
 * Tests the kinds of entry that the synthetic suite does
 * not carry, the untagged, the empty and the speed ones.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_run_kinds(void);

/**
 * Tests the listing of a suite, no function of any of
 * the entries may be called while listing.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_list_suite(void);

/**
 * Tests the labels under which the various statuses
 * of a result are reported.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_status_label(void);

/**
 * Tests the escaping of the entities of xml, including
 * the truncation of a value that does not fit.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_escape_xml(void);

/**
 * Tests the writing of a report in the plain text format.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_write_text(void);

/**
 * Tests the writing of a report following the test
 * anything protocol.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_write_tap(void);

/**
 * Tests the writing of a report as the junit xml that
 * the continuous integration tooling consumes.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_write_junit(void);

/**
 * Tests the writing of a report as a markdown table.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_write_markdown(void);

/**
 * Tests the dispatching of the writing of a report to
 * the writer of the requested format.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_runner_write_report(void);
