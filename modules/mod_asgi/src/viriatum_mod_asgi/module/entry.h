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

#include "handler.h"

#include "../../../../src/viriatum_python/module/handler_asgi.h"
#include "../../../../src/viriatum_python/module/loop.h"

#define VIRIATUM_ACQUIRE_GIL PyGILState_STATE _gstate = PyGILState_Ensure()
#define VIRIATUM_RELEASE_GIL PyGILState_Release(_gstate)

/**
 * The base path to the directory to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_BASE_PATH "."

/**
 * The waiting of the polling that applies while no task of an
 * application is pending, the blocking one the service uses when
 * nothing of the sort is running beside it.
 */
#define VIRIATUM_ASGI_IDLE_TIMEOUT -1

/**
 * Structure describing the internal
 * structures and information for the
 * mod ASGI module.
 */
typedef struct mod_asgi_module_t {
    /**
     * The HTTP handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod ASGI HTTP handler associated
     * with the module.
     */
    struct mod_asgi_http_handler_t *mod_asgi_http_handler;

    /**
     * The loop that the applications of the module run their
     * tasks on, advanced once per cycle of the loop of the
     * service so that a task no request is driving progresses.
     */
    struct loop_python_t *loop_python;

    /**
     * Flag controlling if the startup of the lifespan has been
     * run, so that the shutdown of it is only ever run against
     * an application that was started.
     */
    char started;

    /**
     * Flag controlling if the application takes the scope apart
     * from the callables, which is the older of the two shapes,
     * decided once out of the application itself.
     */
    char double_callable;
} mod_asgi_module;

/**
 * The service that the module has been started against, kept so
 * that the externalized operations are able to reach it.
 */
VIRIATUM_EXTERNAL_PREFIX struct service_t *_service;

/**
 * The module itself, kept so that the operation run once per cycle
 * of the loop is able to reach the loop of the applications.
 */
VIRIATUM_EXTERNAL_PREFIX struct mod_asgi_module_t *_mod_asgi_module;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_asgi_module(struct mod_asgi_module_t **mod_asgi_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_asgi_module(struct mod_asgi_module_t *mod_asgi_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module_asgi(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module_asgi(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module_asgi(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module_asgi(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE cycle_module_asgi(struct service_t *service);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration_asgi(struct service_t *service, struct mod_asgi_http_handler_t *mod_asgi_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_asgi_state(struct service_t *service);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_asgi_state();
