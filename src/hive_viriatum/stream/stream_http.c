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

ERROR_CODE createHttpHandler(struct HttpHandler_t **httpHandlerPointer, unsigned char *name) {
    /* retrieves the http handler size */
    size_t httpHandlerSize = sizeof(struct HttpHandler_t);

    /* allocates space for the http handler */
    struct HttpHandler_t *httpHandler = (struct HttpHandler_t *) MALLOC(httpHandlerSize);

    /* sets the http handler attributes (default) values */
    httpHandler->name = name;
    httpHandler->set = NULL;
    httpHandler->unset = NULL;
    httpHandler->reset = NULL;

    /* sets the http handler in the http handler pointer */
    *httpHandlerPointer = httpHandler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpHandler(struct HttpHandler_t *httpHandler) {
    /* releases the http handler */
    FREE(httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *ioConnection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http connection size */
    size_t httpConnectionSize = sizeof(struct HttpConnection_t);

    /* allocates space for the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) MALLOC(httpConnectionSize);

    /* retrieves the service associated with the connection */
    struct Service_t *service = ioConnection->connection->service;

    /* sets the http handler attributes (default) values */
    httpConnection->ioConnection = ioConnection;
    httpConnection->httpHandler = NULL;

    /* creates the http settings */
    createHttpSettings(&httpConnection->httpSettings);

    /* creates the http parser */
    createHttpParser(&httpConnection->httpParser);

    /* sets the connection as the parser parameter(s) */
    httpConnection->httpParser->parameters = ioConnection->connection;

    /* sets the http connection in the (upper) io connection substrate */
    ioConnection->lower = httpConnection;

    /* retrieves the current (default) service handler and sets the
    connection on it */
    getValueStringHashMap(service->httpHandlersMap, (unsigned char *) "file", (void **) &httpHandler);
    httpHandler->set(httpConnection);

    /* sets the http connection in the http connection pointer */
    *httpConnectionPointer = httpConnection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpConnection(struct HttpConnection_t *httpConnection) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *httpHandler;

    /* retrieves the service from the http connection */
    struct Service_t *service = httpConnection->ioConnection->connection->service;

    /* retrieves the currently assigned handler and usets the connection
    from with (unregister connection) */
    getValueStringHashMap(service->httpHandlersMap, (unsigned char *) "file", (void **) &httpHandler);
    httpHandler->unset(httpConnection);

    /* deletes the http parser */
    deleteHttpParser(httpConnection->httpParser);

    /* deletes the http settings */
    deleteHttpSettings(httpConnection->httpSettings);

    /* releases the http connection */
    FREE(httpConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize) {
    /* allocates space for the http handler reference
    to be used in this connection */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http connection */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;

    /* retrieves the service from the http connection */
    struct Service_t *service = httpConnection->ioConnection->connection->service;

    /* retrieves the current (default) service handler and sets its
    value on the http connection */
    getValueStringHashMap(service->httpHandlersMap, (unsigned char *) "file", (void **) &httpHandler);
    httpConnection->httpHandler = httpHandler;

    /* in case the reset callback is set in the http
    handler, must call it */
    if(httpHandler->reset != NULL) {
        /* calls the reset callback (for the connection)
        in the http handler */
        httpHandler->reset(httpConnection);
    }

    // TODO: tenho de testar quantos bytes processei !!!
    // NAO posso assumir que por cada pacote de dados que recebo
    // tenho uma nova mensagem
    /* process the http data for the http parser */
    processDataHttpParser(httpConnection->httpParser, httpConnection->httpSettings, buffer, bufferSize);

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
