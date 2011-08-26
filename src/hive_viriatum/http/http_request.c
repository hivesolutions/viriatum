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

#include "http_request.h"




int http_should_keep_alive(struct HttpRequest_t *httpRequest) {
    if (httpRequest->httpMajor > 0 && httpRequest->httpMinor > 0) {
        /* HTTP/1.1 */
        if (httpRequest->flags & FLAG_CONNECTION_CLOSE) {
            return 0;
        } else {
            return 1;
        }
    } else {
        /* HTTP/1.0 or earlier */
        if (httpRequest->flags & FLAG_CONNECTION_KEEP_ALIVE) {
            return 1;
        } else {
            return 0;
        }
    }
}














void createHttpRequest(struct HttpRequest_t **httpRequestPointer) {
    /* retrieves the http request size */
    size_t httpRequestSize = sizeof(struct HttpRequest_t);

    /* allocates space for the http request */
    struct HttpRequest_t *httpRequest = (struct HttpRequest_t *) malloc(httpRequestSize);

    /* sets the http request type */
    httpRequest->type = 2;

    /* sets the http request flags */
    httpRequest->flags = 6;

    /* sets the http request state */
    httpRequest->state = STATE_START_REQ;

    /* sets the http request read count */
    httpRequest->readCount = -1;

    /* sets the http request content length */
    httpRequest->contentLength = -1;

    /* sets the http request http major */
    httpRequest->httpMajor = -1;

    /* sets the http request http minor */
    httpRequest->httpMinor = -1;

    /* sets the http request http minor */
    httpRequest->statusCode = -1;

    /* sets the http request method */
    httpRequest->method = 0;

    /* sets the http request upgrade */
    httpRequest->upgrade = 1;

    /* sets the http request in the http request pointer */
    *httpRequestPointer = httpRequest;
}

void deleteHttpRequest(struct HttpRequest_t *httpRequest) {
    /* releases the http request */
    free(httpRequest);
}



int messageBeginPrintCallback(struct HttpRequest_t *httpRequest) {
    /* prints an information */
    V_DEBUG("HTTP request received\n");

    return 0;
}

int urlPrintCallback(struct HttpRequest_t *httpRequest, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the url */
    unsigned char *url = (unsigned char *) malloc(dataSize + 1);

    /* copies the memory from the data to the url */
    memcpy(url, data, dataSize);

    /* puts the end of strng in the url */
    url[dataSize] = '\0';

    /* prints the url */
    V_DEBUG_F("url: %s\n", url);

    /* raise no error */
    RAISE_NO_ERROR;
}

int headerFieldPrintCallback(struct HttpRequest_t *httpRequest, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the header field */
    unsigned char *headerField = (unsigned char *) malloc(dataSize + 1);

    /* copies the memory from the data to the header field */
    memcpy(headerField, data, dataSize);

    /* puts the end of strng in the header field */
    headerField[dataSize] = '\0';

    /* prints the url */
    V_DEBUG_F("header field: %s\n", headerField);

    /* raise no error */
    RAISE_NO_ERROR;
}

int headerValuePrintCallback(struct HttpRequest_t *httpRequest, const unsigned char *data, size_t dataSize) {
    /* allocates the required space for the header value */
    unsigned char *headerValue = (unsigned char *) malloc(dataSize + 1);

    /* copies the memory from the data to the header value */
    memcpy(headerValue, data, dataSize);

    /* puts the end of strng in the header value */
    headerValue[dataSize] = '\0';

    /* prints the url */
    V_DEBUG_F("header value: %s\n", headerValue);

    /* raise no error */
    RAISE_NO_ERROR;
}

int headersCompletePrintCallback(struct HttpRequest_t *httpRequest) {
    /* prints an information */
    V_DEBUG("HTTP headers parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}

int messageCompletePrintCallback(struct HttpRequest_t *httpRequest) {
    /* prints an information */
    V_DEBUG("HTTP request parsed\n");

    /* raise no error */
    RAISE_NO_ERROR;
}



void createHttpSettings(struct HttpSettings_t **httpSettingsPointer) {
    /* retrieves the http settings size */
    size_t httpSettingsSize = sizeof(struct HttpSettings_t);

    /* allocates space for the http settings */
    struct HttpSettings_t *httpSettings = (struct HttpSettings_t *) malloc(httpSettingsSize);

    /* sets the http settings on message begin callback */
    httpSettings->onmessageBegin = messageBeginPrintCallback;

    /* sets the http settings on url callback */
    httpSettings->onurl = urlPrintCallback;

    /* sets the http settings on header field callback */
    httpSettings->onheaderField = headerFieldPrintCallback;

    /* sets the http settings on header value callback */
    httpSettings->onheaderValue = headerValuePrintCallback;

    /* sets the http settings on headers complete callback */
    httpSettings->onheadersComplete = 0;

    /* sets the http settings on body callback */
    httpSettings->onbody = 0;

    /* sets the http settings on message complete callback */
    httpSettings->onmessageComplete = messageCompletePrintCallback;

    /* sets the http settings in the http settings pointer */
    *httpSettingsPointer = httpSettings;
}

void deleteHttpSettings(struct HttpSettings_t *httpSettings) {
    /* releases the http settings */
    free(httpSettings);
}

int processDataHttpRequest(struct HttpRequest_t *httpRequest, struct HttpSettings_t *httpSettings, unsigned char *data, size_t dataSize) {
    /* Inspired from https://github.com/ry/http-parser/blob/master/http_parser.c */

    unsigned char byte;
    unsigned char byteToken;
    const char *matcher;
    enum HttpRequestHeaderState_e headerState;
    size_t toRead;
    const unsigned char *pointerEnd;
    const unsigned char *pointer = data;
    unsigned long long readCount = httpRequest->readCount;
    unsigned long long index = httpRequest->index;
    enum HttpRequestState_e state = httpRequest->state;

    const char *headerFieldMark = 0;
    const char *headerValueMark = 0;
    const char *urlMark = 0;

    /* in case the received data size is empty */
    if(dataSize == 0) {
        /* switches over the state */
        switch(state) {
            case STATE_BODY_IDENTITY_EOF:
                HTTP_CALLBACK2(messageComplete);

                /* returns immediately */
                return 0;

            case STATE_DEAD:
            case STATE_START_REQ_OR_RES:
            case STATE_START_RES:
            case STATE_START_REQ:
                /* returs immediately (nothing to process) */
                return 0;

            default:
                /* SET_ERRNO(HPE_INVALID_EOF_STATE);*/

                /* returns invalid (error) */
                return 1;
        }
    }

    /* iterates over the data incrementing the pointer by a value for iteration */
    for(pointer = data, pointerEnd = data + dataSize; pointer != pointerEnd; pointer++) {
        /* retrieves the current iteration byte */
        byte = *pointer;

        /* switch over the current state */
        switch(state) {
            case STATE_START_REQ_OR_RES:
                /* in case the current byte is a
                newline character */
                if(byte == CR || byte == LF) {
                    /* breaks the switch */
                    break;
                }

                /* sets the initial request values */
                httpRequest->flags = 0;
                httpRequest->contentLength = -1;

                HTTP_CALLBACK2(messageBegin);

                if(byte == 'H') {
                    state = STATE_RES_OR_RESP_H;
                } else {
                    httpRequest->type = HTTP_REQUEST;
                /*    goto start_req_method_assign; */
                }

                /* breaks the switch */
                break;

            case STATE_RES_OR_RESP_H:
                if (byte == 'T') {
                    httpRequest->type = HTTP_RESPONSE;
                    state = STATE_RES_HT;
                } else {
                    if (byte != 'E') {
                        /*SET_ERRNO(HPE_INVALID_CONSTANT); */
                        /*goto error; */
                    }

                    httpRequest->type = HTTP_REQUEST;
                    httpRequest->method = HTTP_HEAD;
                    index = 2;
                    state = STATE_REQ_METHOD;
                }

                /* breaks the switch */
                break;

            case STATE_START_RES:
                httpRequest->flags = 0;
                httpRequest->contentLength = -1;

                HTTP_CALLBACK2(messageBegin);

                switch(byte) {
                    case 'H':
                        state = STATE_RES_H;

                        /* breaks the switch */
                        break;

                    case CR:
                    case LF:
                        break;

/*                     default:*/
                        /*SET_ERRNO(HPE_INVALID_CONSTANT);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_RES_H:
                STRICT_CHECK(byte != 'T');
                state = STATE_RES_HT;

                /* breaks the switch */
                break;

            case STATE_RES_HT:
                STRICT_CHECK(byte != 'T');
                state = STATE_RES_HTT;

                /* breaks the switch */
                break;

            case STATE_RES_HTT:
                STRICT_CHECK(byte != 'P');
                state = STATE_RES_HTTP;

                /* breaks the switch */
                break;

            case STATE_RES_HTTP:
                STRICT_CHECK(byte != '/');
                state = STATE_RES_FIRST_HTTP_MAJOR;

                /* breaks the switch */
                break;

            case STATE_RES_FIRST_HTTP_MAJOR:
                if (byte < '1' || byte  > '9') {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMajor = byte - '0';
                state = STATE_RES_HTTP_MAJOR;

                /* breaks the switch */
                break;

            case STATE_RES_FIRST_HTTP_MINOR:
                if (!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMinor = byte - '0';
                state = STATE_RES_HTTP_MINOR;

                /* breaks the switch */
                break;

            case STATE_RES_HTTP_MINOR:
                if(byte == ' ') {
                    state = STATE_RES_FIRST_STATUS_CODE;

                    /* breaks the switch */
                    break;
                }

                if(!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMinor *= 10;
                httpRequest->httpMinor += byte - '0';

                if(httpRequest->httpMinor > 999) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_RES_FIRST_STATUS_CODE:
                if (!IS_NUM(byte)) {
                    if (byte == ' ') {
                        break;
                    }

                    /*SET_ERRNO(HPE_INVALID_STATUS);
                    goto error;*/
                }

                httpRequest->statusCode = byte - '0';
                state = STATE_RES_STATUS_CODE;

                /* breaks the switch */
                break;

            case STATE_RES_STATUS_CODE:
                if(!IS_NUM(byte)) {
                    switch(byte) {
                        case ' ':
                            state = STATE_RES_STATUS;
                            break;
                        case CR:
                            state = STATE_RES_LINE_ALMOST_DONE;
                            break;
                        case LF:
                            state = STATE_HEADER_FIELD_START;
                            break;
                        /*default:*/
                            /*SET_ERRNO(HPE_INVALID_STATUS);
                            goto error;*/
                    }

                    /* breaks the switch */
                    break;
                }

                httpRequest->statusCode *= 10;
                httpRequest->statusCode += byte - '0';

                if(httpRequest->statusCode > 999) {
                    /*SET_ERRNO(HPE_INVALID_STATUS);
                    goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_RES_STATUS:
                /* the human readable status. e.g. "NOT FOUND"
                we are not humans so just ignore this */
                if(byte == CR) {
                    state = STATE_RES_LINE_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    state = STATE_HEADER_FIELD_START;

                    /* breaks the switch */
                    break;
                }

                /* breaks the switch */
                break;

            case STATE_RES_LINE_ALMOST_DONE:
                STRICT_CHECK(byte != LF);
                state = STATE_HEADER_FIELD_START;

                /* breaks the switch */
                break;

            case STATE_START_REQ:
                if(byte == CR || byte == LF) {
                    break;
                }

                httpRequest->flags = 0;
                httpRequest->contentLength = -1;

                HTTP_CALLBACK2(messageBegin);

                if(!IS_ALPHA(byte)) {
                    /*
                    SET_ERRNO(HPE_INVALID_METHOD);
                    goto error;
                    */
                }

                startReqMethodAssign:
                    httpRequest->method = (enum http_method) 0;
                    index = 1;

                    switch(byte) {
                        case 'C': httpRequest->method = HTTP_CONNECT; /* or COPY, CHECKOUT */ break;
                        case 'D': httpRequest->method = HTTP_DELETE; break;
                        case 'G': httpRequest->method = HTTP_GET; break;
                        case 'H': httpRequest->method = HTTP_HEAD; break;
                        case 'L': httpRequest->method = HTTP_LOCK; break;
                        case 'M': httpRequest->method = HTTP_MKCOL; /* or MOVE, MKACTIVITY, MERGE, M-SEARCH */ break;
                        case 'N': httpRequest->method = HTTP_NOTIFY; break;
                        case 'O': httpRequest->method = HTTP_OPTIONS; break;
                        case 'P': httpRequest->method = HTTP_POST;
                        /* or PROPFIND or PROPPATCH or PUT or PATCH */
                        break;
                        case 'R': httpRequest->method = HTTP_REPORT; break;
                        case 'S': httpRequest->method = HTTP_SUBSCRIBE; break;
                        case 'T': httpRequest->method = HTTP_TRACE; break;
                        case 'U': httpRequest->method = HTTP_UNLOCK; /* or UNSUBSCRIBE */ break;

                        /*default:
                        SET_ERRNO(HPE_INVALID_METHOD);
                        goto error;*/
                    }

                    state = STATE_REQ_METHOD;

                    /* breaks the switch */
                    break;

            case STATE_REQ_METHOD:
                if(byte == '\0') {
                     /*SET_ERRNO(HPE_INVALID_METHOD);
                     goto error;*/
                }

                matcher = HttpMethodStrings[httpRequest->method - 1];

                if(byte == ' ' && matcher[index] == '\0') {
                    state = STATE_REQ_SPACES_BEFORE_URL;
                } else if(byte == matcher[index]) {
                } else if(httpRequest->method == HTTP_CONNECT) {
                    if(index == 1 && byte == 'H') {
                        httpRequest->method = HTTP_CHECKOUT;
                    } else if(index == 2  && byte == 'P') {
                        httpRequest->method = HTTP_COPY;
                    } else {
                        /*goto error;*/
                    }
                } else if(httpRequest->method == HTTP_MKCOL) {
                    if(index == 1 && byte == 'O') {
                        httpRequest->method = HTTP_MOVE;
                    } else if(index == 1 && byte == 'E') {
                        httpRequest->method = HTTP_MERGE;
                    } else if(index == 1 && byte == '-') {
                        httpRequest->method = HTTP_MSEARCH;
                    } else if(index == 2 && byte == 'A') {
                        httpRequest->method = HTTP_MKACTIVITY;
                    } else {
                        /*goto error;*/
                    }
                } else if(index == 1 && httpRequest->method == HTTP_POST) {
                    if(byte == 'R') {
                        httpRequest->method = HTTP_PROPFIND; /* or HTTP_PROPPATCH */
                    } else if(byte == 'U') {
                        httpRequest->method = HTTP_PUT;
                    } else if(byte == 'A') {
                        httpRequest->method = HTTP_PATCH;
                    } else {
                        /*goto error;*/
                    }
                } else if(index == 2 && httpRequest->method == HTTP_UNLOCK && byte == 'S') {
                    httpRequest->method = HTTP_UNSUBSCRIBE;
                } else if(index == 4 && httpRequest->method == HTTP_PROPFIND && byte == 'P') {
                    httpRequest->method = HTTP_PROPPATCH;
                } else {
                    /*SET_ERRNO(HPE_INVALID_METHOD);
                    goto error;*/
                }

                /* increments the index */
                index++;

                /* breaks the switch */
                break;

            case STATE_REQ_SPACES_BEFORE_URL:
                if(byte == ' ') {
                    /* breaks the switch */
                    break;
                }

                if(byte == '/' || byte == '*') {
                    MARK(url);
                    state = STATE_REQ_PATH;

                    /* breaks the switch */
                    break;
                }

                /* Proxied requests are followed by scheme of an absolute URI (alpha).
                 * CONNECT is followed by a hostname, which begins with alphanum.
                 * All other methods are followed by '/' or '*' (handled above).
                 */
                if(IS_ALPHA(byte) || (httpRequest->method == HTTP_CONNECT && IS_NUM(byte))) {
                    MARK(url);
                    state = (httpRequest->method == HTTP_CONNECT) ? STATE_REQ_HOST : STATE_REQ_SCHEMA;

                    /* breaks the switch */
                    break;
                }

                /*SET_ERRNO(HPE_INVALID_URL);
                goto error;*/

            case STATE_REQ_SCHEMA:
                if(IS_ALPHA(byte)) {
                    break;
                }

                if(byte == ':') {
                    state = STATE_REQ_SCHEMA_SLASH;
                    break;
                }

                /*SET_ERRNO(HPE_INVALID_URL);
                goto error;*/

            case STATE_REQ_SCHEMA_SLASH:
                STRICT_CHECK(byte != '/');
                state = STATE_REQ_SCHEMA_SLASH_SLASH;

                /* breaks the switch */
                break;

            case STATE_REQ_SCHEMA_SLASH_SLASH:
                STRICT_CHECK(byte != '/');
                state = STATE_REQ_HOST;

                /* breaks the switch */
                break;

            case STATE_REQ_HOST:
                if(IS_HOST_CHAR(byte)) {
                    break;
                }

                switch(byte) {
                    case ':':
                        state = STATE_REQ_PORT;

                        /* breaks the switch */
                        break;
                    case '/':
                        state = STATE_REQ_PATH;

                        /* breaks the switch */
                        break;
                    case ' ':
                        /* The request line looks like:
                         *   "GET http://foo.bar.com HTTP/1.1"
                         * That is, there is no path.
                         */
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;
                    case '?':
                        state = STATE_REQ_QUERY_STRING_START;

                        /* breaks the switch */
                        break;
                    /*default:
                        SET_ERRNO(HPE_INVALID_HOST);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_PORT:
                if(IS_NUM(byte)) {
                    /* breaks the switch */
                    break;
                }

                switch(byte) {
                    case '/':
                        state = STATE_REQ_PATH;
                        break;
                    case ' ':
                        /* The request line looks like:
                         *   "GET http://foo.bar.com:1234 HTTP/1.1"
                         * That is, there is no path.
                         */
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;
                        break;
                    case '?':
                        state = STATE_REQ_QUERY_STRING_START;
                        break;
                    /*default:
                        SET_ERRNO(HPE_INVALID_PORT);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_PATH:
                if(IS_URL_CHAR(byte)) {
                    /* breaks the switch */
                    break;
                }

                switch(byte) {
                    case ' ':
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_HEADER_FIELD_START;

                        /* breaks the switch */
                        break;

                    case '?':
                        state = STATE_REQ_QUERY_STRING_START;

                        /* breaks the switch */
                        break;

                    case '#':
                        state = STATE_REQ_FRAGMENT_START;

                        /* breaks the switch */
                        break;

                    /*default:
                        SET_ERRNO(HPE_INVALID_PATH);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_QUERY_STRING_START:
                if(IS_URL_CHAR(byte)) {
                    state = STATE_REQ_QUERY_STRING;
                    break;
                }

                switch(byte) {
                    case '?':
                        /* breaks the switch */
                        break;

                     case ' ':
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_HEADER_FIELD_START;

                        /* breaks the switch */
                        break;

                    case '#':
                        state = STATE_REQ_FRAGMENT_START;

                        /* breaks the switch */
                        break;

                    /*default:
                        SET_ERRNO(HPE_INVALID_QUERY_STRING);
                        goto error;*/
                }

                /* breaks the switch */
                break;


            case STATE_REQ_QUERY_STRING:
                if(IS_URL_CHAR(byte)) {
                    /* breaks the switch */
                    break;
                }

                switch(byte) {
                    case '?':
                        /* allows extra '?' in query string */
                        break;

                    case ' ':
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_HEADER_FIELD_START;

                        /* breaks the switch */
                        break;

                    case '#':
                        state = STATE_REQ_FRAGMENT_START;

                        /* breaks the switch */
                        break;

                    /*default:
                        SET_ERRNO(HPE_INVALID_QUERY_STRING);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_FRAGMENT_START:
                if(IS_URL_CHAR(byte)) {
                    state = STATE_REQ_FRAGMENT;

                    /* breaks the switch */
                    break;
                }

                switch(byte) {
                    case ' ':
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_HEADER_FIELD_START;

                        /* breaks the switch */
                        break;

                    case '?':
                        state = STATE_REQ_FRAGMENT;

                        /* breaks the switch */
                        break;

                    case '#':
                        /* breaks the switch */
                        break;
                    /*default:
                        SET_ERRNO(HPE_INVALID_FRAGMENT);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_FRAGMENT:
                if(IS_URL_CHAR(byte)) {
                    /* breaks the switch */
                    break;
                }

                switch(byte) {
                    case ' ':
                        HTTP_CALLBACK(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                  case CR:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                  case LF:
                        HTTP_CALLBACK(url);
                        httpRequest->httpMajor = 0;
                        httpRequest->httpMinor = 9;
                        state = STATE_HEADER_FIELD_START;

                        /* breaks the switch */
                        break;

                  case '?':
                  case '#':
                        /* breaks the switch */
                        break;

                  /*default:
                        SET_ERRNO(HPE_INVALID_FRAGMENT);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_START:
                switch(byte) {
                    case 'H':
                        state = STATE_REQ_HTTP_H;

                        /* breaks the switch */
                        break;

                    case ' ':
                        /* breaks the switch */
                        break;

                    /*default:
                        SET_ERRNO(HPE_INVALID_CONSTANT);
                        goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_H:
                STRICT_CHECK(byte != 'T');
                state = STATE_REQ_HTTP_HT;

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_HT:
                STRICT_CHECK(byte != 'T');
                state = STATE_REQ_HTTP_HTT;

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_HTT:
                STRICT_CHECK(byte != 'P');
                state = STATE_REQ_HTTP_HTTP;

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_HTTP:
                STRICT_CHECK(byte != '/');
                state = STATE_REQ_FIRST_HTTP_MAJOR;

                /* breaks the switch */
                break;

            case STATE_REQ_FIRST_HTTP_MAJOR:
                if(byte < '1' || byte > '9') {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMajor = byte - '0';
                state = STATE_REQ_HTTP_MAJOR;

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_MAJOR:
                if(byte == '.') {
                    state = STATE_REQ_FIRST_HTTP_MINOR;

                    /* breaks the switch */
                    break;
                }

                if(!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMajor *= 10;
                httpRequest->httpMajor += byte - '0';

                if(httpRequest->httpMajor > 999) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_FIRST_HTTP_MINOR:
                if(!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMinor = byte - '0';
                state = STATE_REQ_HTTP_MINOR;

                /* breaks the switch */
                break;

            case STATE_REQ_HTTP_MINOR:
                if(byte == CR) {
                    state = STATE_REQ_LINE_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    state = STATE_HEADER_FIELD_START;

                    /* breaks the switch */
                    break;
                }

                if(!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpRequest->httpMinor *= 10;
                httpRequest->httpMinor += byte - '0';

                if (httpRequest->httpMinor > 999) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_REQ_LINE_ALMOST_DONE:
                if(byte != LF) {
                    /*SET_ERRNO(HPE_LF_EXPECTED);
                    goto error;*/
                }

                state = STATE_HEADER_FIELD_START;

                /* breaks the switch */
                break;

            case STATE_HEADER_FIELD_START:
                headerFieldStart:
                    if(byte == CR) {
                        state = STATE_HEADER_ALMOST_DONE;

                        /* breaks the switch */
                        break;
                    }

                    if(byte == LF) {
                        /* they might be just sending \n instead of \r\n so this would be
                        * the second \n to denote the end of headers*/
                        state = STATE_HEADERS_ALMOST_DONE;

                        goto headersAlmostDone;
                    }

                    byteToken = TOKEN(byte);

                    if (!byteToken) {
                        /*SET_ERRNO(HPE_INVALID_HEADER_TOKEN);
                        goto error;*/
                    }

                    MARK(headerField);

                    index = 0;
                    state = STATE_HEADER_FIELD;

                    switch(byteToken) {
                        case 'c':
                            headerState = HEADER_STATE_C;

                            /* breaks the switch */
                            break;

                        case 'p':
                            headerState = HEADER_STATE_MATCHING_PROXY_CONNECTION;

                            /* breaks the switch */
                            break;

                        case 't':
                            headerState = HEADER_STATE_MATCHING_TRANSFER_ENCODING;

                            /* breaks the switch */
                            break;

                        case 'u':
                            headerState = HEADER_STATE_MATCHING_UPGRADE;

                            /* breaks the switch */
                            break;

                        default:
                            headerState = HEADER_STATE_GENERAL;

                            /* breaks the switch */
                            break;
                    }

                    /* breaks the switch */
                    break;

            case STATE_HEADER_FIELD:
                byteToken = TOKEN(byte);

                if(byteToken) {
                    switch(headerState) {
                        case HEADER_STATE_GENERAL:
                            /* breaks the switch */
                            break;

                        case HEADER_STATE_C:
                            index++;
                            headerState = (byteToken == 'o' ? HEADER_STATE_CO : HEADER_STATE_GENERAL);

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CO:
                            index++;
                            headerState = (byteToken == 'n' ? HEADER_STATE_CON : HEADER_STATE_GENERAL);

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CON:
                            index++;

                            switch(byteToken) {
                                case 'n':
                                    headerState = HEADER_STATE_MATCHING_CONNECTION;
                                    break;

                                case 't':
                                    headerState = HEADER_STATE_MATCHING_CONTENT_LENGTH;
                                    break;

                                default:
                                    headerState = HEADER_STATE_GENERAL;
                                    break;
                            }

                            /* breaks the switch */
                            break;

                        /* connection */
                        case HEADER_STATE_MATCHING_CONNECTION:
                            index++;

                            if(index > sizeof(HTTP_CONNECTION) - 1 || byteToken != HTTP_CONNECTION[index]) {
                                headerState = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_CONNECTION) - 2) {
                                headerState = HEADER_STATE_CONNECTION;
                            }

                            /* breaks the switch */
                            break;

                        /* proxy-connection */
                        case HEADER_STATE_MATCHING_PROXY_CONNECTION:
                            index++;

                            if(index > sizeof(HTTP_PROXY_CONNECTION) - 1 || byteToken != HTTP_PROXY_CONNECTION[index]) {
                                headerState = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_PROXY_CONNECTION) - 2) {
                                headerState = HEADER_STATE_CONNECTION;
                            }

                            /* breaks the switch */
                            break;

                        /* content-length */
                        case HEADER_STATE_MATCHING_CONTENT_LENGTH:
                            index++;

                            if(index > sizeof(HTTP_CONTENT_LENGTH) - 1 || byteToken != HTTP_CONTENT_LENGTH[index]) {
                                headerState = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_CONTENT_LENGTH) - 2) {
                                headerState = HEADER_STATE_CONTENT_LENGTH;
                            }

                            /* breaks the switch */
                            break;

                        /* transfer-encoding */
                        case HEADER_STATE_MATCHING_TRANSFER_ENCODING:
                            index++;

                            if(index > sizeof(HTTP_TRANSFER_ENCODING) - 1 || byteToken != HTTP_TRANSFER_ENCODING[index]) {
                                headerState = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_TRANSFER_ENCODING) - 2) {
                                headerState = HEADER_STATE_TRANSFER_ENCODING;
                            }

                            /* breaks the switch */
                            break;

                        /* upgrade */
                        case HEADER_STATE_MATCHING_UPGRADE:
                            index++;

                            if(index > sizeof(HTTP_UPGRADE) - 1 || byteToken != HTTP_UPGRADE[index]) {
                                headerState = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_UPGRADE) - 2) {
                                headerState = HEADER_STATE_UPGRADE;
                            }

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CONNECTION:
                        case HEADER_STATE_CONTENT_LENGTH:
                        case HEADER_STATE_TRANSFER_ENCODING:
                        case HEADER_STATE_UPGRADE:
                            if(byte != ' ') {
                                headerState = HEADER_STATE_GENERAL;
                            }

                            /* breaks the switch */
                            break;

                        default:
                            assert(0 && "Unknown header state");

                            /* breaks the switch */
                            break;
                    }

                    /* breaks the switch */
                    break;
                }

                if(byte == ':') {
                    HTTP_CALLBACK(headerField);
                    state = STATE_HEADER_VALUE_START;

                    /* breaks the switch */
                    break;
                }

                if(byte == CR) {
                    state = STATE_HEADER_ALMOST_DONE;
                    HTTP_CALLBACK(headerField);

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK(headerField);
                    state = STATE_HEADER_FIELD_START;

                    /* breaks the switch */
                    break;
                }

                /*SET_ERRNO(HPE_INVALID_HEADER_TOKEN);
                goto error;*/

            case STATE_HEADER_VALUE_START:
                if(byte == ' ' || byte == '\t') {
                    /* breaks the switch */
                    break;
                }

                MARK(headerValue);

                state = STATE_HEADER_VALUE;

                index = 0;

                if(byte == CR) {
                    HTTP_CALLBACK(headerValue);
                    headerState = HEADER_STATE_GENERAL;
                    state = STATE_HEADER_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK(headerValue);
                    state = STATE_HEADER_FIELD_START;

                    /* breaks the switch */
                    break;
                }

                byteToken = LOWER(byte);

                switch(headerState) {
                    case HEADER_STATE_UPGRADE:
                        httpRequest->flags |= FLAG_UPGRADE;
                        headerState = HEADER_STATE_GENERAL;

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_TRANSFER_ENCODING:
                        if('c' == byteToken) {
                            headerState = HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED;
                        } else {
                            headerState = HEADER_STATE_GENERAL;
                        }

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_CONTENT_LENGTH:
                        if(!IS_NUM(byte)) {
                            /*SET_ERRNO(HPE_INVALID_CONTENT_LENGTH);
                            goto error;*/
                        }

                        httpRequest->contentLength = byte - '0';

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_CONNECTION:
                        /* looking for 'Connection: keep-alive' */
                        if(byteToken == 'k') {
                            headerState = HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE;
                        /* looking for 'Connection: close' */
                        } else if(byteToken == 'c') {
                            headerState = HEADER_STATE_MATCHING_CONNECTION_CLOSE;
                        } else {
                            headerState = HEADER_STATE_GENERAL;
                        }

                        /* breaks the switch */
                        break;

                    default:
                        headerState = HEADER_STATE_GENERAL;

                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;

            case STATE_HEADER_VALUE:
                if(byte == CR) {
                    HTTP_CALLBACK(headerValue);
                    state = STATE_HEADER_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK(headerValue);
                    goto headerAlmostDone;
                }

                byteToken = LOWER(byte);

                switch(headerState) {
                    case HEADER_STATE_GENERAL:
                        break;

                    case HEADER_STATE_CONNECTION:
                    case HEADER_STATE_TRANSFER_ENCODING:
                        assert(0 && "Shouldn't get here.");
                        break;

                    case HEADER_STATE_CONTENT_LENGTH:
                        if(byte == ' ') {
                            break;
                        }

                        if(!IS_NUM(byte)) {
                            /*SET_ERRNO(HPE_INVALID_CONTENT_LENGTH);
                            goto error;*/
                        }

                        httpRequest->contentLength *= 10;
                        httpRequest->contentLength += byte - '0';

                        /* breaks the switch */
                        break;

                    /* Transfer-Encoding: chunked */
                    case HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED:
                        index++;

                        if(index > sizeof(HTTP_CHUNKED) - 1 || byte != HTTP_CHUNKED[index]) {
                            headerState = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_CHUNKED) - 2) {
                            headerState = HEADER_STATE_TRANSFER_ENCODING_CHUNKED;
                        }

                        /* breaks the switch */
                        break;

                    /* looking for 'Connection: keep-alive' */
                    case HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE:
                        index++;

                        if(index > sizeof(HTTP_KEEP_ALIVE) - 1 || byte != HTTP_KEEP_ALIVE[index]) {
                            headerState = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_KEEP_ALIVE) - 2) {
                            headerState = HEADER_STATE_CONNECTION_KEEP_ALIVE;
                        }

                        /* breaks the switch */
                        break;

                    /* looking for 'Connection: close' */
                    case HEADER_STATE_MATCHING_CONNECTION_CLOSE:
                        index++;

                        if(index > sizeof(HTTP_CLOSE) - 1 || byteToken != HTTP_CLOSE[index]) {
                            headerState = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_CLOSE) - 2) {
                            headerState = HEADER_STATE_CONNECTION_CLOSE;
                        }

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                    case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                    case HEADER_STATE_CONNECTION_CLOSE:
                        if(byte != ' ') {
                            headerState = HEADER_STATE_GENERAL;
                        }

                        /* breaks the switch */
                        break;

                    default:
                        state = STATE_HEADER_VALUE;
                        headerState = HEADER_STATE_GENERAL;

                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;


            case STATE_HEADER_ALMOST_DONE:
                headerAlmostDone:
                    STRICT_CHECK(byte != LF);

                    state = STATE_HEADER_VALUE_LWS;

                    switch(headerState) {
                        case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                            httpRequest->flags |= FLAG_CONNECTION_KEEP_ALIVE;

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CONNECTION_CLOSE:
                            httpRequest->flags |= FLAG_CONNECTION_CLOSE;

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                            httpRequest->flags |= FLAG_CHUNKED;

                            /* breaks the switch */
                            break;

                        default:
                            /* breaks the switch */
                            break;
                    }

                    /* breaks the switch */
                    break;

            case STATE_HEADER_VALUE_LWS:
                if(byte == ' ' || byte == '\t') {
                    state = STATE_HEADER_VALUE_START;
                } else {
                    state = STATE_HEADER_FIELD_START;
                    goto headerFieldStart;
                }

                /* breaks the switch */
                break;

            case STATE_HEADERS_ALMOST_DONE:
                headersAlmostDone:
                    STRICT_CHECK(ch != LF);

                    if(httpRequest->flags & FLAG_TRAILING) {
                        /* End of a chunked request */
                        HTTP_CALLBACK2(messageComplete);
                        state = NEW_MESSAGE();
                        break;
                    }

                    readCount = 0;

                    if(httpRequest->flags & FLAG_UPGRADE || httpRequest->method == HTTP_CONNECT) {
                        httpRequest->upgrade = 1;
                    }

                    /* Here we call the headers_complete callback. This is somewhat
                     * different than other callbacks because if the user returns 1, we
                     * will interpret that as saying that this message has no body. This
                     * is needed for the annoying case of recieving a response to a HEAD
                     * request.
                     */
                    if(httpSettings->onheadersComplete) {
                        switch (httpSettings->onheadersComplete(httpRequest)) {
                            case 0:
                                break;

                            case 1:
                                httpRequest->flags |= FLAG_SKIPBODY;
                                break;

                            default:
                                httpRequest->state = state;
                                /*SET_ERRNO(HPE_CB_headers_complete);*/
                                return pointer - data; /* Error */
                        }
                    }

                    /* Exit, the rest of the connect is in a different protocol. */
                    if(httpRequest->upgrade) {
                        HTTP_CALLBACK2(messageComplete);

                        return (pointer - data) + 1;
                    }

                    if(httpRequest->flags & FLAG_SKIPBODY) {
                        HTTP_CALLBACK2(messageComplete);
                        state = NEW_MESSAGE();
                    } else if(httpRequest->flags & FLAG_CHUNKED) {
                        /* chunked encoding - ignore Content-Length header */
                        state = STATE_CHUNK_SIZE_START;
                    } else {
                        if(httpRequest->contentLength == 0) {
                            /* Content-Length header given but zero: Content-Length: 0\r\n */
                            HTTP_CALLBACK2(messageComplete);
                            state = NEW_MESSAGE();
                        } else if(httpRequest->contentLength > 0) {
                            /* Content-Length header given and non-zero */
                            state = STATE_BODY_IDENTITY;
                        } else {
                            if(httpRequest->type == HTTP_REQUEST || http_should_keep_alive(httpRequest)) {
                                /* Assume content-length 0 - read the next */
                                HTTP_CALLBACK2(messageComplete);
                                state = NEW_MESSAGE();
                            } else {
                                /* Read body until EOF */
                                state = STATE_BODY_IDENTITY_EOF;
                            }
                        }
                    }

                    /* breaks the switch */
                    break;


            case STATE_BODY_IDENTITY:
                toRead = MIN(pointerEnd - pointer, (long long) httpRequest->contentLength);

                if(toRead > 0) {
                    if(httpSettings->onbody) {
                        httpSettings->onbody(httpRequest, pointer, toRead);
                    }

                    pointer += toRead - 1;

                    httpRequest->contentLength -= toRead;

                    if(httpRequest->contentLength == 0) {
                        HTTP_CALLBACK2(messageComplete);
                        state = NEW_MESSAGE();
                    }
                }

                /* breaks the switch */
                break;

            /* read until EOF */
            case STATE_BODY_IDENTITY_EOF:
                toRead = pointerEnd - pointer;

                if(toRead > 0) {
                    if(httpSettings->onbody) {
                        httpSettings->onbody(httpRequest, pointer, toRead);
                    }

                    pointer += toRead - 1;
                }

                /* breaks the switch */
                break;

            /*case SATE_CHUNK_SIZE_START:
                assert(readCount == 1);
                assert(httRequest->flags & FLAG_CHUNKED);

                unhex_val = unhex[(unsigned char)ch];
                if (unhex_val == -1) {
                  SET_ERRNO(HPE_INVALID_CHUNK_SIZE);
                  goto error;
                }

                parser->content_length = unhex_val;
                state = s_chunk_size;
                break;
            */
        }
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
