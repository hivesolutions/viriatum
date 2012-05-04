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
    /* sets the http parser values */
	_setHttpParserHandlerRequest(httpConnection->httpParser);

    /* sets the http settings values */
    _setHttpSettingsHandlerRequest(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unsetHandlerRequest(struct HttpConnection_t *httpConnection) {
    /* unsets the http parser values */
    _unsetHttpParserHandlerRequest(httpConnection->httpParser);

    /* unsets the http settings values */
    _unsetHttpSettingsHandlerRequest(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageBeginCallbackHandlerRequest(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerRequest(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    const char *error;
    int errorOffset;
    pcre *regex;
    int matching;

    struct HttpHandler_t *handler;	
	struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;
	struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;
	struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;
    struct Service_t *service = connection->service;

	char *handlerName;
	
	/* allocates the required space for the url */
    unsigned char *url = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, dataSize);

    /* puts the end of strng in the url */
    url[dataSize] = '\0';

    /* prints the url */
    V_DEBUG_F("url: %s\n", url);




    /* THIS IS EXTREMLY SLOW !!! WARNING */

    regex = pcre_compile("[.]*\\.lua", 0, &error, &errorOffset, NULL);
    matching = pcre_exec(regex, NULL, url, dataSize, 0, 0, NULL, 0);

	if(matching == 0) { handlerName = "lua"; }
	else { handlerName = "file"; }

	printf("HANDLER NAME -> %s\n", handlerName);

	/* END OF WARNING */






	/* retrieves the current handler and then unsets it
	from the connection (detach) */
	handler = httpConnection->httpHandler;
	handler->unset(httpConnection);

    /* sets the current http handler accoring to the current options
    in the service, the http handler must be loaded in the handlers map */
    getValueStringHashMap(service->httpHandlersMap, handlerName, &handler);
	handler->set(httpConnection);
	httpConnection->httpHandler = handler;
	httpConnection->httpSettings->onurl(httpParser, data, dataSize);

    /* releases the url */
    FREE(url);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerRequest(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the header field */
    unsigned char *headerField = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the header field */
    memcpy(headerField, data, dataSize);

    /* puts the end of strng in the header field */
    headerField[dataSize] = '\0';

    /* prints the header field */
    V_DEBUG_F("header field: %s\n", headerField);

    /* releases the header field */
    FREE(headerField);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerValueCallbackHandlerRequest(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the header value */
    unsigned char *headerValue = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the header value */
    memcpy(headerValue, data, dataSize);

    /* puts the end of strng in the header value */
    headerValue[dataSize] = '\0';

    /* prints the header value */
    V_DEBUG_F("header value: %s\n", headerValue);

    /* releases the header value */
    FREE(headerValue);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headersCompleteCallbackHandlerRequest(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP headers parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerRequest(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the body */
    unsigned char *body = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the body */
    memcpy(body, data, dataSize);

    /* puts the end of strng in the body */
    body[dataSize] = '\0';

    /* prints the body */
    V_DEBUG_F("body: %s\n", body);

    /* releases the body */
    FREE(body);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageCompleteCallbackHandlerRequest(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpParserHandlerRequest(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerRequest(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpSettingsHandlerRequest(struct HttpSettings_t *httpSettings) {
    /* sets the various callback functions in the http settings
	structure, these callbacks are going to be used in the runtime
	processing of http parser (runtime execution) */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerRequest;
    httpSettings->onurl = urlCallbackHandlerRequest;
    httpSettings->onheaderField = headerFieldCallbackHandlerRequest;
    httpSettings->onheaderValue = headerValueCallbackHandlerRequest;
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerRequest;
    httpSettings->onbody = bodyCallbackHandlerRequest;
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerRequest;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpSettingsHandlerRequest(struct HttpSettings_t *httpSettings) {
    /* unsets the various callback functions from the http settings */
    httpSettings->onmessageBegin = NULL;
    httpSettings->onurl = NULL;
    httpSettings->onheaderField = NULL;
    httpSettings->onheaderValue = NULL;
    httpSettings->onheadersComplete = NULL;
    httpSettings->onbody = NULL;
    httpSettings->onmessageComplete = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}
