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
    modPhpHttpHandler->basePath = NULL;

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

ERROR_CODE createHandlerPhpContext(struct HandlerPhpContext_t **handlerPhpContextPointer) {
    /* retrieves the handler php context size */
    size_t handlerPhpContextSize = sizeof(struct HandlerPhpContext_t);

    /* allocates space for the handler php context */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) MALLOC(handlerPhpContextSize);

    /* sets the handler php default values */
    handlerPhpContext->outputBuffer = NULL;

    /* sets the handler php context in the  pointer */
    *handlerPhpContextPointer = handlerPhpContext;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHandlerPhpContext(struct HandlerPhpContext_t *handlerPhpContext) {
    /* in case there is a valid output buffer defined in the current
    handler php context it must be removed (linked buffer removal)
    this way serious memory leaks are avoided */
    if(handlerPhpContext->outputBuffer) { deleteLinkedBuffer(handlerPhpContext->outputBuffer); }

    /* releases the handler php context memory */
    FREE(handlerPhpContext);

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
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* retrieves the handler php context from the http parser */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;

    /* allocates space for the file name that will be executed
    by the php interpreter, this is necessary to remove the query
    part of the uri value */
    unsigned char fileName[VIRIATUM_MAX_PATH_SIZE];

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', dataSize);
    size_t pathSize = pointer == NULL ? dataSize : pointer - (char *) data;
    size_t querySize = pointer == NULL ? 0 : dataSize - pathSize - 1;
    querySize = querySize > 0 ? querySize : 0;

    /* copies the part of the data buffer relative to the file name
    this avoids copying the query part */
    memcpy(fileName, data, pathSize);
    fileName[pathSize] = '\0';

    /* in case the pointer is defined (query separator found) copies
    the query contents into the target query buffer */
    if(pointer) { memcpy(handlerPhpContext->query, pointer + 1, querySize); }
    handlerPhpContext->query[querySize] = '\0';

    /* copies the url to the url reference in the handler file context then
    creates the file path from using the base viriatum path */
    memcpy(handlerPhpContext->url, data, dataSize);
	handlerPhpContext->url[pathSize] = '\0';
    SPRINTF((char *) handlerPhpContext->filePath, VIRIATUM_MAX_PATH_SIZE, "%s%s%s", VIRIATUM_CONTENTS_PATH, VIRIATUM_BASE_PATH, fileName);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerValueCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headersCompleteCallbackHandlerModule(struct HttpParser_t *httpParser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageCompleteCallbackHandlerModule(struct HttpParser_t *httpParser) {
    /* sends (and creates) the reponse */
    _sendResponseHandlerModule(httpParser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpParserHandlerModule(struct HttpParser_t *httpParser) {
    /* allocates space for the handler php context and
    then creates and populates the instance after that
    sets the handler file context as the context for
    the http parser*/
    struct HandlerPhpContext_t *handlerPhpContext;
    createHandlerPhpContext(&handlerPhpContext);
    httpParser->context = handlerPhpContext;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerModule(struct HttpParser_t *httpParser) {
    /* retrieves the handler php context from the http parser
    and then deletes (releases memory) */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;
    deleteHandlerPhpContext(handlerPhpContext);

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
    /* allocates the buffer that will hod the message to be sent
    through the connection and then allocates the buffer to hold
    the joined buffer from the linked buffer rerference */
    char *buffer;
    char *outputData;

    /* retrieves the http parser and then uses it to retrieve the
    proper output buffer for the current connection (the context) */
    struct HttpParser_t *httpParser = (struct HttpParser_t *) parameters;
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;
    struct LinkedBuffer_t *outputBuffer = handlerPhpContext->outputBuffer;

    /* retrieves the size of the output buffer, this is going to
    be used to measure the size of the output stream */
    size_t outputLength = outputBuffer->bufferLength;

    /* joins the output buffer so that the buffer is set as continguous
    and then deletes the output buffer (no more need to use it) */
    joinLinkedBuffer(outputBuffer, (unsigned char **) &outputData);
    deleteLinkedBuffer(outputBuffer);

    /* allocates data for the current connection and then copues the
    current output buffer data into it (for writing into the connection) */
    connection->allocData(connection, outputLength, (void **) &buffer);
    memcpy(buffer, outputData, outputLength);

    /* writes the response to the connection, this should flush the current
    data in the output buffer to the network */
    connection->writeConnection(connection, (unsigned char *) buffer, outputLength, _sendResponseCallbackHandlerModule, parameters);

    /* unsets the output buffer from the context (it's not going) to
    be used anymore (must be released) */
    handlerPhpContext->outputBuffer = NULL;

    /* releases the output data no more need to use it */
    FREE(outputData);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseHandlerModule(struct HttpParser_t *httpParser) {
    /* allocates space for the script file structure to be
    during for script execution */
    zend_file_handle script;

    /* allocates space for the buffer that will hold the headers */
    char *headersBuffer;

    /* allocates space for the linked buffer to be used for the
    standard ouput resulting from the php interpreter execution */
    struct LinkedBuffer_t *outputBuffer = NULL;

    /* retrieves the connection from the http parser parameters
    and then retrieves the handler php context*/
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;

    /* creates the linked buffer to be used to store
    the complete output of the php interpreter */
    createLinkedBuffer(&outputBuffer);

    /* sets the output buffer reference in the global output buffer
    variable so that the php write handler is able to store the
    current output stream values, then sets the output buffer also
    in the current http parser structure reference */
    _outputBuffer = outputBuffer;
    handlerPhpContext->outputBuffer = outputBuffer;

    /* updates the current global php reqest information
    this is the main interface for the sapi modules */
    _updateRequest(handlerPhpContext);

    /* populates the "base" script reference structure
    with the required value for execution */
    script.type = ZEND_HANDLE_FILENAME;
    script.filename = (char *) handlerPhpContext->filePath;
    script.opened_path = NULL;
    script.free_filename = 0;

    zend_try {
        /* tries to start the request handling and in case it
        succeedes continues with the execution of it */
        if(php_request_startup(TSRMLS_C) == SUCCESS) {
            /* executes the script in the current instantiated virtual
            machine, this is a blocking call so it will block the current
            general loop (care is required), then after the execution
            closes the current request information */
            php_execute_script(&script TSRMLS_CC);
            php_request_shutdown(NULL);
        }
    } zend_catch {
    } zend_end_try();

    /* allocates space fot the header buffer and then writes the default values
    into it the value is dynamicaly contructed based on the current header values */
    connection->allocData(connection, 1024, (void **) &headersBuffer);
	SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, "text/plain", (long unsigned int) outputBuffer->bufferLength);

    /* writes the response to the connection, this will only write
    the headers the remaining message will be sent on the callback */
    connection->writeConnection(connection, (unsigned char *) headersBuffer, (unsigned int) strlen(headersBuffer), _sendDataCallback, (void *) httpParser);

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

ERROR_CODE _updateRequest(struct HandlerPhpContext_t *handlerPhpContext) {
    SG(sapi_headers).http_response_code = 200;
    SG(sapi_headers).http_status_line = "OK";
    SG(request_info).content_type = "text/html";
    SG(request_info).query_string = (char *) handlerPhpContext->query;
    SG(request_info).request_method = "GET";
    SG(request_info).proto_num = 1001;
    SG(request_info).request_uri = (char *) handlerPhpContext->url;
    SG(request_info).path_translated = (char *) handlerPhpContext->filePath;
    SG(request_info).content_length = 0;
    SG(global_request_time) = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}
