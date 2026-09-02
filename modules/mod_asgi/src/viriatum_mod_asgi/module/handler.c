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

#include "handler.h"

ERROR_CODE create_mod_asgi_http_handler(struct mod_asgi_http_handler_t **mod_asgi_http_handler_pointer, struct http_handler_t *http_handler_pointer) {
    /* retrieves the mod ASGI HTTP handler size */
    size_t mod_asgi_http_handler_size = sizeof(struct mod_asgi_http_handler_t);

    /* allocates space for the mod ASGI HTTP handler */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler = (struct mod_asgi_http_handler_t *) MALLOC(mod_asgi_http_handler_size);

    /* sets the mod ASGI HTTP handler attributes (default) values, the
    path of the file is the one that stands in for a configuration that
    named none and the name of the application the usual one */
    SPRINTF(mod_asgi_http_handler->file_path, VIRIATUM_MAX_PATH_SIZE, "%s", DEFAULT_FILE_PATH);
    SPRINTF(mod_asgi_http_handler->application_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", DEFAULT_APPLICATION_NAME);
    SPRINTF(mod_asgi_http_handler->module_name, VIRIATUM_ASGI_MODULE_SIZE, "%s", "asgi_app");
    mod_asgi_http_handler->module = NULL;
    mod_asgi_http_handler->application = NULL;

    /* sets the mod ASGI HTTP handler in the HTTP handler substrate */
    if(http_handler_pointer != NULL) { http_handler_pointer->lower = (void *) mod_asgi_http_handler; }

    /* sets the mod ASGI HTTP handler in the pointer */
    *mod_asgi_http_handler_pointer = mod_asgi_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_asgi_http_handler(struct mod_asgi_http_handler_t *mod_asgi_http_handler) {
    /* releases the references that are held on the interpreter before
    the structure itself goes, one that outlived it would be released
    against an interpreter that is no longer there */
    unload_application_asgi(mod_asgi_http_handler);

    /* releases the mod ASGI HTTP handler */
    FREE(mod_asgi_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE load_application_asgi(struct mod_asgi_http_handler_t *mod_asgi_http_handler) {
    /* allocates space for the references to the module that is loaded
    and to the application that is taken out of it */
    PyObject *module;
    PyObject *application;

    /* releases whatever was loaded before, so that a second call
    replaces the application rather than leaking the first one */
    unload_application_asgi(mod_asgi_http_handler);

    /* loads the module out of the file that the configuration named,
    a file that is not there leaves the module unset and the loading
    is reported as a failure to the caller */
    _load_module_asgi(
        &module,
        mod_asgi_http_handler->module_name,
        mod_asgi_http_handler->file_path
    );
    if(module == NULL) {
        RAISE_ERROR_M(
            D_ERROR_CODE,
            (unsigned char *) "Problem loading the module of the application"
        );
    }

    /* takes the application out of the module, an attribute that is
    missing or that may not be called is not an application */
    application = PyObject_GetAttrString(module, mod_asgi_http_handler->application_name);
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
    mod_asgi_http_handler->module = module;
    mod_asgi_http_handler->application = application;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unload_application_asgi(struct mod_asgi_http_handler_t *mod_asgi_http_handler) {
    /* releases the reference of the application and of the module it
    came out of, in that order, and unsets both of them so that the
    releasing of them twice is never possible */
    Py_XDECREF(mod_asgi_http_handler->application);
    Py_XDECREF(mod_asgi_http_handler->module);
    mod_asgi_http_handler->application = NULL;
    mod_asgi_http_handler->module = NULL;

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

ERROR_CODE _double_callable_asgi(PyObject *application, char *double_callable_pointer) {
    /* allocates space for the reference to the attribute that says
    how many arguments the application takes */
    PyObject *code;
    PyObject *count;

    /* the shape of an application is the more recent one unless it
    proves otherwise, which is the one that takes the three values
    of the scope and of the two callables in a single call */
    *double_callable_pointer = FALSE;

    /* an application that is a class is called to build the instance
    that is then called with the callables, which is the older of the
    two shapes and the one that takes the scope apart */
    if(PyType_Check(application)) {
        *double_callable_pointer = TRUE;
        RAISE_NO_ERROR;
    }

    /* reads the number of arguments out of the code of the callable,
    anything that carries none of it is left as the more recent shape
    rather than being guessed at */
    code = PyObject_GetAttrString(application, "__code__");
    if(code == NULL) {
        PyErr_Clear();
        RAISE_NO_ERROR;
    }
    count = PyObject_GetAttrString(code, "co_argcount");
    Py_DECREF(code);
    if(count == NULL) {
        PyErr_Clear();
        RAISE_NO_ERROR;
    }

    /* an application taking a single argument is the older of the
    two shapes, the scope reaching it apart from the callables */
    if(PyLong_AsLong(count) == 1) { *double_callable_pointer = TRUE; }
    Py_DECREF(count);

    /* raises no error */
    RAISE_NO_ERROR;
}
