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

#include "runner_test.h"

/* the path of the file into which the reports produced by the
tests are written, it lives in the current directory as the
files written by the other tests of the suite do */
#define RUNNER_TEST_PATH "runner_test.tmp"

/* the size of the buffer used to read a report back from the
file it has been written to, the reports of the tests are only
a handful of lines long */
#define RUNNER_TEST_SIZE 4096

/* the counters that record the amount of times each of the
functions of the synthetic suite has been called, they allow
the tests to verify that a skipped entry is never run and that
the fixtures are set up and torn down exactly once */
static long _ok_calls = 0;
static long _skip_calls = 0;
static long _setup_calls = 0;
static long _cleanup_calls = 0;
static long _suite_setup_calls = 0;
static long _suite_cleanup_calls = 0;
static long _counter_calls = 0;

/**
 * Holds a single case of the parametrized entry of the
 * synthetic suite, the test passes for as long as both of
 * the values are the same one.
 */
typedef struct runner_params_t {

    /**
     * The value handed over to the test.
     */
    long value;

    /**
     * The value the test expects to receive.
     */
    long expected;

} runner_params;

/* the cases of the parametrized entry, the last of them is
meant to fail so that the reporting of an individual case may
be verified by the tests */
static struct runner_params_t _runner_params[] = {{1, 1}, {2, 2}, {3, 4}};

static const char *_ok_test(void) {
    _ok_calls++;
    return NULL;
}

static const char *_fail_test(void) { return "the failure message"; }

static const char *_skip_test(void) {
    _skip_calls++;
    return "a skipped test must never run";
}

static const char *_xfail_test(void) { return "the expected failure"; }

static const char *_xpass_test(void) { return NULL; }

static void *_setup_test(void) {
    /* allocates the context of the fixture and marks it with a
    known value so that the test may verify it was handed the
    very same context that has been created here */
    long *context = (long *) MALLOC(sizeof(long));
    *context = 42;
    _setup_calls++;
    return (void *) context;
}

static void _cleanup_test(void *context) {
    _cleanup_calls++;
    FREE(context);
}

static const char *_context_test(void *context) {
    V_ASSERT_NOT_NULL(context);
    V_ASSERT_EQ_I(*((long *) context), 42);
    return NULL;
}

static const char *_params_test(const void *params) {
    struct runner_params_t *_params = (struct runner_params_t *) params;
    V_ASSERT_EQ_I(_params->value, _params->expected);
    return NULL;
}

static void *_suite_setup_test(void) {
    _suite_setup_calls++;
    return NULL;
}

static void _suite_cleanup_test(void *context) {
    _suite_cleanup_calls++;
    dump(context);
}

/* the table of the synthetic suite, it carries one entry per
outcome an execution may end under plus the fixture and the
parametrized flavours of an entry */
static struct test_entry_t _runner_entries[] = {
    V_TEST_T(_ok_test, "unit"),
    V_TEST_T(_fail_test, "unit"),
    V_TEST_M(_skip_test, "unit", TEST_FLAG_SKIP),
    V_TEST_M(_xfail_test, "unit", TEST_FLAG_XFAIL),
    V_TEST_M(_xpass_test, "unit", TEST_FLAG_XFAIL),
    V_TEST_C(_context_test, "fixture", _setup_test, _cleanup_test),
    V_TEST_P(_params_test, "params", _runner_params, 3, sizeof(struct runner_params_t))
};

/**
 * Populates the provided suite with the synthetic entries and
 * resets every one of the counters, so that a test may verify
 * the amount of calls performed by the run it has started.
 *
 * @param suite The suite to be populated with the entries.
 */
static void _create_suite_test(struct test_suite_t *suite) {
    suite->name = "runner_tests";
    suite->entries = _runner_entries;
    suite->count = V_TEST_COUNT(_runner_entries);
    suite->setup = _suite_setup_test;
    suite->teardown = _suite_cleanup_test;

    _ok_calls = 0;
    _skip_calls = 0;
    _setup_calls = 0;
    _cleanup_calls = 0;
    _suite_setup_calls = 0;
    _suite_cleanup_calls = 0;
}

/**
 * Reads back the report that has been written to the file of
 * the tests, terminating the contents so that they may be
 * searched as a normal string.
 *
 * @param buffer The buffer that receives the contents.
 * @param size The complete size of the target buffer.
 * @return A message in case the file could not be read.
 */
static const char *_read_test(char *buffer, size_t size) {
    FILE *file;
    size_t count;

    FOPEN(&file, RUNNER_TEST_PATH, "rb");
    if(file == NULL) { return "the report file could not be opened"; }
    count = fread(buffer, sizeof(char), size - 1, file);
    buffer[count] = '\0';
    fclose(file);

    return NULL;
}

/**
 * Builds a report with one result per outcome that the writers
 * have to represent, the durations are fixed so that the tests
 * may assert on the values that end up in the report.
 *
 * @param report The report to be populated with the results.
 * @param results The array of five results to be used.
 */
static void _create_report_test(struct test_report_t *report, struct test_result_t *results) {
    STRCPY(results[0].name, TEST_NAME_SIZE, "test_first");
    results[0].status = TEST_STATUS_OK;
    results[0].elapsed = 0.5f;
    results[0].message[0] = '\0';

    STRCPY(results[1].name, TEST_NAME_SIZE, "test_second");
    results[1].status = TEST_STATUS_FAILURE;
    results[1].elapsed = 0.25f;
    STRCPY(results[1].message, TEST_MESSAGE_SIZE, "a & b < c > d \"e\" 'f'");

    STRCPY(results[2].name, TEST_NAME_SIZE, "test_third");
    results[2].status = TEST_STATUS_SKIP;
    results[2].elapsed = 0.0f;
    results[2].message[0] = '\0';

    STRCPY(results[3].name, TEST_NAME_SIZE, "test_fourth");
    results[3].status = TEST_STATUS_XFAIL;
    results[3].elapsed = 0.0f;
    STRCPY(results[3].message, TEST_MESSAGE_SIZE, "the expected failure");

    STRCPY(results[4].name, TEST_NAME_SIZE, "test_fifth");
    results[4].status = TEST_STATUS_XPASS;
    results[4].elapsed = 0.0f;
    results[4].message[0] = '\0';

    report->name = "report_tests";
    report->results = results;
    report->count = 5;
    report->elapsed = 0.75f;
}

static long _counter_test(void) {
    _counter_calls++;
    return 5;
}

static const char *_assert_once_test(void) {
    V_ASSERT_EQ_I(_counter_test(), 5);
    return NULL;
}

static const char *_assert_int_test(long value) {
    V_ASSERT_EQ_I(value, 42);
    return NULL;
}

static const char *_assert_unsigned_test(unsigned long value) {
    V_ASSERT_EQ_U(value, 7);
    return NULL;
}

static const char *_assert_string_test(const char *value) {
    V_ASSERT_EQ_S(value, "expected");
    return NULL;
}

static const char *_assert_string_null_test(const char *value) {
    V_ASSERT_EQ_S(value, NULL);
    return NULL;
}

static const char *_assert_pointer_test(const void *value) {
    V_ASSERT_EQ_P(value, _runner_params);
    return NULL;
}

static const char *_assert_null_test(const void *value) {
    V_ASSERT_NULL(value);
    return NULL;
}

static const char *_assert_not_null_test(const void *value) {
    V_ASSERT_NOT_NULL(value);
    return NULL;
}

static const char *_assert_memory_test(const char *value) {
    V_ASSERT_MEM(value, "abc", 3);
    return NULL;
}

const char *test_runner_format_message(void) {
    /* allocates space for the messages that are going to be
    built, they are expected to share the buffer of the runner */
    const char *first;
    const char *second;

    /* verifies that the values are properly interpolated into
    the format that has been provided */
    first = format_test_message("value is %d and name is %s", 12, "viriatum");
    V_ASSERT_EQ_S(first, "value is 12 and name is viriatum");

    /* verifies that a format without any placeholder is copied
    as it is into the buffer of the runner */
    second = format_test_message("%s", "plain");
    V_ASSERT_EQ_S(second, "plain");

    /* verifies that the buffer of the runner is reused, the
    message that has been built first is no longer available */
    V_ASSERT_EQ_P(first, second);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_assert_values(void) {
    /* allocates space for the message returned by each of the
    helpers that exercise one of the assertions */
    const char *message;
    char memory[4];

    /* verifies that the operand of an assertion is evaluated
    exactly once, a repeated evaluation would run the side
    effects of the expression more than a single time */
    _counter_calls = 0;
    message = _assert_once_test();
    V_ASSERT_NULL(message);
    V_ASSERT_EQ_I(_counter_calls, 1);

    /* verifies the integer assertion, both for a comparison
    that holds and for one that reports the two values */
    V_ASSERT_NULL(_assert_int_test(42));
    message = _assert_int_test(41);
    V_ASSERT_EQ_S(message, "value: expected 42, got 41");

    /* verifies the unsigned assertion, the values are reported
    without any sign being involved */
    V_ASSERT_NULL(_assert_unsigned_test(7));
    message = _assert_unsigned_test(9);
    V_ASSERT_EQ_S(message, "value: expected 7, got 9");

    /* verifies the string assertion, including the null value
    which is reported as such instead of being dereferenced */
    V_ASSERT_NULL(_assert_string_test("expected"));
    message = _assert_string_test("actual");
    V_ASSERT_EQ_S(message, "value: expected 'expected', got 'actual'");
    message = _assert_string_test(NULL);
    V_ASSERT_EQ_S(message, "value: expected 'expected', got '(null)'");

    /* verifies that two null values are considered to be the same
    one, only a null against a value is a failure */
    V_ASSERT_NULL(_assert_string_null_test(NULL));
    message = _assert_string_null_test("actual");
    V_ASSERT_EQ_S(message, "value: expected '(null)', got 'actual'");

    /* verifies the pointer assertion, only the identity of the
    two pointers is taken into account */
    V_ASSERT_NULL(_assert_pointer_test(_runner_params));
    V_ASSERT_NOT_NULL(_assert_pointer_test(memory));

    /* verifies the assertions that compare a pointer against
    the null value in both of the directions */
    V_ASSERT_NULL(_assert_null_test(NULL));
    V_ASSERT_NOT_NULL(_assert_null_test(memory));
    V_ASSERT_NULL(_assert_not_null_test(memory));
    message = _assert_not_null_test(NULL);
    V_ASSERT_EQ_S(message, "value: expected not null, got null");

    /* verifies the memory assertion, the amount of bytes that
    have been compared is part of the reported message */
    V_ASSERT_NULL(_assert_memory_test("abc"));
    message = _assert_memory_test("abd");
    V_ASSERT_EQ_S(message, "value: 3 bytes differ from the expected ones");

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_match_name(void) {
    /* verifies that an unset or empty pattern selects every
    one of the names that may be provided */
    V_ASSERT(match_name_test("test_first", NULL) == TRUE);
    V_ASSERT(match_name_test("test_first", "") == TRUE);

    /* verifies that a pattern without wildcards is matched as
    a sub sequence anywhere inside the name */
    V_ASSERT(match_name_test("test_websocket_frame", "websocket") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "test") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "frame") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "socketx") == FALSE);

    /* verifies the anchoring of a pattern that carries wildcards,
    the parts around them are matched in the order they appear */
    V_ASSERT(match_name_test("test_websocket_frame", "test_*") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "*_frame") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "test_*_frame") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "*websocket*") == TRUE);
    V_ASSERT(match_name_test("test_websocket_frame", "websocket*") == FALSE);
    V_ASSERT(match_name_test("test_websocket_frame", "*websocket") == FALSE);

    /* verifies the case that forces the matcher to backtrack, the
    first candidate for the part after the wildcard is not the one
    that allows the complete pattern to be matched */
    V_ASSERT(match_name_test("test_ababc", "test_*abc") == TRUE);
    V_ASSERT(match_name_test("test_ababd", "test_*abc") == FALSE);

    /* verifies the boundaries of the matching, a pattern that is
    longer than the name may never match it */
    V_ASSERT(match_name_test("ab", "abc*") == FALSE);
    V_ASSERT(match_name_test("", "*") == TRUE);
    V_ASSERT(match_name_test("anything", "*") == TRUE);
    V_ASSERT(match_name_test("anything", "**") == TRUE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_match_tags(void) {
    /* verifies that an unset or empty selection selects every
    test, even the ones that do not carry any tag at all */
    V_ASSERT(match_tags_test("structures", NULL) == TRUE);
    V_ASSERT(match_tags_test(NULL, NULL) == TRUE);
    V_ASSERT(match_tags_test(NULL, "") == TRUE);

    /* verifies that a test without tags is never selected by a
    selection that does carry at least one tag */
    V_ASSERT(match_tags_test(NULL, "structures") == FALSE);

    /* verifies the matching of a single tag against a test that
    carries one or several of them */
    V_ASSERT(match_tags_test("structures", "structures") == TRUE);
    V_ASSERT(match_tags_test("structures,slow", "slow") == TRUE);
    V_ASSERT(match_tags_test("structures,slow", "structures") == TRUE);
    V_ASSERT(match_tags_test("structures,slow", "network") == FALSE);

    /* verifies that any of the tags of the selection is enough
    for the test to be selected for the run */
    V_ASSERT(match_tags_test("structures", "network,structures") == TRUE);
    V_ASSERT(match_tags_test("structures", "network,stream") == FALSE);

    /* verifies that the tags are compared as complete tokens, a
    tag that is a prefix of another one may not select it */
    V_ASSERT(match_tags_test("structures", "struct") == FALSE);
    V_ASSERT(match_tags_test("struct", "structures") == FALSE);

    /* verifies that the spacing around the tags is ignored on
    both of the sides of the comparison */
    V_ASSERT(match_tags_test("structures, slow", "slow") == TRUE);
    V_ASSERT(match_tags_test("structures,slow", " slow , network") == TRUE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_match_entry(void) {
    /* allocates space for the entry and for the options that
    are going to be matched against each other */
    struct test_options_t options;
    struct test_entry_t entry = V_TEST_T(_ok_test, "unit,fast");

    /* verifies that an unset set of options selects the entry,
    no filtering is meant to happen in such a situation */
    V_ASSERT(match_entry_test(&entry, NULL) == TRUE);

    /* verifies that the default options select the entry as no
    filter and no tag selection are set in them */
    create_test_options(&options);
    V_ASSERT(match_entry_test(&entry, &options) == TRUE);

    /* verifies the selection performed by the name alone */
    options.filter = "_ok";
    V_ASSERT(match_entry_test(&entry, &options) == TRUE);
    options.filter = "_nope";
    V_ASSERT(match_entry_test(&entry, &options) == FALSE);

    /* verifies the selection performed by the tags alone */
    options.filter = NULL;
    options.tags = "fast";
    V_ASSERT(match_entry_test(&entry, &options) == TRUE);
    options.tags = "slow";
    V_ASSERT(match_entry_test(&entry, &options) == FALSE);

    /* verifies that both of the criteria have to be matched for
    the entry to be selected for the run */
    options.filter = "_ok";
    options.tags = "unit";
    V_ASSERT(match_entry_test(&entry, &options) == TRUE);
    options.filter = "_nope";
    V_ASSERT(match_entry_test(&entry, &options) == FALSE);
    options.filter = "_ok";
    options.tags = "slow";
    V_ASSERT(match_entry_test(&entry, &options) == FALSE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_options(void) {
    /* allocates space for the options to be created with the
    default values and verifies each one of them */
    struct test_options_t options;

    create_test_options(&options);

    V_ASSERT_NULL(options.filter);
    V_ASSERT_NULL(options.tags);
    V_ASSERT_NULL(options.format);
    V_ASSERT_NULL(options.path);
    V_ASSERT_EQ_I(options.list, FALSE);
    V_ASSERT_EQ_I(options.echo, TRUE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_run_suite(void) {
    /* allocates space for the suite, the options and the buffer
    into which the produced report is read back */
    ERROR_CODE error;
    struct test_suite_t suite;
    struct test_options_t options;
    char buffer[RUNNER_TEST_SIZE];

    /* creates the synthetic suite and requests a textual report
    to be written to the file of the tests, the echoing of the
    progress is turned off so that the output stays clean */
    _create_suite_test(&suite);
    create_test_options(&options);
    options.echo = FALSE;
    options.format = "text";
    options.path = RUNNER_TEST_PATH;

    /* the run is expected to fail, the suite carries both a
    failing entry and one that passed while expected to fail */
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error));

    /* verifies that the fixture of the entry has been set up and
    torn down exactly once and the same for the one of the suite */
    V_ASSERT_EQ_I(_setup_calls, 1);
    V_ASSERT_EQ_I(_cleanup_calls, 1);
    V_ASSERT_EQ_I(_suite_setup_calls, 1);
    V_ASSERT_EQ_I(_suite_cleanup_calls, 1);

    /* verifies that the entry marked as skipped has never been
    called while the plain one has been called once */
    V_ASSERT_EQ_I(_skip_calls, 0);
    V_ASSERT_EQ_I(_ok_calls, 1);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));

    /* verifies that every one of the outcomes is reported under
    the status that corresponds to it */
    V_ASSERT_NOT_NULL(strstr(buffer, "  _ok_test ... ok ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _fail_test ... not ok ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "    the failure message"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _skip_test ... skip ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _xfail_test ... xfail ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _xpass_test ... xpass ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _context_test ... ok ("));

    /* verifies that each of the cases of the parametrized entry
    is reported on its own, carrying the index of the case */
    V_ASSERT_NOT_NULL(strstr(buffer, "  _params_test[0] ... ok ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _params_test[1] ... ok ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "  _params_test[2] ... not ok ("));
    V_ASSERT_NOT_NULL(strstr(buffer, "_params->value: expected 4, got 3"));

    /* verifies the totals of the run, the nine executions of the
    seven entries of which three did not end well */
    V_ASSERT_NOT_NULL(strstr(buffer, "Ran 9 tests in "));
    V_ASSERT_NOT_NULL(strstr(buffer, "(3 not ok, 2 skipped)"));

    remove(RUNNER_TEST_PATH);

    /* runs the suite once more selecting a single entry by name
    and verifies that nothing else has been executed */
    _create_suite_test(&suite);
    options.filter = "_ok_test";
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_EQ_I(_ok_calls, 1);
    V_ASSERT_EQ_I(_setup_calls, 0);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "Ran 1 tests in "));

    remove(RUNNER_TEST_PATH);

    /* runs the suite selecting by tag, the parametrized entry is
    the only one carrying the tag and it accounts for three of
    the executions of the run */
    _create_suite_test(&suite);
    options.filter = NULL;
    options.tags = "params";
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error));

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "Ran 3 tests in "));

    remove(RUNNER_TEST_PATH);

    /* runs the suite with a filter that selects nothing at all,
    the run has to succeed reporting an empty set of results */
    _create_suite_test(&suite);
    options.tags = NULL;
    options.filter = "there_is_no_such_test";
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_EQ_I(_ok_calls, 0);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "Ran 0 tests in "));

    remove(RUNNER_TEST_PATH);

    /* runs a selection whose tests all pass but asks for a format
    that is not known, the run must fail as the report that has
    been requested was never produced */
    _create_suite_test(&suite);
    options.filter = "_ok_test";
    options.format = "there_is_no_such_format";
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_EQ_I(_ok_calls, 1);

    remove(RUNNER_TEST_PATH);

    /* the very same situation for a destination that may not be
    opened, the tests pass but the report never reaches the disk */
    _create_suite_test(&suite);
    options.format = "text";
    options.path = "there_is_no_such_directory/report.txt";
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_run_kinds(void) {
    /* allocates space for the suites that are built by hand, each
    of them carrying a single entry of the kind under test */
    ERROR_CODE error;
    struct test_suite_t suite;
    struct test_options_t options;
    struct test_entry_t entries[1];
    struct test_entry_t plain[1] = {V_TEST(_ok_test)};
    struct test_entry_t speed[1] = {V_TEST_S(_ok_test, "speed", 1)};
    char buffer[RUNNER_TEST_SIZE];

    /* lists a suite of a single entry that carries no tags at all,
    it is expected to be listed under its name alone */
    suite.name = "runner_defaults";
    suite.entries = plain;
    suite.count = V_TEST_COUNT(plain);
    suite.setup = NULL;
    suite.teardown = NULL;

    create_test_options(&options);
    error = list_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);

    /* runs a suite whose single entry carries no function of any
    kind, the run is expected to report it as a failure instead of
    calling through a null pointer */
    entries[0].name = "_empty_test";
    entries[0].function = NULL;
    entries[0].function_c = NULL;
    entries[0].function_p = NULL;
    entries[0].tags = NULL;
    entries[0].flags = TEST_FLAG_NONE;
    entries[0].params = NULL;
    entries[0].params_count = 0;
    entries[0].params_size = 0;
    entries[0].iterations = 0;
    entries[0].setup = NULL;
    entries[0].teardown = NULL;

    suite.entries = entries;
    suite.count = V_TEST_COUNT(entries);

    options.echo = FALSE;
    options.format = "text";
    options.path = RUNNER_TEST_PATH;

    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error));

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "no function has been defined for the entry"));

    remove(RUNNER_TEST_PATH);

    /* runs a suite whose single entry is a speed one, it is measured
    instead of being asserted and always counts as a success */
    suite.entries = speed;
    suite.count = V_TEST_COUNT(speed);

    _ok_calls = 0;
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_EQ_I(_ok_calls, 1);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "_ok_test ... ok ("));

    remove(RUNNER_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_list_suite(void) {
    /* allocates space for the suite and for the options that
    request the listing of the tests instead of their run */
    ERROR_CODE error;
    struct test_suite_t suite;
    struct test_options_t options;

    _create_suite_test(&suite);
    create_test_options(&options);
    options.list = TRUE;
    options.filter = "_ok_test";

    /* the listing must succeed even though the suite carries
    entries that would fail if they were to be run */
    error = run_test_suite(&suite, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);

    /* verifies that none of the functions of the suite has been
    called, not even the setup of the suite itself */
    V_ASSERT_EQ_I(_ok_calls, 0);
    V_ASSERT_EQ_I(_skip_calls, 0);
    V_ASSERT_EQ_I(_setup_calls, 0);
    V_ASSERT_EQ_I(_suite_setup_calls, 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_status_label(void) {
    V_ASSERT_EQ_S(status_label_report(TEST_STATUS_OK), "ok");
    V_ASSERT_EQ_S(status_label_report(TEST_STATUS_FAILURE), "not ok");
    V_ASSERT_EQ_S(status_label_report(TEST_STATUS_SKIP), "skip");
    V_ASSERT_EQ_S(status_label_report(TEST_STATUS_XFAIL), "xfail");
    V_ASSERT_EQ_S(status_label_report(TEST_STATUS_XPASS), "xpass");

    /* verifies that a status outside of the known ones is still
    reported under a label instead of a null value */
    V_ASSERT_EQ_S(status_label_report((enum test_status_e) 0), "unknown");

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_escape_xml(void) {
    /* allocates space for the buffer that receives the escaped
    version of each of the values under test */
    char buffer[64];

    /* verifies that a value without any entity is copied as it
    is into the target buffer */
    escape_xml_report("plain value", buffer, sizeof(buffer));
    V_ASSERT_EQ_S(buffer, "plain value");

    /* verifies that every one of the five entities of xml is
    properly replaced by its escaped version */
    escape_xml_report("a&b", buffer, sizeof(buffer));
    V_ASSERT_EQ_S(buffer, "a&amp;b");
    escape_xml_report("a<b>c", buffer, sizeof(buffer));
    V_ASSERT_EQ_S(buffer, "a&lt;b&gt;c");
    escape_xml_report("a\"b'c", buffer, sizeof(buffer));
    V_ASSERT_EQ_S(buffer, "a&quot;b&apos;c");

    /* verifies that a null value produces an empty string, the
    buffer must always end up properly terminated */
    escape_xml_report(NULL, buffer, sizeof(buffer));
    V_ASSERT_EQ_S(buffer, "");

    /* verifies that an empty value is handled as such */
    escape_xml_report("", buffer, sizeof(buffer));
    V_ASSERT_EQ_S(buffer, "");

    /* verifies that a value that does not fit is truncated
    instead of overflowing the buffer that receives it */
    escape_xml_report("abcdefghij", buffer, 6);
    V_ASSERT_EQ_S(buffer, "abcde");

    /* verifies that an entity is never truncated in the middle,
    it is left out completely when it does not fit */
    escape_xml_report("ab&cd", buffer, 6);
    V_ASSERT_EQ_S(buffer, "ab");

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_write_text(void) {
    /* allocates space for the report to be written and for the
    buffer into which it is read back */
    FILE *file;
    struct test_report_t report;
    struct test_result_t results[5];
    char buffer[RUNNER_TEST_SIZE];

    _create_report_test(&report, results);

    FOPEN(&file, RUNNER_TEST_PATH, "wb");
    V_ASSERT_NOT_NULL(file);
    write_text_report(&report, file);
    fclose(file);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));

    V_ASSERT_NOT_NULL(strstr(buffer, "report_tests\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  test_first ... ok (0.50 seconds)"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  test_second ... not ok (0.25 seconds)"));
    V_ASSERT_NOT_NULL(strstr(buffer, "    a & b < c > d \"e\" 'f'"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  test_third ... skip (0.00 seconds)"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  test_fourth ... xfail (0.00 seconds)"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  test_fifth ... xpass (0.00 seconds)"));
    V_ASSERT_NOT_NULL(strstr(buffer, "Ran 5 tests in 0.75 seconds (2 not ok, 2 skipped)"));

    remove(RUNNER_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_write_tap(void) {
    /* allocates space for the report to be written and for the
    buffer into which it is read back */
    FILE *file;
    struct test_report_t report;
    struct test_result_t results[5];
    char buffer[RUNNER_TEST_SIZE];

    _create_report_test(&report, results);

    FOPEN(&file, RUNNER_TEST_PATH, "wb");
    V_ASSERT_NOT_NULL(file);
    write_tap_report(&report, file);
    fclose(file);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));

    V_ASSERT_NOT_NULL(strstr(buffer, "TAP version 13\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "1..5\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "ok 1 - test_first\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "not ok 2 - test_second\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "  message: 'a & b < c > d \"e\" 'f''\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "ok 3 - test_third # SKIP\n"));

    /* an expected failure is reported as a todo directive, the one
    that unexpectedly passed carries the very same directive over a
    result that did end well */
    V_ASSERT_NOT_NULL(strstr(buffer, "not ok 4 - test_fourth # TODO\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "ok 5 - test_fifth # TODO\n"));

    remove(RUNNER_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_write_junit(void) {
    /* allocates space for the report to be written and for the
    buffer into which it is read back */
    FILE *file;
    struct test_report_t report;
    struct test_result_t results[5];
    char buffer[RUNNER_TEST_SIZE];

    _create_report_test(&report, results);

    FOPEN(&file, RUNNER_TEST_PATH, "wb");
    V_ASSERT_NOT_NULL(file);
    write_junit_report(&report, file);
    fclose(file);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));

    V_ASSERT_NOT_NULL(strstr(buffer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"));
    V_ASSERT_NOT_NULL(
        strstr(
            buffer,
            "<testsuite name=\"report_tests\" tests=\"5\" failures=\"2\" skipped=\"2\" time=\"0.750\">"
        )
    );

    /* verifies that a case that ended well carries no child at
    all, the element is closed straight away */
    V_ASSERT_NOT_NULL(
        strstr(buffer, "<testcase name=\"test_first\" classname=\"report_tests\" time=\"0.500\"/>")
    );

    /* verifies that the message of a failure is escaped, none of
    the five entities of xml may reach the report as it is */
    V_ASSERT_NOT_NULL(
        strstr(
            buffer,
            "<failure message=\"a &amp; b &lt; c &gt; d &quot;e&quot; &apos;f&apos;\"/>"
        )
    );

    V_ASSERT_NOT_NULL(strstr(buffer, "<skipped/>"));

    /* an expected failure has no representation of its own in the
    format and so it is reported as a skipped case carrying a reason,
    the one that unexpectedly passed is a failure instead */
    V_ASSERT_NOT_NULL(strstr(buffer, "<skipped message=\"expected failure\"/>"));
    V_ASSERT_NOT_NULL(
        strstr(buffer, "<failure message=\"the test was expected to fail but it passed\"/>")
    );

    V_ASSERT_NOT_NULL(strstr(buffer, "</testsuite>"));
    V_ASSERT_NOT_NULL(strstr(buffer, "</testsuites>"));

    remove(RUNNER_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_write_markdown(void) {
    /* allocates space for the report to be written and for the
    buffer into which it is read back */
    FILE *file;
    struct test_report_t report;
    struct test_result_t results[5];
    char buffer[RUNNER_TEST_SIZE];

    _create_report_test(&report, results);

    FOPEN(&file, RUNNER_TEST_PATH, "wb");
    V_ASSERT_NOT_NULL(file);
    write_markdown_report(&report, file);
    fclose(file);

    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));

    V_ASSERT_NOT_NULL(strstr(buffer, "## Tests\n"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| Test | Status | Duration | |"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| `test_first` | ok | 0.50s |"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| `test_second` | not ok | 0.25s |"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| `test_third` | skip | 0.00s |"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| `test_fourth` | xfail | 0.00s |"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| `test_fifth` | xpass | 0.00s |"));
    V_ASSERT_NOT_NULL(strstr(buffer, "| **total** | **2 not ok** | **0.75s** |"));

    remove(RUNNER_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_runner_write_report(void) {
    /* allocates space for the report to be dispatched and for
    the options that describe the format and the destination */
    ERROR_CODE error;
    struct test_report_t report;
    struct test_report_t empty;
    struct test_result_t results[5];
    struct test_options_t options;
    char buffer[RUNNER_TEST_SIZE];

    _create_report_test(&report, results);
    create_test_options(&options);
    options.path = RUNNER_TEST_PATH;

    /* makes sure that no report file is left over from a test
    that ran before this one, the first verification depends on
    the destination not existing yet */
    remove(RUNNER_TEST_PATH);

    /* verifies that no report at all is written when no format
    has been requested, not even an empty file */
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_NOT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));

    /* verifies that no report is written for a null set of
    options, the same situation as an unset format */
    error = write_test_report(&report, NULL);
    V_ASSERT(IS_ERROR_CODE(error) == 0);

    /* verifies that the requested format reaches the writer that
    corresponds to it, each of them producing its own header */
    options.format = "text";
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "report_tests\n"));

    options.format = "tap";
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "TAP version 13"));

    options.format = "junit";
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "<testsuites>"));

    options.format = "markdown";
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);
    V_ASSERT_NULL(_read_test(buffer, RUNNER_TEST_SIZE));
    V_ASSERT_NOT_NULL(strstr(buffer, "## Tests"));

    /* verifies that a format that is not known is reported as an
    error instead of silently producing nothing */
    options.format = "there_is_no_such_format";
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error));

    remove(RUNNER_TEST_PATH);

    /* verifies that a destination that may not be opened is
    reported as an error by the writing of the report */
    options.format = "text";
    options.path = "there_is_no_such_directory/report.txt";
    error = write_test_report(&report, &options);
    V_ASSERT(IS_ERROR_CODE(error));

    /* verifies that the standard output is the destination whenever
    no path has been set, an empty report under the textual format is
    used for it so that the run that is in progress is neither made
    unreadable nor handed a second header of a machine readable one */
    empty.name = "empty_tests";
    empty.results = NULL;
    empty.count = 0;
    empty.elapsed = 0.0f;

    options.format = "text";
    options.path = NULL;
    error = write_test_report(&empty, &options);
    V_ASSERT(IS_ERROR_CODE(error) == 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
