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

void setHandlerDefault(struct HttpParser_t *httpParser, struct HttpSettings_t *httpSettings) {
    /* sets the http parser values */
    setHttpParserHandlerDefault(httpParser);

    /* sets the http settings values */
    setHttpSettingsHandlerDefault(httpSettings);
}

void unsetHandlerDefault(struct HttpParser_t *httpParser, struct HttpSettings_t *httpSettings) {
    /* unsets the http parser values */
    unsetHttpParserHandlerDefault(httpParser);

    /* unsets the http settings values */
    unsetHttpSettingsHandlerDefault(httpSettings);
}

void setHttpParserHandlerDefault(struct HttpParser_t *httpParser) {
}

void unsetHttpParserHandlerDefault(struct HttpParser_t *httpParser) {
}

void setHttpSettingsHandlerDefault(struct HttpSettings_t *httpSettings) {
    /* sets the http settings on message begin callback */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerDefault;

    /* sets the http settings on url callback */
    httpSettings->onurl = urlCallbackHandlerDefault;

    /* sets the http settings on header field callback */
    httpSettings->onheaderField = headerFieldCallbackHandlerDefault;

    /* sets the http settings on header value callback */
    httpSettings->onheaderValue = headerValueCallbackHandlerDefault;

    /* sets the http settings on headers complete callback */
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerDefault;

    /* sets the http settings on body callback */
    httpSettings->onbody = bodyCallbackHandlerDefault;

    /* sets the http settings on message complete callback */
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerDefault;
}

void unsetHttpSettingsHandlerDefault(struct HttpSettings_t *httpSettings) {
    /* unsets the http settings on message begin callback */
    httpSettings->onmessageBegin = NULL;

    /* unsets the http settings on url callback */
    httpSettings->onurl = NULL;

    /* unsets the http settings on header field callback */
    httpSettings->onheaderField = NULL;

    /* unsets the http settings on header value callback */
    httpSettings->onheaderValue = NULL;

    /* unsets the http settings on headers complete callback */
    httpSettings->onheadersComplete = NULL;

    /* unsets the http settings on body callback */
    httpSettings->onbody = NULL;

    /* unsets the http settings on message complete callback */
    httpSettings->onmessageComplete = NULL;
}

ERROR_CODE messageBeginCallbackHandlerDefault(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request received\n");

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
    V_DEBUG("HTTP headers parsed\n");

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
    V_DEBUG("HTTP request parsed\n");

    /* sends (and creates) the reponse */
    _sendResponseHandlerDefault(httpParser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseHandlerDefault(struct HttpParser_t *httpParser) {
    /* allocates the response buffer */
    char *responseBuffer = MALLOC(1024);

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* writes the http static headers to the response */
    SPRINTF(responseBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s @ %s)\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n\r\nhello world", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, strlen("hello world"));

    /* writes the response to the connection */
    writeConnection(connection, (unsigned char *) responseBuffer, strlen(responseBuffer), _sendResponseCallbackHandlerDefault, (void *) httpParser->flags);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseCallbackHandlerDefault(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* retrieves the (http) flags */
    unsigned char flags = (unsigned char) parameters;

    /* in case the connection is not meant to be kept alive */
    if(!(flags & FLAG_CONNECTION_KEEP_ALIVE)) {
        /* closes the connection */
        connection->closeConnection(connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
