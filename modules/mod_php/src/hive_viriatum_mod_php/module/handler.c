/*
 Hive Viriatum Modules
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "handler.h"

ERROR_CODE createModPhpHttpHandler(struct ModPhpHttpHandler_t **modPhpHttpHandlerPointer, struct HttpHandler_t *httpHandler) {
    /* retrieves the mod php http handler size */
    size_t modPhpHttpHandlerSize = sizeof(struct ModPhpHttpHandler_t);

    /* allocates space for the mod php http handler */
    struct ModPhpHttpHandler_t *modPhpHttpHandler = (struct ModPhpHttpHandler_t *) MALLOC(modPhpHttpHandlerSize);

    /* sets the mod php http handler attributes (default) values */
    modPhpHttpHandler->filePath = NULL;
    modPhpHttpHandler->fileDirty = 0;

    /* sets the mod php http handler in the upper http handler substrate */
    httpHandler->lower = (void *) modPhpHttpHandler;

    /* sets the mod php http handler in the mod php http handler pointer */
    *modPhpHttpHandlerPointer = modPhpHttpHandler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteModPhpHttpHandler(struct ModPhpHttpHandler_t *modPhpHttpHandler) {
    /* releases the mod php http handler */
    FREE(modPhpHttpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE setHandlerModule(struct HttpConnection_t *httpConnection) {
    /* sets the http parser values */
    _setHttpParserHandlerModule(httpConnection->httpParser);

    /* sets the http settings values */
    _setHttpSettingsHandlerModule(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unsetHandlerModule(struct HttpConnection_t *httpConnection) {
    /* unsets the http parser values */
    _unsetHttpParserHandlerModule(httpConnection->httpParser);

    /* unsets the http settings values */
    _unsetHttpSettingsHandlerModule(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageBeginCallbackHandlerModule(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request received\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headerFieldCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headerValueCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE headersCompleteCallbackHandlerModule(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP headers parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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

ERROR_CODE messageCompleteCallbackHandlerModule(struct HttpParser_t *httpParser) {
    /* prints an information */
    V_DEBUG("HTTP request parsed\n");

    /* sends (and creates) the reponse */
    _sendResponseHandlerModule(httpParser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpParserHandlerModule(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerModule(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpSettingsHandlerModule(struct HttpSettings_t *httpSettings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerModule;
    httpSettings->onurl = urlCallbackHandlerModule;
    httpSettings->onheaderField = headerFieldCallbackHandlerModule;
    httpSettings->onheaderValue = headerValueCallbackHandlerModule;
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerModule;
    httpSettings->onbody = bodyCallbackHandlerModule;
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerModule;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpSettingsHandlerModule(struct HttpSettings_t *httpSettings) {
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

ERROR_CODE _sendDataCallback(struct Connection_t *connection, struct Data_t *data, void *parameters) {




    /* writes the response to the connection */
    connection->writeConnection(connection, _buffer, _bufferSize, _sendResponseCallbackHandlerModule, parameters);



    /* TODO: TENHO DE TER CUIDADO PORQUE TAMBEM TENHO DE DE FAZER RELEASE
    disot em caso de erro */


    FREE(_buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}


ERROR_CODE _sendResponseHandlerModule(struct HttpParser_t *httpParser) {
    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* retrieves the http connection from the io connection and uses it to retrieve
    the correct (mod php) handler to operate around it */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ((struct IoConnection_t *) connection->lower)->lower;
    struct ModPhpHttpHandler_t *modPhpHttpHandler = (struct ModPhpHttpHandler_t *) httpConnection->httpHandler->lower;




    char *buffer;

    size_t dataSize;
    char *stringValue;

    char *headersBuffer;



    /* creates the string buffer to be used to store
    the complete output of the php interpreter */
    createStringBuffer(&_outputBuffer);

    zend_eval_string("phpinfo();", NULL, "Embedded Code" TSRMLS_CC);

    /* "joins" the string buffer values into a single
    value (from the internal string list) */
    joinStringBuffer(_outputBuffer, &stringValue);

    /* deletes the string buffer, no more need to store
    the output from the php interpreter */
    deleteStringBuffer(_outputBuffer);

    printf("Vai fazer print %d:\n", _outputBuffer->stringLength);

    dataSize = _outputBuffer->stringLength;



    /* allocates the data buffer (in a safe maner) then
    copies the data (from lua) into the buffer */
    connection->allocData(connection, dataSize * sizeof(unsigned char), (void **) &buffer);
    memcpy(buffer, stringValue, dataSize);






    connection->allocData(connection, 1024, (void **) &headersBuffer);

    SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) dataSize);

    /* writes the response to the connection */
    connection->writeConnection(connection, headersBuffer, 1024, _sendDataCallback, (void *) httpParser);





    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseCallbackHandlerModule(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* retrieves the http parser */
    struct HttpParser_t *httpParser = (struct HttpParser_t *) parameters;

    /* in case the connection is not meant to be kept alive */
    if(!(httpParser->flags & FLAG_CONNECTION_KEEP_ALIVE)) {
        /* closes the connection */
        connection->closeConnection(connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _writeErrorConnection(struct HttpParser_t *httpParser, char *message) {
    /* allocates space for the buffer to be used in the message */
    unsigned char *buffer;

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* retrieves the length of the message so that it's possible to print
    the proper error */
    size_t messageLength = strlen(message);

    /* allocates the data buffer (in a safe maner) then
    writes the http static headers to the response */
    connection->allocData(connection, 1024 * sizeof(unsigned char), (void **) &buffer);
    SPRINTF((char *) buffer, 1024, "HTTP/1.1 500 Internal Server Error\r\nServer: %s/%s (%s @ %s)\r\nConnection: Keep-Alive\r\nContent-Length: %d\r\n\r\n%s", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (unsigned int) messageLength, message);

    /* writes the response to the connection, registers for the appropriate callbacks */
    connection->writeConnection(connection, buffer, (unsigned int) strlen((char *) buffer), _sendResponseCallbackHandlerModule, (void *) httpParser);

    /* raise no error */
    RAISE_NO_ERROR;
}
