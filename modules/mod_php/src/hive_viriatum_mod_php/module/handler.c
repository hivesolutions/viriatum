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
    handlerPhpContext->method = NULL;
    handlerPhpContext->postData = NULL;
    handlerPhpContext->flags = 0;
    handlerPhpContext->contentLength = 0;
    handlerPhpContext->outputBuffer = NULL;
    handlerPhpContext->_nextHeader = UNDEFINED_HEADER;

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
    /* retrieves the handler php context from the http parser
    in order tu use it to update the content type to the default
    empty value (avoids possible problems in php interpreter)*/
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;
    handlerPhpContext->contentType[0] = '\0';

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    stringPopulate(&handlerPhpContext->_contentTypeString, handlerPhpContext->contentType, 0, 0);
    stringPopulate(&handlerPhpContext->_cookieString, handlerPhpContext->cookie, 0, 0);

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

    /* populates the various generated strings, avoids possible recalculation
    of the lengths of the string */
    stringPopulate(&handlerPhpContext->_queryString, handlerPhpContext->query, querySize, 0);
    stringPopulate(&handlerPhpContext->_urlString, handlerPhpContext->url, pathSize, 0);
    stringPopulate(&handlerPhpContext->_filePathString, handlerPhpContext->filePath, 0, 1);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* retrieves the handler php context from the http parser */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;

    /* checks if the current header is a valid "capturable"
    header in such case changes the next header value accordingly
    otherwise sets the undefined header */
    if(memcmp(data, "Content-Type", dataSize) == 0) { handlerPhpContext->_nextHeader = CONTENT_TYPE; }
    else if(memcmp(data, "Cookie", dataSize) == 0) { handlerPhpContext->_nextHeader = COOKIE; }
    else { handlerPhpContext->_nextHeader = UNDEFINED_HEADER; }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerValueCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* retrieves the handler php context from the http parser */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;

    /* switchs over the next header possible values to
    copy the current header buffer into the appropriate place */
    switch(handlerPhpContext->_nextHeader) {
        case CONTENT_TYPE:
            /* copies the content type header value into the
            appropriate buffer in the php context */
            memcpy(handlerPhpContext->contentType, data, dataSize);
            handlerPhpContext->contentType[dataSize] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            stringPopulate(&handlerPhpContext->_contentTypeString, handlerPhpContext->contentType, dataSize, 0);

            /* breaks the switch */
            break;

        case COOKIE:
            /* copies the cookie header value into the
            appropriate buffer in the php context */
            memcpy(handlerPhpContext->cookie, data, dataSize);
            handlerPhpContext->cookie[dataSize] = '\0';

            /* populates the various generated strings, avoids possible recalculation
            of the lengths of the string */
            stringPopulate(&handlerPhpContext->_cookieString, handlerPhpContext->cookie, dataSize, 0);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

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

    /* retrieves the current php context and then uses it to retrieve
    the proper output buffer for the current connection (the context) */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) parameters;
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

    /* allocates space for both the index to be used for iteration
    and for the count to be used in pointer increment  */
    size_t index;
    size_t count;

    /* allocates space for the buffer that will hold the headers and
    the pointer that will hold the reference to the buffer containing
    the post data information */
    char *headersBuffer;
    unsigned char *postData;

    /* allocates space for the linked buffer to be used for the
    standard ouput resulting from the php interpreter execution */
    struct LinkedBuffer_t *outputBuffer = NULL;

    /* retrieves the connection from the http parser parameters
    and then retrieves the handler php context*/
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ioConnection->lower;
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;

    /* retrieves the http method string value accessing the array of
    static string values */
    char *method = (char *) httpMethodStrings[httpParser->method - 1];

    /* in case there is contents to be read retrieves the appropriate
    reference to the start of the post data in the connection buffer */
    if(httpParser->_contentLength > 0) { postData = &httpConnection->buffer[httpConnection->bufferOffset - httpParser->_contentLength];
    } else { postData = NULL; }

    /* sets the global reference to the connection currently being
    used for the php interpreter this is the reference that is going
    to be used while accessing global server values */
    _connection = connection;

    /* creates the linked buffer to be used to store
    the complete output of the php interpreter */
    createLinkedBuffer(&outputBuffer);

    /* sets the output buffer reference in the global output buffer
    variable so that the php write handler is able to store the
    current output stream values, then sets the output buffer also
    in the current http parser structure reference */
    _outputBuffer = outputBuffer;
    handlerPhpContext->method = method;
    handlerPhpContext->postData = postData;
    handlerPhpContext->flags = httpParser->flags;
    handlerPhpContext->contentLength = httpParser->_contentLength;
    handlerPhpContext->outputBuffer = outputBuffer;

    /* resets the number of headers for the current php request
    to be processed (this is a new php request) */
    _phpRequest.headerCount = 0;

    /* sets the php context in the php request for global reference
    this should be able to allow global access from the handler methods */
    _phpRequest.phpContext = handlerPhpContext;

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
    connection->allocData(connection, 25602, (void **) &headersBuffer);
    count = SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) outputBuffer->bufferLength);

    /* iterates over all the headers present in the current php request to copy
    their content into the current headers buffer */
    for(index = 0; index < _phpRequest.headerCount; index++) {
        /* copies the current php header into the current position of the headers
        buffer (header copy) */
        count += SPRINTF(&headersBuffer[count], 1026, "%s\r\n", _phpRequest.headers[index]);
    }

    /* finishes the current headers sequence with the final carriage return newline
    character values to closes the headers part of the envelope */
    memcpy(&headersBuffer[count], "\r\n", 2);
	count += 2;

    /* writes the response to the connection, this will only write
    the headers the remaining message will be sent on the callback */
    connection->writeConnection(connection, (unsigned char *) headersBuffer, (unsigned int) count, _sendDataCallback, (void *) handlerPhpContext);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseCallbackHandlerModule(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* retrieves the current php context fro the parameters */
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) parameters;

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
    if(!(handlerPhpContext->flags & FLAG_CONNECTION_KEEP_ALIVE)) {
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
    struct HandlerPhpContext_t *handlerPhpContext = (struct HandlerPhpContext_t *) httpParser->context;

    /* retrieves the length of the message so that it's possible to print
    the proper error */
    size_t messageLength = strlen(message);

    /* updates the flags value in the php context, this is required
    to avoid problems in the callback handler */
    handlerPhpContext->flags = httpParser->flags;

    /* allocates the data buffer (in a safe maner) then
    writes the http static headers to the response */
    connection->allocData(connection, 1024 * sizeof(unsigned char), (void **) &buffer);
    SPRINTF((char *) buffer, 1024, "HTTP/1.1 500 Internal Server Error\r\nServer: %s/%s (%s @ %s)\r\nConnection: Keep-Alive\r\nContent-Length: %d\r\n\r\n%s", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (unsigned int) messageLength, message);

    /* writes the response to the connection, registers for the appropriate callbacks */
    connection->writeConnection(connection, buffer, (unsigned int) strlen((char *) buffer), _sendResponseCallbackHandlerModule, (void *) handlerPhpContext);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _updateRequest(struct HandlerPhpContext_t *handlerPhpContext) {
    /* sets the various sapi headers and request info parameters
    from the current php context object values */
    SG(sapi_headers).http_response_code = 200;
    SG(sapi_headers).http_status_line = "OK";
    SG(request_info).content_type = (char *) handlerPhpContext->contentType;
    SG(request_info).query_string = (char *) handlerPhpContext->query;
    SG(request_info).request_method = handlerPhpContext->method;
    SG(request_info).proto_num = 1001;
    SG(request_info).request_uri = (char *) handlerPhpContext->url;
    SG(request_info).path_translated = (char *) handlerPhpContext->filePath;
    SG(request_info).content_length = handlerPhpContext->contentLength;

    /* updates the global values with the previously defined (static)
    values the server context is set as one to allow correct php running */
    SG(global_request_time) = 0;
    SG(read_post_bytes) = 1;
    SG(server_context) = (void *) 1;

    /* raises no error */
    RAISE_NO_ERROR;
}
