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

ERROR_CODE createModLuaHttpHandler(struct ModLuaHttpHandler_t **modLuaHttpHandlerPointer, struct HttpHandler_t *httpHandler) {
    /* retrieves the mod lua http handler size */
    size_t modLuaHttpHandlerSize = sizeof(struct ModLuaHttpHandler_t);

    /* allocates space for the mod lua http handler */
    struct ModLuaHttpHandler_t *modLuaHttpHandler = (struct ModLuaHttpHandler_t *) MALLOC(modLuaHttpHandlerSize);

    /* sets the mod lua http handler attributes (default) values */
    modLuaHttpHandler->luaState = NULL;

    /* sets the mod lua http handler in the upper http handler substrate */
    httpHandler->lower = (void *) modLuaHttpHandler;

    /* sets the mod lua http handler in the mod lua http handler pointer */
    *modLuaHttpHandlerPointer = modLuaHttpHandler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteModLuaHttpHandler(struct ModLuaHttpHandler_t *modLuaHttpHandler) {
    /* releases the mod lua http handler */
    FREE(modLuaHttpHandler);

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
    /* sets the http settings on message begin callback */
    httpSettings->onmessageBegin = messageBeginCallbackHandlerModule;

    /* sets the http settings on url callback */
    httpSettings->onurl = urlCallbackHandlerModule;

    /* sets the http settings on header field callback */
    httpSettings->onheaderField = headerFieldCallbackHandlerModule;

    /* sets the http settings on header value callback */
    httpSettings->onheaderValue = headerValueCallbackHandlerModule;

    /* sets the http settings on headers complete callback */
    httpSettings->onheadersComplete = headersCompleteCallbackHandlerModule;

    /* sets the http settings on body callback */
    httpSettings->onbody = bodyCallbackHandlerModule;

    /* sets the http settings on message complete callback */
    httpSettings->onmessageComplete = messageCompleteCallbackHandlerModule;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpSettingsHandlerModule(struct HttpSettings_t *httpSettings) {
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

ERROR_CODE _sendResponseHandlerModule(struct HttpParser_t *httpParser) {
    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ((struct IoConnection_t *) connection->lower)->lower;

    struct ModLuaHttpHandler_t *modLuaHttpHandler = (struct ModLuaHttpHandler_t *) httpConnection->httpHandler->lower;

    #ifdef VIRIATUM_PLATFORM_WIN32
    char *filePath = "c:/teste.lua";
    #endif

    #ifdef VIRIATUM_PLATFORM_UNIX
    char *filePath = "/teste.lua";
    #endif

    ERROR_CODE resultCode;

    /* registers the current connection in lua */
    lua_pushlightuserdata(modLuaHttpHandler->luaState, (void *) httpParser);
    lua_setglobal(modLuaHttpHandler->luaState, "connection");

    /* registers the write connection function in lua */
    lua_register(modLuaHttpHandler->luaState, "write_connection", _luaWriteConnection);

    /* runs the script */
    resultCode = luaL_dofile(modLuaHttpHandler->luaState, filePath);

    /* in case there was an error in lua */
    if(LUA_ERROR(resultCode)) {
        /* prints a warning message */
        V_WARNING_F("There was a problem executing: %s\n" , filePath);

        /* cleanup lua */
        lua_close(modLuaHttpHandler->luaState);

        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem executing script file");
    }

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

int _luaWriteConnection(lua_State *luaState) {
    /* allocates the data (original message data) */
    const char *data;

    /* allocates the (final) buffer to send the message */
    unsigned char *buffer;

    /* allocates the data size */
    unsigned int dataSize;

    /* allocates the http parser */
    struct HttpParser_t *httpParser;

    /* allocates the connection */
    struct Connection_t *connection;

    /* retrieves the number of (received) arguments */
    unsigned int numberArguments = lua_gettop(luaState);

    /* in case the number of arguments is invalid */
    if(numberArguments != 2) {
        /* prints a warning message */
        V_WARNING("Invalid number of arguments\n");

        /* pushes an error message to lua */
        lua_pushstring(luaState, "Invalid number of arguments");
        lua_error(luaState);
    }

    if(!lua_isstring(luaState, -1)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected string'\n");

        /* pushes an error message to lua */
        lua_pushstring(luaState, "Incorrect argument to 'expected string'");
        lua_error(luaState);
    }

    if(!lua_islightuserdata(luaState, 1)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected lightuserdata'\n");

        lua_pushstring(luaState, "Incorrect argument 'expected lightuserdata'");
        lua_error(luaState);
    }

    /* converts the second argument into a string */
    data = lua_tostring(luaState, -1);

    /* converts the first argument into http parser structure */
    httpParser = (struct HttpParser_t *) lua_touserdata(luaState, -2);

    /* retrieves the data (string) size */
    dataSize = strlen(data);

    /* retrieves the connection from the http parser parameters */
    connection = (struct Connection_t *) httpParser->parameters;

    /* allocates the data buffer (in a safe maner) */
    connection->allocData(connection, dataSize * sizeof(unsigned char), (void **) &buffer);

    /* copies the data (from lua) into a buffer */
    memcpy(buffer, data, dataSize);

    /* writes the response to the connection */
    connection->writeConnection(connection, (unsigned char *) buffer, dataSize, _sendResponseCallbackHandlerModule, (void *) httpParser);

    /* return the number of results */
    return 0;
}
