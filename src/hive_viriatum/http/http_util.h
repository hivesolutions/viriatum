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

#include "../system/service.h"

#define GET_HTTP_STATUS(code) http_status_codes[(code / 100) - 1][code % 100]

/* forward references (avoids loop) */
struct data_t;
struct connection_t;
typedef ERROR_CODE (*_connection_data_callback) (struct connection_t *, struct data_t *, void *);

/**
 * The buffer containing sequences of the
 * descriptions to the various http error
 * codes orderes by major and minor parts
 * of the error code.
 */
const char *http_status_codes[5][64];

ERROR_CODE write_http_error(struct connection_t *connection, char *header, char *error_code, char *error_message, char *error_description, _connection_data_callback callback, void *callback_parameters);
ERROR_CODE log_http_request(char *host, char *identity, char *user, char *method, char *uri, char *protocol, int error_code, size_t content_length);
const char *_get_http_status_code(size_t major, size_t minor);
