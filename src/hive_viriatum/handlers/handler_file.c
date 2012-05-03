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
    handlerFileContext->cacheControlStatus = 0;
    handlerFileContext->etagStatus = 0;

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

    /* in case there is a template handler defined
    in the handler file context */
    if(handlerFileContext->templateHandler) {
        /* deletes the template handler (releases memory) */
        deleteTemplateHandler(handlerFileContext->templateHandler);
    }

    /* releases the handler file context */
    FREE(handlerFileContext);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE registerHandlerFile(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* creates the http handler */
    service->createHttpHandler(service, &httpHandler, (unsigned char *) "file");

    /* sets the http handler attributes */
    httpHandler->set = setHandlerFile;
    httpHandler->unset = unsetHandlerFile;
    httpHandler->reset = resetHandlerFile;

    /* adds the http handler to the service */
    service->addHttpHandler(service, httpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregisterHandlerFile(struct Service_t *service) {
    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* retrieves the http handler from the service, then
    remove it from the service after that delete the handler */
    service->getHttpHandler(service, &httpHandler, (unsigned char *) "file");
    service->removeHttpHandler(service, httpHandler);
    service->deleteHttpHandler(service, httpHandler);

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

ERROR_CODE resetHandlerFile(struct HttpConnection_t *httpConnection) {
    /* resets the http parser values */
    _resetHttpParserHandlerFile(httpConnection->httpParser);

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

    /* retrieves the connection from the http parser parameters and then
    uses it to access the service options using the service */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;
    struct Service_t *service = connection->service;
    struct ServiceOptions_t *options = service->options;

    /* allocates the required space for the url */
    unsigned char *url = (unsigned char *) MALLOC(dataSize + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, dataSize);

    /* puts the end of string in the url */
    url[dataSize] = '\0';

    /* prints a debug message */
    V_INFO_F("%s %s\n", getHttpMethodString(httpParser->method), url);

    /* in case the string refers the base path (default handler must be used)
    the selection of the index file as defautl is conditioned by the default
    index configuration option */
    if(options->defaultIndex && (strcmp((char *) url, "/") == 0 || strcmp((char *) url, "") == 0)) {
        /* reallocates the space for the index reference */
        url = (unsigned char *) REALLOC(url, 12);

        /* copies the index reference as the url */
        memcpy(url, "/index.html", 12);
    }

    /* copies the url to the url reference in the handler file context */
    memcpy(handlerFileContext->url, url, strlen((char *) url) + 1);

    /* creates the file path from using the base viriatum path */
    SPRINTF((char *) handlerFileContext->filePath, VIRIATUM_MAX_PATH_SIZE, "%s%s%s", VIRIATUM_CONTENTS_PATH, VIRIATUM_BASE_PATH, url);

    /* releases the url */
    FREE(url);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerFieldCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* TODO: THIS SHOULD BE A BETTER PARSER STRUCTURE */

    /* checks for the if none match header value */
    switch(dataSize) {
        case 13:
            if(data[0] == 'I' && data[1] == 'f' && data[2] == '-' && data[3] == 'N') {
                /* updates the etag status value (is next) */
                handlerFileContext->etagStatus = 1;

                /* breaks the switch */
                break;
            }

            if(data[0] == 'C' && data[1] == 'a' && data[2] == 'c' && data[3] == 'h') {
                /* updates the cache control status value (is next) */
                handlerFileContext->cacheControlStatus = 1;

                /* breaks the switch */
                break;
            }

            /* breaks the switch */
            break;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headerValueCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* TODO: THIS SHOULD BE A SWITCH */
    if(handlerFileContext->etagStatus == 1) {
        memcpy(handlerFileContext->etag, data, 10);
        handlerFileContext->etag[10] = '\0';
        handlerFileContext->etagStatus = 2;

        RAISE_NO_ERROR;
    }

    if(handlerFileContext->cacheControlStatus == 1) {
        memcpy(handlerFileContext->cacheControl, data, dataSize);
        handlerFileContext->cacheControl[dataSize] = '\0';
        handlerFileContext->cacheControlStatus = 2;

        RAISE_NO_ERROR;
    }

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
    struct LinkedList_t *directoryEntriesMap;
    struct TemplateHandler_t *templateHandler;

    /* allocates space for the is directory and the is redirect flags */
    unsigned int isDirectory = 0;
    unsigned int isRedirect = 0;

    /* allocates space for the new location value for
    redirect request cases and for the path to the
    template (for directory listing) */
    unsigned char location[1024];
    unsigned char templatePath[1024];

    /* allocates space for the computation of the time
    and of the time string, then allocates space for the
    etag calculation structure (crc32 value) and for the etag*/
    struct DateTime_t time;
    char timeString[20];
    unsigned long crc32Value;
    char etag[11];

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
    isDirectoryFile((char *) handlerFileContext->filePath, &isDirectory);

    /* in case the file path being request referes a directory
    it must be checked and the entries retrieved to be rendered */
    if(isDirectory) {
        if(handlerFileContext->url[strlen((char *) handlerFileContext->url) - 1] != '/') {
            /* creates the new location by adding the slash character to the current
            handler file context url (avoids directory confusion) */
            memcpy(location, handlerFileContext->url, strlen((char *) handlerFileContext->url));
            location[strlen((char *) handlerFileContext->url)] = '/';
            location[strlen((char *) handlerFileContext->url) + 1] = '\0';

            /* sets the is redirect flag (forces temporary redirect) */
            isRedirect = 1;
        } else {
            /* creates the complete path to the template file */
            SPRINTF((char *) templatePath, 1024, "%s%s", VIRIATUM_RESOURCES_PATH, VIRIATUM_LISTING_PATH);

            /* creates the directory entries (linked list) */
            createLinkedList(&directoryEntries);

            /* lists the directory file into the directory
            entries linked list and then converts them into maps */
            listDirectoryFile((char *) handlerFileContext->filePath, directoryEntries);
            entriesToMapFile(directoryEntries, &directoryEntriesMap);

            /* creates the template handler */
            createTemplateHandler(&templateHandler);

            /* assigns the directory entries to the template handler */
            assignListTemplateHandler(templateHandler, (unsigned char *) "entries", directoryEntriesMap);
            assignIntegerTemplateHandler(templateHandler, (unsigned char *) "items", (int) directoryEntriesMap->size);

            /* processes the file as a template handler */
            processTemplateHandler(templateHandler, templatePath);

            /* sets the template handler in the handler file context and unsets
            the flushed flag */
            handlerFileContext->templateHandler = templateHandler;
            handlerFileContext->flushed = 0;

            /* deletes the directory entries map and the
            directory entries */
            deleteDirectoryEntriesMapFile(directoryEntriesMap);
            deleteDirectoryEntriesFile(directoryEntries);

            /* deletes the directory entries (linked list) and
            the entries map (linked list) */
            deleteLinkedList(directoryEntries);
            deleteLinkedList(directoryEntriesMap);
        }
    }
    /* otherwise the file path must refered a "normal" file path and
    it must be checked */
    else {
        /* counts the total size (in bytes) of the contents in the file path */
        errorCode = countFile((char *) handlerFileContext->filePath, &fileSize);

        /* in case there is no error count the file size, avoids
        extra problems while computing the etag */
        if(!IS_ERROR_CODE(errorCode)) {
            /* resets the date time structure to avoid invalid
            date requests */
            memset(&time, 0, sizeof(struct DateTime_t));

            /* retrieve the time of the last write in the file path */
            getWriteTimeFile((char *) handlerFileContext->filePath, &time);

            /* creates the date time string for the file entry */
            SPRINTF(timeString, 20, "%04d-%02d-%02d %02d:%02d:%02d", time.year, time.month, time.day, time.hour, time.minute, time.second);

            /* creates the crc32 value and prints it into the
            etag as an heexadecimal string value */
            crc32Value = crc32((unsigned char *) timeString, 19);
            SPRINTF(etag, 11, "\"%08x\"", (unsigned int) crc32Value);
        }
    }

    /* sets the (http) flags in the handler file context */
    handlerFileContext->flags = httpParser->flags;

    /* tests the error code for error */
    if(IS_ERROR_CODE(errorCode)) {
        /* prints the error */
        V_DEBUG_F("%s\n", getLastErrorMessageSafe());

        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 404 Not Found\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: %lu\r\n\r\n404 - Not Found (%s)", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) strlen((char *) handlerFileContext->filePath) + 18, handlerFileContext->filePath);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, (unsigned int) strlen(headersBuffer), _cleanupHandlerFile, handlerFileContext);
    } else if(isRedirect) {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 307 Temporary Redirect\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: 0\r\nLocation: %s\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, location);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, (unsigned int) strlen(headersBuffer), _cleanupHandlerFile, handlerFileContext);
    }
    /* in case the current situation is a directory list */
    else if(isDirectory) {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) strlen((char *) handlerFileContext->templateHandler->stringValue));

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, (unsigned int) strlen(headersBuffer), _sendDataHandlerFile, handlerFileContext);
    }
    else if(handlerFileContext->etagStatus == 2 && strcmp(etag, (char *) handlerFileContext->etag) == 0) {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 304 Not Modified\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: 0\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        writeConnection(connection, (unsigned char *) headersBuffer, (unsigned int) strlen(headersBuffer), _cleanupHandlerFile, handlerFileContext);
    }
    /* otherwise there was no error in the file and it's a simple
    file situation (no directory) */
    else {
        /* writes the http static headers to the response */
        SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nETag: %s\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, etag, (long unsigned int) fileSize);

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

    /* resets both the etag and the cache control status */
    handlerFileContext->etagStatus = 0;
    handlerFileContext->cacheControlStatus = 0;

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
        /* deletes the template handler (releases memory) and
        unsets the reference in the handler file context */
        deleteTemplateHandler(templateHandler);
        handlerFileContext->templateHandler = NULL;

        /* runs the cleanup handler file (releases internal structures) */
        _cleanupHandlerFile(connection, data, parameters);
    }
    /* otherwise the "normal" write connection applies */
    else {
        /* writes the (file) data to the connection and sets the handler
        file context as flushed */
        writeConnection(connection, templateHandler->stringValue, (unsigned int) strlen((char *) templateHandler->stringValue), _sendDataHandlerFile, handlerFileContext);
        handlerFileContext->flushed = 1;

        /* unsets the string value in the template handler (avoids double release) */
        templateHandler->stringValue = NULL;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
