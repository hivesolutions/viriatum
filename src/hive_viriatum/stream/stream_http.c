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

#include "stream_http.h"

void createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *ioConnection) {
    /* retrieves the http connection size */
    size_t httpConnectionSize = sizeof(struct HttpConnection_t);

    /* allocates space for the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) malloc(httpConnectionSize);

    /* sets the http connection io connection */
    httpConnection->ioConnection = ioConnection;

    /* creates the http settings */
    createHttpSettings(&httpConnection->httpSettings);

    /* creates the http parser */
    createHttpParser(&httpConnection->httpParser);

    /* sets the connection as the parser parameter(s) */
    httpConnection->httpParser->parameters = ioConnection->connection;

    /* sets the http connection in the (upper) io connection substrate */
    ioConnection->lower = httpConnection;

    /* sets the http connection in the http connection pointer */
    *httpConnectionPointer = httpConnection;
}

void deleteHttpConnection(struct HttpConnection_t *httpConnection) {
    /* deletes the http parser */
    deleteHttpParser(httpConnection->httpParser);

    /* deletes the http settings */
    deleteHttpSettings(httpConnection->httpSettings);

    /* releases the http connection */
    free(httpConnection);
}

ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize) {
    /* retrieves the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;


    /* sets the structures for the handler */
    setHandlerFile(httpConnection->httpParser, httpConnection->httpSettings);


    // TODO: tenho de testar quantos bytes processei !!!
    /* process the http data for the http parser */
    processDataHttpParser(httpConnection->httpParser, httpConnection->httpSettings, buffer, bufferSize);



    /* unsets the structures for the handler */
    unsetHandlerFile(httpConnection->httpParser, httpConnection->httpSettings);



    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE openHandlerStreamHttp(struct IoConnection_t *ioConnection) {
    /* allocates the http connection */
    struct HttpConnection_t *httpConnection;

    /* creates the http connection */
    createHttpConnection(&httpConnection, ioConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeHandlerStreamHttp(struct IoConnection_t *ioConnection) {
    /* retrieves the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;

    /* deletes the http connection */
    deleteHttpConnection(httpConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}
