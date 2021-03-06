/*
 Hive Viriatum Modules
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "handler.h"
#include "extension.h"

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
    struct linked_buffer_t *_output_buffer;\
    struct php_request_t _php_request

/**
 * Structure describing the internal
 * structures and information for the
 * mod PHP module.
 */
typedef struct mod_php_module_t {
    /**
     * The HTTP handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod PHP HTTP handler associated
     * with the module.
     */
    struct mod_php_http_handler_t *mod_php_http_handler;
} mod_php_module;

/**
 * Structure representing a PHP request
 * for rendering of a page.
 * This structure contains a series of
 * values useful for request flush.
 */
typedef struct php_request_t {
    /**
     * The mime type value for the current PHP
     * request, this header value is treated as
     * a special case by the PHP interpreter.
     */
    char mime_type[VIRIATUM_MAX_HEADER_V_SIZE];

    /**
     * The matrix buffer containing the maximum
     * possible count for various headers for the
     * PHP interpreter.
     */
    char headers[VIRIATUM_MAX_HEADER_COUNT][VIRIATUM_MAX_HEADER_C_SIZE];

    /**
     * The number of headers currently present in
     * the current PHP request.
     */
    size_t header_count;

    /**
     * The reference to the current PHP context
     * structure in use.
     */
    struct handler_php_context_t *php_context;
} php_request;

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
 * The global reference to the linked buffer to
 * be used to hold the various strings resulting
 * from the PHP default execution output.
 */
VIRIATUM_EXTERNAL_PREFIX struct linked_buffer_t *_output_buffer;

/**
 * The global structure to be used to "pass" PHP
 * information from the virtual machine into the
 * appropriate viriatum request handler.
 */
VIRIATUM_EXTERNAL_PREFIX struct php_request_t _php_request;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_php_module(struct mod_php_module_t **mod_php_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_php_module(struct mod_php_module_t *mod_php_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module_php(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module_php(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module_php(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module_php(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration_php(struct service_t *service, struct mod_php_http_handler_t *mod_php_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_php_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_php_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reload_php_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_php_state();
VIRIATUM_EXPORT_PREFIX int _write_php_state(const char *data, unsigned int data_size TSRMLS_DC);
VIRIATUM_EXPORT_PREFIX void _log_php_state(char *message);
VIRIATUM_EXPORT_PREFIX void _error_php_state(int type, const char *message, ...);
