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

#include "stdafx.h"

#include "entry.h"

/* starts the memory structures */
START_MEMORY;

/* initializes the module global variables, these
values will be used across functions */
struct service_t *_service;
struct mod_asgi_module_t *_mod_asgi_module;

ERROR_CODE create_mod_asgi_module(struct mod_asgi_module_t **mod_asgi_module_pointer, struct module_t *module) {
    /* retrieves the mod ASGI module size */
    size_t mod_asgi_module_size = sizeof(struct mod_asgi_module_t);

    /* allocates space for the mod ASGI module */
    struct mod_asgi_module_t *mod_asgi_module = (struct mod_asgi_module_t *) MALLOC(mod_asgi_module_size);

    /* sets the mod ASGI module attributes (default) values */
    mod_asgi_module->http_handler = NULL;
    mod_asgi_module->mod_asgi_http_handler = NULL;
    mod_asgi_module->loop_python = NULL;
    mod_asgi_module->started = FALSE;

    /* sets the mod ASGI module in the (upper) module substrate */
    module->lower = (void *) mod_asgi_module;

    /* sets the mod ASGI module in the module pointer */
    *mod_asgi_module_pointer = mod_asgi_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_asgi_module(struct mod_asgi_module_t *mod_asgi_module) {
    /* releases the mod ASGI module */
    FREE(mod_asgi_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE start_module_asgi(struct environment_t *environment, struct module_t *module) {
    /* allocates the mod ASGI module */
    struct mod_asgi_module_t *mod_asgi_module;

    /* allocates the HTTP handler */
    struct http_handler_t *http_handler;

    /* allocates the mod ASGI HTTP handler */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_asgi();
    unsigned char *version = version_viriatum_mod_asgi();
    unsigned char *description = description_viriatum_mod_asgi();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_CTX_F("mod_asgi", "Starting the module '%s' (%s) v%s\n", name, description, version);

    /* sets the global service reference to be used in the
    externalized function for the interpreter */
    _service = service;

    /* creates the mod ASGI module */
    create_mod_asgi_module(&mod_asgi_module, module);

    /* sets the global module reference so that the operation run
    once per cycle is able to reach the loop of it */
    _mod_asgi_module = mod_asgi_module;

    /* populates the module structure */
    info_module_asgi(module);

    /* loads the ASGI state populating all the required values
    for state initialization */
    _load_asgi_state(service);

    /* creates the HTTP handler */
    service->create_http_handler(service, &http_handler, VIRIATUM_ASGI_HANDLER_NAME);

    /* creates the mod ASGI HTTP handler */
    create_mod_asgi_http_handler(&mod_asgi_http_handler, http_handler);

    /* sets the mod ASGI module attributes */
    mod_asgi_module->http_handler = http_handler;
    mod_asgi_module->mod_asgi_http_handler = mod_asgi_http_handler;

    /* loads the service configuration for the HTTP handler
    this should change some of it's behavior, naming the file
    the application is loaded out of */
    _load_configuration_asgi(service, mod_asgi_http_handler);

    /* loads the application out of the file that the configuration
    named, a failure of it leaves the module registered with nothing
    to serve, which the handler reports on every request */
    if(IS_ERROR_CODE(load_application_asgi(mod_asgi_http_handler))) {
        V_DEBUG_CTX_F("mod_asgi", "No application was loaded from '%s'\n", mod_asgi_http_handler->file_path);
        RAISE_NO_ERROR;
    }

    /* builds the loop that the tasks of the application run on, the
    handler of the interface takes it and advances it while a request
    is being served, the cycle of the service advances the rest */
    if(IS_ERROR_CODE(create_loop_python(&mod_asgi_module->loop_python))) {
        V_DEBUG_CTX("mod_asgi", "Problem creating the loop of the applications\n");
        RAISE_NO_ERROR;
    }

    /* attaches the loop as the running one of the interpreter so that
    the accessors of the asynchronous module resolve to it inside the
    code of the application */
    attach_loop_python(mod_asgi_module->loop_python);

    /* hands the serving of the interface to the handler that the tree
    already implements, taking a reference on the application, and
    detects the shape of the callable rather than requiring it */
    _double_callable_asgi(
        mod_asgi_http_handler->application,
        &mod_asgi_module->double_callable
    );
    register_handler_asgi(
        service,
        mod_asgi_http_handler->application,
        mod_asgi_module->loop_python,
        mod_asgi_module->double_callable
    );

    /* runs the startup event of the lifespan, an application that
    refuses to boot leaves the module with nothing to serve */
    if(IS_ERROR_CODE(startup_handler_asgi(service))) {
        V_DEBUG_CTX("mod_asgi", "The application refused to start\n");
        RAISE_NO_ERROR;
    }
    mod_asgi_module->started = TRUE;

    /* takes the operation run once per cycle of the loop of the
    service, which is what advances a task that no request is
    driving, the writing of a websocket among them */
    service->on_cycle = cycle_module_asgi;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_module_asgi(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_asgi();
    unsigned char *version = version_viriatum_mod_asgi();
    unsigned char *description = description_viriatum_mod_asgi();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* retrieves the mod ASGI module (from the module) */
    struct mod_asgi_module_t *mod_asgi_module = (struct mod_asgi_module_t *) module->lower;

    /* retrieves the HTTP handler from the mod ASGI module */
    struct http_handler_t *http_handler = mod_asgi_module->http_handler;

    /* retrieves the mod ASGI HTTP handler from the mod ASGI module */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler = mod_asgi_module->mod_asgi_http_handler;

    /* prints a debug message */
    V_DEBUG_CTX_F("mod_asgi", "Stopping the module '%s' (%s) v%s\n", name, description, version);

    /* gives the operation run once per cycle back, the loop it
    advances is about to go */
    service->on_cycle = NULL;

    /* runs the shutdown event of the lifespan, only ever against an
    application whose startup was run in the first place */
    if(mod_asgi_module->started == TRUE) {
        shutdown_handler_asgi(service);
        mod_asgi_module->started = FALSE;
    }

    /* hands the serving back, which releases the reference that the
    handler took on the application */
    if(mod_asgi_module->loop_python != NULL) {
        unregister_handler_asgi(service);
    }

    /* in case the mod ASGI HTTP handler is valid and
    initialized (correct state) */
    if(mod_asgi_http_handler != NULL) {
        /* deletes the mod ASGI HTTP handler */
        delete_mod_asgi_http_handler(mod_asgi_http_handler);
    }

    /* in case the HTTP handler is valid and
    initialized (correct state) */
    if(http_handler != NULL) {
        /* deletes the HTTP handler */
        service->delete_http_handler(service, http_handler);
    }

    /* releases the loop, cancelling whatever task may still be
    pending on it, before the interpreter that owns them goes */
    if(mod_asgi_module->loop_python != NULL) {
        delete_loop_python(mod_asgi_module->loop_python);
        mod_asgi_module->loop_python = NULL;
    }

    /* unloads the ASGI state destroying all the required values
    for state destroyed */
    _unload_asgi_state();

    /* deletes the mod ASGI module */
    delete_mod_asgi_module(mod_asgi_module);

    /* sets the global references this is no longer necessary
    because the module has been unloaded */
    _service = NULL;
    _mod_asgi_module = NULL;

    /* cleans up the pool based memory allocation system releasing all
    of its memory before the exit (no leaks) then returns the control
    flow to the caller function with success state */
    cleanup_palloc();
    RAISE_NO_ERROR;
}

ERROR_CODE info_module_asgi(struct module_t *module) {
    /* retrieves the various elements that are going
    to be used in the construction of the of the module */
    unsigned char *name = name_viriatum_mod_asgi();
    unsigned char *name_s = name_s_viriatum_mod_asgi();
    unsigned char *version = version_viriatum_mod_asgi();

    /* populates the module structure */
    module->name = name;
    module->name_s = name_s;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = start_module_asgi;
    module->stop = stop_module_asgi;
    module->info = info_module_asgi;
    module->error = error_module_asgi;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE error_module_asgi(unsigned char **message_pointer) {
    /* sets the error message in the (error) message pointer */
    *message_pointer = get_last_error_message();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE cycle_module_asgi(struct service_t *service) {
    /* in case there is no module or no loop for it there is nothing
    to be advanced, which is the state a failed loading leaves */
    if(_mod_asgi_module == NULL) { RAISE_NO_ERROR; }
    if(_mod_asgi_module->loop_python == NULL) { RAISE_NO_ERROR; }

    /* advances the loop of the applications by a single iteration,
    which is what makes a task that no request is driving progress,
    then shortens the waiting of the polling while any of them is
    pending so that the timers they schedule keep a usable resolution */
    run_once_loop_python(_mod_asgi_module->loop_python);
    service->polling->timeout =
        pending_loop_python(_mod_asgi_module->loop_python) > 0 ? VIRIATUM_ASGI_POLL_TIMEOUT : VIRIATUM_ASGI_IDLE_TIMEOUT;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_configuration_asgi(struct service_t *service, struct mod_asgi_http_handler_t *mod_asgi_http_handler) {
    /* allocates space for both a configuration item reference
    (value) and for the configuration to be retrieved */
    void *value;
    struct sort_map_t *configuration;

    /* in case the current service configuration is not set
    must return immediately (not possible to load it) */
    if(service->configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the mod ASGI section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(service->configuration, (unsigned char *) "mod_asgi", (void **) &configuration);
    if(configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the script path from the ASGI configuration and in
    case it exists resolves it against the contents path when relative */
    get_value_string_sort_map(configuration, (unsigned char *) "script_path", &value);
    if(value != NULL) {
        unsigned char *script_path = (unsigned char *) value;
        if(script_path[0] != '/') {
            struct service_options_t *options = service->options;
            if(script_path[0] == '\\') { script_path++; }
            SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "%s", options->contents_path, script_path);
        } else {
            SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", script_path);
        }
    }

    /* tries to retrieve the name of the application from the ASGI
    configuration, the usual one applies when none is named */
    get_value_string_sort_map(configuration, (unsigned char *) "application", &value);
    if(value != NULL) {
        SPRINTF(mod_asgi_http_handler->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", (char *) value);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_asgi_state(struct service_t *service) {
    /* initializes the python interpreter using the PyConfig API,
    setting the program name from the service configuration */
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    /* converts the program name to wide char and sets it in
    the interpreter configuration structure */
    wchar_t *program_name = Py_DecodeLocale(
        (char *) service->program_name, NULL
    );
    if(program_name != NULL) {
        PyConfig_SetString(&config, &config.program_name, program_name);
        PyMem_RawFree(program_name);
    }

    /* starts the python interpreter initializing all the resources
    related with the virtual machine, this is the main entry point
    for the python interpreter (virtual machine) */
    Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);

    /* retrieves the system path list and then appends (inserts)
    the various relative local paths into it (for relative usage) */
    PyObject *current_path = PyUnicode_FromString("");
    PyObject *path = PySys_GetObject("path");
    PyList_Insert(path, 0, current_path);
    Py_DECREF(current_path);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unload_asgi_state() {
    /* shuts down the python interpreter, releasing all the resources
    associated with it (everything is destroyed), the atexit handlers
    registered by modules will be called automatically by Py_Finalize */
    Py_Finalize();

    /* raises no error */
    RAISE_NO_ERROR;
}
