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
    struct type_t *type;

    decode_bencoding((unsigned char *) data, dataSize, &type);
    print_type(type);
    free_type(type);

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

ERROR_CODE dataHandlerStreamHttpClient(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the temporary variable to
    hold the ammount of bytes processed in a given http
    data parsing iteration */
    int processedSize;

    /* retrieves the http client connection */
    struct HttpClientConnection_t *httpClientConnection = (struct HttpClientConnection_t *) ioConnection->lower;

    /* process the http data for the http parser, this should be
    a partial processing and some data may remain unprocessed (in
    case there are multiple http requests) */
    processedSize = processDataHttpParser(httpClientConnection->httpParser, httpClientConnection->httpSettings, buffer, buffer_size);

    printf("%d", processedSize);

    /* raises no error */
    RAISE_NO_ERROR;
}








ERROR_CODE randomBuffer(unsigned char *buffer, size_t buffer_size) {
    size_t index;
    time_t seconds;
    int random;
    unsigned char byte;

    time(&seconds);
    srand((unsigned int) seconds);

    for(index = 0; index < buffer_size; index++) {
        random = rand();
        byte = (unsigned char) (random % 94);
        buffer[index] = byte + 34;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE generateParameters(struct hash_map_t *hash_map, unsigned char **buffer_pointer, size_t *bufferLengthPointer) {
    /* allocates space for an iterator object for an hash map element
    and for the string buffer to be used to collect all the partial
    strings that compose the complete url parameters string */
    struct iterator_t *iterator;
    struct hash_map_element_t *element;
    struct string_buffer_t *string_buffer;

    /* allocates space for the string structure to hold the value of
    the element for the string value reference for the joined string,
    for the buffer string from the element and for the corresponding
    string lengths for both cases */
    struct string_t *string;
    unsigned char *string_value;
    unsigned char *_buffer;
    size_t string_length;
    size_t _length;

    /* allocates and sets the initial value on the flag that controls
    if the iteratio to generate key values is the first one */
    char isFirst = 1;

    /* creates a new string buffer and a new hash map
    iterator, these structures are going to be used to
    handle the string from the hash map and to iterate
    over the hash map elements */
    create_string_buffer(&string_buffer);
    create_element_iterator_hash_map(hash_map, &iterator);

    /* iterates continuously arround the hash map element
    the iterator is going to stop the iteration */
    while(1) {
        /* retrieves the next element from the iterator
        and in case such element is invalid breaks the loop */
        get_next_iterator(iterator, (void **) &element);
        if(element == NULL) { break; }

        /* checks if this is the first loop in the iteration
        in it's not emits the and character */
        if(isFirst) { isFirst = 0; }
        else { append_string_buffer(string_buffer, (unsigned char *) "&"); }

        /* retrieves the current element value as a string structure
        then encodes that value using the url encoding (percent encoding)
        and resets the string reference to contain the new buffer as it'
        own contents (avoids extra memory usage) */
        string = (struct string_t *) element->value;
        urlEncode(string->buffer, string->length, &_buffer, &_length);
        string->buffer = _buffer;
        string->length = _length;

        /* adds the various elements for the value to the string buffer
        first the key the the attribution operator and then the value */
        append_string_buffer(string_buffer, (unsigned char *) element->key_string);
        append_string_l_buffer(string_buffer, (unsigned char *) "=", sizeof("=") - 1);
        _append_string_t_buffer(string_buffer, string);
    }

    /* "joins" the string buffer values into a single
    value (from the internal string list) and then
    retrieves the length of the string buffer */
    join_string_buffer(string_buffer, &string_value);
    string_length = string_buffer->string_length;

    /* deletes the hash map iterator and string buffer
    structures, to avoid memory leak */
    delete_iterator_hash_map(hash_map, iterator);
    delete_string_buffer(string_buffer);

    /* updates the buffer pointer reference and the
    buffer length pointer reference with the string
    value and the string length values */
    *buffer_pointer = string_value;
    *bufferLengthPointer = string_length;

    /* raises no error */
    RAISE_NO_ERROR;
}



ERROR_CODE openHandlerStreamHttpClient(struct IoConnection_t *ioConnection) {
    /* allocates space for the temporary error variable to
    be used to detect errors in calls */
    ERROR_CODE error;

    /* allocates the http client connection and retrieves the
    "upper" connection (for parameters retrieval) */
    struct HttpClientConnection_t *httpClientConnection;
    struct Connection_t *connection = (struct Connection_t *) ioConnection->connection;
    struct HttpClientParameters_t *parameters = (struct HttpClientParameters_t *) connection->parameters;
    struct type_t *type;
    struct type_t *_type;
    unsigned char *_buffer;
    size_t _bufferSize;
    unsigned char infoHash[SHA1_DIGEST_SIZE + 1];
    unsigned char random[12];
    unsigned char peerId[21];
    struct hash_map_t *parametersMap;
    unsigned char *getString;
    size_t getStringSize;
    struct string_t strings[9];
    char *buffer = MALLOC(1024);

    SPRINTF((char *) peerId, 20, "-%s%d%d%d0-", VIRIATUM_PREFIX, VIRIATUM_MAJOR, VIRIATUM_MINOR, VIRIATUM_MICRO);
    randomBuffer(random, 12);
    memcpy(peerId + 8, random, 12);

    error = decode_bencoding_file("C:/verysleepy_0_82.exe.torrent", &type);
    if(error) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading torrent file"); }

    get_value_string_sort_map(type->value.value_sort_map, (unsigned char *) "info", (void **) &_type);
    encode_bencoding(_type, &_buffer, &_bufferSize);
    sha1(_buffer, _bufferSize, infoHash);
    print_type(type);
    free_type(type);
    FREE(_buffer);

    /* tenho de fazer gerador de get parameters !!!! */
    /* pega nas chaves e nos valores do hash map e gera a get string para um string buffer */
    create_hash_map(&parametersMap, 0);
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
    set_value_string_hash_map(parametersMap, (unsigned char *) "info_hash", (void *) &strings[0]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "peer_id", (void *) &strings[1]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "port", (void *) &strings[2]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "uploaded", (void *) &strings[3]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "downloaded", (void *) &strings[4]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "left", (void *) &strings[5]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "compact", (void *) &strings[6]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "no_peer_id", (void *) &strings[7]);
    set_value_string_hash_map(parametersMap, (unsigned char *) "event", (void *) &strings[8]);
    generateParameters(parametersMap, &getString, &getStringSize);
    delete_hash_map(parametersMap);

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
