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

#include "entry.h"

/**
 * The structure that holds the internal
 * structure to support the context
 * of the wsgi module.
 */
typedef struct mod_wsgi_http_handler_t {
    /**
     * The path to the default file to
     * be used for the parsing.
     */
    char *file_path;

    /**
     * Flag that controls if the script should
     * be reload everytime a request si received.
     * This options is usefull for debugging purposes,
     * and should never be used in production.
     */
    char reload;

	/**
  	 * The reference to the module object reprenting
	 * the parsed and compiled file to be used.
	 * This reference is used to avoid constant reloading
	 * of the file in case the reload flag is not set.
	 */ 
	PyObject *module;
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
     * The path to the file to be handled by
     * the current php request.
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
     * after exposing it to the php interpreter.
     */
    struct http_headers_t *headers;

    /**
     * The current value for the flags describing
     * the status of the current http request.
     */
    unsigned char flags;

    /**
     * Enumeration value that controls the type
     * of the next header to be read from the
     * http input buffer.
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
     * String reference to the query buffer, useful
     * for fast attribute calculation (eg: size).
     */
    struct string_t _query_string;

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
     * to be used to retrive data from the wsgi
     * application sequence of data, and to use
     * it to print to the output buffer.
     */
    PyObject *iterator;
} handler_wsgi_context;

ERROR_CODE create_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t **mod_wsgi_http_handler_pointer, struct http_handler_t *http_handler_pointer);
ERROR_CODE delete_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t *mod_wsgi_http_handler);
ERROR_CODE create_handler_wsgi_context(struct handler_wsgi_context_t **handler_wsgi_context_pointer);
ERROR_CODE delete_handler_wsgi_context(struct handler_wsgi_context_t *handler_wsgi_context);
ERROR_CODE set_handler_module(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_module(struct http_connection_t *http_connection);
ERROR_CODE url_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_module(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_module(struct http_parser_t *http_parser);
ERROR_CODE _set_http_parser_handler_module(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_module(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_module(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_module(struct http_settings_t *http_settings);
ERROR_CODE _message_begin_callback_handler_module(struct http_parser_t *http_parser);
ERROR_CODE _send_response_handler_module(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_module(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _start_environ(PyObject *environ, struct http_parser_t *http_parser);
ERROR_CODE _load_module(PyObject **module_pointer, char *name, char *file_path);
