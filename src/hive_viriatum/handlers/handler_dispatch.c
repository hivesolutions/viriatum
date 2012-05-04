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

#include "handler_dispatch.h"

ERROR_CODE createDispatchHandler(struct DispatchHandler_t **dispatchHandlerPointer, struct HttpHandler_t *httpHandler) {
    /* retrieves the dispatch handler size */
    size_t dispatchHandlerSize = sizeof(struct DispatchHandler_t);

    /* allocates space for the dispatch handler */
    struct DispatchHandler_t *dispatchHandler = (struct DispatchHandler_t *) MALLOC(dispatchHandlerSize);

    /* sets the dispatch handler attributes (default) values */
    dispatchHandler->regex = NULL;
    dispatchHandler->names = NULL;
    dispatchHandler->regexCount = 0;

    /* sets the dispatch handler in the upper http handler substrate */
    httpHandler->lower = (void *) dispatchHandler;

    /* sets the dispatch handler in the mod lua http handler pointer */
    *dispatchHandlerPointer = dispatchHandler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteDispatchHandler(struct DispatchHandler_t *dispatchHandler) {
	/* allocates space for the index counter */
	size_t index;

	/* iterates over all the regular expressions to release their
	internal memory contents */
	for(index = 0; index < dispatchHandler->regexCount; index++ ) {  pcre_free(dispatchHandler->regex[index]); }

    /* in case the names buffer is defined releases it */
    if(dispatchHandler->names != NULL) { FREE(dispatchHandler->names); }
	
	/* in case the regex buffer is defined releases it */
    if(dispatchHandler->regex != NULL) { FREE(dispatchHandler->regex); }

    /* releases the dispatch handler */
    FREE(dispatchHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE registerHandlerDispatch(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* allocates space for the dispatch handler */
    struct DispatchHandler_t *dispatchHandler;

    /* allocates space for the regex related variables */
    const char *error;
    int errorOffset;

    /* creates the http handler and then uses it to create
    the dispatch handler (lower substrate) */
    service->createHttpHandler(service, &httpHandler, (unsigned char *) "dispatch");
    createDispatchHandler(&dispatchHandler, httpHandler);

    /* sets the http handler attributes */
    httpHandler->set = setHandlerDispatch;
    httpHandler->unset = unsetHandlerDispatch;
    httpHandler->reset = NULL;


    dispatchHandler->regexCount = 2;
    dispatchHandler->regex = (pcre **) MALLOC(sizeof(pcre *) * dispatchHandler->regexCount);
    dispatchHandler->regex[0] = pcre_compile("[.]*\\.lua", 0, &error, &errorOffset, NULL);
    dispatchHandler->regex[1] = pcre_compile("[.]*\\.default", 0, &error, &errorOffset, NULL);
    dispatchHandler->names = (unsigned char **) MALLOC(sizeof(pcre *) * dispatchHandler->regexCount);
    dispatchHandler->names[0] = (unsigned char *) "lua";
    dispatchHandler->names[1] = (unsigned char *) "default";



    /* adds the http handler to the service */
    service->addHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterHandlerDispatch(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* allocates space for the dispatch handler */
    struct DispatchHandler_t *dispatchHandler;

    /* retrieves the http handler from the service, then retrieves
    the lower substrate as the dispatch handler */
    service->getHttpHandler(service, &httpHandler, (unsigned char *) "dispatch");
    dispatchHandler = (struct DispatchHandler_t *) httpHandler->lower;

    /* deletes the dispatch handler reference */
    deleteDispatchHandler(dispatchHandler);

    /* remove the http handler from the service after
    that deletes the handler reference */
    service->removeHttpHandler(service, httpHandler);
    service->deleteHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE setHandlerDispatch(struct HttpConnection_t *httpConnection) {
    /* sets the http parser values */
    _setHttpParserHandlerDispatch(httpConnection->httpParser);

    /* sets the http settings values */
    _setHttpSettingsHandlerDispatch(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unsetHandlerDispatch(struct HttpConnection_t *httpConnection) {
    /* unsets the http parser values */
    _unsetHttpParserHandlerDispatch(httpConnection->httpParser);

    /* unsets the http settings values */
    _unsetHttpSettingsHandlerDispatch(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageBeginCallbackHandlerDispatch(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    int matching;

    size_t index;


    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;
    struct Service_t *service = connection->service;
    struct HttpHandler_t *handler = httpConnection->httpHandler;
    struct DispatchHandler_t *dispatchHandler = (struct DispatchHandler_t *) handler->lower;

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

    handlerName = "file";

    /* iterates over all the regular expressions so that they
    may be tested agains the current url */
    for(index = 0; index < dispatchHandler->regexCount; index++) {
        /* tries to match the current url agains the registered
        regular expression in case it fails continues the loop */
        matching = pcre_exec(dispatchHandler->regex[index], NULL, url, dataSize, 0, 0, NULL, 0);
        if(matching != 0) { continue; }

        /* sets the name of the handler as the name in the current index
        the breaks the loop to process it */
        handlerName = dispatchHandler->names[index];
        break;
    }

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

ERROR_CODE headerFieldCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headerValueCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headersCompleteCallbackHandlerDispatch(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP headers parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE messageCompleteCallbackHandlerDispatch(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpParserHandlerDispatch(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerDispatch(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpSettingsHandlerDispatch(struct HttpSettings_t *httpSettings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerDispatch;
    httpSettings->onurl = urlCallbackHandlerDispatch;
    httpSettings->onheaderField = headerFieldCallbackHandlerDispatch;
    httpSettings->onheaderValue = headerValueCallbackHandlerDispatch;
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerDispatch;
    httpSettings->onbody = bodyCallbackHandlerDispatch;
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerDispatch;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpSettingsHandlerDispatch(struct HttpSettings_t *httpSettings) {
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
