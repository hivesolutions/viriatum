/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "../stdafx.h"

#include "mod_asgi_test.h"

/**
 * The path to the script that the tests load an application out
 * of, handed by the build so that a run from anywhere finds it.
 */
#ifdef VIRIATUM_TEST_SCRIPT_PATH
#define ASGI_TEST_SCRIPT_PATH VIRIATUM_TEST_SCRIPT_PATH
#else
#define ASGI_TEST_SCRIPT_PATH "test_handler.py"
#endif

/**
 * The path that a file which does not compile is written to, so
 * that the refusing of one may be driven.
 */
#define ASGI_TEST_BROKEN_PATH "./viriatum_mod_asgi_broken.py"

const char *test_mod_asgi_http_handler(void) {
    /* allocates space for the handler that is going to be built
    and for the one of the upper layer it is set in */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    struct http_handler_t http_handler;

    /* the handler starts with nothing loaded and with the values
    that stand in for a configuration that named none */
    create_mod_asgi_http_handler(&mod_asgi_http_handler, &http_handler);
    V_ASSERT_EQ_S(mod_asgi_http_handler->file_path, DEFAULT_FILE_PATH);
    V_ASSERT_EQ_S(mod_asgi_http_handler->application_name, DEFAULT_APPLICATION_NAME);
    V_ASSERT_NULL(mod_asgi_http_handler->module);
    V_ASSERT_NULL(mod_asgi_http_handler->application);

    /* the handler of the upper layer reaches the one that has just
    been built, which is how the serving finds it */
    V_ASSERT_EQ_P(http_handler.lower, mod_asgi_http_handler);

    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application(void) {
    /* allocates space for the handler that the application is
    loaded into and for the error the loading raises */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    ERROR_CODE error;

    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);

    /* the application named by the handler is taken out of the
    module and both of them are held, the module being what keeps
    the globals the application reads alive */
    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(mod_asgi_http_handler->module);
    V_ASSERT_NOT_NULL(mod_asgi_http_handler->application);
    V_ASSERT(PyCallable_Check(mod_asgi_http_handler->application));

    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_missing(void) {
    /* allocates space for the handler that the loading is driven
    against and for the error it raises */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    ERROR_CODE error;

    /* a path that no file sits under is refused and leaves the
    handler with nothing set rather than half of something */
    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", "./viriatum_mod_asgi_absent.py");
    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_asgi_http_handler->module);
    V_ASSERT_NULL(mod_asgi_http_handler->application);
    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* a name the module does not carry is refused the same way,
    the module having loaded is not enough on its own */
    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);
    SPRINTF(mod_asgi_http_handler->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", "absent");
    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_asgi_http_handler->module);
    V_ASSERT_NULL(mod_asgi_http_handler->application);
    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_uncallable(void) {
    /* allocates space for the handler that the loading is driven
    against and for the error it raises */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    ERROR_CODE error;

    /* a name that resolves to something that may not be called is
    not an application, the module carries a number under one */
    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);
    SPRINTF(mod_asgi_http_handler->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", "number");
    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(mod_asgi_http_handler->application);

    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_reload(void) {
    /* allocates space for the handler and for the reference that
    the first of the two loadings left in it */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    PyObject *application;
    ERROR_CODE error;

    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);

    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT_EQ_U(error, 0);
    application = mod_asgi_http_handler->application;
    Py_INCREF(application);

    /* the second of the loadings replaces what the first one left
    rather than writing over it, a reference that was written over
    would never be released and the count of it says so */
    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(mod_asgi_http_handler->application);
    V_ASSERT_EQ_I(Py_REFCNT(application), 1);
    Py_DECREF(application);

    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_application_unload(void) {
    /* allocates space for the handler that the unloading is
    driven against and for the error the loading raises */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    ERROR_CODE error;

    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", ASGI_TEST_SCRIPT_PATH);
    error = load_application_asgi(mod_asgi_http_handler);
    V_ASSERT_EQ_U(error, 0);

    /* the unloading leaves nothing set, so that the releasing of
    the very same reference a second time is never possible */
    unload_application_asgi(mod_asgi_http_handler);
    V_ASSERT_NULL(mod_asgi_http_handler->module);
    V_ASSERT_NULL(mod_asgi_http_handler->application);

    /* and running it again over a handler that carries nothing is
    harmless, which is what the deleting of one relies on */
    unload_application_asgi(mod_asgi_http_handler);
    V_ASSERT_NULL(mod_asgi_http_handler->module);
    V_ASSERT_NULL(mod_asgi_http_handler->application);

    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_load(void) {
    /* allocates space for the module that is loaded and for the
    name that is taken out of it */
    PyObject *module;
    PyObject *value;
    ERROR_CODE error;

    /* the module is loaded out of the file and carries the names
    that the file defines, the application among them */
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_load", (char *) ASGI_TEST_SCRIPT_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(module);

    value = PyObject_GetAttrString(module, "application");
    V_ASSERT_NOT_NULL(value);
    V_ASSERT(PyCallable_Check(value));
    Py_DECREF(value);

    Py_DECREF(module);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_broken(void) {
    /* allocates space for the module that the loading writes and
    for the error it raises */
    PyObject *module;
    ERROR_CODE error;

    /* a file that is not there leaves the module unset without
    raising, the loading of an application is what tells that
    apart and reports it */
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_absent", (char *) "./viriatum_mod_asgi_absent.py");
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(module);

    /* a file that compiles and then raises while it runs is left
    the very same way, the module of it never coming to be */
    write_file(
        (char *) ASGI_TEST_BROKEN_PATH,
        (unsigned char *) "raise ValueError('boom')\n",
        25
    );
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_raising", (char *) ASGI_TEST_BROKEN_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(module);
    V_ASSERT_NULL(PyErr_Occurred());
    remove(ASGI_TEST_BROKEN_PATH);

    /* a file that does not compile is left the very same way, the
    error of the interpreter being cleared rather than carried out
    into a request that has nothing to do with it */
    write_file(
        (char *) ASGI_TEST_BROKEN_PATH,
        (unsigned char *) "def application(:\n",
        18
    );
    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_broken", (char *) ASGI_TEST_BROKEN_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NULL(module);
    V_ASSERT_NULL(PyErr_Occurred());
    remove(ASGI_TEST_BROKEN_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_double_callable(void) {
    /* allocates space for the module the shapes are taken out of,
    for each of them and for the flag that is decided */
    PyObject *module;
    PyObject *value;
    char double_callable;
    ERROR_CODE error;

    module = NULL;
    error = _load_module_asgi(&module, "asgi_app_test_callable", (char *) ASGI_TEST_SCRIPT_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(module);

    /* the single callable of the more recent version takes the
    three values in one call and is not the double one */
    value = PyObject_GetAttrString(module, "application");
    V_ASSERT_NOT_NULL(value);
    _double_callable_asgi(value, &double_callable);
    V_ASSERT_EQ_U(double_callable, FALSE);
    Py_DECREF(value);

    /* the pair of the older takes the scope apart, which the
    single argument of it is what says */
    value = PyObject_GetAttrString(module, "legacy");
    V_ASSERT_NOT_NULL(value);
    _double_callable_asgi(value, &double_callable);
    V_ASSERT_EQ_U(double_callable, TRUE);
    Py_DECREF(value);

    /* a class stands for the very same pair, the building of the
    instance taking the scope and the calling of it the callables */
    value = PyObject_GetAttrString(module, "Application");
    V_ASSERT_NOT_NULL(value);
    _double_callable_asgi(value, &double_callable);
    V_ASSERT_EQ_U(double_callable, TRUE);
    Py_DECREF(value);

    /* something that carries no code of its own is left as the
    more recent shape rather than being guessed at */
    value = PyObject_GetAttrString(module, "number");
    V_ASSERT_NOT_NULL(value);
    _double_callable_asgi(value, &double_callable);
    V_ASSERT_EQ_U(double_callable, FALSE);
    V_ASSERT_NULL(PyErr_Occurred());
    Py_DECREF(value);

    Py_DECREF(module);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_info(void) {
    /* allocates space for the module that is populated by the
    operation that describes one */
    struct module_t module;

    /* the module carries the name and the version of the build
    together with the operations the service reaches it through */
    info_module_asgi(&module);
    V_ASSERT_EQ_S((char *) module.name, "viriatum_mod_asgi");
    V_ASSERT_EQ_S((char *) module.name_s, "asgi");
    V_ASSERT_EQ_U(module.type, MODULE_TYPE_HTTP_HANDLER);
    V_ASSERT_EQ_P(module.start, start_module_asgi);
    V_ASSERT_EQ_P(module.stop, stop_module_asgi);
    V_ASSERT_EQ_P(module.info, info_module_asgi);
    V_ASSERT_EQ_P(module.error, error_module_asgi);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_error(void) {
    /* allocates space for the message that the operation hands
    back, which is the reason of the last failure */
    unsigned char *message;

    /* the reason reaches the caller rather than being left for it
    to be read out of somewhere else */
    error_module_asgi(&message);
    V_ASSERT_EQ_P(message, get_last_error_message());

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_configuration(void) {
    /* allocates space for the handler the values reach, for the
    service that carries them and for the maps of it */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;
    struct service_t service;
    struct service_options_t options;
    struct sort_map_t *configuration;
    struct sort_map_t *section;

    /* a service that carries no configuration at all leaves the
    handler with the values that stand in for one */
    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    service.configuration = NULL;
    _load_configuration_asgi(&service, mod_asgi_http_handler);
    V_ASSERT_EQ_S(mod_asgi_http_handler->file_path, DEFAULT_FILE_PATH);
    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* and so does one whose configuration carries no section of
    this module, the reading of it stopping there */
    create_sort_map(&configuration, 8);
    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    service.configuration = configuration;
    _load_configuration_asgi(&service, mod_asgi_http_handler);
    V_ASSERT_EQ_S(mod_asgi_http_handler->file_path, DEFAULT_FILE_PATH);
    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* a path that is absolute reaches the handler as it was
    written, together with the name of the application */
    create_sort_map(&section, 8);
    set_value_string_sort_map(section, (unsigned char *) "script_path", (void *) "/tmp/app.py");
    set_value_string_sort_map(section, (unsigned char *) "application", (void *) "other");
    set_value_string_sort_map(configuration, (unsigned char *) "mod_asgi", (void *) section);

    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    _load_configuration_asgi(&service, mod_asgi_http_handler);
    V_ASSERT_EQ_S(mod_asgi_http_handler->file_path, "/tmp/app.py");
    V_ASSERT_EQ_S(mod_asgi_http_handler->application_name, "other");
    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    /* one that is relative is resolved against the contents of the
    service, so that a configuration never has to name the tree */
    set_value_string_sort_map(section, (unsigned char *) "script_path", (void *) "app.py");
    SPRINTF((char *) options.contents_path, VIRIATUM_MAX_PATH_SIZE, "%s", "/srv");
    service.options = &options;

    create_mod_asgi_http_handler(&mod_asgi_http_handler, NULL);
    _load_configuration_asgi(&service, mod_asgi_http_handler);
    V_ASSERT_EQ_S(mod_asgi_http_handler->file_path, "/srv" VIRIATUM_PATH_SEPARATOR "app.py");
    delete_mod_asgi_http_handler(mod_asgi_http_handler);

    delete_sort_map(section);
    delete_sort_map(configuration);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_cycle(void) {
    /* allocates space for the service the operation is run
    against, which it only ever reaches the polling of */
    struct service_t service;
    ERROR_CODE error;

    /* the operation is harmless while no module has been started,
    which is the state a loading that failed leaves behind, and it
    never reaches the polling of the service in that case */
    _mod_asgi_module = NULL;
    error = cycle_module_asgi(&service);
    V_ASSERT_EQ_U(error, 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_asgi_module_lifecycle(void) {
    /* allocates space for the service the module is started against,
    for the environment that carries it and for the module itself */
    struct service_t *service;
    struct environment_t environment;
    struct module_t module;
    struct http_handler_t *http_handler;
    struct mod_asgi_module_t *mod_asgi_module;
    struct sort_map_t *configuration;
    struct sort_map_t *section;

    /* builds the service and points its configuration at the script
    the suite loads an application out of, which is what the starting
    of the module reads the path out of */
    create_service(&service, (unsigned char *) "viriatum", (unsigned char *) "viriatum");
    create_sort_map(&configuration, 8);
    create_sort_map(&section, 8);
    set_value_string_sort_map(section, (unsigned char *) "script_path", (void *) ASGI_TEST_SCRIPT_PATH);
    set_value_string_sort_map(configuration, (unsigned char *) "mod_asgi", (void *) section);
    service->configuration = configuration;
    environment.name = (unsigned char *) "test";
    environment.service = service;

    /* starts the module, which brings the interpreter up, loads the
    application and hands the serving of it to the handler */
    start_module_asgi(&environment, &module);
    mod_asgi_module = (struct mod_asgi_module_t *) module.lower;
    V_ASSERT_NOT_NULL(mod_asgi_module);
    V_ASSERT_NOT_NULL(mod_asgi_module->http_handler);
    V_ASSERT_NOT_NULL(mod_asgi_module->mod_asgi_http_handler);
    V_ASSERT_NOT_NULL(mod_asgi_module->mod_asgi_http_handler->application);
    V_ASSERT_NOT_NULL(mod_asgi_module->loop_python);

    /* the handler of the interface reaches the service under the name
    that a configuration and the command line both name it by */
    http_handler = NULL;
    service->get_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    V_ASSERT_NOT_NULL(http_handler);

    /* the operation run once per cycle is taken by the module, which
    is what advances a task that no request is driving */
    V_ASSERT_EQ_P(service->on_cycle, cycle_module_asgi);

    /* and running it advances the loop rather than raising, the
    waiting of the polling being left at a value it may wait on */
    V_ASSERT_EQ_U(cycle_module_asgi(service), 0);

    /* stops the module, which takes the application down, gives the
    operation of the cycle back and releases everything it held */
    stop_module_asgi(&environment, &module);
    V_ASSERT_NULL(service->on_cycle);

    http_handler = NULL;
    service->get_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);
    V_ASSERT_NULL(http_handler);

    delete_sort_map(section);
    delete_sort_map(configuration);
    service->configuration = NULL;
    delete_service(service);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

void exec_mod_asgi_tests(struct test_case_t *test_case) {
    /* the building and the releasing of the handler */
    V_RUN_TEST(test_mod_asgi_http_handler, test_case);

    /* the loading of an application out of a module */
    V_RUN_TEST(test_asgi_application, test_case);
    V_RUN_TEST(test_asgi_application_missing, test_case);
    V_RUN_TEST(test_asgi_application_uncallable, test_case);
    V_RUN_TEST(test_asgi_application_reload, test_case);
    V_RUN_TEST(test_asgi_application_unload, test_case);

    /* the loading of the module itself */
    V_RUN_TEST(test_asgi_module_load, test_case);
    V_RUN_TEST(test_asgi_module_broken, test_case);

    /* the shape of the callable of an application */
    V_RUN_TEST(test_asgi_double_callable, test_case);

    /* the entry point of the module itself */
    V_RUN_TEST(test_asgi_module_info, test_case);
    V_RUN_TEST(test_asgi_module_error, test_case);
    V_RUN_TEST(test_asgi_configuration, test_case);
    V_RUN_TEST(test_asgi_cycle, test_case);

    /* the starting and the stopping of the module, which takes the
    interpreter down with it and so closes the suite */
    V_RUN_TEST(test_asgi_module_lifecycle, test_case);
}

ERROR_CODE run_mod_asgi_tests(void) {
    ERROR_CODE return_value = run_test_case(exec_mod_asgi_tests, "mod_asgi_tests");
    RAISE_AGAIN(return_value);
}
