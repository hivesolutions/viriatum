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

void updateHandlerFile(struct HttpParser_t *httpParser, struct HttpSettings_t *httpSettings) {
    /* updates the http parser values */
    updateHttpParserHandlerFile(httpParser);

    /* updates the http settings values */
    updateHttpSettingsHandlerFile(httpSettings);
}

void updateHttpParserHandlerFile(struct HttpParser_t *httpParser) {
    /* retrieves the handler file context size */
    size_t handlerFileContextSize = sizeof(struct HandlerFileContext_t);

    /* allocates space for the handler file context */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) malloc(handlerFileContextSize);

	handlerFileContext->file = NULL;

	handlerFileContext->filePath = NULL;

    /* sets the handler file context as the context for the http parser */
    httpParser->context = handlerFileContext;
}

void updateHttpSettingsHandlerFile(struct HttpSettings_t *httpSettings) {
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
}

ERROR_CODE messageBeginCallbackHandlerFile(struct HttpParser_t *httpParser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE urlCallbackHandlerFile(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    /* allocates the file path */
    unsigned char *filePath = (unsigned char *) malloc(1024);

    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* allocates the required space for the url */
    unsigned char *url = (unsigned char *) malloc(dataSize + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, dataSize);

    /* puts the end of strng in the url */
    url[dataSize] = '\0';

    /* creates the file path from using the base viriatum path */
    SPRINTF((char *) filePath, 1024, "%s%s%s", RESOURCES_PATH, "/html/welcome", url);

    /* sets the file path in the handler file context */
    handlerFileContext->filePath = filePath;

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

    /* allocates the headers buffer */
    char headersBuffer[1024];

	char *buffer;
	char *buffer2 = malloc(1000000);

    /* retrieves the handler file context from the http parser */
    struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) httpParser->context;

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) httpParser->parameters;

    /* reads (the complete) file contents */
    ERROR_CODE readFileResult = countFile((char *) handlerFileContext->filePath, &fileSize);

    /* tests the error code for error */
    if(IS_ERROR_CODE(readFileResult)) {
        /* prints the error */
        V_DEBUG_F("%s\n", getLastErrorMessageSafe());
    }
    /* otherwise there was no error in the file */
    else {
        /* writes the http static headers to the response */
		SPRINTF(headersBuffer, 1024, "HTTP/1.1 200 OK\r\nServer: viriatum/1.0.0 (%s - %s)\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, fileSize);

        /* writes both the headers and the file buffer to the connection */
        /*writeConnection(connection, (unsigned char *) headersBuffer, strlen(headersBuffer), sendChunkHandlerFile, handlerFileContext);*/

		/* TODO: WE NEED THIS HACK IN ORDER TO AVOID THE BROWSER BUG WITH NO LOADING */
		/* USING NON BLOCKING SOCKET WE'LL PROBABLY BE ABLE TO OVERCOME THIS PROBLEM */
		readFile(handlerFileContext->filePath, &buffer, &fileSize);

		memcpy(buffer2, headersBuffer, strlen(headersBuffer));
		memcpy(buffer2 + strlen(headersBuffer), buffer, fileSize);

		writeConnection(connection, buffer2, strlen(headersBuffer) + fileSize, NULL, NULL);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE sendChunkHandlerFile(struct Connection_t *connection, void *parameters) {
	/* allocates the number of bytes */
	size_t numberBytes;

	/* casts the parameters as handler file context */
	struct HandlerFileContext_t *handlerFileContext = (struct HandlerFileContext_t *) parameters;

	/* retrieves the file path from the handler file context */
	unsigned char *filePath = handlerFileContext->filePath;

	/* retrieves the file from the handler file context */
	FILE *file = handlerFileContext->file;


	/* TODO ARRUMAR ESTE HARDCODE !!!! */
	unsigned char *fileBuffer = malloc(FILE_BUFFER_SIZE_HANDLER_FILE);


	/* in case the file is not defined (should be opened) */
	if(file == NULL) {
		/* opens the file */
		FOPEN(&file, (char *) filePath, "rb");

		/* in case the file is not found */
		if(file == NULL) {
			/* raises an error */
			RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
		}
	}

	/* reads the file contents */
    numberBytes = fread(fileBuffer, 1, FILE_BUFFER_SIZE_HANDLER_FILE, file);

	/* in case the number of read bytes is valid */
	if(numberBytes > 0) {
		/* writes both the headers and the file buffer to the connection */
		writeConnection(connection, fileBuffer, numberBytes, sendChunkHandlerFile, handlerFileContext);
	} else {
		/* closes the file */
		fclose(file);
	}

	/* sets the file in the handler file context */
	handlerFileContext->file = file;

    /* raise no error */
    RAISE_NO_ERROR;
}

