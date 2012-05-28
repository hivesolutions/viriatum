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

#include "handler_default.h"

ERROR_CODE registerHandlerDefault(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* creates the http handler */
    service->createHttpHandler(service, &httpHandler, (unsigned char *) "default");

    /* sets the http handler attributes */
    httpHandler->set = setHandlerDefault;
    httpHandler->unset = unsetHandlerDefault;
    httpHandler->reset = NULL;

    /* adds the http handler to the service */
    service->addHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterHandlerDefault(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http handler from the service, then
    remove it from the service after that delete the handler */
    service->getHttpHandler(service, &httpHandler, (unsigned char *) "default");
    service->removeHttpHandler(service, httpHandler);
    service->deleteHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE setHandlerDefault(struct HttpConnection_t *httpConnection) {
    /* sets the http parser values */
    _setHttpParserHandlerDefault(httpConnection->httpParser);

    /* sets the http settings values */
    _setHttpSettingsHandlerDefault(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unsetHandlerDefault(struct HttpConnection_t *httpConnection) {
    /* unsets the http parser values */
    _unsetHttpParserHandlerDefault(httpConnection->httpParser);

    /* unsets the http settings values */
    _unsetHttpSettingsHandlerDefault(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageBeginCallbackHandlerDefault(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("http request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the url */
    unsigned char *url = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, dataSize);

    /* puts the end of strng in the url */
    url[dataSize] = '\0';

    /* prints the url */
    V_DEBUG_F("url: %s\n", url);

    /* releases the url */
    FREE(url);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headerValueCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headersCompleteCallbackHandlerDefault(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("http headers parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE messageCompleteCallbackHandlerDefault(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("http request parsed\n");

    /* sends (and creates) the reponse */
    _sendResponseHandlerDefault(httpParser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpParserHandlerDefault(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerDefault(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpSettingsHandlerDefault(struct HttpSettings_t *httpSettings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerDefault;
    httpSettings->onurl = urlCallbackHandlerDefault;
    httpSettings->onheaderField = headerFieldCallbackHandlerDefault;
    httpSettings->onheaderValue = headerValueCallbackHandlerDefault;
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerDefault;
    httpSettings->onbody = bodyCallbackHandlerDefault;
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerDefault;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpSettingsHandlerDefault(struct HttpSettings_t *httpSettings) {
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

ERROR_CODE _sendResponseHandlerDefault(struct HttpParser_t *httpParser) {
    /* allocates the response buffer */
    char *responseBuffer = MALLOC(256);

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* writes the http static headers to the response */
    SPRINTF(responseBuffer, 256, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s @ %s)\r\nConnection: Keep-Alive\r\nContent-Length: 14\r\n\r\nHello Viriatum", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU);

    /* writes the response to the connection, registers for the appropriate callbacks */
    writeConnection(connection, (unsigned char *) responseBuffer, (unsigned int) strlen(responseBuffer), _sendResponseCallbackHandlerDefault, (void *) (size_t) httpParser->flags);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseCallbackHandlerDefault(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* retrieves the current http flags */
    unsigned char flags = (unsigned char) (size_t) parameters;

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
    if(!(flags & FLAG_CONNECTION_KEEP_ALIVE)) {
        /* closes the connection */
        connection->closeConnection(connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
