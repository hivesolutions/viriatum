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

#pragma once

#include "../stdafx.h"

/**
 * The maximum size of the name of the module
 * to be used in the ASGI module.
 */
#define VIRIATUM_ASGI_MODULE_SIZE 128

/**
 * The name of the attribute that is taken out of the loaded
 * module as the application, in case the configuration does
 * not name another one.
 */
#define DEFAULT_APPLICATION_NAME "application"

/**
 * The path to the file to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_FILE_PATH "default.asgi"

/**
 * The structure that holds the internal
 * structure to support the context
 * of the ASGI module.
 */
typedef struct mod_asgi_http_handler_t {
    /**
     * The path to the default file to
     * be used for the parsing.
     */
    char file_path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The name of the attribute that is taken out of the
     * loaded module as the application to be served.
     */
    char application_name[VIRIATUM_ASGI_MODULE_SIZE];

    /**
     * The buffer used to store the "main" module name
     * so that it may be referred in the python interpreter.
     */
    char module_name[VIRIATUM_ASGI_MODULE_SIZE];

    /**
     * The reference to the module object representing
     * the parsed and compiled file to be used.
     */
    PyObject *module;

    /**
     * The reference to the application that is served, taken
     * out of the module once it has been loaded.
     */
    PyObject *application;
} mod_asgi_http_handler;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_asgi_http_handler(struct mod_asgi_http_handler_t **mod_asgi_http_handler_pointer, struct http_handler_t *http_handler_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_asgi_http_handler(struct mod_asgi_http_handler_t *mod_asgi_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE load_application_asgi(struct mod_asgi_http_handler_t *mod_asgi_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE unload_application_asgi(struct mod_asgi_http_handler_t *mod_asgi_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_module_asgi(PyObject **module_pointer, char *name, char *file_path);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _double_callable_asgi(PyObject *application, char *double_callable_pointer);
