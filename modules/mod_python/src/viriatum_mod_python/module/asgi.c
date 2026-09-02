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

#include "asgi.h"

#ifdef VIRIATUM_ASGI

char _is_absolute_asgi(unsigned char *file_path) {
    /* a path that opens with a separator is absolute everywhere, and
    one that opens with a drive letter and a separator is absolute on
    the platform that carries them, a configuration naming either is
    never resolved against the contents of the service */
    if(file_path[0] == '/' || file_path[0] == '\\') { return TRUE; }
    if(file_path[0] == '\0' || file_path[1] != ':') { return FALSE; }
    return file_path[2] == '/' || file_path[2] == '\\' ? TRUE : FALSE;
}

ERROR_CODE create_mod_python_asgi(struct mod_python_asgi_t **mod_python_asgi_pointer) {
    /* retrieves the mod ASGI HTTP handler size */
    size_t mod_python_asgi_size = sizeof(struct mod_python_asgi_t);

    /* allocates space for the mod ASGI HTTP handler */
    struct mod_python_asgi_t *mod_python_asgi = (struct mod_python_asgi_t *) MALLOC(mod_python_asgi_size);

    /* sets the state attributes (default) values, the path of the
    file is left empty as a configuration that names none leaves the
    interface unserved, and the name of the application the usual one */
    mod_python_asgi->file_path[0] = '\0';
    SPRINTF(mod_python_asgi->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", DEFAULT_APPLICATION_NAME);
    SPRINTF(mod_python_asgi->module_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", "asgi_app");
    mod_python_asgi->module = NULL;
    mod_python_asgi->application = NULL;

    /* sets the mod ASGI HTTP handler in the pointer */
    *mod_python_asgi_pointer = mod_python_asgi;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_python_asgi(struct mod_python_asgi_t *mod_python_asgi) {
    /* releases the references that are held on the interpreter before
    the structure itself goes, one that outlived it would be released
    against an interpreter that is no longer there */
    unload_application_asgi(mod_python_asgi);

    /* releases the mod ASGI HTTP handler */
    FREE(mod_python_asgi);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE load_application_asgi(struct mod_python_asgi_t *mod_python_asgi) {
    /* allocates space for the references to the module that is loaded
    and to the application that is taken out of it */
    PyObject *module;
    PyObject *application;

    /* releases whatever was loaded before, so that a second call
    replaces the application rather than leaking the first one */
    unload_application_asgi(mod_python_asgi);

    /* loads the module out of the file that the configuration named,
    a file that is not there leaves the module unset and the loading
    is reported as a failure to the caller */
    _load_module_asgi(
        &module,
        mod_python_asgi->module_name,
        mod_python_asgi->file_path
    );
    if(module == NULL) {
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "Problem loading the module of the application"
        );
    }

    /* takes the application out of the module, an attribute that is
    missing or that may not be called is not an application */
    application = PyObject_GetAttrString(module, mod_python_asgi->application_name);
    if(application == NULL || !PyCallable_Check(application)) {
        Py_XDECREF(application);
        Py_DECREF(module);
        PyErr_Clear();
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "Problem retrieving the application from the module"
        );
    }

    /* sets both references in the handler, the module is held as well
    so that the application is never left without the module it came
    out of, which is what keeps the globals of it alive */
    mod_python_asgi->module = module;
    mod_python_asgi->application = application;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unload_application_asgi(struct mod_python_asgi_t *mod_python_asgi) {
    /* releases the reference of the application and of the module it
    came out of, in that order, and unsets both of them so that the
    releasing of them twice is never possible */
    Py_XDECREF(mod_python_asgi->application);
    Py_XDECREF(mod_python_asgi->module);
    mod_python_asgi->application = NULL;
    mod_python_asgi->module = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_module_asgi(PyObject **module_pointer, char *name, char *file_path) {
    /* allocates space for the pointer to the file object to be
    used for reading the module file */
    FILE *file;

    /* allocates space for the code and module python objects */
    PyObject *code;
    PyObject *module;

    /* allocates space for the number of bytes for the file size
    and for the buffer that will hold the file */
    size_t number_bytes;
    size_t file_size;
    char *file_buffer;

    /* resets the provided module pointer to the "default" invalid
    value to provide error detection */
    *module_pointer = NULL;

    /* prints a debug message to notify the system about the loading
    of the ASGI module (provides logging) */
    V_DEBUG_CTX_F("mod_asgi", "Loading ASGI module '%s'\n", file_path);

    /* opens the file for reading (in binary mode) and checks if
    there was a problem opening it, raising an error in such case */
    FOPEN(&file, file_path, "rb");
    if(file == NULL) {
        V_DEBUG_CTX_F("mod_asgi", "Module file not found '%s'\n", file_path);
        RAISE_NO_ERROR;
    }

    /* seeks the file until the end of the file and then
    retrieves the current position as the size at the end
    restores the file position back the beginning */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocates space for the file buffer that will contain the
    complete python file, this should be a null terminated string */
    file_buffer = (char *) MALLOC(file_size + 1);
    number_bytes = fread(file_buffer, 1, file_size, file);
    file_buffer[number_bytes] = '\0';

    /* closes the file to avoid any file memory leaking */
    fclose(file);

    /* compiles the contents of the file into a code object and then
    releases the buffer that held them, a file that does not compile
    leaves the module unset and the error of the interpreter cleared */
    code = Py_CompileString(file_buffer, file_path, Py_file_input);
    FREE(file_buffer);
    if(code == NULL) {
        PyErr_Clear();
        V_DEBUG_CTX_F("mod_asgi", "Module file did not compile '%s'\n", file_path);
        RAISE_NO_ERROR;
    }

    /* executes the code object as a module of the provided name and
    releases the code, which the module holds whatever it needs of */
    module = PyImport_ExecCodeModule(name, code);
    Py_DECREF(code);
    if(module == NULL) {
        PyErr_Clear();
        V_DEBUG_CTX_F("mod_asgi", "Module file did not run '%s'\n", file_path);
        RAISE_NO_ERROR;
    }

    /* sets the loaded module in the pointer that was provided */
    *module_pointer = module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_configuration_asgi(struct service_t *service, struct mod_python_asgi_t *mod_python_asgi) {
    /* allocates space for both a configuration item reference
    (value) and for the configuration to be retrieved */
    void *value;
    struct sort_map_t *configuration;

    /* in case the current service configuration is not set
    must return immediately (not possible to load it) */
    if(service->configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the mod ASGI section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(service->configuration, (unsigned char *) "mod_python", (void **) &configuration);
    if(configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the script path from the ASGI configuration and in
    case it exists resolves it against the contents path when relative */
    get_value_string_sort_map(configuration, (unsigned char *) "asgi_script_path", &value);
    if(value != NULL) {
        unsigned char *script_path = (unsigned char *) value;
        if(!_is_absolute_asgi(script_path)) {
            struct service_options_t *options = service->options;
            if(script_path[0] == '\\') { script_path++; }
            SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "%s", options->contents_path, script_path);
        } else {
            SPRINTF(mod_python_asgi->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", script_path);
        }
    }

    /* tries to retrieve the name of the application from the ASGI
    configuration, the usual one applies when none is named */
    get_value_string_sort_map(configuration, (unsigned char *) "asgi_application", &value);
    if(value != NULL) {
        SPRINTF(mod_python_asgi->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", (char *) value);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _path_asgi_state(char *file_path) {
    /* allocates space for the list of the paths of the imports, for
    the directory the application sits in and for the buffer that it
    is built into out of the path of the file */
    PyObject *path;
    PyObject *value;
    char base_path[VIRIATUM_MAX_PATH_SIZE];
    char *separator;

    /* retrieves the system path list and then inserts the working
    directory into it, which is what a relative import resolves
    against and what the interpreter of a script would carry */
    path = PySys_GetObject("path");
    if(path == NULL) { RAISE_NO_ERROR; }
    value = PyUnicode_FromString(VIRIATUM_ASGI_BASE_PATH);
    PyList_Insert(path, 0, value);
    Py_DECREF(value);

    /* takes the directory out of the path of the application, a file
    that carries no separator at all sits in the working directory
    and so is already covered by the insertion above */
    SPRINTF(base_path, VIRIATUM_MAX_PATH_SIZE, "%s", file_path);
    separator = strrchr(base_path, '/');
#ifdef VIRIATUM_PLATFORM_WINDOWS
    if(separator == NULL) { separator = strrchr(base_path, '\\'); }
#endif
    if(separator == NULL) { RAISE_NO_ERROR; }
    *separator = '\0';

    /* and puts it ahead of everything else, so that a module sitting
    beside the application is the one that is found for it */
    value = PyUnicode_FromString(base_path);
    PyList_Insert(path, 0, value);
    Py_DECREF(value);

    /* raises no error */
    RAISE_NO_ERROR;
}

#endif
