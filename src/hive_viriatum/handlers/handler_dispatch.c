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
#ifdef VIRIATUM_PCRE
    dispatchHandler->regex = NULL;
#endif
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
#ifdef VIRIATUM_PCRE
    /* allocates space for the index counter */
    size_t index;
#endif

#ifdef VIRIATUM_PCRE
    /* iterates over all the regular expressions to release their
    internal memory contents */
    for(index = 0; index < dispatchHandler->regexCount; index++ ) {  pcre_free(dispatchHandler->regex[index]); }
#endif

    /* in case the names buffer is defined releases it */
    if(dispatchHandler->names != NULL) { FREE(dispatchHandler->names); }

#ifdef VIRIATUM_PCRE
    /* in case the regex buffer is defined releases it */
    if(dispatchHandler->regex != NULL) { FREE(dispatchHandler->regex); }
#endif

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

#ifdef VIRIATUM_PCRE
    /* allocates space for the regex related variables */
    const char *error;
    int errorOffset;
#endif

    /* creates the http handler and then uses it to create
    the dispatch handler (lower substrate) */
    service->createHttpHandler(service, &httpHandler, (unsigned char *) "dispatch");
    createDispatchHandler(&dispatchHandler, httpHandler);

    /* sets the http handler attributes */
    httpHandler->set = setHandlerDispatch;
    httpHandler->unset = unsetHandlerDispatch;
    httpHandler->reset = NULL;


    dispatchHandler->regexCount = 3;
#ifdef VIRIATUM_PCRE
    dispatchHandler->regex = (pcre **) MALLOC(sizeof(pcre *) * dispatchHandler->regexCount);
    dispatchHandler->regex[0] = pcre_compile("[.]*\\.lua", 0, &error, &errorOffset, NULL);
    dispatchHandler->regex[1] = pcre_compile("[.]*\\.php", 0, &error, &errorOffset, NULL);
    dispatchHandler->regex[2] = pcre_compile("[.]*\\.default", 0, &error, &errorOffset, NULL);
#endif
    dispatchHandler->names = (unsigned char **) MALLOC(sizeof(unsigned char *) * dispatchHandler->regexCount);
    dispatchHandler->names[0] = (unsigned char *) "lua";
    dispatchHandler->names[1] = (unsigned char *) "php";
    dispatchHandler->names[2] = (unsigned char *) "dedault";



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
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
#ifdef VIRIATUM_PCRE
    int matching;
    size_t index;
#endif

    /* allocates space for the name of the handler to be
    used for the dispatching operation (target handler) */
    unsigned char *handlerName;

    /* allocates the required space for the url, this
    is done through static allocation */
    unsigned char url[VIRIATUM_MAX_URL_SIZE];

    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;
    struct Service_t *service = connection->service;
    struct HttpHandler_t *handler = httpConnection->httpHandler;
#ifdef VIRIATUM_PCRE
    struct DispatchHandler_t *dispatchHandler = (struct DispatchHandler_t *) handler->lower;
#endif

    /* copies the memory from the data to the url, then
    puts the end of string in the url */
    memcpy(url, data, dataSize);
    url[dataSize] = '\0';





    /* THIS IS EXTREMLY SLOW !!! WARNING may be a better
    idea to compile all the regex into a single regex must
    refer to the nginx documentation for that */

    /* sets the default handler, this is considered to be
    the fallback in case no handler is found */
    handlerName = (unsigned char *) DISPATCH_DEFAULT_HANDLER;

#ifdef VIRIATUM_PCRE
    /* iterates over all the regular expressions so that they
    may be tested agains the current url */
    for(index = 0; index < dispatchHandler->regexCount; index++) {
        /* tries to match the current url agains the registered
        regular expression in case it fails continues the loop */
        matching = pcre_exec(dispatchHandler->regex[index], NULL, (char *) url, dataSize, 0, 0, NULL, 0);
        if(matching != 0) { continue; }

        /* sets the name of the handler as the name in the current index
        the breaks the loop to process it */
        handlerName = dispatchHandler->names[index];
        break;
    }
#endif

    /* END OF WARNING */

    /* sets the current http handler accoring to the current options
    in the service, the http handler must be loaded in the handlers map
    in case the handler is not currently available an error is printed */
    getValueStringHashMap(service->httpHandlersMap, handlerName, (void **) &handler);
    if(handler) {
        /* retrieves the current handler and then unsets it
        from the connection (detach) then sets the the prper
        handler in the connection and notifies it of the url */
        httpConnection->httpHandler->unset(httpConnection);
        handler->set(httpConnection);
        httpConnection->httpHandler = handler;
        httpConnection->httpSettings->onmessageBegin(httpParser);
        httpConnection->httpSettings->onurl(httpParser, data, dataSize);
    } else {
        /* prints an error message to the output */
        V_ERROR_F("Error retrieving '%s' handler reference\n", handlerName);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerValueCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headersCompleteCallbackHandlerDispatch(struct HttpParser_t *httpParser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageCompleteCallbackHandlerDispatch(struct HttpParser_t *httpParser) {
    /* sends (and creates) the reponse */
    _sendResponseHandlerDispatch(httpParser);

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

ERROR_CODE _sendResponseHandlerDispatch(struct HttpParser_t *httpParser) {
    /* allocates the response buffer */
    char *responseBuffer = MALLOC(256);

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* writes the http static headers to the response */
    SPRINTF(responseBuffer, 256, "HTTP/1.1 500 Internal Server Error\r\nServer: %s/%s (%s @ %s)\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n\r\n%s", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) sizeof(DISPATCH_ERROR_MESSAGE) - 1, DISPATCH_ERROR_MESSAGE);

    /* writes the response to the connection, registers for the appropriate callbacks */
    writeConnection(connection, (unsigned char *) responseBuffer, (unsigned int) strlen(responseBuffer), _sendResponseCallbackHandlerDispatch, (void *) httpParser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseCallbackHandlerDispatch(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* retrieves the http parser */
    struct HttpParser_t *httpParser = (struct HttpParser_t *) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;

    /* in case there is an http handler in the current connection must
    unset it (remove temporary information) */
    if(httpConnection->httpHandler) {
        /* unsets the current http connection and then sets the reference
        to it in the http connection as unset */
        httpConnection->httpHandler->unset(httpConnection);
        httpConnection->httpHandler = NULL;
    }

    /* in case the connection is not meant to be kept alive */
    if(!(httpParser->flags & FLAG_CONNECTION_KEEP_ALIVE)) {
        /* closes the connection */
        connection->closeConnection(connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
