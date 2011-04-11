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

#include "stdafx.h"

#include "http_request.h"

void createHttpRequest(struct HttpRequest_t **httpRequestPointer) {
    /* retrieves the http request size */
    size_t httpRequestSize = sizeof(struct HttpRequest_t);

    /* allocates space for the http request */
    struct HttpRequest_t *httpRequest = (struct HttpRequest_t *) malloc(httpRequestSize);

    /* sets the http request received data size */
    httpRequest->receivedDataSize = 0;

    /* sets the http request start line loaded */
    httpRequest->startLineLoaded = 0;

    /* sets the http request headers loaded */
    httpRequest->headersLoaded = 0;

    /* sets the http request in the http request pointer */
    *httpRequestPointer = httpRequest;
}

void deleteHttpRequest(struct HttpRequest_t *httpRequest) {
    /* releases the http request */
    free(httpRequest);
}

void parseDataHttpRequest(struct HttpRequest_t *httpRequest, unsigned char *data, size_t dataSize) {
    /**
     * NOTE: durring the parsing the index (and rest of the state) of
     *       the find must be saved during the various parsing iterations
     *
     * 1. must find the first \r\n to load the first line of the http message
     * 2. sets the startLineLoaded flag
     * 3. tries to find the \r\n\r\n end to load the headers
     * 4. sets the headersLoaded flag
     */

    /* in case the start line is not yet loaded */
    if(!httpRequest->startLineLoaded) {

    }

    printf("ola mundo\n");
}
