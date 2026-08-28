/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "unit.h"

/**
 * The size of the buffer that holds an escaped value, the
 * longest of the entities takes six bytes for every byte of
 * the value that originated it.
 */
#define TEST_ESCAPE_SIZE (TEST_MESSAGE_SIZE * 6)

/**
 * Returns the label under which the provided status is reported,
 * the labels are the same ones echoed during the run.
 *
 * @param status The status for which the label is retrieved.
 * @return The label of the provided status.
 */
VIRIATUM_EXPORT_PREFIX const char *status_label_report(enum test_status_e status);

/**
 * Escapes the five entities of xml from the provided value into
 * the provided buffer, the value is truncated in case the escaped
 * version of it does not fit the buffer.
 *
 * @param value The value to be escaped into the buffer.
 * @param buffer The buffer that receives the escaped value.
 * @param size The complete size of the target buffer.
 */
VIRIATUM_EXPORT_PREFIX void escape_xml_report(const char *value, char *buffer, size_t size);

/**
 * Writes the provided report to the provided file as plain text,
 * one line per test followed by the totals of the run.
 *
 * @param report The report to be written to the file.
 * @param file The file to which the report is written.
 * @return The error code resulting from the write operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE write_text_report(struct test_report_t *report, FILE *file);

/**
 * Writes the provided report to the provided file following the
 * test anything protocol, the skipped and the expected failures
 * are reported through the skip and todo directives.
 *
 * @param report The report to be written to the file.
 * @param file The file to which the report is written.
 * @return The error code resulting from the write operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE write_tap_report(struct test_report_t *report, FILE *file);

/**
 * Writes the provided report to the provided file as the junit
 * xml that the continuous integration tooling consumes, a suite
 * carrying one case per executed test.
 *
 * @param report The report to be written to the file.
 * @param file The file to which the report is written.
 * @return The error code resulting from the write operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE write_junit_report(struct test_report_t *report, FILE *file);

/**
 * Writes the provided report to the provided file as a markdown
 * table, in the same shape of the one built for the coverage so
 * that both may be appended to the same step summary.
 *
 * @param report The report to be written to the file.
 * @param file The file to which the report is written.
 * @return The error code resulting from the write operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE write_markdown_report(struct test_report_t *report, FILE *file);

/**
 * Writes the provided report under the format described by the
 * provided options, to the path that they carry or to the standard
 * output in case no path has been set.
 *
 * No report is written in case no format has been requested, the
 * progress of the run has already been echoed in such a situation.
 *
 * @param report The report to be written.
 * @param options The options that describe both the format and
 * the destination of the report.
 * @return The error code resulting from the write operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE write_test_report(struct test_report_t *report, struct test_options_t *options);
