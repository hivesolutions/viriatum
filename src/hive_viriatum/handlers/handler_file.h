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

#include "../io/io.h"
#include "../http/http.h"
#include "../system/system.h"

#define FILE_BUFFER_SIZE_HANDLER_FILE 4096

/**
 * The context structure to be used allong
 * the interpretation of the request for
 * the file handler.
 */
typedef struct HandlerFileContext_t {
    unsigned char filePath[1024];
    FILE *file;
    unsigned char flags;
} HandlerFileContext;

void createHandlerFileContext(struct HandlerFileContext_t **handlerFileContextPointer);
void deleteHandlerFileContext(struct HandlerFileContext_t *handlerFileContext);
void setHandlerFile(struct HttpParser_t *httpParser, struct HttpSettings_t *httpSettings);
void unsetHandlerFile(struct HttpParser_t *httpParser, struct HttpSettings_t *httpSettings);
void setHttpParserHandlerFile(struct HttpParser_t *httpParser);
void unsetHttpParserHandlerFile(struct HttpParser_t *httpParser);
void setHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings);
void unsetHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings);
ERROR_CODE messageBeginCallbackHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE urlCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerFieldCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerValueCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headersCompleteCallbackHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE bodyCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE messageCompleteCallbackHandlerFile(struct HttpParser_t *httpParser);
ERROR_CODE _cleanupHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameters);
ERROR_CODE _sendChunkHandlerFile(struct Connection_t *connection, struct Data_t *data, void *parameter);
