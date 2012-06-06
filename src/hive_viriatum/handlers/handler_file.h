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

#pragma once

#include "../http/http.h"
#include "../system/system.h"

/* forward references (avoids loop) */
struct Data_t;
struct Connection_t;
struct HttpConnection_t;

/**
 * The size of the file buffer to be used
 * durring a file transfer.
 * Increasing this value will allow the transfer
 * of bigger chunks.
 */
#define FILE_BUFFER_SIZE_HANDLER_FILE 4096

/**
 * The context structure to be used allong
 * the interpretation of the request for
 * the file handler.
 */
typedef struct HandlerFileContext_t {
    /**
     * The url to be used for retrieving the file.
     */
    unsigned char url[VIRIATUM_MAX_URL_SIZE];

    /**
     * The path to the file to be handled by
     * the current file request.
     */
    unsigned char file_path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The reference to the file stream to be
     * used in the file request.
     */
    FILE *file;

    /**
     * The flags to be used during the file
     * handling process.
     */
    unsigned char flags;

    /**
     * The template handler to be used for requests
     * that provide dynamic data, that must be
     * processed in the beginning of the workflows
     * (eg: listing the entries of a directory).
     */
    struct template_handler_t *template_handler;

    /**
     * The flag that controlls the flushing of the
     * internal structures of the file handler.
     */
    unsigned int flushed;

    unsigned char cacheControlStatus;
    unsigned char cacheControl[128];

    unsigned char etagStatus;
    unsigned char etag[11];
} HandlerFileContext;

ERROR_CODE createHandlerFileContext(struct HandlerFileContext_t **handlerFileContextPointer);
ERROR_CODE deleteHandlerFileContext(struct HandlerFileContext_t *handlerFileContext);
ERROR_CODE registerHandlerFile(struct Service_t *service);
ERROR_CODE unregisterHandlerFile(struct Service_t *service);
ERROR_CODE setHandlerFile(struct HttpConnection_t *httpConnection);
ERROR_CODE unsetHandlerFile(struct HttpConnection_t *httpConnection);
ERROR_CODE resetHandlerFile(struct HttpConnection_t *httpConnection);
ERROR_CODE messageBeginCallbackHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE urlCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerFieldCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerValueCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headersCompleteCallbackHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE bodyCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE messageCompleteCallbackHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpParserHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE _unsetHttpParserHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE _resetHttpParserHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings);
ERROR_CODE _unsetHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings);
ERROR_CODE _cleanupHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameters);
ERROR_CODE _sendChunkHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameter);
ERROR_CODE _sendDataHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameters);
