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

#include "../system/system.h"

typedef struct DispatchHandler_t {
    /**
     * The array of regular expressions that are meant
     * to be executed in the ordered defined for execution.
     * In case this array grows too much the performance
     * of the server may lower significantly.
     */
    pcre **regex;

    /**
     * The arrays of (handler) names in the same order as
     * the corresponding regular expressions, they are going
     * to be used at runtime for handler resolution.
     */
    unsigned char **names;

    /**
     * The number of regular expressions curently present
     * in the dispatch handler.
     */
    size_t regexCount;
} DispatchHandler;

ERROR_CODE createDispatchHandler(struct DispatchHandler_t **dispatchHandlerPointer, struct HttpHandler_t *httpHandler);
ERROR_CODE deleteDispatchHandler(struct DispatchHandler_t *dispatchHandler);
ERROR_CODE registerHandlerDispatch(struct Service_t *service);
ERROR_CODE unregisterHandlerDispatch(struct Service_t *service);
ERROR_CODE setHandlerDispatch(struct HttpConnection_t *httpConnection);
ERROR_CODE unsetHandlerDispatch(struct HttpConnection_t *httpConnection);
ERROR_CODE resetHandlerDispatch(struct HttpConnection_t *httpConnection);
ERROR_CODE messageBeginCallbackHandlerDispatch(struct HttpParser_t *httpParser);
ERROR_CODE urlCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerFieldCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerValueCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headersCompleteCallbackHandlerDispatch(struct HttpParser_t *httpParser);
ERROR_CODE bodyCallbackHandlerDispatch(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE messageCompleteCallbackHandlerDispatch(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpParserHandlerDispatch(struct HttpParser_t *httpParser);
ERROR_CODE _unsetHttpParserHandlerDispatch(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpSettingsHandlerDispatch(struct HttpSettings_t *httpSettings);
ERROR_CODE _unsetHttpSettingsHandlerDispatch(struct HttpSettings_t *httpSettings);
