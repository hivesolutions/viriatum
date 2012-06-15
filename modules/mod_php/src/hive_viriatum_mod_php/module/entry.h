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

#pragma once

#include "handler.h"
#include "extension.h"

/**
 * The bae path to the directory to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_BASE_PATH "."

/**
 * Structure describing the internal
 * structures and information for the
 * mod php module.
 */
typedef struct mod_php_module_t {
    /**
     * The http handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;

    /**
     * The mod php http handler associated
     * with the module.
     */
    struct mod_php_http_handler_t *mod_php_http_handler;
} mod_php_module;

/**
 * Structure representing a php request
 * for rendering of a page.
 * This structure contains a series of
 * values useful for request flush.
 */
typedef struct php_request_t {
    /**
     * The mime type value for the current php
     * request, this header value is treated as
     * a special case by the php interpreter.
     */
    char mime_type[VIRIATUM_MAX_HEADER_V_SIZE];

    /**
     * The matrix buffer containing the maximum
     * possible count for various headers for the
     * php interpreter.
     */
    char headers[VIRIATUM_MAX_HEADER_COUNT][VIRIATUM_MAX_HEADER_C_SIZE];

    /**
     * The number of headers currently present in
     * the current php request.
     */
    size_t header_count;

    /**
     * The reference to the current php context
     * structure in use.
     */
    struct handler_php_context_t *php_context;
} php_request;

/**
 * The global reference to the currently loaded service
 * this is the reference required for the basic interaction
 * with the service.
 */
struct service_t *_service;

/**
 * The global reference to the current connection being
 * used, this is going to be used to access connection
 * information values.
 */
struct connection_t *_connection;

/**
 * The global reference to the linked buffer to
 * be used to hold the various strings resulting
 * from the php default execution output.
 */
struct linked_buffer_t *_output_buffer;

/**
 * The global structure to be used to "pass" php
 * information from the virtual machine into the
 * appropriate viriatum request handler.
 */
struct php_request_t _php_request;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_php_module(struct mod_php_module_t **mod_php_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_php_module(struct mod_php_module_t *mod_php_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module(unsigned char **message_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_configuration(struct service_t *service, struct mod_php_http_handler_t *mod_php_http_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _load_php_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unload_php_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reload_php_state();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_php_state();
VIRIATUM_EXPORT_PREFIX int _write_php_state(const char *data, unsigned int data_size TSRMLS_DC);
VIRIATUM_EXPORT_PREFIX void _log_php_state(char *message);
VIRIATUM_EXPORT_PREFIX void _error_php_state(int type, const char *message, ...);
