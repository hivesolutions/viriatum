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

#include "../debug/debug.h"

/**
 * The size of the buffer that holds the message of a failed
 * assertion, it must be large enough for the expression that
 * failed plus the textual representation of both values.
 */
#define TEST_MESSAGE_SIZE 1024

/**
 * The size of the buffer that holds the name of a test as it
 * is reported, a parametrized test appends the index or the
 * label of the case to the name of the function.
 */
#define TEST_NAME_SIZE 256

/**
 * The amount of tests that are listed in the summary of the
 * slowest tests printed at the end of a run.
 */
#define TEST_SLOWEST_COUNT 5

/**
 * The amount of tests that are listed in the summary of the
 * ones that left the most allocations outstanding, printed at
 * the end of a run just like the slowest ones are.
 */
#define TEST_OUTSTANDING_COUNT 5

/* the flags that may be set on a test entry, they control
whether the test is run at all and the way the result of its
execution is to be interpreted by the runner */
#define TEST_FLAG_NONE 0x00
#define TEST_FLAG_SKIP 0x01
#define TEST_FLAG_XFAIL 0x02

typedef enum test_status_e {
    /**
     * The test ran and none of its assertions
     * failed, the expected outcome.
     */
    TEST_STATUS_OK = 1,

    /**
     * The test ran and one of its assertions
     * failed, the message of the assertion is
     * carried by the result.
     */
    TEST_STATUS_FAILURE,

    /**
     * The test was not run at all because it was
     * marked with the skip flag.
     */
    TEST_STATUS_SKIP,

    /**
     * The test was marked as an expected failure
     * and did fail, counted as a success.
     */
    TEST_STATUS_XFAIL,

    /**
     * The test was marked as an expected failure
     * and unexpectedly passed, counted as a failure
     * as the marker is no longer accurate.
     */
    TEST_STATUS_XPASS
} test_status;

#define V_ASSERT(test) V_ASSERT_M(test, #test)
#define V_ASSERT_M(test, message)       \
    do {                                \
        if(!(test)) { return message; } \
    } while(0)
#define V_ASSERT_HEX(actual, expected, size)                                                                                         \
    do {                                                                                                                             \
        if(memcmp(actual, expected, size) != 0) {                                                                                    \
            size_t _i;                                                                                                               \
            V_PRINT_CF(V_COLOR_ERROR, "  [%s:%d] hex comparison failed\n", base_string_value((unsigned char *) __FILE__), __LINE__); \
            V_PRINT("  expected: ");                                                                                                 \
            for(_i = 0; _i < (size_t) (size); _i++) { V_PRINT_F("%02x", ((unsigned char *) (expected))[_i]); }                       \
            V_PRINT("\n  actual:   ");                                                                                               \
            for(_i = 0; _i < (size_t) (size); _i++) { V_PRINT_F("%02x", ((unsigned char *) (actual))[_i]); }                         \
            V_PRINT("\n");                                                                                                           \
            return "hex comparison failed";                                                                                          \
        }                                                                                                                            \
    } while(0)

/* the assertions that report the values that took part in the
comparison, the message is formatted into the buffer of the runner
and returned as the description of the failure, note that both of
the sides are evaluated exactly once so that an expression with a
side effect is not run twice and that the reported strings are
bounded, they carry values that come from the run itself */
#define V_ASSERT_EQ_I(actual, expected)                                  \
    do {                                                                 \
        long _actual = (long) (actual);                                  \
        long _expected = (long) (expected);                              \
        if(_actual != _expected) {                                       \
            return format_test_message(                                  \
                "%s: expected %ld, got %ld", #actual, _expected, _actual \
            );                                                           \
        }                                                                \
    } while(0)
#define V_ASSERT_EQ_U(actual, expected)                                  \
    do {                                                                 \
        unsigned long _actual = (unsigned long) (actual);                \
        unsigned long _expected = (unsigned long) (expected);            \
        if(_actual != _expected) {                                       \
            return format_test_message(                                  \
                "%s: expected %lu, got %lu", #actual, _expected, _actual \
            );                                                           \
        }                                                                \
    } while(0)
#define V_ASSERT_EQ_S(actual, expected)                    \
    do {                                                   \
        const char *_actual = (const char *) (actual);     \
        const char *_expected = (const char *) (expected); \
        if(_actual != _expected &&                         \
           (_actual == NULL || _expected == NULL ||        \
            strcmp(_actual, _expected) != 0)) {            \
            return format_test_message(                    \
                "%s: expected '%.400s', got '%.400s'",     \
                #actual,                                   \
                _expected == NULL ? "(null)" : _expected,  \
                _actual == NULL ? "(null)" : _actual       \
            );                                             \
        }                                                  \
    } while(0)
#define V_ASSERT_EQ_P(actual, expected)                                \
    do {                                                               \
        const void *_actual = (const void *) (actual);                 \
        const void *_expected = (const void *) (expected);             \
        if(_actual != _expected) {                                     \
            return format_test_message(                                \
                "%s: expected %p, got %p", #actual, _expected, _actual \
            );                                                         \
        }                                                              \
    } while(0)
#define V_ASSERT_NULL(actual)                                                          \
    do {                                                                               \
        const void *_actual = (const void *) (actual);                                 \
        if(_actual != NULL) {                                                          \
            return format_test_message("%s: expected null, got %p", #actual, _actual); \
        }                                                                              \
    } while(0)
#define V_ASSERT_NOT_NULL(actual)                                                   \
    do {                                                                            \
        const void *_actual = (const void *) (actual);                              \
        if(_actual == NULL) {                                                       \
            return format_test_message("%s: expected not null, got null", #actual); \
        }                                                                           \
    } while(0)
#define V_ASSERT_MEM(actual, expected, size)                   \
    do {                                                       \
        size_t _size = (size_t) (size);                        \
        if(memcmp(actual, expected, _size) != 0) {             \
            return format_test_message(                        \
                "%s: %ld bytes differ from the expected ones", \
                #actual,                                       \
                (long) _size                                   \
            );                                                 \
        }                                                      \
    } while(0)

#define V_RUN_TEST(test, test_case)                                                                          \
    do {                                                                                                     \
        const char *message;                                                                                 \
        if(test_case->echo == TRUE) {                                                                        \
            V_PRINT_F("%s ... ", #test);                                                                     \
            PRINT_FLUSH();                                                                                   \
        }                                                                                                    \
        message = test();                                                                                    \
        test_case->total++;                                                                                  \
        if(message == NULL) {                                                                                \
            if(test_case->echo == TRUE) {                                                                    \
                V_PRINT_C(V_COLOR_INFO, "ok\n");                                                             \
            }                                                                                                \
            test_case->success++;                                                                            \
        } else {                                                                                             \
            if(test_case->echo == TRUE) {                                                                    \
                V_PRINT_C(V_COLOR_ERROR, "not ok\n");                                                        \
                V_PRINT_F("[%s:%d] %s\n", base_string_value((unsigned char *) __FILE__), __LINE__, message); \
            }                                                                                                \
            test_case->failure++;                                                                            \
        }                                                                                                    \
    } while(0)
#define V_RUN_SPEED(test, count, test_case) \
    do {                                    \
        run_speed_test(#test, test, count); \
        test_case->total++;                 \
        test_case->success++;               \
    } while(0)

/* the constructors of a test entry, they are written as complete
positional initializers so that no designated initializer is needed,
those are not available on every compiler the project builds with */
#define V_TEST(function) V_TEST_T(function, NULL)
#define V_TEST_T(function, tags) \
    {#function, function, NULL, NULL, tags, TEST_FLAG_NONE, NULL, 0, 0, 0, NULL, NULL}
#define V_TEST_M(function, tags, flags) \
    {#function, function, NULL, NULL, tags, flags, NULL, 0, 0, 0, NULL, NULL}
#define V_TEST_C(function, tags, setup, teardown) \
    {#function, NULL, function, NULL, tags, TEST_FLAG_NONE, NULL, 0, 0, 0, setup, teardown}
#define V_TEST_P(function, tags, params, count, size) \
    {#function, NULL, NULL, function, tags, TEST_FLAG_NONE, params, count, size, 0, NULL, NULL}
#define V_TEST_S(function, tags, iterations) \
    {#function, function, NULL, NULL, tags, TEST_FLAG_NONE, NULL, 0, 0, iterations, NULL, NULL}

/* the size of a test entry table, used by the suites to report
the amount of entries they carry without repeating the value */
#define V_TEST_COUNT(entries) (sizeof(entries) / sizeof((entries)[0]))

typedef struct test_case_t {
    unsigned int total;
    unsigned int success;
    unsigned int failure;
    unsigned int skipped;
    unsigned char echo;
} test_case;

/**
 * General function used for testing purposes, this is the
 * simplest definition for a test function. This kind of
 * functions should return a description of the result.
 */
typedef const char *(*test_function)(void);

/**
 * Test function that receives the context created by the
 * setup of its fixture, the context is destroyed by the
 * teardown once the function returns.
 */
typedef const char *(*test_function_c)(void *context);

/**
 * Test function that receives one of the parameter cases
 * of its entry, the function is run once per case and each
 * of the executions is reported on its own.
 */
typedef const char *(*test_function_p)(const void *params);

/**
 * Function responsible for the creation of the context of
 * a fixture, the returned value is handed over to the test
 * function and then to the teardown.
 */
typedef void *(*test_fixture_function)(void);

/**
 * Function responsible for the destruction of the context
 * created by the setup of a fixture.
 */
typedef void (*test_cleanup_function)(void *context);

/**
 * Definition of the general entry point for a function that
 * is responsible for a test case execution.
 *
 * This function should run the various test associated with
 * the test case and then populate the test case structure.
 */
typedef void (*test_case_function)(struct test_case_t *test_case);

/**
 * Describes a single test of a suite as data, so that the
 * runner is able to list, filter and report the tests without
 * any of them having to be named in an execution sequence.
 */
typedef struct test_entry_t {

    /**
     * The name under which the test is listed, filtered
     * and reported, normally the name of the function.
     */
    const char *name;

    /**
     * The function to be run for a test that takes no
     * context and no parameters.
     */
    test_function function;

    /**
     * The function to be run for a test that consumes the
     * context created by the setup of its fixture.
     */
    test_function_c function_c;

    /**
     * The function to be run once per parameter case for
     * a parametrized test.
     */
    test_function_p function_p;

    /**
     * The comma separated tags of the test, used to select
     * a group of tests without naming each of them.
     */
    const char *tags;

    /**
     * The flags that control whether the test is run and
     * how its result is to be interpreted.
     */
    unsigned char flags;

    /**
     * The array of parameter cases of a parametrized test,
     * one execution is performed for each of them.
     */
    const void *params;

    /**
     * The amount of parameter cases carried by the array
     * of parameters.
     */
    size_t params_count;

    /**
     * The size in bytes of a single parameter case, used
     * to walk the array of parameters.
     */
    size_t params_size;

    /**
     * The amount of iterations to be performed for a speed
     * test, a value of zero marks a normal test.
     */
    size_t iterations;

    /**
     * The setup of the fixture of the test, run immediately
     * before the function of the test.
     */
    test_fixture_function setup;

    /**
     * The teardown of the fixture of the test, run once the
     * function of the test has returned.
     */
    test_cleanup_function teardown;

} test_entry;

/**
 * Groups the test entries that are run together and the
 * fixture that is shared by all of them.
 */
typedef struct test_suite_t {

    /**
     * The name of the suite, used both in the header of the
     * run and as the name of the suite in the reports.
     */
    const char *name;

    /**
     * The table of entries that compose the suite.
     */
    struct test_entry_t *entries;

    /**
     * The amount of entries carried by the table.
     */
    size_t count;

    /**
     * The setup run once before the first entry of the suite,
     * its context is not handed over to the tests.
     */
    test_fixture_function setup;

    /**
     * The teardown run once after the last entry of the suite.
     */
    test_cleanup_function teardown;

} test_suite;

/**
 * The options that control the selection of the tests to be
 * run and the format under which the results are reported.
 */
typedef struct test_options_t {

    /**
     * The pattern that the name of a test must match for it
     * to be run, a null value runs every test. The pattern
     * supports the asterisk as a wildcard.
     */
    const char *filter;

    /**
     * The comma separated tags that a test must carry at least
     * one of for it to be run, a null value runs every test.
     */
    const char *tags;

    /**
     * The format under which the results are to be reported,
     * one of text, tap, junit or markdown.
     */
    const char *format;

    /**
     * The path of the file to which the report is written, a
     * null value writes the report to the standard output.
     */
    const char *path;

    /**
     * If the tests should be listed instead of run, no test
     * function is called when this flag is set.
     */
    unsigned char list;

    /**
     * If the progress of the run should be echoed as each of
     * the tests is executed.
     */
    unsigned char echo;

} test_options;

/**
 * The outcome of the execution of a single test, retained so
 * that the report may be written once the run has finished.
 */
typedef struct test_result_t {

    /**
     * The name under which the test was run, for a parametrized
     * test it carries the case appended to the name.
     */
    char name[TEST_NAME_SIZE];

    /**
     * The status under which the execution of the test ended.
     */
    enum test_status_e status;

    /**
     * The amount of seconds the execution of the test took.
     */
    float elapsed;

    /**
     * The amount of allocations the execution of the test left
     * outstanding, which a later test may still release, a negative
     * value for one that released more than it took and zero for a
     * build that does not count them at all.
     */
    long outstanding;

    /**
     * The message of the assertion that failed, an empty string
     * for a test that did not fail.
     */
    char message[TEST_MESSAGE_SIZE];

} test_result;

/**
 * The complete set of results of the run of a suite, it is the
 * value consumed by the various writers of reports.
 */
typedef struct test_report_t {

    /**
     * The name of the suite that produced the results.
     */
    const char *name;

    /**
     * The array of results, one per executed test.
     */
    struct test_result_t *results;

    /**
     * The amount of results carried by the array.
     */
    size_t count;

    /**
     * The amount of seconds the complete run took.
     */
    float elapsed;

} test_report;

/**
 * Formats the message of a failed assertion into the buffer of
 * the runner, the returned value stays valid until the next call
 * of this function which is safe as the tests are run in sequence.
 *
 * @param format The format of the message to be built, following
 * the same conventions of the printf family of functions.
 * @return The formatted message, owned by the runner and reused
 * by the next assertion that fails.
 */
VIRIATUM_EXPORT_PREFIX const char *format_test_message(const char *format, ...);

/**
 * Verifies if the provided name matches the provided pattern, the
 * pattern is matched as a sub sequence of the name unless it uses
 * the asterisk wildcard in which case the parts around it are
 * matched in order.
 *
 * @param name The name of the test to be verified.
 * @param pattern The pattern against which the name is matched.
 * @return If the name matches the provided pattern.
 */
VIRIATUM_EXPORT_PREFIX int match_name_test(const char *name, const char *pattern);

/**
 * Verifies if the provided comma separated tags of a test contain
 * at least one of the provided comma separated tags of a selection.
 *
 * @param tags The tags carried by the test to be verified.
 * @param selection The tags that have been selected for the run.
 * @return If at least one of the tags of the test is selected.
 */
VIRIATUM_EXPORT_PREFIX int match_tags_test(const char *tags, const char *selection);

/**
 * Verifies if the provided entry is selected by the provided
 * options, both the name and the tags must match for the entry
 * to be considered selected.
 *
 * @param entry The entry to be verified against the options.
 * @param options The options that describe the selection.
 * @return If the entry is selected for execution.
 */
VIRIATUM_EXPORT_PREFIX int match_entry_test(struct test_entry_t *entry, struct test_options_t *options);

/**
 * Populates the provided options with the default values, running
 * every test of the suite and reporting in the textual format.
 *
 * @param options The options structure to be populated.
 */
VIRIATUM_EXPORT_PREFIX void create_test_options(struct test_options_t *options);

/**
 * Runs a single speed test and prints a series of messages
 * to the standard output according to the provided name for
 * the speed function.
 *
 * @param name The name of the test function to be executed
 * and measured for time.
 * @param function Pointer to the function to be executed
 * and have its execution time measured.
 * @param iterations The number of iterations to be executed
 * in the performance test in case this value is not provided
 * the value defaults to one.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE run_speed_test(char *name, test_function function, size_t iterations);

/**
 * Runs the test case defined by the provided function and
 * described thought the provided name.
 *
 * The test case execution will be verbose meaning that a
 * message output will be performed.
 *
 * @param function The function that is responsible for the
 * the execution of the various test functions.
 * @param name The name of the test case that is going to be
 * for some debug output.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE run_test_case(test_case_function function, const char *name);

/**
 * Runs the entries of the provided suite that are selected by
 * the provided options, writing the resulting report under the
 * format that the options describe.
 *
 * @param suite The suite whose entries are going to be run.
 * @param options The options that control both the selection of
 * the entries and the format of the report.
 * @return The error code resulting from the run, an error is
 * raised in case at least one of the tests has failed.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE run_test_suite(struct test_suite_t *suite, struct test_options_t *options);

/**
 * Prints the name and the tags of every entry of the suite that
 * is selected by the provided options, no test is executed.
 *
 * @param suite The suite whose entries are going to be listed.
 * @param options The options that control the selection of the
 * entries to be listed.
 * @return The error code resulting from the listing operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE list_test_suite(struct test_suite_t *suite, struct test_options_t *options);
