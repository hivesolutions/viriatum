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
typedef enum HttpHeader_e {
    UNDEFINED_HEADER = 1,
    CONTENT_TYPE,
    COOKIE
} HttpHeader;

typedef enum HttpRequestMethod_e {
    GET_REQUEST_METHOD = 1,
    POST_REQUEST_METHOD = 1
} HttpRequestMethod;

typedef struct HttpRequest_t {
    enum HttpRequestMethod_e method;
    unsigned char *data;
} HttpRequest;

/**
 * Constructor of the http request.
 *
 * @param httpRequestPointer The pointer to the http request to be constructed.
 */
void createHttpRequest(struct HttpRequest_t **httpRequestPointer);

/**
 * Destructor of the http request.
 *
 * @param httpRequest The http request to be destroyed.
 */
void deleteHttpRequest(struct HttpRequest_t *httpRequest);
