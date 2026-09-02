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

#include "asgi.h"
#include "handler.h"
#include "extension.h"

#ifdef VIRIATUM_ASGI
#include "../../../../src/viriatum_python/module/handler_asgi.h"
#include "../../../../src/viriatum_python/module/loop.h"
#endif

#define VIRIATUM_ACQUIRE_GIL PyGILState_STATE _gstate = PyGILState_Ensure()
#define VIRIATUM_RELEASE_GIL PyGILState_Release(_gstate)

/**
 * The path to the file to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_FILE_PATH "default.wsgi"

/**
 * The bae path to the directory to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_BASE_PATH "."

/**
 * Starts (initializes) the global variables defined
 * as external in the header definition.
 */
#define START_GLOBALS                 \
    struct service_t *_service;       \
    struct connection_t *_connection; \
    struct http_headers_t _headers;   \
    struct wsgi_request_t _wsgi_request;

/**
 * Structure describing the internal
 * structures and information for the
 * mod python module.
 */
typedef struct mod_python_module_t {
    /**
     * The HTTP handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod python HTTP handler associated
     * with the module.
     */
    struct mod_python_http_handler_t *mod_python_http_handler;

#ifdef VIRIATUM_ASGI
    /**
     * The state of the more recent of the two interfaces, which
     * carries the application that is served through it.
     */
    struct mod_python_asgi_t *mod_python_asgi;

    /**
     * The loop that the tasks of an application of the more recent
     * interface run on, advanced once per cycle of the loop of the
     * service so that a task no request is driving progresses.
     */
    struct loop_python_t *loop_python;

    /**
     * Flag controlling if the startup of the lifespan has been
     * run, so that the shutdown of it is only ever run against
     * an application that was started.
     */
    char started;
#endif
} mod_python_module;

/**
 * Structure representing a WSGI request
 * for rendering of a page.
 * This structure contains a series of
 * values useful for request flush.
 */
typedef struct wsgi_request_t {
    /**
     * The integer code describing the status
     * of the response assocaited with this
     * request, this information is provided
     * by the WSGI application.
     */
    int status_code;

    /**
     * The message string describing the status
     * of the response assocaited with this
     * request, this information is provided
     * by the WSGI application.
     */
    char status_message[256];

    /**
     * The matrix buffer containing the maximum
     * possible count for various headers for the
     * WSGI interpreter.
     */
    char headers[VIRIATUM_MAX_HEADER_COUNT][VIRIATUM_MAX_HEADER_C_SIZE];

    /**
     * The number of headers currently present in
     * the current WSGI request.
     */
    size_t header_count;

    /**
     * Flag that controls if the current WSGI resquest
     * contains the content length header, and so the
     * length is specified.
     */
    char has_length;

    /**
     * The reference to the current WSGI context
     * structure in use.
     */
    struct handler_wsgi_context_t *wsgi_context;
} wsgi_request;

/**
 * The global reference to the currently loaded service
 * this is the reference required for the basic interaction
 * with the service.
 */
VIRIATUM_EXTERNAL_PREFIX struct service_t *_service;

/**
 * The global reference to the current connection being
 * used, this is going to be used to access connection
 * information values.
 */
VIRIATUM_EXTERNAL_PREFIX struct connection_t *_connection;

/**
 * The global headers structure that provides a "cache"
 * like mechanism for the buffer that contains the various
 * headers to be parsed.
 * This strategy avoid the allocation of a "huge buffer
 * for each request received.
 */
VIRIATUM_EXTERNAL_PREFIX struct http_headers_t _headers;

/**
 * The global structure to be used to "pass" WSGI
 * information from the virtual machine into the
 * appropriate viriatum request handler.
 */
VIRIATUM_EXTERNAL_PREFIX struct wsgi_request_t _wsgi_request;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_python_module(struct mod_python_module_t **mod_python_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_python_module(struct mod_python_module_t *mod_python_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module_python(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module_python(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module_python(struct module_t *module);
#ifdef VIRIATUM_ASGI
/**
 * The module itself, kept so that the operation run once per cycle
 * of the loop is able to reach the loop of the applications.
 */
VIRIATUM_EXTERNAL_PREFIX struct mod_python_module_t *_mod_python_module;

VIRIATUM_EXPORT_PREFIX ERROR_CODE cycle_module_python(struct service_t *service);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_asgi_module(struct service_t *service, struct mod_python_module_t *mod_python_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _stop_asgi_module(struct service_t *service, struct mod_python_module_t *mod_python_module);
#endif
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module_python(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration_wsgi(struct service_t *service, struct mod_python_http_handler_t *mod_python_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_locations_wsgi(struct service_t *service, struct mod_python_http_handler_t *mod_python_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_python_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_python_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reload_python_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_python_state();
