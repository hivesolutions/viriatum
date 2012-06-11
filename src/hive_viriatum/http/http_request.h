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

/**
 * Enumeration defining the various possible
 * and "recognizable" http header types, this
 * may be used to provided static reference.
 */
typedef enum http_header_e {
    UNDEFINED_HEADER = 1,
    CONTENT_TYPE,
    COOKIE,
	HOST
} http_header;

typedef enum http_request_method_e {
    GET_REQUEST_METHOD = 1,
    POST_REQUEST_METHOD = 1
} http_request_method;

typedef struct http_request_t {
    enum http_request_method_e method;
    unsigned char *data;
} http_request;

/**
 * Constructor of the http request.
 *
 * @param http_request_pointer The pointer to the http request to be constructed.
 */
void create_http_request(struct http_request_t **http_request_pointer);

/**
 * Destructor of the http request.
 *
 * @param http_request The http request to be destroyed.
 */
void delete_http_request(struct http_request_t *http_request);
