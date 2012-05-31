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

ERROR_CODE generateParameters(struct HashMap_t *hashMap, unsigned char **bufferPointer, size_t *bufferLengthPointer) {
    /* allocates space for an iterator object for an hash map element
    and for the string buffer to be used to collect all the partial
    strings that compose the complete url parameters string */
    struct Iterator_t *iterator;
    struct HashMapElement_t *element;
    struct StringBuffer_t *stringBuffer;

    /* allocates space for the string structure to hold the value of
    the element for the string value reference for the joined string,
    for the buffer string from the element and for the corresponding
    string lengths for both cases */
    struct String_t *string;
    unsigned char *stringValue;
    unsigned char *_buffer;
    size_t stringLength;
    size_t _length;

    /* allocates and sets the initial value on the flag that controls
    if the iteratio to generate key values is the first one */
    char isFirst = 1;

    /* creates a new string buffer and a new hash map
    iterator, these structures are going to be used to
    handle the string from the hash map and to iterate
    over the hash map elements */
    createStringBuffer(&stringBuffer);
    createElementIteratorHashMap(hashMap, &iterator);

    /* iterates continuously arround the hash map element
    the iterator is going to stop the iteration */
    while(1) {
        /* retrieves the next element from the iterator
        and in case such element is invalid breaks the loop */
        getNextIterator(iterator, (void **) &element);
        if(element == NULL) { break; }

        /* checks if this is the first loop in the iteration
        in it's not emits the and character */
        if(isFirst) { isFirst = 0; }
        else { appendStringBuffer(stringBuffer, (unsigned char *) "&"); }

        /* retrieves the current element value as a string structure
        then encodes that value using the url encoding (percent encoding)
        and resets the string reference to contain the new buffer as it'
        own contents (avoids extra memory usage) */
        string = (struct String_t *) element->value;
        urlEncode(string->buffer, string->length, &_buffer, &_length);
        string->buffer = _buffer;
        string->length = _length;

        /* adds the various elements for the value to the string buffer
        first the key the the attribution operator and then the value */
        appendStringBuffer(stringBuffer, (unsigned char *) element->keyString);
        appendStringLBuffer(stringBuffer, (unsigned char *) "=", sizeof("=") - 1);
        _appendStringTBuffer(stringBuffer, string);
    }

    /* "joins" the string buffer values into a single
    value (from the internal string list) and then
    retrieves the length of the string buffer */
    joinStringBuffer(stringBuffer, &stringValue);
    stringLength = stringBuffer->stringLength;

    /* deletes the hash map iterator and string buffer
    structures, to avoid memory leak */
    deleteIteratorHashMap(hashMap, iterator);
    deleteStringBuffer(stringBuffer);

    /* updates the buffer pointer reference and the
    buffer length pointer reference with the string
    value and the string length values */
    *bufferPointer = stringValue;
    *bufferLengthPointer = stringLength;

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

	ERROR_CODE error;

    char *buffer = MALLOC(1024);

    struct Type_t *_type;
    unsigned char *_buffer;
    size_t _bufferSize;
    unsigned char infoHash[SHA1_DIGEST_SIZE + 1];
    unsigned char random[12];
    unsigned char peerId[21];
    struct HashMap_t *parametersMap;
    unsigned char *getString;
    size_t getStringSize;
    struct String_t strings[9];

    SPRINTF((char *) peerId, 20, "-%s%d%d%d0-", VIRIATUM_PREFIX, VIRIATUM_MAJOR, VIRIATUM_MINOR, VIRIATUM_MICRO);
    randomBuffer(random, 12);
    memcpy(peerId + 8, random, 12);

    error = decodeBencodingFile("C:/verysleepy_0_82.exe.torrent", &type);
	if(error) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, "Problem reading torrent file"); }

    getValueStringSortMap(type->value.valueSortMap, (unsigned char *) "info", (void **) &_type);
    encodeBencoding(_type, &_buffer, &_bufferSize);
    sha1(_buffer, _bufferSize, infoHash);
    printType(type);
    freeType(type);
    FREE(_buffer);

    /* tenho de fazer gerador de get parameters !!!! */
    /* pega nas chaves e nos valores do hash map e gera a get string para um string buffer */
    createHashMap(&parametersMap, 0);
    strings[0].buffer = infoHash;
    strings[0].length = 20;
    strings[1].buffer = peerId;
    strings[1].length = 20;
    strings[2].buffer = (unsigned char *) "8080";
    strings[2].length = sizeof("8080") - 1;
    strings[3].buffer = (unsigned char *) "0";
    strings[3].length = sizeof("0") - 1;
    strings[4].buffer = (unsigned char *) "0";
    strings[4].length = sizeof("0") - 1;
    strings[5].buffer = (unsigned char *) "3213210"; /* must calculate this value */
    strings[5].length = sizeof("3213210") - 1; /* must calculate this value */
    strings[6].buffer = (unsigned char *) "0";
    strings[6].length = sizeof("0") - 1;
    strings[7].buffer = (unsigned char *) "0";
    strings[7].length = sizeof("0") - 1;
    strings[8].buffer = (unsigned char *) "started";
    strings[8].length = sizeof("started") - 1;
    setValueStringHashMap(parametersMap, (unsigned char *) "info_hash", (void *) &strings[0]);
    setValueStringHashMap(parametersMap, (unsigned char *) "peer_id", (void *) &strings[1]);
    setValueStringHashMap(parametersMap, (unsigned char *) "port", (void *) &strings[2]);
    setValueStringHashMap(parametersMap, (unsigned char *) "uploaded", (void *) &strings[3]);
    setValueStringHashMap(parametersMap, (unsigned char *) "downloaded", (void *) &strings[4]);
    setValueStringHashMap(parametersMap, (unsigned char *) "left", (void *) &strings[5]);
    setValueStringHashMap(parametersMap, (unsigned char *) "compact", (void *) &strings[6]);
    setValueStringHashMap(parametersMap, (unsigned char *) "no_peer_id", (void *) &strings[7]);
    setValueStringHashMap(parametersMap, (unsigned char *) "event", (void *) &strings[8]);
    generateParameters(parametersMap, &getString, &getStringSize);
    deleteHashMap(parametersMap);

    SPRINTF(buffer, 1024, "GET %s?%s HTTP/1.1\r\nUser-Agent: viriatum/0.1.0 (linux - intel x64)\r\nConnection: keep-alive\r\n\r\n", parameters->url, getString);

    FREE(getString);

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
