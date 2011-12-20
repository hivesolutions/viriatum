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

ERROR_CODE registerHandlerDefault(struct Service_t *service);
ERROR_CODE setHandlerDefault(struct HttpConnection_t *httpConnection);
ERROR_CODE unsetHandlerDefault(struct HttpConnection_t *httpConnection);
ERROR_CODE messageBeginCallbackHandlerDefault(struct HttpParser_t *httpParser);
ERROR_CODE urlCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerFieldCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerValueCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headersCompleteCallbackHandlerDefault(struct HttpParser_t *httpParser);
ERROR_CODE bodyCallbackHandlerDefault(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE messageCompleteCallbackHandlerDefault(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpParserHandlerDefault(struct HttpParser_t *httpParser);
ERROR_CODE _unsetHttpParserHandlerDefault(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpSettingsHandlerDefault(struct HttpSettings_t *httpSettings);
ERROR_CODE _unsetHttpSettingsHandlerDefault(struct HttpSettings_t *httpSettings);
ERROR_CODE _sendResponseHandlerDefault(struct HttpParser_t *httpParser);
ERROR_CODE _sendResponseCallbackHandlerDefault(struct Connection_t *connection, struct Data_t *data, void *parameters);
