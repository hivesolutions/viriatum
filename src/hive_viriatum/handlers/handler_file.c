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

#include "handler_file.h"

ERROR_CODE createHandlerFileContext(struct HandlerFileContext_t **handlerFileContextPointer) {
    /* retrieves the handler file context size */
    size_t handlerFileContextSize = sizeof(struct HandlerFileContext_t);

    /* allocates space for the handler file context */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) MALLOC(handlerFileContextSize);

    /* sets the handler file default values */
    handlerFileContext->file = NULL;
    handlerFileContext->flags = 0;
    handlerFileContext->templateHandler = NULL;

    /* sets the handler file context in the  pointer */
    *handlerFileContextPointer = handlerFileContext;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHandlerFileContext(struct HandlerFileContext_t *handlerFileContext) {
    /* in case there is a file defined in
    the handler file context */
    if(handlerFileContext->file != NULL) {
        /* closes the file */
        fclose(handlerFileContext->file);
    }

    /* releases the handler file context */
    FREE(handlerFileContext);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE setHandlerFile(struct HttpConnection_t *httpConnection) {
    /* sets the http parser values */
    _setHttpParserHandlerFile(httpConnection->httpParser);

    /* sets the http settings values */
    _setHttpSettingsHandlerFile(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unsetHandlerFile(struct HttpConnection_t *httpConnection) {
    /* unsets the http parser values */
    _unsetHttpParserHandlerFile(httpConnection->httpParser);

    /* unsets the http settings values */
    _unsetHttpSettingsHandlerFile(httpConnection->httpSettings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageBeginCallbackHandlerFile(struct HttpParser_t *httpParser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* allocates the required space for the url */
    unsigned char *url = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, dataSize);

    /* puts the end of string in the url */
    url[dataSize] = '\0';

    /* in case the string refers the base path (default handler must be used) */
    if(strcmp((char *) url, "/") == 0 || strcmp((char *) url, "") == 0) {
        /* reallocates the space for the index reference */
        url = (unsigned char *) REALLOC(url, 12);

        /* copies the index reference as the url */
        memcpy(url, "/index.html", 12);
    }

    /* copies the url to the url reference in the handler file context */
    memcpy(handlerFileContext->url, url, strlen(url) + 1);

    /* creates the file path from using the base viriatum path */
    SPRINTF((char *) handlerFileContext->filePath, 1024, "%s%s%s", VIRIATUM_CONTENTS_PATH, VIRIATUM_WELCOME_PATH, url);

    /* releases the url */
    FREE(url);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerValueCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headersCompleteCallbackHandlerFile(struct HttpParser_t *httpParser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE bodyCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageCompleteCallbackHandlerFile(struct HttpParser_t *httpParser) {
    /* allocates the file size */
    size_t fileSize;

    /* allocates space for the directory entries and for
    the template handler */
    struct LinkedList_t *directoryEntries;
    struct TemplateHandler_t *templateHandler;

    /* allocates space for the is directory and the is redirect flags */
    unsigned int isDirectory = 0;
    unsigned int isRedirect = 0;

    /* allocates space for the new location value for
    redirect request cases */
    unsigned char location[1024];

    /* allocates the space for the "read" result
    error code (valid by default) */
    ERROR_CODE errorCode = 0;

    /* allocates the headers buffer (it will be releases automatically by the writter) */
    char *headersBuffer = MALLOC(1024);

    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* checks if the path being request is in fact a directory */
    isDirectoryFile(handlerFileContext->filePath, &isDirectory);

    /* in case the file path being request referes a directory
    it must be checked and the entries retrieved to be rendered */
    if(isDirectory) {
        if(handlerFileContext->url[strlen(handlerFileContext->url) - 1] != '/') {
            /* creates the new location by adding the slash character to the current
            handler file context url (avoids directory confusion) */
            memcpy(location, handlerFileContext->url, strlen(handlerFileContext->url));
            location[strlen(handlerFileContext->url)] = '/';
            location[strlen(handlerFileContext->url) + 1] = '\0';

            /* sets the is redirect flag (forces temporary redirect) */
            isRedirect = 1;
        } else {
            /* creates the directory entries (linked list) */
            createLinkedList(&directoryEntries);

            /* lists the directory file into the directory
            entries linked list */
            listDirectoryFile(handlerFileContext->filePath, directoryEntries);

            /* creates the template handler */
            createTemplateHandler(&templateHandler);

            /* assigns the cirectory entries to the template handler */
            assignTemplateHandler(templateHandler, "entries", directoryEntries);

            /* processes the file as a template handler */
            processTemplateHandler(templateHandler, "C:\\repo_extra\\viriatum\\src\\hive_viriatum\\resources\\html\\welcome\\listing.html");

            /* sets the template handler in the handler file context and unsets
            the flushed flag */
            handlerFileContext->templateHandler = templateHandler;
            handlerFileContext->flushed = 0;

            /* deletes the directory entries (linked list) */
            deleteLinkedList(directoryEntries);
        }
    }
    /* otherwise the file path must refered a "normal" file path and
    it must be checked */
    else {
        /* counts the total size (in bytes) of the contents in the file path */
        errorCode = countFile((char *) handlerFileContext->filePath, &fileSize);
    }

    /* sets the (http) flags in the handler file context */
    handlerFileContext->flags = httpParser->flags;

    /* tests the error code for error */
    if(IS_ERROR_CODE(errorCode)) {
        /* prints the error */
        V_DEBUG_F("%s\n", getLastErrorMessageSafe());

        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 404 Not Found\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: 15\r\n\r\n404 - Not Found", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, strlen(headersBuffer), _cleanupHandlerFile, handlerFileContext);
    } else if(isRedirect) {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 307 Temporary Redirect\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nLocation: %s\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, location);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, strlen(headersBuffer), NULL, handlerFileContext);
    }
    /* in case the current situation is a directory list */
    else if(isDirectory) {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, strlen(handlerFileContext->templateHandler->stringValue));

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, strlen(headersBuffer), _sendDataHandlerFile, handlerFileContext);
    }
    /* otherwise there was no error in the file and it's a simple
    file situation (no directory) */
    else {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, fileSize);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, strlen(headersBuffer), _sendChunkHandlerFile, handlerFileContext);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpParserHandlerFile(struct HttpParser_t *httpParser) {
    /* allocates space for the handler file context */
    struct HandlerFileContext_t *handlerFileContext;

    /* creates the handler file context */
    createHandlerFileContext(&handlerFileContext);

    /* sets the handler file context as the context for the http parser */
    httpParser->context = handlerFileContext;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerFile(struct HttpParser_t *httpParser) {
    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* deletes the handler file context */
    deleteHandlerFileContext(handlerFileContext);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _resetHttpParserHandlerFile(struct HttpParser_t *httpParser) {
    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* unsets the handler file context file */
    handlerFileContext->file = NULL;

    /* unsets the handler file context flags */
    handlerFileContext->flags = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _setHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings) {
    /* sets the http settings on message begin callback */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerFile;

    /* sets the http settings on url callback */
    httpSettings->onurl = urlCallbackHandlerFile;

    /* sets the http settings on header field callback */
    httpSettings->onheaderField = headerFieldCallbackHandlerFile;

    /* sets the http settings on header value callback */
    httpSettings->onheaderValue = headerValueCallbackHandlerFile;

    /* sets the http settings on headers complete callback */
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerFile;

    /* sets the http settings on body callback */
    httpSettings->onbody = bodyCallbackHandlerFile;

    /* sets the http settings on message complete callback */
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerFile;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings) {
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

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _cleanupHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* casts the parameters as handler file context */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) parameters;

    /* in case the connection is not meant to be kept alive */
    if(!(handlerFileContext->flags & FLAG_CONNECTION_KEEP_ALIVE)) {
        /* closes the connection */
        connection->closeConnection(connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendChunkHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* allocates the number of bytes */
    size_t numberBytes;

    /* casts the parameters as handler file context */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) parameters;

    /* retrieves the file path from the handler file context */
    unsigned char *filePath = handlerFileContext->filePath;

    /* retrieves the file from the handler file context */
    FILE *file = handlerFileContext->file;

    /* allocates the required buffer for the file */
    unsigned char *fileBuffer = MALLOC(FILE_BUFFER_SIZE_HANDLER_FILE);

    /* in case the file is not defined (should be opened) */
    if(file == NULL) {
        /* opens the file */
        FOPEN(&file, (char *) filePath, "rb");

        /* in case the file is not found */
        if(file == NULL) {
            /* raises an error */
            RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
        }

        /* sets the file in the handler file context */
        handlerFileContext->file = file;
    }

    /* reads the file contents */
    numberBytes = fread(fileBuffer, 1, FILE_BUFFER_SIZE_HANDLER_FILE, file);

    /* in case the number of read bytes is valid */
    if(numberBytes > 0) {
        /* writes both the file buffer to the connection */
        writeConnection(connection, fileBuffer, numberBytes, _sendChunkHandlerFile, handlerFileContext);
    }
    /* otherwise the file "transfer" is complete */
    else {
        /* unsets the file from the handler file context */
        handlerFileContext->file = NULL;

        /* runs the cleanup handler file (releases internal structures) */
        _cleanupHandlerFile(connection, data, parameters);

        /* closes the file */
        fclose(file);

        /* releases the current file buffer */
        FREE(fileBuffer);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendDataHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* casts the parameters as handler file context and then
    retrieves the templat handler from it */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) parameters;
    struct TemplateHandler_t *templateHandler = handlerFileContext->templateHandler;

    /* in case the handler file context is already flushed
    time to clenaup pending structures */
    if(handlerFileContext->flushed) {
        /* deletes the template handler (releases memory) */
        deleteTemplateHandler(templateHandler);
    } else {
        /* writes the (file) data to the connection and sets the handler
        file context as flushed */
        writeConnection(connection, templateHandler->stringValue, strlen(templateHandler->stringValue), _sendDataHandlerFile, handlerFileContext);
        handlerFileContext->flushed = 1;

        /* unsets the string value in the template handler (avoids double release) */
        templateHandler->stringValue = NULL;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
