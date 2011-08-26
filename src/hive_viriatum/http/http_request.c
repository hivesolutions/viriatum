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

    /* sets the http request content length */
    httpRequest->contentLength = 0;

    /* sets the http request in the http request pointer */
    *httpRequestPointer = httpRequest;
}

void deleteHttpRequest(struct HttpRequest_t *httpRequest) {
    /* releases the http request */
    free(httpRequest);
}

int processDataHttpRequest(struct HttpRequest_t *httpRequest, unsigned char *data, size_t dataSize) {
    /* Inspired from https://github.com/ry/http-parser/blob/master/http_parser.c */

    unsigned char byte;
    const unsigned char *pointerEnd;
    const unsigned char *pointer = data;
    enum HttpRequestState_e state = httpRequest->state;

    /* in case the received data size is empty */
    if(dataSize == 0) {
        /* switches over the state */
        switch(state) {
            case STATE_BODY_IDENTITY_EOF:
                /* CALLBACK2(message_complete); */

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

                /* CALLBACK2(message_begin); */

                if(byte == 'H') {
                    state = STATE_RES_OR_RESP_H;
                } else {
                    httpRequest->type = HTTP_REQUEST;
                /*    goto start_req_method_assign; */
                }

                /* breaks the switch */
                break;
        }
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
