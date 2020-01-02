/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../http/http.h"
#include "../system/system.h"

/* forward references (avoids loop) */
struct data_t;
struct connection_t;
struct http_handler_t;
struct http_connection_t;

ERROR_CODE register_handler_default(struct service_t *service);
ERROR_CODE unregister_handler_default(struct service_t *service);
ERROR_CODE set_handler_default(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_default(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_default(struct http_parser_t *http_parser);
ERROR_CODE url_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_default(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_default(struct http_parser_t *http_parser);
ERROR_CODE path_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_default(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_default(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_parser_handler_default(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_default(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_default(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_default(struct http_settings_t *http_settings);
ERROR_CODE _send_response_handler_default(struct http_parser_t *http_parser);
ERROR_CODE _send_response_callback_handler_default(struct connection_t *connection, struct data_t *data, void *parameters);
