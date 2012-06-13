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

#include "stdafx.h"

#include "handler.h"

ERROR_CODE create_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t **mod_wsgi_http_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the mod wsgi http handler size */
    size_t mod_wsgi_http_handler_size = sizeof(struct mod_wsgi_http_handler_t);

    /* allocates space for the mod wsgi http handler */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler = (struct mod_wsgi_http_handler_t *) MALLOC(mod_wsgi_http_handler_size);

    /* sets the mod wsgi http handler in the upper http handler substrate */
    http_handler->lower = (void *) mod_wsgi_http_handler;

    /* sets the mod wsgi http handler in the mod wsgi http handler pointer */
    *mod_wsgi_http_handler_pointer = mod_wsgi_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* releases the mod wsgi http handler */
    FREE(mod_wsgi_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_wsgi_context(struct handler_wsgi_context_t **handler_wsgi_context_pointer) {
    /* retrieves the handler wsgi context size */
    size_t handler_wsgi_context_size = sizeof(struct handler_wsgi_context_t);

    /* allocates space for the handler wsgi context */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) MALLOC(handler_wsgi_context_size);

    /* sets the handler wsgi context in the  pointer */
    *handler_wsgi_context_pointer = handler_wsgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_wsgi_context(struct handler_wsgi_context_t *handler_wsgi_context) {
    /* releases the handler wsgi context memory */
    FREE(handler_wsgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_module(struct http_connection_t *http_connection) {
    /* sets the http parser values */
    _set_http_parser_handler_module(http_connection->http_parser);

    /* sets the http settings values */
    _set_http_settings_handler_module(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_module(struct http_connection_t *http_connection) {
    /* unsets the http parser values */
    _unset_http_parser_handler_module(http_connection->http_parser);

    /* unsets the http settings values */
    _unset_http_settings_handler_module(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_module(struct http_parser_t *http_parser) {
    /* sends (and creates) the reponse */
    _send_response_handler_module(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* allocates space for the handler wsgi context and
    then creates and populates the instance after that
    sets the handler file context as the context for
    the http parser*/
    struct handler_wsgi_context_t *handler_wsgi_context;
    create_handler_wsgi_context(&handler_wsgi_context);
    http_parser->context = handler_wsgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* retrieves the handler wsgi context from the http parser
    and then deletes (releases memory) */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;
    delete_handler_wsgi_context(handler_wsgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_module(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_module;
    http_settings->on_url = url_callback_handler_module;
    http_settings->on_header_field = header_field_callback_handler_module;
    http_settings->on_header_value = header_value_callback_handler_module;
    http_settings->on_headers_complete = headers_complete_callback_handler_module;
    http_settings->on_body = body_callback_handler_module;
    http_settings->on_message_complete = message_complete_callback_handler_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_module(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the http settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_data_callback(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_module(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* raise no error */
    RAISE_NO_ERROR;
}
