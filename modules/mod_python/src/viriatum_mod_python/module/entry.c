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
struct connection_t *_connection;
struct http_headers_t _headers;
struct wsgi_request_t _wsgi_request;
#ifdef VIRIATUM_ASGI
struct mod_python_module_t *_mod_python_module;
#endif

ERROR_CODE create_mod_python_module(struct mod_python_module_t **mod_python_module_pointer, struct module_t *module) {
    /* retrieves the mod python module size */
    size_t mod_python_module_size = sizeof(struct mod_python_module_t);

    /* allocates space for the mod python module */
    struct mod_python_module_t *mod_python_module = (struct mod_python_module_t *) MALLOC(mod_python_module_size);

    /* sets the mod python module attributes (default) values */
    mod_python_module->http_handler = NULL;
    mod_python_module->mod_python_http_handler = NULL;
#ifdef VIRIATUM_ASGI
    mod_python_module->mod_python_asgi = NULL;
    mod_python_module->loop_python = NULL;
    mod_python_module->started = FALSE;
#endif

    /* sets the mod python module in the (upper) module substrate */
    module->lower = (void *) mod_python_module;

    /* sets the mod python module in the module pointer */
    *mod_python_module_pointer = mod_python_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_python_module(struct mod_python_module_t *mod_python_module) {
    /* releases the mod python module */
    FREE(mod_python_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE start_module_python(struct environment_t *environment, struct module_t *module) {
    /* allocates the mod python module */
    struct mod_python_module_t *mod_python_module;

    /* allocates the HTTP handler */
    struct http_handler_t *http_handler;

    /* allocates the mod python HTTP handler */
    struct mod_python_http_handler_t *mod_python_http_handler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_python();
    unsigned char *version = version_viriatum_mod_python();
    unsigned char *description = description_viriatum_mod_python();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_CTX_F("mod_python", "Starting the module '%s' (%s) v%s\n", name, description, version);

    /* sets the global service reference to be used in the
    externalized function for the interpreter */
    _service = service;

    /* creates the mod python module */
    create_mod_python_module(&mod_python_module, module);

#ifdef VIRIATUM_ASGI
    /* sets the global module reference so that the operation run
    once per cycle is able to reach the loop of the applications */
    _mod_python_module = mod_python_module;
#endif

    /* populates the module structure */
    info_module_python(module);

    /* loads the python state populating all the required values
    for state initialization, a failure of it is a failure of the
    starting as nothing may be built upon an interpreter that is
    not running */
    if(IS_ERROR_CODE(_load_python_state())) {
        V_DEBUG_CTX("mod_python", "Problem starting the interpreter\n");
        RAISE_AGAIN(D_ERROR_CODE);
    }

    /* creates the HTTP handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "wsgi");

    /* creates the mod python HTTP handler */
    create_mod_python_http_handler(&mod_python_http_handler, http_handler);

    /* sets the HTTP handler attributes */
    http_handler->resolve_index = 0;
    http_handler->set = set_handler_wsgi;
    http_handler->unset = unset_handler_wsgi;
    http_handler->reset = NULL;

    /* sets the mod python module attributes */
    mod_python_module->http_handler = http_handler;
    mod_python_module->mod_python_http_handler = mod_python_http_handler;

    /* adds the HTTP handler to the service */
    service->add_http_handler(service, http_handler);

    /* loads the service configuration for the HTTP handler
    this should change some of it's behavior then loads the
    locations (configurations) associated with the current
    service environment */
    _load_configuration_wsgi(service, mod_python_http_handler);
    _load_locations_wsgi(service, mod_python_http_handler);

#ifdef VIRIATUM_ASGI
    /* starts the serving of the more recent of the two interfaces,
    which only happens when an application has been named for it, the
    module being usable for either of them on its own, a failure of
    it is a failure of the starting of the module itself */
    if(IS_ERROR_CODE(_start_asgi_module(service, mod_python_module))) {
        RAISE_AGAIN(D_ERROR_CODE);
    }
#endif

    /* raises no error */
    RAISE_NO_ERROR;
}

#ifdef VIRIATUM_ASGI

ERROR_CODE _start_asgi_module(struct service_t *service, struct mod_python_module_t *mod_python_module) {
    /* allocates the state of the more recent interface, which carries
    the application that is served through it */
    struct mod_python_asgi_t *mod_python_asgi;

    /* creates the state and reads the configuration into it, a
    configuration that names no application at all leaves the
    interface unserved and the module carrying the older one alone */
    create_mod_python_asgi(&mod_python_asgi);
    _load_configuration_asgi(service, mod_python_asgi);
    if(mod_python_asgi->file_path[0] == '\0') {
        delete_mod_python_asgi(mod_python_asgi);
        RAISE_NO_ERROR;
    }
    mod_python_module->mod_python_asgi = mod_python_asgi;

    /* puts the directory the application sits in on the path of the
    imports, so that a script reading the modules beside it finds
    them, which the working directory alone never provides */
    _path_asgi_state(mod_python_asgi->file_path);

    /* loads the application out of the file that the configuration
    named, a failure of it is a failure of the starting, nothing has
    been handed to the service at this point and so nothing serves */
    if(IS_ERROR_CODE(load_application_asgi(mod_python_asgi))) {
        V_DEBUG_CTX_F("mod_python", "No application was loaded from '%s'\n", mod_python_asgi->file_path);
        _stop_asgi_module(service, mod_python_module);
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "No application was loaded for the asgi handler"
        );
    }

    /* builds the loop that the tasks of the application run on, the
    handler of the interface takes it and advances it while a request
    is being served, the cycle of the service advances the rest */
    if(IS_ERROR_CODE(create_loop_python(&mod_python_module->loop_python))) {
        V_DEBUG_CTX("mod_python", "Problem creating the loop of the applications\n");
        _stop_asgi_module(service, mod_python_module);
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "Problem creating the loop of the applications"
        );
    }

    /* attaches the loop as the running one of the interpreter so that
    the accessors of the asynchronous module resolve to it inside the
    code of the application */
    attach_loop_python(mod_python_module->loop_python);

    /* hands the serving of the interface to the handler that the tree
    already implements, taking a reference on the application, and
    detects the shape of the callable rather than requiring it */
    register_handler_asgi(
        service,
        mod_python_asgi->application,
        mod_python_module->loop_python,
        double_callable_handler_asgi(mod_python_asgi->application)
    );

    /* runs the startup event of the lifespan, an application that
    refuses to boot is taken back out of the service rather than left
    serving requests against something that never started */
    if(IS_ERROR_CODE(startup_handler_asgi(service))) {
        V_DEBUG_CTX("mod_python", "The application refused to start\n");
        unregister_handler_asgi(service);
        _stop_asgi_module(service, mod_python_module);
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "The application refused to start"
        );
    }
    mod_python_module->started = TRUE;

    /* takes the operation run once per cycle of the loop of the
    service, which is what advances a task that no request is
    driving, the writing of a websocket among them */
    service->on_cycle = cycle_module_python;

    /* shortens the waiting of the first of the polls, which starts
    out blocking, so that a task the starting of the application left
    pending is advanced by the first cycle rather than waiting for a
    request to arrive and wake the loop up */
    service->polling->timeout = VIRIATUM_ASGI_POLL_TIMEOUT;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _stop_asgi_module(struct service_t *service, struct mod_python_module_t *mod_python_module) {
    /* nothing at all was started for the more recent interface when
    no application was named for it in the configuration */
    if(mod_python_module->mod_python_asgi == NULL) { RAISE_NO_ERROR; }

    /* the lock of the interpreter is taken for the whole of the
    taking down, every part of it reaching python and the loop of the
    service holding none of it */
    VIRIATUM_ACQUIRE_GIL;

    /* gives the operation run once per cycle back, the loop it
    advances is about to go */
    service->on_cycle = NULL;

    /* runs the shutdown event of the lifespan, only ever against an
    application whose startup was run in the first place */
    if(mod_python_module->started == TRUE) {
        shutdown_handler_asgi(service);
        mod_python_module->started = FALSE;
    }

    /* hands the serving back, which releases the reference that the
    handler took on the application */
    if(mod_python_module->loop_python != NULL) {
        unregister_handler_asgi(service);
    }

    /* releases the state of the interface and the loop of it, in that
    order, before the interpreter that owns them goes */
    delete_mod_python_asgi(mod_python_module->mod_python_asgi);
    mod_python_module->mod_python_asgi = NULL;
    if(mod_python_module->loop_python != NULL) {
        delete_loop_python(mod_python_module->loop_python);
        mod_python_module->loop_python = NULL;
    }

    VIRIATUM_RELEASE_GIL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE cycle_module_python(struct service_t *service) {
    /* in case there is no module or no loop for it there is nothing
    to be advanced, which is the state a module serving the older of
    the two interfaces alone is left in */
    if(_mod_python_module == NULL) { RAISE_NO_ERROR; }
    if(_mod_python_module->loop_python == NULL) { RAISE_NO_ERROR; }

    /* advances the loop of the applications by a single iteration,
    which is what makes a task that no request is driving progress,
    then shortens the waiting of the polling while any of them is
    pending so that the timers they schedule keep a usable resolution,
    the lock of the interpreter is taken for the whole of it as the
    loop of the service reaches this holding none of it */
    VIRIATUM_ACQUIRE_GIL;
    run_once_loop_python(_mod_python_module->loop_python);
    service->polling->timeout =
        pending_loop_python(_mod_python_module->loop_python) > 0 ? VIRIATUM_ASGI_POLL_TIMEOUT : VIRIATUM_ASGI_IDLE_TIMEOUT;
    VIRIATUM_RELEASE_GIL;

    /* raises no error */
    RAISE_NO_ERROR;
}

#endif

ERROR_CODE stop_module_python(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_python();
    unsigned char *version = version_viriatum_mod_python();
    unsigned char *description = description_viriatum_mod_python();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* retrieves the mod python module (from the module) */
    struct mod_python_module_t *mod_python_module = (struct mod_python_module_t *) module->lower;

    /* retrieves the HTTP handler from the mod python module */
    struct http_handler_t *http_handler = mod_python_module->http_handler;

    /* retrieves the mod python HTTP handler from the mod python module */
    struct mod_python_http_handler_t *mod_python_http_handler = mod_python_module->mod_python_http_handler;

    /* prints a debug message */
    V_DEBUG_CTX_F("mod_python", "Stopping the module '%s' (%s) v%s\n", name, description, version);

#ifdef VIRIATUM_ASGI
    /* stops the serving of the more recent of the two interfaces,
    which is only ever running when one was named for it */
    _stop_asgi_module(service, mod_python_module);
#endif

    /* removes the HTTP handler from the service */
    service->remove_http_handler(service, http_handler);

    /* in case the mod python HTTP handler is valid and
    initialized (correct state) */
    if(mod_python_http_handler != NULL) {
        /* deletes the mod python HTTP handler */
        delete_mod_python_http_handler(mod_python_http_handler);
    }

    /* in case the HTTP handler is valid and
    initialized (correct state) */
    if(http_handler != NULL) {
        /* deletes the HTTP handler */
        service->delete_http_handler(service, http_handler);
    }

    /* unloads the WSGI state destroying all the required values
    for state destroyed */
    _unload_python_state();

    /* deletes the mod python module */
    delete_mod_python_module(mod_python_module);

    /* sets the global references this is no longer necessary
    because the module has been unloaded */
    _service = NULL;
#ifdef VIRIATUM_ASGI
    _mod_python_module = NULL;
#endif

    /* cleans up the pool based memory allocation system releasing all
    of its memory before the exit (no leaks) then returns the control
    flow to the caller function with success state */
    cleanup_palloc();
    RAISE_NO_ERROR;
}

ERROR_CODE info_module_python(struct module_t *module) {
    /* retrieves the various elements that are going
    to be used in the construction of the of the module */
    unsigned char *name = name_viriatum_mod_python();
    unsigned char *name_s = name_s_viriatum_mod_python();
    unsigned char *version = version_viriatum_mod_python();

    /* populates the module structure */
    module->name = name;
    module->name_s = name_s;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = start_module_python;
    module->stop = stop_module_python;
    module->info = info_module_python;
    module->error = error_module_python;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE error_module_python(unsigned char **message_pointer) {
    /* sets the error message in the (error) message pointer */
    *message_pointer = get_last_error_message();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_configuration_wsgi(struct service_t *service, struct mod_python_http_handler_t *mod_python_http_handler) {
    /* allocates space for both a configuration item reference
    (value) and for the configuration to be retrieved */
    void *value;
    struct sort_map_t *configuration;

    /* in case the current service configuration is not set
    must return immediately (not possible to load it) */
    if(service->configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the mod python section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(service->configuration, (unsigned char *) "mod_python", (void **) &configuration);
    if(configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the script path from the WSGI configuration and in
    case it exists resolves it against the contents path when relative */
    get_value_string_sort_map(configuration, (unsigned char *) "script_path", &value);
    if(value != NULL) {
        unsigned char *script_path = (unsigned char *) value;
        if(script_path[0] != '/') {
            struct service_options_t *options = service->options;
            if(script_path[0] == '\\') { script_path++; }
            SPRINTF(mod_python_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "%s", options->contents_path, script_path);
        } else {
            SPRINTF(mod_python_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", script_path);
        }
    }

    /* tries to retrieve the script argument from the arguments map, then
    sets the reload (boolean) value for the service */
    get_value_string_sort_map(configuration, (unsigned char *) "script_reload", &value);
    if(value != NULL) { mod_python_http_handler->reload = (unsigned char) atob(value); }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_locations_wsgi(struct service_t *service, struct mod_python_http_handler_t *mod_python_http_handler) {
    /* allocates space for the temporary value object and for
    the index counter to be used in the iteration of configurations */
    void *value;
    size_t index;

    /* allocates space for both the location and the configuration
    reference stuctures */
    struct location_t *location;
    struct sort_map_t *configuration;

    /* allocates space for the mod python location structure
    reference to be used to resolve the request */
    struct mod_python_location_t *_location;

    /* allocates space for the various location structures
    that will be used to resolve the WSGI request */
    mod_python_http_handler->locations = (struct mod_python_location_t *)
        MALLOC(service->locations.count * sizeof(struct mod_python_location_t));
    memset(mod_python_http_handler->locations, 0, service->locations.count * sizeof(struct mod_python_location_t));

    /* updates the locations count variable in the WSGI handler so
    that it's possible to iterate over the locations */
    mod_python_http_handler->locations_count = service->locations.count;

    /* iterates over all the locations in the service to create the
    proper configuration structures to the module */
    for(index = 0; index < service->locations.count; index++) {
        /* retrieves the current (service) location and then uses it
        to retrieve the configuration sort map */
        location = &service->locations.values[index];
        configuration = location->configuration;

        /* retrieves the current mod python configuration reference from
        the location buffer, this is going to be populated and sets the
        default values in it */
        _location = &mod_python_http_handler->locations[index];
        _location->file_path[0] = '\0';
        _location->reload = UNSET;

        /* tries to retrieve the script path from the WSGI configuration and in
        case it exists resolves it against the contents path when relative */
        get_value_string_sort_map(configuration, (unsigned char *) "script_path", &value);
        if(value != NULL) {
            unsigned char *script_path = (unsigned char *) value;
            if(script_path[0] != '/') {
                struct service_options_t *options = service->options;
                if(script_path[0] == '\\') { script_path++; }
                SPRINTF(_location->file_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "%s", options->contents_path, script_path);
            } else {
                SPRINTF(_location->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", script_path);
            }
        }

        /* tries to retrieve the script argument from the arguments map, then
        sets the reload (boolean) value for the location */
        get_value_string_sort_map(configuration, (unsigned char *) "script_reload", &value);
        if(value != NULL) { _location->reload = (unsigned char) atob(value); }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_python_state() {
    /* allocates space for the configuration of the interpreter and
    for the state that the starting of it reports */
    PyConfig config;
    PyStatus status;
    wchar_t *program_name;

    /* initializes the python interpreter using the PyConfig API,
    setting the program name from the service configuration */
    PyConfig_InitPythonConfig(&config);

    /* converts the program name to wide char and sets it in
    the interpreter configuration structure */
    program_name = Py_DecodeLocale((char *) _service->program_name, NULL);
    if(program_name != NULL) {
        PyConfig_SetString(&config, &config.program_name, program_name);
        PyMem_RawFree(program_name);
    }

    /* starts the python interpreter initializing all the resources
    related with the virtual machine, this is the main entry point
    for the python interpreter (virtual machine), a failure of it
    leaves nothing running and must never be built upon */
    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);
    if(PyStatus_Exception(status)) {
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "Problem starting the python interpreter"
        );
    }

    /* starts the WSGI state updating the major global value in
    the current interpreter state */
    _start_python_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unload_python_state() {
    /* shuts down the python interpreter, releasing all the resources
    associated with it (everything is destroyed), the atexit handlers
    registered by modules will be called automatically by Py_Finalize */
    Py_Finalize();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reload_python_state() {
    _unload_python_state();
    _load_python_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

static PyObject *_init_wsgi_module(void) {
    static struct PyModuleDef wsgi_module_def = {
        PyModuleDef_HEAD_INIT,
        "viriatum_wsgi",
        NULL,
        -1,
        wsgi_methods
    };
    return PyModule_Create(&wsgi_module_def);
}

ERROR_CODE _start_python_state() {
    /* allocates space for the reference to the to be created
    python module and the type to be exported */
    PyObject *wsgi_module;
    PyTypeObject *type;

    /* retrieves the system path list and then appends (inserts)
    the various relative local paths into it (for relative usage) */
    PyObject *current_path = PyUnicode_FromString("");
    PyObject *path = PySys_GetObject("path");
    PyList_Insert(path, 0, current_path);
    Py_DECREF(current_path);

    /* registers the viriatum python module in the python interpreter
    this module may be used to provide WSGI functions */
    wsgi_module = _init_wsgi_module();
    if(wsgi_module == NULL) { RAISE_NO_ERROR; }

    /* adds the module to the interpreter so it can be imported */
    PyObject *modules = PyImport_GetModuleDict();
    PyDict_SetItemString(modules, "viriatum_wsgi", wsgi_module);

    PyModule_AddStringConstant(wsgi_module, "NAME", (char *) _service->name);
    PyModule_AddStringConstant(wsgi_module, "VERSION", (char *) _service->version);
    PyModule_AddStringConstant(wsgi_module, "PLATFORM", (char *) _service->platform);
    PyModule_AddStringConstant(wsgi_module, "FLAGS", (char *) _service->flags);
    PyModule_AddStringConstant(wsgi_module, "DESCRIPTION", (char *) _service->description);
    PyModule_AddStringConstant(wsgi_module, "COMPILER", (char *) _service->compiler);
    PyModule_AddStringConstant(wsgi_module, "COMPILER_VERSION", (char *) _service->compiler_version);
    PyModule_AddStringConstant(wsgi_module, "COMPILATION_DATE", (char *) _service->compilation_date);
    PyModule_AddStringConstant(wsgi_module, "COMPILATION_TIME", (char *) _service->compilation_time);
    PyModule_AddStringConstant(wsgi_module, "COMPILATION_FLAGS", (char *) _service->compilation_flags);
    PyModule_AddStringConstant(wsgi_module, "OBSERVATIONS", VIRIATUM_OBSERVATIONS);
    PyModule_AddStringConstant(wsgi_module, "COPYRIGHT", VIRIATUM_COPYRIGHT);
    PyModule_AddStringConstant(wsgi_module, "PLATFORM_CPU", VIRIATUM_PLATFORM_CPU);

    /* checks the input type for readiness and then casts the
    type as a python type and registers it as input */
    PyType_Ready(&input_type);
    type = &input_type;
    Py_INCREF(type);
    PyModule_AddObject(wsgi_module, "input", (PyObject *) type);

    /* releases the local reference to the module, the interpreter
    dictionary still holds a reference keeping it alive */
    Py_DECREF(wsgi_module);

    /* raises no error */
    RAISE_NO_ERROR;
}
