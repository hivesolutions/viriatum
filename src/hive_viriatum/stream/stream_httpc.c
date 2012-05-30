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

#include "stream_httpc.h"

ERROR_CODE bodyCallbackHandlerClient(struct HttpParser_t *httpParser, const unsigned char *data, size_t dataSize) {
    struct Type_t *type;

    decodeBencoding((unsigned char *) data, dataSize, &type);
    printType(type);
    freeType(type);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE messageCompleteCallbackHandlerClient(struct HttpParser_t *httpParser) {
    /* raises no error */
    RAISE_NO_ERROR;
}




ERROR_CODE createHttpClientConnection(struct HttpClientConnection_t **httpClientConnectionPointer, struct IoConnection_t *ioConnection) {
    /* retrieves the http client connection size */
    size_t httpClientConnectionSize = sizeof(struct HttpClientConnection_t);

    /* allocates space for the http client connection */
    struct HttpClientConnection_t *httpClientConnection = (struct HttpClientConnection_t *) MALLOC(httpClientConnectionSize);

    /* sets the http handler attributes (default) values */
    httpClientConnection->ioConnection = ioConnection;

    /* creates the http settings */
    createHttpSettings(&httpClientConnection->httpSettings);

    /* creates the http parser (for a response) */
    createHttpParser(&httpClientConnection->httpParser, 0);

    /* sets the default callback functions in the http settings
    these function are going to be called by the parser */
    httpClientConnection->httpSettings->onbody = bodyCallbackHandlerClient;
    httpClientConnection->httpSettings->onmessageComplete = messageCompleteCallbackHandlerClient;

    /* sets the connection as the parser parameter(s) */
    httpClientConnection->httpParser->parameters = ioConnection->connection;

    /* sets the http client connection in the (upper) io connection substrate */
    ioConnection->lower = httpClientConnection;

    /* sets the http client connection in the http client connection pointer */
    *httpClientConnectionPointer = httpClientConnection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteHttpClientConnection(struct HttpClientConnection_t *httpClientConnection) {
    /* deletes the http parser */
    deleteHttpParser(httpClientConnection->httpParser);

    /* deletes the http settings */
    deleteHttpSettings(httpClientConnection->httpSettings);

    /* releases the http client connection */
    FREE(httpClientConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE dataHandlerStreamHttpClient(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize) {
    /* allocates space for the temporary variable to
    hold the ammount of bytes processed in a given http
    data parsing iteration */
    int processedSize;

    /* retrieves the http client connection */
    struct HttpClientConnection_t *httpClientConnection = (struct HttpClientConnection_t *) ioConnection->lower;

    /* process the http data for the http parser, this should be
    a partial processing and some data may remain unprocessed (in
    case there are multiple http requests) */
    processedSize = processDataHttpParser(httpClientConnection->httpParser, httpClientConnection->httpSettings, buffer, bufferSize);

    printf("%d", processedSize);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE randomBuffer(unsigned char *buffer, size_t bufferSize) {
	size_t index;
	time_t seconds;
	int random;
	unsigned char byte;

	time(&seconds);
	srand((unsigned int) seconds);

	for(index = 0; index < bufferSize; index++) {
		random = rand();
		byte = (unsigned char) (random % 94);
		buffer[index] = byte + 34;
	}

    /* raises no error */
    RAISE_NO_ERROR;
}


ERROR_CODE openHandlerStreamHttpClient(struct IoConnection_t *ioConnection) {
    /* allocates the http client connection and retrieves the
    "upper" connection (for parameters retrieval) */
    struct HttpClientConnection_t *httpClientConnection;
    struct Connection_t *connection = (struct Connection_t *) ioConnection->connection;
    struct HttpClientParameters_t *parameters = (struct HttpClientParameters_t *) connection->parameters;

    struct Type_t *type;

    char *buffer = MALLOC(1024);

    struct Type_t *_type;
    char *_buffer;
    size_t _bufferSize;
    unsigned char infoHash[SHA1_DIGEST_SIZE];
	unsigned char random[12];
	unsigned char peerId[20];

	SPRINTF(peerId, 20, "-%s%d%d%d0-", VIRIATUM_PREFIX, VIRIATUM_MAJOR, VIRIATUM_MINOR, VIRIATUM_MICRO);
	randomBuffer(random, 12);
	memcpy(peerId + 8, random, 12);
	peerId[20] = '\0';

    decodeBencodingFile("C:/verysleepy_0_82.exe.torrent", &type);
    getValueStringSortMap(type->value.valueSortMap, "info", (void **) &_type);
    encodeBencoding(_type, &_buffer, &_bufferSize);
    sha1(_buffer, _bufferSize, infoHash);
    printType(type);
    freeType(type);
    FREE(_buffer);

	
	/* tenho de fazer gerador de get parameters !!!! */





    SPRINTF(buffer, 1024, "GET %s HTTP/1.1\r\nUser-Agent: viriatum/0.1.0 (linux - intel x64)\r\nConnection: keep-alive\r\n\r\n", parameters->url);


    /* creates the http client connection */
    createHttpClientConnection(&httpClientConnection, ioConnection);

    writeConnection(ioConnection->connection, (unsigned char *) buffer, strlen(buffer), NULL, NULL);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE closeHandlerStreamHttpClient(struct IoConnection_t *ioConnection) {
    /* retrieves the http client connection */
    struct HttpClientConnection_t *httpClientConnection = (struct HttpClientConnection_t *) ioConnection->lower;

    /* deletes the http client connection */
    deleteHttpClientConnection(httpClientConnection);

    /* raises no error */
    RAISE_NO_ERROR;
}
