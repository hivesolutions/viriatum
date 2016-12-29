/*
 Hive Viriatum Modules
 Copyright (c) 2008-2017 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2017 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "handler.h"
#include "extension.h"

#ifdef PYTHON_THREADS
#define VIRIATUM_ACQUIRE_GIL PyEval_AcquireLock();\
    PyThreadState_Swap(_state)
#define VIRIATUM_RELEASE_GIL PyThreadState_Swap(NULL);\
    PyEval_ReleaseLock()
#else
#define VIRIATUM_ACQUIRE_GIL
#define VIRIATUM_RELEASE_GIL
#endif

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
#define START_GLOBALS struct service_t *_service;\
    struct connection_t *_connection;\
    struct http_headers_t _headers;\
    struct wsgi_request_t _wsgi_request;\
    PyThreadState *_state

/**
 * Structure describing the internal
 * structures and information for the
 * mod wsgi module.
 */
typedef struct mod_wsgi_module_t {
    /**
     * The http handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod wsgi http handler associated
     * with the module.
     */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler;
} mod_wsgi_module;

/**
 * Structure representing a wsgi request
 * for rendering of a page.
 * This structure contains a series of
 * values useful for request flush.
 */
typedef struct wsgi_request_t {
    /**
     * The integer code describing the status
     * of the response assocaited with this
     * request, this information is provided
     * by the wsgi application.
     */
    int status_code;

    /**
     * The message string describing the status
     * of the response assocaited with this
     * request, this information is provided
     * by the wsgi application.
     */
    char status_message[256];

    /**
     * The matrix buffer containing the maximum
     * possible count for various headers for the
     * wsgi interpreter.
     */
    char headers[VIRIATUM_MAX_HEADER_COUNT][VIRIATUM_MAX_HEADER_C_SIZE];

    /**
     * The number of headers currently present in
     * the current wsgi request.
     */
    size_t header_count;

    /**
     * Flag that controls if the current wsgi resquest
     * contains the content length header, and so the
     * length is specified.
     */
    char has_length;

    /**
     * The reference to the current wsgi context
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
 * The global structure to be used to "pass" wsgi
 * information from the virtual machine into the
 * appropriate viriatum request handler.
 */
VIRIATUM_EXTERNAL_PREFIX struct wsgi_request_t _wsgi_request;

/**
 * The global reference to the state of the main controlling
 * thread so that it's possible to chenge the state to the
 * main thread for processing.
 */
VIRIATUM_EXTERNAL_PREFIX PyThreadState *_state;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_wsgi_module(struct mod_wsgi_module_t **mod_wsgi_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_wsgi_module(struct mod_wsgi_module_t *mod_wsgi_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module_wsgi(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module_wsgi(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module_wsgi(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module_wsgi(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration_wsgi(struct service_t *service, struct mod_wsgi_http_handler_t *mod_wsgi_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_locations_wsgi(struct service_t *service, struct mod_wsgi_http_handler_t *mod_wsgi_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_wsgi_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_wsgi_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reload_wsgi_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_wsgi_state();
