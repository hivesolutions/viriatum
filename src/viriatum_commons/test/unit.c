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

#include "unit.h"
#include "report.h"

/* the buffer that holds the message of the assertion that has
failed most recently, the tests are run in sequence so a single
buffer is enough to carry the message until it is reported */
static char _test_message[TEST_MESSAGE_SIZE];

/**
 * Copies the provided value into the provided buffer truncating
 * it in case it does not fit, a null value produces an empty
 * string so that the buffer is always properly terminated.
 *
 * @param buffer The buffer into which the value is copied.
 * @param size The complete size of the target buffer.
 * @param value The value to be copied into the buffer.
 */
static void _copy_test(char *buffer, size_t size, const char *value) {
    size_t length;

    if(value == NULL) {
        buffer[0] = '\0';
        return;
    }

    length = strlen(value);
    if(length > size - 1) { length = size - 1; }
    memcpy(buffer, value, length);
    buffer[length] = '\0';
}

/**
 * Matches the provided name against a pattern that makes use of
 * the asterisk wildcard, the matching is iterative and performs
 * the backtracking by hand so that no recursion is involved.
 *
 * @param name The name to be matched against the pattern.
 * @param pattern The pattern containing the wildcards.
 * @return If the complete name is matched by the pattern.
 */
static int _match_test(const char *name, const char *pattern) {
    const char *name_mark = NULL;
    const char *pattern_mark = NULL;

    while(*name != '\0') {
        if(*pattern == '*') {
            pattern++;
            if(*pattern == '\0') { return TRUE; }
            pattern_mark = pattern;
            name_mark = name;
        } else if(*pattern == *name) {
            pattern++;
            name++;
        } else if(pattern_mark != NULL) {
            pattern = pattern_mark;
            name_mark++;
            name = name_mark;
        } else {
            return FALSE;
        }
    }

    /* consumes the trailing wildcards of the pattern, they are
    allowed to match the empty remainder of the name */
    while(*pattern == '*') { pattern++; }

    return *pattern == '\0' ? TRUE : FALSE;
}

/**
 * Verifies if the provided comma separated sequence of tokens
 * carries the provided token, the comparison is bounded by the
 * provided size so that no copy of the token is required.
 *
 * @param tokens The comma separated sequence to be searched.
 * @param token The token to be searched for in the sequence.
 * @param size The size in bytes of the token to be searched.
 * @return If the sequence carries the provided token.
 */
static int _token_test(const char *tokens, const char *token, size_t size) {
    const char *start;
    size_t length;

    while(*tokens != '\0') {
        while(*tokens == ' ' || *tokens == ',') { tokens++; }
        start = tokens;
        while(*tokens != '\0' && *tokens != ',') { tokens++; }
        length = (size_t) (tokens - start);
        while(length > 0 && start[length - 1] == ' ') { length--; }
        if(length == size && memcmp(start, token, size) == 0) { return TRUE; }
    }

    return FALSE;
}

/**
 * Runs a single execution of the provided entry populating the
 * provided result with the outcome, the parameters are only used
 * by the entries that have been declared as parametrized ones.
 *
 * @param entry The entry that is going to be executed.
 * @param params The parameter case to be handed to the test.
 * @param result The result to be populated with the outcome.
 * @param options The options that control the echoing of the
 * progress of the execution.
 */
static void _run_test(
    struct test_entry_t *entry,
    const void *params,
    struct test_result_t *result,
    struct test_options_t *options
) {
    long start_time;
    long end_time;
    size_t allocated;
    void *context = NULL;
    const char *message = NULL;

    /* a speed entry prints a couple of lines of its own so the
    name is not echoed for it, otherwise the output of the run
    would be interleaved with the one of the measurement */
    if(options->echo == TRUE && entry->iterations == 0) {
        V_PRINT_F("%s ... ", result->name);
        PRINT_FLUSH();
    }

    result->elapsed = 0.0f;
    result->outstanding = 0;
    result->message[0] = '\0';

    /* a skipped entry is reported without any of its functions
    being called, not even the setup of its fixture */
    if(entry->flags & TEST_FLAG_SKIP) {
        result->status = TEST_STATUS_SKIP;
        if(options->echo == TRUE) { V_PRINT_C(V_COLOR_WARNING, "skip\n"); }
        return;
    }

    start_time = clock();
    allocated = ALLOCATIONS;

    if(entry->setup != NULL) { context = entry->setup(); }

    if(entry->iterations > 0) {
        run_speed_test((char *) entry->name, entry->function, entry->iterations);
    } else if(entry->function_p != NULL) {
        message = entry->function_p(params);
    } else if(entry->function_c != NULL) {
        message = entry->function_c(context);
    } else if(entry->function != NULL) {
        message = entry->function();
    } else {
        message = "no function has been defined for the entry";
    }

    if(entry->teardown != NULL) { entry->teardown(context); }

    end_time = clock();
    result->elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC;
    result->outstanding = (long) ALLOCATIONS - (long) allocated;

    /* an entry marked as an expected failure inverts the meaning
    of its outcome, a failure becomes the expected result and a
    success becomes a failure as the marker is no longer accurate */
    if(entry->flags & TEST_FLAG_XFAIL) {
        result->status = message == NULL ? TEST_STATUS_XPASS : TEST_STATUS_XFAIL;
    } else {
        result->status = message == NULL ? TEST_STATUS_OK : TEST_STATUS_FAILURE;
    }

    _copy_test(result->message, TEST_MESSAGE_SIZE, message);

    if(options->echo == FALSE || entry->iterations > 0) { return; }

    switch(result->status) {
        case TEST_STATUS_OK:
            V_PRINT_C(V_COLOR_INFO, "ok\n");
            break;

        case TEST_STATUS_XFAIL:
            V_PRINT_C(V_COLOR_WARNING, "xfail\n");
            break;

        case TEST_STATUS_XPASS:
            V_PRINT_C(V_COLOR_ERROR, "xpass\n");
            V_PRINT("  the test was expected to fail but it passed\n");
            break;

        default:
            V_PRINT_C(V_COLOR_ERROR, "not ok\n");
            V_PRINT_F("  %s\n", result->message);
            break;
    }
}

/**
 * Prints the tests that took the longest to run, the listing is
 * built by selecting the largest value that is still smaller than
 * the one selected before it, avoiding a sort of the results.
 *
 * @param report The report whose results are going to be listed.
 */
static void _print_slowest_test(struct test_report_t *report) {
    size_t index;
    size_t count;
    size_t position;
    size_t selected;
    float limit = -1.0f;

    count = report->count < TEST_SLOWEST_COUNT ? report->count : TEST_SLOWEST_COUNT;
    if(count == 0) { return; }

    V_PRINT("Slowest tests\n");

    for(position = 0; position < count; position++) {
        selected = report->count;
        for(index = 0; index < report->count; index++) {
            if(limit >= 0.0f && report->results[index].elapsed >= limit) { continue; }
            if(selected != report->count &&
               report->results[index].elapsed <= report->results[selected].elapsed) {
                continue;
            }
            selected = index;
        }
        if(selected == report->count) { break; }
        limit = report->results[selected].elapsed;
        V_PRINT_F("  %s (%.2f seconds)\n", report->results[selected].name, limit);
    }
}

/**
 * Prints the tests that left the most allocations outstanding,
 * the listing is built the same way the one of the slowest is
 * and a run that left none prints nothing at all. An allocation
 * that is still outstanding is not necessarily a leak, a value
 * built by one test may well be released by a later one.
 *
 * @param report The report whose results are going to be listed.
 */
static void _print_outstanding_test(struct test_report_t *report) {
    size_t index;
    size_t count;
    size_t position;
    size_t selected;
    size_t previous = report->count;
    size_t listed = 0;
    long limit = -1;

    count = report->count < TEST_OUTSTANDING_COUNT ? report->count : TEST_OUTSTANDING_COUNT;
    if(count == 0) { return; }

    for(position = 0; position < count; position++) {
        selected = report->count;
        for(index = 0; index < report->count; index++) {
            if(report->results[index].outstanding <= 0) { continue; }

            /* a count above the one already listed has been listed
            with it, and one equal to it only when it came before,
            which is what keeps the tests that left the very same
            count from being dropped after the first of them */
            if(limit >= 0) {
                if(report->results[index].outstanding > limit) { continue; }
                if(report->results[index].outstanding == limit && index <= previous) {
                    continue;
                }
            }

            if(selected != report->count &&
               report->results[index].outstanding <= report->results[selected].outstanding) {
                continue;
            }
            selected = index;
        }
        if(selected == report->count) { break; }
        limit = report->results[selected].outstanding;
        previous = selected;
        if(listed == 0) { V_PRINT("Outstanding allocations\n"); }
        listed++;
        V_PRINT_F("  %s (%ld allocations)\n", report->results[selected].name, limit);
    }
}

const char *format_test_message(const char *format, ...) {
    va_list args;
    va_start(args, format);
    VSPRINTF(_test_message, TEST_MESSAGE_SIZE, format, args);
    va_end(args);
    return _test_message;
}

int match_name_test(const char *name, const char *pattern) {
    /* an unset or empty pattern selects every test, no filtering
    is meant to be performed in such a situation */
    if(pattern == NULL || *pattern == '\0') { return TRUE; }

    /* a pattern without any wildcard is matched as a sub sequence
    of the name, the behaviour expected from a name filter */
    if(strchr(pattern, '*') == NULL) {
        return strstr(name, pattern) == NULL ? FALSE : TRUE;
    }

    return _match_test(name, pattern);
}

int match_tags_test(const char *tags, const char *selection) {
    const char *start;
    size_t length;

    /* an unset or empty selection selects every test, including
    the ones that do not carry any tag at all */
    if(selection == NULL || *selection == '\0') { return TRUE; }

    /* a test without tags may never be selected by a selection
    that does carry at least one tag */
    if(tags == NULL) { return FALSE; }

    while(*selection != '\0') {
        while(*selection == ' ' || *selection == ',') { selection++; }
        start = selection;
        while(*selection != '\0' && *selection != ',') { selection++; }
        length = (size_t) (selection - start);
        while(length > 0 && start[length - 1] == ' ') { length--; }
        if(length > 0 && _token_test(tags, start, length) == TRUE) { return TRUE; }
    }

    return FALSE;
}

int match_entry_test(struct test_entry_t *entry, struct test_options_t *options) {
    if(options == NULL) { return TRUE; }
    if(match_name_test(entry->name, options->filter) == FALSE) { return FALSE; }
    if(match_tags_test(entry->tags, options->tags) == FALSE) { return FALSE; }
    return TRUE;
}

void create_test_options(struct test_options_t *options) {
    options->filter = NULL;
    options->tags = NULL;
    options->format = NULL;
    options->path = NULL;
    options->list = FALSE;
    options->echo = TRUE;
}

ERROR_CODE run_speed_test(char *name, test_function function, size_t iterations) {
    /* allocates the variables (long variables) that are going to be
    used to stores the values for the speed measuring and the float
    value to be used at the final type conversion */
    size_t index;
    long start_time;
    long end_time;
    float elapsed;
    float elapsed_f;

    /* defaults the iterations count value to the one value
    in case no valid has been provided */
    iterations = iterations == 0 ? 1 : iterations;

    /* prints a debug message at the beginning of the speed function
    execution process indicating the name of the function */
    PRINTF_F("Running '%s' for %ld times ...\n", name, (long int) iterations);

    /* retrieves the initial clock value for the execution of the
    function, it's going to be used to measure time at the end */
    start_time = clock();

    /* executes the function, the control flow may only be returned
    after a couple of seconds or more (assume worst case) */
    for(index = 0; index < iterations; index++) { function(); }

    /* retrieves the final clock for the program execution and ten
    calculates the elapsed time with the difference between the start
    time and the current time */
    end_time = clock();
    elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC * 1000.0f;

    /* calculates the elapsed time as a float value and then prints
    a message regarding the amount of time it took to execute */
    elapsed_f = (float) elapsed / 1000.0f;
    PRINTF_F("Executed '%s' in %.2f seconds\n", name, elapsed_f);

    /* returns a no error value indicating that nothing outside
    the normal values has occurred */
    RAISE_NO_ERROR;
}

ERROR_CODE run_test_case(test_case_function function, const char *name) {
    long start_time;
    long end_time;
    float elapsed;
    float elapsed_f;
    struct test_case_t test_case;
    test_case.total = 0;
    test_case.success = 0;
    test_case.failure = 0;
    test_case.skipped = 0;
    test_case.echo = TRUE;
    V_PRINT_F("Running %s test case ...\n", name);
    start_time = clock();
    function(&test_case);
    end_time = clock();
    elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC * 1000.0f;
    elapsed_f = (float) elapsed / 1000.0f;
    V_PRINT_F("Ran %d tests in %.2f seconds (", test_case.total, elapsed_f);
    V_PRINT_CF(V_COLOR_INFO, "%d ok", test_case.success);
    V_PRINT(", ");
    V_PRINT_CF(test_case.failure > 0 ? V_COLOR_ERROR : V_COLOR_INFO, "%d not ok", test_case.failure);
    V_PRINT(")\n");
    RAISE_ERROR_S(test_case.failure > 0 ? 1 : 0);
}

ERROR_CODE run_test_suite(struct test_suite_t *suite, struct test_options_t *options) {
    size_t index;
    size_t case_index;
    size_t count = 0;
    size_t position = 0;
    long start_time;
    long end_time;
    void *context = NULL;
    const void *params;
    ERROR_CODE return_value;
    struct test_entry_t *entry;
    struct test_result_t *result;
    struct test_report_t report;
    struct test_case_t test_case;
    struct test_options_t _options;

    /* defaults the options so that a suite may be run without
    any kind of extra preparation from the caller */
    if(options == NULL) {
        create_test_options(&_options);
        options = &_options;
    }

    /* the listing of the tests replaces the run completely, none
    of the functions of the suite is going to be called */
    if(options->list == TRUE) { return list_test_suite(suite, options); }

    /* counts the executions that are going to be performed, a
    parametrized entry accounts for one execution per case */
    for(index = 0; index < suite->count; index++) {
        entry = &suite->entries[index];
        if(match_entry_test(entry, options) == FALSE) { continue; }
        count += entry->params_count > 0 ? entry->params_count : 1;
    }

    report.name = suite->name;
    report.count = count;
    report.elapsed = 0.0f;
    report.results = count == 0
                         ? NULL
                         : (struct test_result_t *) MALLOC(sizeof(struct test_result_t) * count);

    test_case.total = 0;
    test_case.success = 0;
    test_case.failure = 0;
    test_case.skipped = 0;
    test_case.echo = options->echo;

    if(options->echo == TRUE) { V_PRINT_F("Running %s test case ...\n", suite->name); }
    start_time = clock();

    if(suite->setup != NULL) { context = suite->setup(); }

    for(index = 0; index < suite->count; index++) {
        entry = &suite->entries[index];
        if(match_entry_test(entry, options) == FALSE) { continue; }

        for(case_index = 0;
            case_index < (entry->params_count > 0 ? entry->params_count : 1);
            case_index++) {
            result = &report.results[position];
            position++;

            /* builds the name under which the execution is reported,
            a parametrized entry carries the index of its case so that
            each of the executions may be told apart */
            if(entry->params_count > 0) {
                SPRINTF(
                    result->name,
                    TEST_NAME_SIZE,
                    "%s[%ld]",
                    entry->name,
                    (long) case_index
                );
                params = (const void *) ((const char *) entry->params +
                                         case_index * entry->params_size);
            } else {
                _copy_test(result->name, TEST_NAME_SIZE, entry->name);
                params = NULL;
            }

            _run_test(entry, params, result, options);

            test_case.total++;
            switch(result->status) {
                case TEST_STATUS_OK:
                case TEST_STATUS_XFAIL:
                    test_case.success++;
                    break;

                case TEST_STATUS_SKIP:
                    test_case.skipped++;
                    break;

                default:
                    test_case.failure++;
                    break;
            }
        }
    }

    if(suite->teardown != NULL) { suite->teardown(context); }

    end_time = clock();
    report.elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC;

    /* the summary and the listing of the slowest tests are only
    echoed for a run that is already echoing its progress, a report
    written to the standard output would otherwise be polluted */
    if(options->echo == TRUE) {
        V_PRINT_F("Ran %d tests in %.2f seconds (", test_case.total, report.elapsed);
        V_PRINT_CF(V_COLOR_INFO, "%d ok", test_case.success);
        V_PRINT(", ");
        V_PRINT_CF(test_case.failure > 0 ? V_COLOR_ERROR : V_COLOR_INFO, "%d not ok", test_case.failure);
        if(test_case.skipped > 0) { V_PRINT_F(", %d skipped", test_case.skipped); }
        V_PRINT(")\n");
        _print_slowest_test(&report);
        _print_outstanding_test(&report);
    }

    return_value = write_test_report(&report, options);

    if(report.results != NULL) { FREE(report.results); }

    /* the failure of a test takes precedence over the one of the
    report, the error of the writing is only raised afterwards so
    that a run whose report was never produced is not reported as
    a successful one to whoever asked for it */
    if(test_case.failure > 0) { RAISE_ERROR_S(1); }

    RAISE_AGAIN(return_value);
}

ERROR_CODE list_test_suite(struct test_suite_t *suite, struct test_options_t *options) {
    size_t index;
    size_t count = 0;
    struct test_entry_t *entry;

    for(index = 0; index < suite->count; index++) {
        entry = &suite->entries[index];
        if(match_entry_test(entry, options) == FALSE) { continue; }
        count++;
        if(entry->tags == NULL) {
            V_PRINT_F("%s\n", entry->name);
        } else {
            V_PRINT_F("%s [%s]\n", entry->name, entry->tags);
        }
    }

    V_PRINT_F("%ld tests listed\n", (long) count);

    RAISE_NO_ERROR;
}
