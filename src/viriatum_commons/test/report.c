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

#include "stdafx.h"

#include "report.h"

/**
 * Counts the results of the provided report that ended under a
 * status that is considered a failure, both the plain failures
 * and the expected failures that did not happen.
 *
 * @param report The report whose results are going to be counted.
 * @return The amount of results considered a failure.
 */
static size_t _failures_report(struct test_report_t *report) {
    size_t index;
    size_t count = 0;

    for(index = 0; index < report->count; index++) {
        if(report->results[index].status == TEST_STATUS_FAILURE ||
           report->results[index].status == TEST_STATUS_XPASS) {
            count++;
        }
    }

    return count;
}

/**
 * Counts the results of the provided report that were not run at
 * all, the skipped ones and the expected failures which have no
 * representation of their own in the junit format.
 *
 * @param report The report whose results are going to be counted.
 * @return The amount of results that were not run as normal ones.
 */
static size_t _skipped_report(struct test_report_t *report) {
    size_t index;
    size_t count = 0;

    for(index = 0; index < report->count; index++) {
        if(report->results[index].status == TEST_STATUS_SKIP ||
           report->results[index].status == TEST_STATUS_XFAIL) {
            count++;
        }
    }

    return count;
}

const char *status_label_report(enum test_status_e status) {
    switch(status) {
        case TEST_STATUS_OK:
            return "ok";

        case TEST_STATUS_FAILURE:
            return "not ok";

        case TEST_STATUS_SKIP:
            return "skip";

        case TEST_STATUS_XFAIL:
            return "xfail";

        case TEST_STATUS_XPASS:
            return "xpass";

        default:
            return "unknown";
    }
}

void escape_xml_report(const char *value, char *buffer, size_t size) {
    size_t position = 0;
    const char *entity;
    size_t length;

    if(value == NULL) {
        buffer[0] = '\0';
        return;
    }

    while(*value != '\0') {
        switch(*value) {
            case '&':
                entity = "&amp;";
                break;

            case '<':
                entity = "&lt;";
                break;

            case '>':
                entity = "&gt;";
                break;

            case '"':
                entity = "&quot;";
                break;

            case '\'':
                entity = "&apos;";
                break;

            default:
                entity = NULL;
                break;
        }

        /* verifies that the replacement still fits the buffer,
        the value is truncated instead of overflowing it */
        length = entity == NULL ? 1 : strlen(entity);
        if(position + length > size - 1) { break; }

        if(entity == NULL) {
            buffer[position] = *value;
        } else {
            memcpy(&buffer[position], entity, length);
        }

        position += length;
        value++;
    }

    buffer[position] = '\0';
}

ERROR_CODE write_text_report(struct test_report_t *report, FILE *file) {
    size_t index;
    struct test_result_t *result;

    fprintf(file, "%s\n", report->name);

    for(index = 0; index < report->count; index++) {
        result = &report->results[index];
        fprintf(
            file,
            "  %s ... %s (%.2f seconds)\n",
            result->name,
            status_label_report(result->status),
            result->elapsed
        );
        if(result->message[0] != '\0') { fprintf(file, "    %s\n", result->message); }
    }

    fprintf(
        file,
        "Ran %ld tests in %.2f seconds (%ld not ok, %ld skipped)\n",
        (long) report->count,
        report->elapsed,
        (long) _failures_report(report),
        (long) _skipped_report(report)
    );

    RAISE_NO_ERROR;
}

ERROR_CODE write_tap_report(struct test_report_t *report, FILE *file) {
    size_t index;
    struct test_result_t *result;

    fprintf(file, "TAP version 13\n");
    fprintf(file, "1..%ld\n", (long) report->count);

    for(index = 0; index < report->count; index++) {
        result = &report->results[index];
        switch(result->status) {
            case TEST_STATUS_OK:
                fprintf(file, "ok %ld - %s\n", (long) (index + 1), result->name);
                break;

            case TEST_STATUS_SKIP:
                fprintf(file, "ok %ld - %s # SKIP\n", (long) (index + 1), result->name);
                break;

            case TEST_STATUS_XFAIL:
                fprintf(file, "not ok %ld - %s # TODO\n", (long) (index + 1), result->name);
                break;

            case TEST_STATUS_XPASS:
                fprintf(file, "ok %ld - %s # TODO\n", (long) (index + 1), result->name);
                break;

            default:
                fprintf(file, "not ok %ld - %s\n", (long) (index + 1), result->name);
                fprintf(file, "  ---\n");
                fprintf(file, "  message: '%s'\n", result->message);
                fprintf(file, "  ...\n");
                break;
        }
    }

    RAISE_NO_ERROR;
}

ERROR_CODE write_junit_report(struct test_report_t *report, FILE *file) {
    size_t index;
    struct test_result_t *result;
    char escaped[TEST_ESCAPE_SIZE];

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<testsuites>\n");

    escape_xml_report(report->name, escaped, TEST_ESCAPE_SIZE);
    fprintf(
        file,
        "  <testsuite name=\"%s\" tests=\"%ld\" failures=\"%ld\" skipped=\"%ld\" time=\"%.3f\">\n",
        escaped,
        (long) report->count,
        (long) _failures_report(report),
        (long) _skipped_report(report),
        report->elapsed
    );

    for(index = 0; index < report->count; index++) {
        result = &report->results[index];

        escape_xml_report(result->name, escaped, TEST_ESCAPE_SIZE);
        fprintf(
            file,
            "    <testcase name=\"%s\" classname=\"%s\" time=\"%.3f\"",
            escaped,
            report->name,
            result->elapsed
        );

        if(result->status == TEST_STATUS_OK) {
            fprintf(file, "/>\n");
            continue;
        }

        fprintf(file, ">\n");

        switch(result->status) {
            case TEST_STATUS_SKIP:
                fprintf(file, "      <skipped/>\n");
                break;

            case TEST_STATUS_XFAIL:
                fprintf(file, "      <skipped message=\"expected failure\"/>\n");
                break;

            case TEST_STATUS_XPASS:
                fprintf(
                    file,
                    "      <failure message=\"the test was expected to fail but it passed\"/>\n"
                );
                break;

            default:
                escape_xml_report(result->message, escaped, TEST_ESCAPE_SIZE);
                fprintf(file, "      <failure message=\"%s\"/>\n", escaped);
                break;
        }

        fprintf(file, "    </testcase>\n");
    }

    fprintf(file, "  </testsuite>\n");
    fprintf(file, "</testsuites>\n");

    RAISE_NO_ERROR;
}

ERROR_CODE write_markdown_report(struct test_report_t *report, FILE *file) {
    size_t index;
    size_t failures;
    struct test_result_t *result;

    failures = _failures_report(report);

    fprintf(file, "## Tests\n\n");
    fprintf(file, "| Test | Status | Duration | |\n");
    fprintf(file, "| --- | :-: | ---: | :-: |\n");

    for(index = 0; index < report->count; index++) {
        result = &report->results[index];
        fprintf(
            file,
            "| `%s` | %s | %.2fs | %s |\n",
            result->name,
            status_label_report(result->status),
            result->elapsed,
            result->status == TEST_STATUS_FAILURE || result->status == TEST_STATUS_XPASS
                ? "\xe2\x9a\xa0\xef\xb8\x8f"
                : "\xe2\x9c\x85"
        );
    }

    fprintf(
        file,
        "| **total** | **%ld not ok** | **%.2fs** | %s |\n",
        (long) failures,
        report->elapsed,
        failures > 0 ? "\xe2\x9a\xa0\xef\xb8\x8f" : "\xe2\x9c\x85"
    );

    RAISE_NO_ERROR;
}

ERROR_CODE write_test_report(struct test_report_t *report, struct test_options_t *options) {
    FILE *file;
    ERROR_CODE return_value;

    /* no report is meant to be written in case no format has been
    requested, the progress of the run has already been echoed */
    if(options == NULL || options->format == NULL) { RAISE_NO_ERROR; }

    /* opens the destination of the report, the standard output is
    used whenever no path has been provided */
    if(options->path == NULL) {
        file = stdout;
    } else {
        FOPEN(&file, options->path, "wb");
        if(file == NULL) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem opening the report file"
            );
        }
    }

    if(strcmp(options->format, "text") == 0) {
        return_value = write_text_report(report, file);
    } else if(strcmp(options->format, "tap") == 0) {
        return_value = write_tap_report(report, file);
    } else if(strcmp(options->format, "junit") == 0) {
        return_value = write_junit_report(report, file);
    } else if(strcmp(options->format, "markdown") == 0) {
        return_value = write_markdown_report(report, file);
    } else {
        return_value = RUNTIME_EXCEPTION_ERROR_CODE;
        V_ERROR_F("Unknown test report format '%s'\n", options->format);
    }

    if(options->path == NULL) {
        fflush(file);
    } else {
        fclose(file);
    }

    RAISE_AGAIN(return_value);
}
