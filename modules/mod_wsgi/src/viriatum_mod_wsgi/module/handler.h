/*
 Hive Viriatum Modules
 Copyright (c) 2008-2018 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "entry.h"

/**
 * The maximum size of the name of the module
 * to be used in the WSGI module.
 */
#define VIRIATUM_WSGI_MODULE_SIZE 128

/**
 * Structure describing the internal parameters
 * for a location in the WSGI context.
 */
typedef struct mod_wsgi_location_t {
    /**
     * The path to the default file to
     * be used for the parsing.
     */
    char *file_path;

    /**
     * Flag that controls if the script should
     * be reload everytime a request is received.
     * This options is usefull for debugging purposes,
     * and should never be used in production.
     */
    char reload;

    /**
     * The reference to the module object representing
     * the parsed and compiled file to be used.
     * This reference is used to avoid constant reloading
     * of the file in case the reload flag is not set.
     */
    PyObject *module;

    /**
     * The name generated for the module associated with
     * location, this value should be generated by the
     * handler in a way that no duplicates occur.
     */
    char module_name[VIRIATUM_WSGI_MODULE_SIZE];
} mod_wsgi_location_t;

/**
 * The structure that holds the internal
 * structure to support the context
 * of the WSGI module.
 */
typedef struct mod_wsgi_http_handler_t {
    /**
     * The path to the default file to
     * be used for the parsing.
     */
    char *file_path;

    /**
     * Flag that controls if the script should
     * be reload everytime a request is received.
     * This options is usefull for debugging purposes,
     * and should never be used in production.
     */
    char reload;

    /**
     * The counter value to be used to generated the
     * various module name in an unique order.
     * This value should be used in an incremental order
     * so that the handler is able to generate unique
     * names for the modules (mandatory).
     */
    size_t counter;

    /**
     * The reference to the module object representing
     * the parsed and compiled file to be used.
     * This reference is used to avoid constant reloading
     * of the file in case the reload flag is not set.
     */
    PyObject *module;

    /**
     * The buffer used to store the "main" module name
     * so that it may be refered in the python interpreter.
     */
    char module_name[VIRIATUM_WSGI_MODULE_SIZE];

    /**
     * The various locations loaded from the configuration
     * they refer the cofiruation attributes associated
     * with the WSGI structures.
     */
    struct mod_wsgi_location_t *locations;

    /**
     * The number of locations currently loaded in the handler
     * this value is used for iteration arround the locations
     * buffer.
     */
    size_t locations_count;
} mod_wsgi_http_handler;

typedef struct handler_wsgi_context_t {
    /**
     * The url to be used for retrieving the file.
     */
    unsigned char url[VIRIATUM_MAX_URL_SIZE];

    /**
     * The file name to be used for retrieving the file.
     */
    unsigned char file_name[VIRIATUM_MAX_URL_SIZE];

    /**
     * The query string (contents after the '?')
     * to be used to create the get parameters map.
     */
    unsigned char query[VIRIATUM_MAX_URL_SIZE];

    /**
     * The name to be considered the preffix in the
     * internal WSGI structure for internal guide.
     * This value is used for "virtual" location guide.
     */
    unsigned char prefix_name[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The path to the file to be handled by
     * the current PHP request.
     */
    unsigned char file_path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The content type for the current request being
     * made this is a header value.
     */
    unsigned char content_type[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * The content length for the current request being
     * made this is a header value.
     */
    unsigned char content_length_[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * The cookie for the current request being
     * made this is a header value.
     */
    unsigned char cookie[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * The host for the current request being
     * made this is a header value.
     */
    unsigned char host[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * The server name for the current request being
     * made this is a header value.
     */
    unsigned char server_name[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * The current header structure, represeting the
     * header currently being parsed.
     * In case no value is defined there's no header
     * "in parsing".
     */
    struct http_header_value_t *header;

    /**
     * The list of headers parsed for the current request
     * this value changes over the parsing of the request.
     * The name of the header is not reliable as it changes
     * after exposing it to the PHP interpreter.
     */
    struct http_headers_t *headers;

    /**
     * The current value for the flags describing
     * the status of the current HTTP request.
     */
    unsigned char flags;

    /**
     * The module in selection to be used in the current
     * context, this value may be unset in case the there
     * is no module to be executed.
     */
    PyObject *module;

    /**
     * The pointer to the module reference currently in use
     * so that it's possible to the handler to change the
     * value of the module in the original "position".
     */
    PyObject **module_pointer;

    /**
     * The name of the module currently in execution, this value
     * may be changed by the handler in case there is a load or
     * reload for the module.
     */
    char *module_name;

    /**
     * If the module currently in loading should be reloaded and
     * re-imported for the current execution operation.
     */
    char reload;

    /**
     * Enumeration value that controls the type
     * of the next header to be read from the
     * HTTP input buffer.
     */
    enum http_header_e _next_header;

    /**
     * String reference to the url buffer, usefull
     * for fast attribute calculation (eg: size).
     */
    struct string_t _url_string;

    /**
     * String reference to the file name buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _file_name_string;

    /**
     * String reference to the query string, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _query_string;

    /**
     * String reference to the prefix name buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _prefix_name_string;

    /**
     * String reference to the file path buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _file_path_string;

    /**
     * String reference to the content type buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _content_type_string;

    /**
     * String reference to the content length buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _content_length_string;

    /**
     * String reference to the cookie buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _cookie_string;

    /**
     * String reference to the host buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _host_string;

    /**
     * String reference to the server name buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _server_name_string;

    /**
     * The reference to the buffer iterator
     * to be used to retrive data from the WSGI
     * application sequence of data, and to use
     * it to print to the output buffer.
     */
    PyObject *iterator;
} handler_wsgi_context;

ERROR_CODE create_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t **mod_wsgi_http_handler_pointer, struct http_handler_t *http_handler_pointer);
ERROR_CODE delete_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t *mod_wsgi_http_handler);
ERROR_CODE create_handler_wsgi_context(struct handler_wsgi_context_t **handler_wsgi_context_pointer);
ERROR_CODE delete_handler_wsgi_context(struct handler_wsgi_context_t *handler_wsgi_context);
ERROR_CODE set_handler_wsgi(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_wsgi(struct http_connection_t *http_connection);
ERROR_CODE url_callback_handler_wsgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_wsgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_wsgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_wsgi(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_wsgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_wsgi(struct http_parser_t *http_parser);
ERROR_CODE path_callback_handler_wsgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_wsgi(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_wsgi(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_parser_handler_wsgi(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_wsgi(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_wsgi(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_wsgi(struct http_settings_t *http_settings);
ERROR_CODE _send_data_callback_wsgi(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _message_begin_callback_handler_wsgi(struct http_parser_t *http_parser);
ERROR_CODE _send_response_handler_wsgi(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_wsgi(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _write_error_connection_wsgi(struct http_parser_t *http_parser, char *message);
ERROR_CODE _start_environ_wsgi(PyObject *environ, struct http_parser_t *http_parser);
ERROR_CODE _load_module_wsgi(PyObject **module_pointer, char *name, char *file_path);
