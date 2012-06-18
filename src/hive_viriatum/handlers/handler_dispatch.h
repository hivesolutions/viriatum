/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef VIRIATUM_PCRE
#include <pcre.h>
#endif

#include "../http/http.h"
#include "../system/system.h"

/**
 * The message to be printed to the client in case
 * the handler propagates to the top (error case).
 */
#define DISPATCH_ERROR_MESSAGE "500 - Internal Server Error (dispatch problem)"

/**
 * The default handler to be used in case no rule
 * is matched for a handler.
 */
#define DISPATCH_DEFAULT_HANDLER "file"

/* forward references (avoids loop) */
struct data_t;
struct connection_t;
struct http_connection_t;

typedef struct dispatch_handler_t {
    /**
     * The array of regular expressions that are meant
     * to be executed in the ordered defined for execution.
     * In case this array grows too much the performance
     * of the server may lower significantly.
     */
#ifdef VIRIATUM_PCRE
    pcre **regex;
#endif

    /**
     * The arrays of (handler) names in the same order as
     * the corresponding regular expressions, they are going
     * to be used at runtime for handler resolution.
     */
    unsigned char **names;

    /**
     * The number of regular expressions curently present
     * in the dispatch handler.
     */
    size_t regex_count;
} dispatch_handler;

ERROR_CODE create_dispatch_handler(struct dispatch_handler_t **dispatch_handler_pointer, struct http_handler_t *http_handler);
ERROR_CODE delete_dispatch_handler(struct dispatch_handler_t *dispatch_handler);
ERROR_CODE register_handler_dispatch(struct service_t *service);
ERROR_CODE unregister_handler_dispatch(struct service_t *service);
ERROR_CODE set_handler_dispatch(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_dispatch(struct http_connection_t *http_connection);
ERROR_CODE reset_handler_dispatch(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_dispatch(struct http_parser_t *http_parser);
ERROR_CODE url_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_dispatch(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_dispatch(struct http_parser_t *http_parser);
ERROR_CODE location_callback_handler_dispatch(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_parser_handler_dispatch(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_dispatch(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_dispatch(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_dispatch(struct http_settings_t *http_settings);
ERROR_CODE _send_response_handler_dispatch(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_dispatch(struct connection_t *connection, struct data_t *data, void *parameters);
