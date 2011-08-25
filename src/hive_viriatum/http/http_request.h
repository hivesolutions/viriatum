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

/**
 * Defines the various http request mehtods.
 * The methods are defined in a random order.
 */
enum HttpMethod {
    HTTP_DELETE = 1,
    HTTP_GET,
    HTTP_HEAD,
    HTTP_POST,
    HTTP_PUT,
    HTTP_CONNECT,
    HTTP_OPTIONS,
    HTTP_TRACE,
    HTTP_COPY,
    HTTP_LOCK,
    HTTP_MKCOL,
    HTTP_MOVE,
    HTTP_PROPFIND,
    HTTP_PROPPATCH,
    HTTP_UNLOCK,
    HTTP_REPORT,
    HTTP_MKACTIVITY,
    HTTP_CHECKOUT,
    HTTP_MERGE,
    HTTP_MSEARCH,
    HTTP_NOTIFY,
    HTTP_SUBSCRIBE,
    HTTP_UNSUBSCRIBE,
    HTTP_PATCH
};

enum HttpRequestState {
    STATE_DEAD = 1,
    STATE_START_REQ_OR_RES,

    STATE_RES_OR_RESP_H,
    STATE_START_RES,
    STATE_RES_H,
    STATE_RES_HT,
    STATE_RES_HTT,
    STATE_RES_HTTP,
    STATE_RES_FIRST_HTTP_MAJOR,
    STATE_RES_HTTP_MAJOR,
    STATE_RES_FIRST_HTTP_MINOR,
    STATE_RES_HTTP_MINOR,
    STATE_RES_FIRST_STATUS_CODE,
    STATE_RES_STATUS_CODE,
    STATE_RES_STATUS,
    STATE_RES_LINE_ALMOST_DONE,

    STATE_START_REQ,
    STATE_REQ_METHOD,
    STATE_REQ_SPACES_BEFORE_URL,
    STATE_REQ_SCHEMA,
    STATE_REQ_SCHEMA_SLASH,
    STATE_REQ_SCHEMA_SLASH_SLASH,
    STATE_REQ_HOST,
    STATE_REQ_PORT,
    STATE_REQ_PATH,
    STATE_REQ_QUERY_STRING_START,
    STATE_REQ_QUERY_STRING,
    STATE_REQ_FRAGMENT_START,
    STATE_REQ_FRAGMENT,
    STATE_REQ_HTTP_START,
    STATE_REQ_HTTP_H,
    STATE_REQ_HTTP_HT,
    STATE_REQ_HTTP_HTT,
    STATE_REQ_HTTP_HTTP,
    STATE_REQ_FIRST_HTTP_MAJOR,
    STATE_REQ_HTTP_MAJOR,
    STATE_REQ_FIRST_HTTP_MINOR,
    STATE_REQ_HTTP_MINOR,
    STATE_REQ_LINE_ALMOST_DONE,

    STATE_HEADER_FIELD_START,
    STATE_HEADER_FIELD,
    STATE_HEADER_VALUE_START,
    STATE_HEADER_VALUE,
    STATE_HEADER_VALUE_LWS,
    STATE_HEADER_ALMOST_DONE,


    STATE_CHUNK_SIZE_START,
    STATE_CHUNK_SIZE,
    STATE_CHUNK_PARAMETERS,
    STATE_CHUNK_SIZE_ALMOST_DONE,

    STATE_HEADERS_ALMOST_DONE,

    STATE_CHUNK_DATA,
    STATE_CHUNK_DATA_ALMOST_DONE,
    STATE_CHUNK_DATA_DONE,

    STATE_BODY_IDENTITY,
    STATE_BODY_EOF
};

/**
 * Structure representing an http request
 * it contains information about the request
 * including size contents and control flags.
 */
typedef struct HttpRequest_t {
    size_t receivedDataSize;
    unsigned char startLineLoaded;
    unsigned char headersLoaded;
} HttpRequest;

void createHttpRequest(struct HttpRequest_t **httpRequestPointer);
void deleteHttpRequest(struct HttpRequest_t *httpRequest);

/**
 * Called to parse a new data chunk in the context
 * of an http request.
 * This function should be called whenever a new data
 * chunk is received.
 *
 * @param httpRequest The http request object.
 * @param data The data to be parsed.
 * @param dataSize The size of the data to be parsed.
 */
void parseDataHttpRequest(struct HttpRequest_t *httpRequest, unsigned char *data, size_t dataSize);
