/*
 Hive Viriatum Modules
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "entry.h"

/* starts the memory structures */
START_MEMORY;

/* initializes the module global variables this
values will be used accross functions */
START_GLOBALS;

ERROR_CODE create_mod_wsgi_module(struct mod_wsgi_module_t **mod_wsgi_module_pointer, struct module_t *module) {
    /* retrieves the mod wsgi module size */
    size_t mod_wsgi_module_size = sizeof(struct mod_wsgi_module_t);

    /* allocates space for the mod wsgi module */
    struct mod_wsgi_module_t *mod_wsgi_module = (struct mod_wsgi_module_t *) MALLOC(mod_wsgi_module_size);

    /* sets the mod wsgi module attributes (default) values */
    mod_wsgi_module->http_handler = NULL;
    mod_wsgi_module->mod_wsgi_http_handler = NULL;

    /* sets the mod wsgi module in the (upper) module substrate */
    module->lower = (void *) mod_wsgi_module;

    /* sets the mod wsgi module in the module pointer */
    *mod_wsgi_module_pointer = mod_wsgi_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_wsgi_module(struct mod_wsgi_module_t *mod_wsgi_module) {
    /* releases the mod wsgi module */
    FREE(mod_wsgi_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE start_module_wsgi(struct environment_t *environment, struct module_t *module) {
    /* allocates the mod wsgi module */
    struct mod_wsgi_module_t *mod_wsgi_module;

    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates the mod wsgi http handler */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_wsgi();
    unsigned char *version = version_viriatum_mod_wsgi();
    unsigned char *description = description_viriatum_mod_wsgi();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* sets the global service reference to be used in the
    externalized function for the interpreter */
    _service = service;

    /* creates the mod wsgi module */
    create_mod_wsgi_module(&mod_wsgi_module, module);

    /* populates the module structure */
    info_module_wsgi(module);

    /* loads the wsgi state populating all the required values
    for state initialization */
    _load_wsgi_state();

    /* creates the http handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "wsgi");

    /* creates the mod wsgi http handler */
    create_mod_wsgi_http_handler(&mod_wsgi_http_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->resolve_index = 0;
    http_handler->set = set_handler_wsgi;
    http_handler->unset = unset_handler_wsgi;
    http_handler->reset = NULL;

    /* sets the mod wsgi module attributes */
    mod_wsgi_module->http_handler = http_handler;
    mod_wsgi_module->mod_wsgi_http_handler = mod_wsgi_http_handler;

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* loads the service configuration for the http handler
    this should change some of it's behavior then loads the
    locations (configurations) associated with the current
    service environment */
    _load_configuration_wsgi(service, mod_wsgi_http_handler);
    _load_locations_wsgi(service, mod_wsgi_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_module_wsgi(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_wsgi();
    unsigned char *version = version_viriatum_mod_wsgi();
    unsigned char *description = description_viriatum_mod_wsgi();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* retrieves the mod wsgi module (from the module) */
    struct mod_wsgi_module_t *mod_wsgi_module = (struct  mod_wsgi_module_t *) module->lower;

    /* retrieves the http handler from the mod wsgi module */
    struct http_handler_t *http_handler = mod_wsgi_module->http_handler;

    /* retrieves the mod wsgi http handler from the mod wsgi module */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler = mod_wsgi_module->mod_wsgi_http_handler;

    /* prints a debug message */
    V_DEBUG_F("Stopping the module '%s' (%s) v%s\n", name, description, version);

    /* removes the http handler from the service */
    service->remove_http_handler(service, http_handler);

    /* in case the mod wsgi http handler is valid and
    initialized (correct state) */
    if(mod_wsgi_http_handler != NULL) {
        /* deletes the mod wsgi http handler */
        delete_mod_wsgi_http_handler(mod_wsgi_http_handler);
    }

    /* in case the http handler is valid and
    initialized (correct state) */
    if(http_handler != NULL) {
        /* deletes the http handler */
        service->delete_http_handler(service, http_handler);
    }

    /* unloads the wsgi state destroying all the required values
    for state destroyed */
    _unload_wsgi_state();

    /* deletes the mod wsgi module */
    delete_mod_wsgi_module(mod_wsgi_module);

    /* sets the global service reference this is no longer necessary
    because the module has been unloaded */
    _service = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE info_module_wsgi(struct module_t *module) {
    /* retrieves the name */
    unsigned char *name = name_viriatum_mod_wsgi();

    /* retrieves the version */
    unsigned char *version = version_viriatum_mod_wsgi();

    /* populates the module structure */
    module->name = name;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = start_module_wsgi;
    module->stop = stop_module_wsgi;
    module->info = info_module_wsgi;
    module->error = error_module_wsgi;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE error_module_wsgi(unsigned char **message_pointer) {
    /* sets the error message in the (error) message pointer */
    *message_pointer = get_last_error_message();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_configuration_wsgi(struct service_t *service, struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* allocates space for both a configuration item reference
    (value) and for the configuration to be retrieved */
    void *value;
    struct sort_map_t *configuration;

    /* in case the current service configuration is not set
    must return immediately (not possible to load it) */
    if(service->configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the mod wsgi section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(service->configuration, (unsigned char *) "mod_wsgi", (void **) &configuration);
    if(configuration == NULL) { RAISE_NO_ERROR; }

    /* tries ro retrieve the script path from the wsgi configuration and in
    case it exists sets it in the mod wsgi handler (attribute reference change) */
    get_value_string_sort_map(configuration, (unsigned char *) "script_path", &value);
    if(value != NULL) { mod_wsgi_http_handler->file_path = (char *) value; }

    /* tries to retrieve the script argument from the arguments map, then
    sets the reload (boolean) value for the service */
    get_value_string_sort_map(configuration, (unsigned char *) "script_reload", &value);
    if(value != NULL) { mod_wsgi_http_handler->reload = (unsigned char) atob(value); }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_locations_wsgi(struct service_t *service, struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* allocates space for the temporary value object and for
    the index counter to be used in the iteration of configurations */
    void *value;
    size_t index;

    /* allocates space for both the location and the configuration
    reference stuctures */
    struct location_t *location;
    struct sort_map_t *configuration;

    /* allocates space for the mod wsgi location structure
    reference to be used to resolve the request */
    struct mod_wsgi_location_t *_location;

    /* allocates space for the various location structures
    that will be used to resolve the wsgi request */
    mod_wsgi_http_handler->locations = (struct mod_wsgi_location_t *)
        MALLOC(service->locations.count * sizeof(struct mod_wsgi_location_t));
    memset(mod_wsgi_http_handler->locations, 0,
        service->locations.count * sizeof(struct mod_wsgi_location_t));

	/* updates the locations count variable in the wsgi handler so
	that it's possible to iterate over the locations */
	mod_wsgi_http_handler->locations_count = service->locations.count;

    /* iterates over all the locations in the service to create the
    proper configuration structures to the module */
    for(index = 0; index < service->locations.count; index++) {
        /* retrieves the current (service) location and then uses it
        to retrieve the configuration sort map */
        location = &service->locations.values[index];
        configuration = location->configuration;

        /* retrieves the current mod wsgi configuration reference from
        the location buffer, this is going ot be populated and sets the
		default values in it */
        _location = &mod_wsgi_http_handler->locations[index];
		_location->reload = UNSET;

        /* tries ro retrieve the script path from the wsgi configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "script_path", &value);
        if(value != NULL) { _location->file_path = (char *) value; }

        /* tries to retrieve the script argument from the arguments map, then
        sets the reload (boolean) value for the location */
        get_value_string_sort_map(configuration, (unsigned char *) "script_reload", &value);
        if(value != NULL) { _location->reload = (unsigned char) atob(value); }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_wsgi_state() {
    /* sets the current program name in the python interpreter
    the name used is the same as the process running the service */
    Py_SetProgramName((char *) _service->program_name);

    /* starts the python interpreter initializing all the resources
    related with the virtual machine, this is the main entry point
    for the python interpreter (virtual machine) */
    Py_Initialize();
#ifdef PYTHON_THREADS
    PyEval_InitThreads();
    _state = PyThreadState_Get();
    PyEval_ReleaseLock();
#endif

    /* starts the wsgi state updating the major global value in
    the current interpreter state */
    _start_wsgi_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unload_wsgi_state() {
    /* allocates space that will hold the reference to the
    function to notify the modules about the end of the
    python interpreter execution */
    PyObject *exit_func;

    /* retrieves the reference to the exit function to be
    uses to notify the caller modules about the finaliztion
    process from the interpreter */
    VIRIATUM_ACQUIRE_GIL;
    exit_func = PySys_GetObject("exitfunc");
    if(exit_func != NULL) { PyEval_CallObject(exit_func, (PyObject *) NULL); }

    /* shutsdown the python interpreter, releasing all the resources
    associated with it (everything is destroyed) */
    Py_Finalize();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reload_wsgi_state() {
    _unload_wsgi_state();
    _load_wsgi_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _start_wsgi_state() {
    /* allocates space for the reference to the to be created
    wsgi module and the type to be exported */
    PyObject *wsgi_module;
    PyTypeObject *type;

    /* retrieves the system path list and then appends (inserts)
    the various relative local paths into it (for relative usage) */
    PyObject *current_path = PyString_FromString("");
    PyObject *path = PySys_GetObject("path");
    PyList_Insert(path, 0, current_path);
    Py_DECREF(current_path);

    /* registers the viriatum wsgi module in the python interpreter
    this module may be used to provide wsgi functions */
    wsgi_module = Py_InitModule("viriatum_wsgi", wsgi_methods);
    PyModule_AddStringConstant(wsgi_module, "NAME", (char *) _service->name);
    PyModule_AddStringConstant(wsgi_module, "VERSION", (char *) _service->version);
    PyModule_AddStringConstant(wsgi_module, "PLATFORM", (char *) _service->platform);
    PyModule_AddStringConstant(wsgi_module, "FLAGS", (char *) _service->flags);
    PyModule_AddStringConstant(wsgi_module, "COMPILER", (char *) _service->compiler);
    PyModule_AddStringConstant(wsgi_module, "COMPILER_VERSION", (char *) _service->compiler_version);
    PyModule_AddStringConstant(wsgi_module, "COMPILATION_DATE", (char *) _service->compilation_date);
    PyModule_AddStringConstant(wsgi_module, "DESCRIPTION", (char *) _service->compiler);

    /* checks the input type for readyness and then casts the
    type as a python type and registers it as input */
    PyType_Ready(&input_type);
    type = &input_type;
    PyModule_AddObject(wsgi_module, "input", (PyObject *) type);

    /* raises no error */
    RAISE_NO_ERROR;
}
