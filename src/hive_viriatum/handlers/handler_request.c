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

#include "handler_request.h"

ERROR_CODE registerHandlerRequest(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* creates the http handler */
    service->createHttpHandler(service, &httpHandler, (unsigned char *) "request");

    /* sets the http handler attributes */
    httpHandler->set = setHandlerRequest;
    httpHandler->unset = unsetHandlerRequest;
    httpHandler->reset = NULL;

    /* adds the http handler to the service */
    service->addHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterHandlerRequest(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http handler from the service, then
    remove it from the service after that delete the handler */
    service->getHttpHandler(service, &httpHandler, (unsigned char *) "request");
    service->removeHttpHandler(service, httpHandler);
    service->deleteHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE setHandlerRequest(struct HttpConnection_t *httpConnection) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unsetHandlerRequest(struct HttpConnection_t *httpConnection) {
    /* raises no error */
    RAISE_NO_ERROR;
}
