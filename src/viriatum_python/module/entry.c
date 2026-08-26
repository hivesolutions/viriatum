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

#include "entry.h"
#include "server.h"

static int _exec_module_python(PyObject *module) {
    /* creates the server type and adds it to the module, the
    reference is stolen by the addition operation */
    PyObject *server_type = create_server_type_python();
    if(server_type == NULL) { return -1; }
    if(PyModule_AddObject(module, "Server", server_type) < 0) {
        Py_DECREF(server_type);
        return -1;
    }

    /* adds the various constants describing the underlying server
    to the module, mirroring the ones exposed by mod_wsgi */
    if(PyModule_AddStringConstant(module, "NAME", VIRIATUM_NAME) < 0) { return -1; }
    if(PyModule_AddStringConstant(module, "VERSION", VIRIATUM_VERSION) < 0) { return -1; }
    if(PyModule_AddStringConstant(module, "PLATFORM", VIRIATUM_PLATFORM_STRING) < 0) { return -1; }
    if(PyModule_AddStringConstant(module, "COMPILER", VIRIATUM_COMPILER) < 0) { return -1; }
    if(PyModule_AddStringConstant(module, "COMPILATION_DATE", VIRIATUM_COMPILATION_DATE) < 0) { return -1; }
    if(PyModule_AddStringConstant(module, "COMPILATION_TIME", VIRIATUM_COMPILATION_TIME) < 0) { return -1; }

    /* returns the success value as the execution of the module
    went without any kind of problem */
    return 0;
}

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, (void *) _exec_module_python},
    {0, NULL}
};

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "viriatum._viriatum",
    "Viriatum web server bindings for the python interpreter.",
    0,
    NULL,
    module_slots,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC PyInit__viriatum(void) {
    /* initializes the module using the multi phase initialization
    protocol, the execution happens in the exec slot */
    return PyModuleDef_Init(&module_def);
}
