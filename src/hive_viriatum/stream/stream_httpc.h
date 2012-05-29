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

#include "../http/http.h"
#include "stream_io.h"

/* forward references (avoids loop) */
struct HttpClientConnection_t;

/**
 * Function used to update the given http client connection
 * with new information.
 *
 * @param httpClientConnection The http client connection to be
 * update with new information.
 */
typedef ERROR_CODE (*httpClientConnectionUpdate) (struct HttpClientConnection_t *httpClientConnection);

/**
 * Structure defining a logical
 * http client connection.
 */
typedef struct HttpClientConnection_t {
    /**
     * The (upper) io connection that owns
     * manages this connection.
     */
    struct IoConnection_t *ioConnection;

    /**
     * Structure containig the settings to be
     * used by the http parser.
     */
    struct HttpSettings_t *httpSettings;

    /**
     * Parser to be used during the interpretation
     * of the http requests.
     */
    struct HttpParser_t *httpParser;
} HttpClientConnection;

typedef struct HttpClientParameters_t {
    char *url;
} HttpClientParameters;

ERROR_CODE createHttpClientConnection(struct HttpClientConnection_t **httpClientConnectionPointer, struct IoConnection_t *ioConnection);
ERROR_CODE deleteHttpClientConnection(struct HttpClientConnection_t *httpClientConnection);
ERROR_CODE dataHandlerStreamHttpClient(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize);
ERROR_CODE openHandlerStreamHttpClient(struct IoConnection_t *ioConnection);
ERROR_CODE closeHandlerStreamHttpClient(struct IoConnection_t *ioConnection);
