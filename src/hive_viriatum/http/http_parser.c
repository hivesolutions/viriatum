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
 __credits__   = Ryan Dahl <ry@tinyclouds.org>
*/

#include "stdafx.h"

#include "http_parser.h"

int httpShouldKeepAlive(struct HttpParser_t *httpParser) {
    /* in case the request is of type http 1.1 */
    if(httpParser->httpMajor > 0 && httpParser->httpMinor > 0) {
        if(httpParser->flags & FLAG_CONNECTION_CLOSE) {
            return 0;
        } else {
            return 1;
        }
    }
    /* in case the request is of type http 1.0 or earlier */
    else {
        if(httpParser->flags & FLAG_CONNECTION_KEEP_ALIVE) {
            return 1;
        } else {
            return 0;
        }
    }
}














void createHttpParser(struct HttpParser_t **httpParserPointer) {
    /* retrieves the http parser size */
    size_t httpParserSize = sizeof(struct HttpParser_t);

    /* allocates space for the http parser */
    struct HttpParser_t *httpParser = (struct HttpParser_t *) MALLOC(httpParserSize);

    /* sets the http parser type */
    httpParser->type = 2;

    /* sets the http parser flags */
    httpParser->flags = 6;

    /* sets the http parser state */
    httpParser->state = STATE_START_REQ;

    /* sets the http parser header state */
    httpParser->headerState = 0;

    /* sets the http parser read count */
    httpParser->readCount = 0;

    /* sets the http parser content length */
    httpParser->contentLength = 0;

    /* sets the http parser http major */
    httpParser->httpMajor = 0;

    /* sets the http parser http minor */
    httpParser->httpMinor = 0;

    /* sets the http parser http minor */
    httpParser->statusCode = 0;

    /* sets the http parser method */
    httpParser->method = 0;

    /* sets the http parser upgrade */
    httpParser->upgrade = 1;

    /* sets the http parser context */
    httpParser->context = NULL;

    /* sets the http parser parameters */
    httpParser->parameters = NULL;

    /* sets the http parser in the http parser pointer */
    *httpParserPointer = httpParser;
}

void deleteHttpParser(struct HttpParser_t *httpParser) {
    /* releases the http parser */
    FREE(httpParser);
}

void createHttpSettings(struct HttpSettings_t **httpSettingsPointer) {
    /* retrieves the http settings size */
    size_t httpSettingsSize = sizeof(struct HttpSettings_t);

    /* allocates space for the http settings */
    struct HttpSettings_t *httpSettings = (struct HttpSettings_t *) MALLOC(httpSettingsSize);

    /* sets the http settings on message begin callback */
    httpSettings->onmessageBegin = NULL;

    /* sets the http settings on url callback */
    httpSettings->onurl = NULL;

    /* sets the http settings on header field callback */
    httpSettings->onheaderField = NULL;

    /* sets the http settings on header value callback */
    httpSettings->onheaderValue = NULL;

    /* sets the http settings on headers complete callback */
    httpSettings->onheadersComplete = NULL;

    /* sets the http settings on body callback */
    httpSettings->onbody = NULL;

    /* sets the http settings on message complete callback */
    httpSettings->onmessageComplete = NULL;

    /* sets the http settings in the http settings pointer */
    *httpSettingsPointer = httpSettings;
}

void deleteHttpSettings(struct HttpSettings_t *httpSettings) {
    /* releases the http settings */
    FREE(httpSettings);
}

int processDataHttpParser(struct HttpParser_t *httpParser, struct HttpSettings_t *httpSettings, unsigned char *data, size_t dataSize) {
    unsigned char byte;
    unsigned char byteToken;
    char unhexValue;
    const char *matcher;
    size_t toRead;
    const unsigned char *pointerEnd;
    const unsigned char *pointer = data;
    size_t readCount = httpParser->readCount;
    size_t index = httpParser->index;
    unsigned char state = httpParser->state;
    unsigned char headerState = 0;

    const unsigned char *headerFieldMark = 0;
    const unsigned char *headerValueMark = 0;
    const unsigned char *urlMark = 0;

    /* in case the received data size is empty */
    if(dataSize == 0) {
        /* switches over the state */
        switch(state) {
            case STATE_BODY_IDENTITY_EOF:
                HTTP_CALLBACK(messageComplete);

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

                /* sets the initial parser values */
                httpParser->flags = 0;
                httpParser->contentLength = -1;

                HTTP_CALLBACK(messageBegin);

                if(byte == 'H') {
                    state = STATE_RES_OR_RESP_H;
                } else {
                    httpParser->type = HTTP_REQUEST;
                    goto startReqMethodAssign;
                }

                /* breaks the switch */
                break;

            case STATE_RES_OR_RESP_H:
                if (byte == 'T') {
                    httpParser->type = HTTP_RESPONSE;
                    state = STATE_RES_HT;
                } else {
                    if (byte != 'E') {
                        /*SET_ERRNO(HPE_INVALID_CONSTANT); */
                        /*goto error; */
                    }

                    httpParser->type = HTTP_REQUEST;
                    httpParser->method = HTTP_HEAD;
                    index = 2;
                    state = STATE_REQ_METHOD;
                }

                /* breaks the switch */
                break;

            case STATE_START_RES:
                httpParser->flags = 0;
                httpParser->contentLength = -1;

                HTTP_CALLBACK(messageBegin);

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

                httpParser->httpMajor = byte - '0';
                state = STATE_RES_HTTP_MAJOR;

                /* breaks the switch */
                break;

            case STATE_RES_FIRST_HTTP_MINOR:
                if (!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                httpParser->httpMinor = byte - '0';
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

                httpParser->httpMinor *= 10;
                httpParser->httpMinor += byte - '0';

                if(httpParser->httpMinor > 999) {
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

                httpParser->statusCode = byte - '0';
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

                httpParser->statusCode *= 10;
                httpParser->statusCode += byte - '0';

                if(httpParser->statusCode > 999) {
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

                httpParser->flags = 0;
                httpParser->contentLength = -1;

                HTTP_CALLBACK(messageBegin);

                if(!IS_ALPHA(byte)) {
                    /*
                    SET_ERRNO(HPE_INVALID_METHOD);
                    goto error;
                    */
                }

                startReqMethodAssign:
                    httpParser->method = (unsigned char) 0;
                    index = 1;

                    switch(byte) {
                        case 'C': httpParser->method = HTTP_CONNECT; /* or COPY, CHECKOUT */ break;
                        case 'D': httpParser->method = HTTP_DELETE; break;
                        case 'G': httpParser->method = HTTP_GET; break;
                        case 'H': httpParser->method = HTTP_HEAD; break;
                        case 'L': httpParser->method = HTTP_LOCK; break;
                        case 'M': httpParser->method = HTTP_MKCOL; /* or MOVE, MKACTIVITY, MERGE, M-SEARCH */ break;
                        case 'N': httpParser->method = HTTP_NOTIFY; break;
                        case 'O': httpParser->method = HTTP_OPTIONS; break;
                        case 'P': httpParser->method = HTTP_POST; /* or PROPFIND or PROPPATCH or PUT or PATCH */ break;
                        case 'R': httpParser->method = HTTP_REPORT; break;
                        case 'S': httpParser->method = HTTP_SUBSCRIBE; break;
                        case 'T': httpParser->method = HTTP_TRACE; break;
                        case 'U': httpParser->method = HTTP_UNLOCK; /* or UNSUBSCRIBE */ break;

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

                matcher = HttpMethodStrings[httpParser->method - 1];

                if(byte == ' ' && matcher[index] == '\0') {
                    state = STATE_REQ_SPACES_BEFORE_URL;
                } else if(byte == matcher[index]) {
                } else if(httpParser->method == HTTP_CONNECT) {
                    if(index == 1 && byte == 'H') {
                        httpParser->method = HTTP_CHECKOUT;
                    } else if(index == 2  && byte == 'P') {
                        httpParser->method = HTTP_COPY;
                    } else {
                        /*goto error;*/
                    }
                } else if(httpParser->method == HTTP_MKCOL) {
                    if(index == 1 && byte == 'O') {
                        httpParser->method = HTTP_MOVE;
                    } else if(index == 1 && byte == 'E') {
                        httpParser->method = HTTP_MERGE;
                    } else if(index == 1 && byte == '-') {
                        httpParser->method = HTTP_MSEARCH;
                    } else if(index == 2 && byte == 'A') {
                        httpParser->method = HTTP_MKACTIVITY;
                    } else {
                        /*goto error;*/
                    }
                } else if(index == 1 && httpParser->method == HTTP_POST) {
                    if(byte == 'R') {
                        httpParser->method = HTTP_PROPFIND; /* or HTTP_PROPPATCH */
                    } else if(byte == 'U') {
                        httpParser->method = HTTP_PUT;
                    } else if(byte == 'A') {
                        httpParser->method = HTTP_PATCH;
                    } else {
                        /*goto error;*/
                    }
                } else if(index == 2 && httpParser->method == HTTP_UNLOCK && byte == 'S') {
                    httpParser->method = HTTP_UNSUBSCRIBE;
                } else if(index == 4 && httpParser->method == HTTP_PROPFIND && byte == 'P') {
                    httpParser->method = HTTP_PROPPATCH;
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
                    HTTP_MARK(url);
                    state = STATE_REQ_PATH;

                    /* breaks the switch */
                    break;
                }

                /* Proxied requests are followed by scheme of an absolute URI (alpha).
                 * CONNECT is followed by a hostname, which begins with alphanum.
                 * All other methods are followed by '/' or '*' (handled above).
                 */
                if(IS_ALPHA(byte) || (httpParser->method == HTTP_CONNECT && IS_NUM(byte))) {
                    HTTP_MARK(url);
                    state = (httpParser->method == HTTP_CONNECT) ? STATE_REQ_HOST : STATE_REQ_SCHEMA;

                    /* breaks the switch */
                    break;
                }

                /*SET_ERRNO(HPE_INVALID_URL);
                goto error;*/

            case STATE_REQ_SCHEMA:
                if(IS_ALPHA(byte)) {
                    /* breaks the switch */
                    break;
                }

                if(byte == ':') {
                    state = STATE_REQ_SCHEMA_SLASH;

                    /* breaks the switch */
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
                    /* breaks the switch */
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
                         * "GET http://foo.bar.com HTTP/1.1"
                         * That is, there is no path.
                         */
                        HTTP_CALLBACK_DATA(url);
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
                        HTTP_CALLBACK_DATA(url);
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
                        HTTP_CALLBACK_DATA(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
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
                        HTTP_CALLBACK_DATA(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
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
                        HTTP_CALLBACK_DATA(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
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
                        HTTP_CALLBACK_DATA(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                    case CR:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
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
                        HTTP_CALLBACK_DATA(url);
                        state = STATE_REQ_HTTP_START;

                        /* breaks the switch */
                        break;

                  case CR:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                  case LF:
                        HTTP_CALLBACK_DATA(url);
                        httpParser->httpMajor = 0;
                        httpParser->httpMinor = 9;
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

                httpParser->httpMajor = byte - '0';
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

                httpParser->httpMajor *= 10;
                httpParser->httpMajor += byte - '0';

                if(httpParser->httpMajor > 999) {
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

                httpParser->httpMinor = byte - '0';
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

                httpParser->httpMinor *= 10;
                httpParser->httpMinor += byte - '0';

                if (httpParser->httpMinor > 999) {
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
                        state = STATE_HEADERS_ALMOST_DONE;

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

                    HTTP_MARK(headerField);

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
                    HTTP_CALLBACK_DATA(headerField);
                    state = STATE_HEADER_VALUE_START;

                    /* breaks the switch */
                    break;
                }

                if(byte == CR) {
                    state = STATE_HEADER_ALMOST_DONE;
                    HTTP_CALLBACK_DATA(headerField);

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK_DATA(headerField);
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

                HTTP_MARK(headerValue);

                state = STATE_HEADER_VALUE;

                index = 0;

                if(byte == CR) {
                    HTTP_CALLBACK_DATA(headerValue);
                    headerState = HEADER_STATE_GENERAL;
                    state = STATE_HEADER_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK_DATA(headerValue);
                    state = STATE_HEADER_FIELD_START;

                    /* breaks the switch */
                    break;
                }

                byteToken = LOWER(byte);

                switch(headerState) {
                    case HEADER_STATE_UPGRADE:
                        httpParser->flags |= FLAG_UPGRADE;
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

                        httpParser->contentLength = byte - '0';

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
                    HTTP_CALLBACK_DATA(headerValue);
                    state = STATE_HEADER_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK_DATA(headerValue);
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

                        httpParser->contentLength *= 10;
                        httpParser->contentLength += byte - '0';

                        /* breaks the switch */
                        break;

                    /* Transfer-Encoding: chunked */
                    case HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED:
                        index++;

                        if(index > sizeof(HTTP_CHUNKED) - 1 || byteToken != HTTP_CHUNKED[index]) {
                            headerState = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_CHUNKED) - 2) {
                            headerState = HEADER_STATE_TRANSFER_ENCODING_CHUNKED;
                        }

                        /* breaks the switch */
                        break;

                    /* looking for 'Connection: keep-alive' */
                    case HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE:
                        index++;

                        if(index > sizeof(HTTP_KEEP_ALIVE) - 1 || byteToken != HTTP_KEEP_ALIVE[index]) {
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
                            httpParser->flags |= FLAG_CONNECTION_KEEP_ALIVE;

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CONNECTION_CLOSE:
                            httpParser->flags |= FLAG_CONNECTION_CLOSE;

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                            httpParser->flags |= FLAG_CHUNKED;

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
                    STRICT_CHECK(byte != LF);

                    if(httpParser->flags & FLAG_TRAILING) {
                        /* End of a chunked request */
                        HTTP_CALLBACK(messageComplete);
                        state = NEW_MESSAGE();
                        break;
                    }

                    readCount = 0;

                    if(httpParser->flags & FLAG_UPGRADE || httpParser->method == HTTP_CONNECT) {
                        httpParser->upgrade = 1;
                    }

                    /* Here we call the headers_complete callback. This is somewhat
                     * different than other callbacks because if the user returns 1, we
                     * will interpret that as saying that this message has no body. This
                     * is needed for the annoying case of recieving a response to a HEAD
                     * request.
                     */
                    if(httpSettings->onheadersComplete) {
                        switch (httpSettings->onheadersComplete(httpParser)) {
                            case 0:
                                break;

                            case 1:
                                httpParser->flags |= FLAG_SKIPBODY;
                                break;

                            default:
                                httpParser->state = state;
                                /*SET_ERRNO(HPE_CB_headers_complete);*/
                                return pointer - data; /* Error */
                        }
                    }

                    /* Exit, the rest of the connect is in a different protocol. */
                    if(httpParser->upgrade) {
                        HTTP_CALLBACK(messageComplete);

                        return (pointer - data) + 1;
                    }

                    if(httpParser->flags & FLAG_SKIPBODY) {
                        HTTP_CALLBACK(messageComplete);
                        state = NEW_MESSAGE();
                    } else if(httpParser->flags & FLAG_CHUNKED) {
                        /* chunked encoding - ignore Content-Length header */
                        state = STATE_CHUNK_SIZE_START;
                    } else {
                        if(httpParser->contentLength == 0) {
                            /* Content-Length header given but zero: Content-Length: 0\r\n */
                            HTTP_CALLBACK(messageComplete);
                            state = NEW_MESSAGE();
                        } else if(httpParser->contentLength > 0) {
                            /* Content-Length header given and non-zero */
                            state = STATE_BODY_IDENTITY;
                        } else {
                            if(httpParser->type == HTTP_REQUEST || httpShouldKeepAlive(httpParser)) {
                                /* Assume content-length 0 - read the next */
                                HTTP_CALLBACK(messageComplete);
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
                toRead = MIN(pointerEnd - pointer, (long long) httpParser->contentLength);

                if(toRead > 0) {
                    if(httpSettings->onbody) {
                        httpSettings->onbody(httpParser, pointer, toRead);
                    }

                    pointer += toRead - 1;

                    httpParser->contentLength -= toRead;

                    if(httpParser->contentLength == 0) {
                        HTTP_CALLBACK(messageComplete);
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
                        httpSettings->onbody(httpParser, pointer, toRead);
                    }

                    pointer += toRead - 1;
                }

                /* breaks the switch */
                break;

            case STATE_CHUNK_SIZE_START:
                assert(readCount == 1);
                assert(httpParser->flags & FLAG_CHUNKED);

                unhexValue = unhex[byte];

                if(unhexValue == -1) {
                    /*SET_ERRNO(HPE_INVALID_CHUNK_SIZE);
                    goto error;*/
                }

                httpParser->contentLength = unhexValue;
                state = STATE_CHUNK_SIZE;

                /* breaks the switch */
                break;

            case STATE_CHUNK_SIZE:
                assert(httpParser->flags & FLAG_CHUNKED);

                if(byte == CR) {
                    state = STATE_CHUNK_SIZE_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                unhexValue = unhex[byte];

                if(unhexValue == -1) {
                    if(byte == ';' || byte == ' ') {
                        state = STATE_CHUNK_PARAMETERS;

                        /* breaks the switch */
                        break;
                    }

                    /*SET_ERRNO(HPE_INVALID_CHUNK_SIZE);
                    goto error;*/
                }

                httpParser->contentLength *= 16;
                httpParser->contentLength += unhexValue;

                /* breaks the switch */
                break;

            case STATE_CHUNK_PARAMETERS:
                assert(httpParser->flags & FLAG_CHUNKED);

                /* just ignore this shit. TODO check for overflow */
                if(byte == CR) {
                    state = STATE_CHUNK_SIZE_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                /* breaks the switch */
                break;

            case STATE_CHUNK_SIZE_ALMOST_DONE:
                assert(httpParser->flags & FLAG_CHUNKED);
                STRICT_CHECK(byte != LF);

                readCount = 0;

                if(httpParser->contentLength == 0) {
                    httpParser->flags |= FLAG_TRAILING;
                    state = STATE_HEADER_FIELD_START;
                } else {
                    state = STATE_CHUNK_DATA;
                }

                /* breaks the switch */
                break;

            case STATE_CHUNK_DATA:
                assert(httpParser->flags & FLAG_CHUNKED);

                toRead = MIN(pointerEnd - pointer, (long long) (httpParser->contentLength));

                if(toRead > 0) {
                    if (httpSettings->onbody) {
                        httpSettings->onbody(httpParser, pointer, toRead);
                    }

                    pointer += toRead - 1;
                }

                if(toRead == httpParser->contentLength) {
                    state = STATE_CHUNK_DATA_ALMOST_DONE;
                }

                httpParser->contentLength -= toRead;

                /* breaks the switch */
                break;

            case STATE_CHUNK_DATA_ALMOST_DONE:
                assert(httpParser->flags & FLAG_CHUNKED);
                STRICT_CHECK(byte != CR);
                state = STATE_CHUNK_DATA_DONE;

                /* breaks the switch */
                break;

            case STATE_CHUNK_DATA_DONE:
                assert(httpParser->flags & FLAG_CHUNKED);
                STRICT_CHECK(byte != LF);
                state = STATE_CHUNK_SIZE_START;

                /* breaks the switch */
                break;

            default:
                assert(0 && "unhandled state");
                /*SET_ERRNO(HPE_INVALID_INTERNAL_STATE);
                goto error;*/
        }
    }

    HTTP_CALLBACK_DATA(headerField);
    HTTP_CALLBACK_DATA(headerValue);
    HTTP_CALLBACK_DATA(url);

    httpParser->state = state;
    httpParser->headerState = headerState;
    httpParser->index = index;
    httpParser->readCount = readCount;

    /* returns the data size (processed data size) */
    return dataSize;
}
