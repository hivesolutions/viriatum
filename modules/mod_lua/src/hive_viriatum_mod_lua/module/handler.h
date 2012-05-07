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

#pragma once

#include "entry.h"

#define LUA_ERROR(error) error > 0

/**
 * The structure that holds the internal
 * structure to support the context
 * of the lua module.
 */
typedef struct ModLuaHttpHandler_t {
    /**
     * The global lua state used over
     * all the operations in lua.
     */
    lua_State *luaState;

    /**
     * The path to the default file to
     * be used for the parsing.
     */
    char *filePath;

    /**
     * Flag that controls if the script file
     * to be executed is currently dirty.
     */
    unsigned int fileDirty;
} ModLuaHttpHandler;

ERROR_CODE createModLuaHttpHandler(struct ModLuaHttpHandler_t **modLuaHttpHandlerPonter, struct HttpHandler_t *httpHandlerPonter);
ERROR_CODE deleteModLuaHttpHandler(struct ModLuaHttpHandler_t *modLuaHttpHandler);
ERROR_CODE setHandlerModule(struct HttpConnection_t *httpConnection);
ERROR_CODE unsetHandlerModule(struct HttpConnection_t *httpConnection);
ERROR_CODE urlCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerFieldCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headerValueCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE headersCompleteCallbackHandlerModule(struct HttpParser_t *httpParser);
ERROR_CODE bodyCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize);
ERROR_CODE messageCompleteCallbackHandlerModule(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpParserHandlerModule(struct HttpParser_t *httpParser);
ERROR_CODE _unsetHttpParserHandlerModule(struct HttpParser_t *httpParser);
ERROR_CODE _setHttpSettingsHandlerModule(struct HttpSettings_t *httpSettings);
ERROR_CODE _unsetHttpSettingsHandlerModule(struct HttpSettings_t *httpSettings);
ERROR_CODE _messageBeginCallbackHandlerModule(struct HttpParser_t *httpParser);
ERROR_CODE _sendResponseHandlerModule(struct HttpParser_t *httpParser);
ERROR_CODE _sendResponseCallbackHandlerModule(struct Connection_t *connection, struct Data_t *data, void *parameters);
ERROR_CODE _writeErrorConnection(struct HttpParser_t *httpParser, char *message);
int _luaWriteConnection(lua_State *luaState);
