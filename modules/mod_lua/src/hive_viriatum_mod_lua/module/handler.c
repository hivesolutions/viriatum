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
    modLuaHttpHandler->filePath = NULL;
    modLuaHttpHandler->fileDirty = 0;

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
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerModule(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
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
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unsetHttpParserHandlerModule(struct HttpParser_t *httpParser) {
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

ERROR_CODE _sendResponseHandlerModule(struct HttpParser_t *httpParser) {
    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* retrieves the http connection from the io connection and uses it to retrieve
    the correct (mod lua) handler to operate around it */
    struct HttpConnection_t *httpConnection = (struct HttpConnection_t *) ((struct IoConnection_t *) connection->lower)->lower;
    struct ModLuaHttpHandler_t *modLuaHttpHandler = (struct ModLuaHttpHandler_t *) httpConnection->httpHandler->lower;

    /* allocates space for the result code */
    ERROR_CODE resultCode;

    /* in case the lua state is not started an error must
    have occured so need to return immediately in error */
    if(modLuaHttpHandler->luaState == NULL) {
        /* writes the error to the connection and then returns
        in error to the caller function */
        _writeErrorConnection(httpParser, "no lua state available");
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem accessing lua state");
    }

    /* registers the current connection in lua */
    lua_pushlightuserdata(modLuaHttpHandler->luaState, (void *) httpParser);
    lua_setglobal(modLuaHttpHandler->luaState, "connection");

    /* registers the write connection function in lua */
    lua_register(modLuaHttpHandler->luaState, "write_connection", _luaWriteConnection);

    /* runs the script in case the current file is considered to be
    dirty, this is the case for the loading of the module and reloading*/
    if(modLuaHttpHandler->fileDirty) { resultCode = luaL_dofile(modLuaHttpHandler->luaState, modLuaHttpHandler->filePath); modLuaHttpHandler->fileDirty = 0; }
    else { resultCode = 0; }

    /* in case there was an error in lua */
    if(LUA_ERROR(resultCode)) {
        /* retrieves the error message and then writes it to the connection
        so that the end user may be able to respond to it */
        char *errorMessage = (char *) lua_tostring(modLuaHttpHandler->luaState, -1);
        _writeErrorConnection(httpParser, errorMessage);

        /* sets the file as dirty (forces reload) and then reloads the curernt
        internal lua state, virtual machine reset (to avoid corruption) */
        modLuaHttpHandler->fileDirty = 1;
        _reloadLuaState(&modLuaHttpHandler->luaState);

        /* prints a warning message, closes the lua interpreter and then
        raises the error to the upper levels */
        V_WARNING_F("There was a problem executing: %s\n", modLuaHttpHandler->filePath);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem executing script file");
    }

    /* retrieves the field reference to the handle method that symbol
    to call the function (in protected mode) and retrieves the result */
    lua_getfield(modLuaHttpHandler->luaState, LUA_GLOBALSINDEX, "handle");
    resultCode = lua_pcall(modLuaHttpHandler->luaState, 0, 0, 0);

    /* in case there was an error in lua */
    if(LUA_ERROR(resultCode)) {
        /* retrieves the error message and then writes it to the connection
        so that the end user may be able to respond to it */
        char *errorMessage = (char *) lua_tostring(modLuaHttpHandler->luaState, -1);
        _writeErrorConnection(httpParser, errorMessage);

        /* sets the file reference as dirty, this will force the script file
        to be reload on next request */
        modLuaHttpHandler->fileDirty = 1;

        /* prints a warning message, closes the lua interpreter and then
        raises the error to the upper levels */
        V_WARNING_F("There was a problem running call on file: %s\n", modLuaHttpHandler->filePath);
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem calling the handle method");
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _sendResponseCallbackHandlerModule(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* retrieves the current http flags */
    unsigned char flags = (unsigned char) (size_t) parameters;

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
    if(!(flags & FLAG_CONNECTION_KEEP_ALIVE)) {
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
    connection->writeConnection(connection, buffer, (unsigned int) strlen((char *) buffer), _sendResponseCallbackHandlerModule, (void *) (size_t) httpParser->flags);

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
    if(numberArguments != 3) {
        /* prints a warning message */
        V_WARNING("Invalid number of arguments\n");

        /* pushes an error message to lua */
        lua_pushstring(luaState, "Invalid number of arguments");
        lua_error(luaState);
    }

    if(!lua_isnumber(luaState, -1)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected number'\n");

        /* pushes an error message to lua */
        lua_pushstring(luaState, "Incorrect argument to 'expected number'");
        lua_error(luaState);
    }

    if(!lua_isstring(luaState, -2)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected string'\n");

        /* pushes an error message to lua */
        lua_pushstring(luaState, "Incorrect argument to 'expected string'");
        lua_error(luaState);
    }

    if(!lua_islightuserdata(luaState, -3)) {
        /* prints a warning message */
        V_WARNING("Incorrect argument 'expected lightuserdata'\n");

        /* pushes an error message to lua */
        lua_pushstring(luaState, "Incorrect argument 'expected lightuserdata'");
        lua_error(luaState);
    }

    /* converts the third argument into an integer */
    dataSize = lua_tointeger(luaState, -1);

    /* converts the second argument into a string */
    data = lua_tostring(luaState, -2);

    /* converts the first argument into http parser structure */
    httpParser = (struct HttpParser_t *) lua_touserdata(luaState, -3);

    /* retrieves the connection from the http parser parameters */
    connection = (struct Connection_t *) httpParser->parameters;

    /* allocates the data buffer (in a safe maner) then
    copies the data (from lua) into the buffer */
    connection->allocData(connection, dataSize * sizeof(unsigned char), (void **) &buffer);
    memcpy(buffer, data, dataSize);

    /* writes the response to the connection */
    connection->writeConnection(connection, buffer, dataSize, _sendResponseCallbackHandlerModule, (void *) (size_t) httpParser->flags);

    /* return the number of results */
    return 0;
}
