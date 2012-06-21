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

int http_should_keep_alive(struct http_parser_t *http_parser) {
    /* in case the request is of type http 1.1 */
    if(http_parser->http_major > 0 && http_parser->http_minor > 0) {
        if(http_parser->flags & FLAG_CONNECTION_CLOSE) {
            return 0;
        } else {
            return 1;
        }
    }
    /* in case the request is of type http 1.0 or earlier */
    else {
        if(http_parser->flags & FLAG_CONNECTION_KEEP_ALIVE) {
            return 1;
        } else {
            return 0;
        }
    }
}

void create_http_parser(struct http_parser_t **http_parser_pointer, char request) {
    /* retrieves the http parser size */
    size_t http_parser_size = sizeof(struct http_parser_t);

    /* allocates space for the http parser */
    struct http_parser_t *http_parser = (struct http_parser_t *) MALLOC(http_parser_size);

    /* sets the http parser attributes, in the original
    definition, this should be able to start a parsing */
    http_parser->type = 2;
    http_parser->flags = 6;
    http_parser->state = request ? STATE_START_REQ : STATE_START_RES;
    http_parser->header_state = 0;
    http_parser->read_count = 0;
    http_parser->content_length = -1;
    http_parser->http_major = 0;
    http_parser->http_minor = 0;
    http_parser->status_code = 0;
    http_parser->method = 0;
    http_parser->upgrade = 0;
    http_parser->context = NULL;
    http_parser->parameters = NULL;
    http_parser->_content_length = 0;

    /* sets the http parser in the http parser pointer */
    *http_parser_pointer = http_parser;
}

void delete_http_parser(struct http_parser_t *http_parser) {
    /* releases the http parser */
    FREE(http_parser);
}

void create_http_settings(struct http_settings_t **http_settings_pointer) {
    /* retrieves the http settings size */
    size_t http_settings_size = sizeof(struct http_settings_t);

    /* allocates space for the http settings */
    struct http_settings_t *http_settings = (struct http_settings_t *) MALLOC(http_settings_size);

    /* sets the http settings callback values to
    the default settings (unset) */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;
    http_settings->on_path = NULL;
    http_settings->on_location = NULL;
    http_settings->on_virtual_url = NULL;

    /* sets the http settings in the http settings pointer */
    *http_settings_pointer = http_settings;
}

void delete_http_settings(struct http_settings_t *http_settings) {
    /* releases the http settings */
    FREE(http_settings);
}

int process_data_http_parser(struct http_parser_t *http_parser, struct http_settings_t *http_settings, unsigned char *data, size_t data_size) {
    unsigned char byte;
    unsigned char byte_token;
    char unhex_value;
    const char *matcher;
    size_t to_read;
    const unsigned char *pointer_end;
    const unsigned char *pointer = data;
    size_t read_count = http_parser->read_count;
    size_t index = http_parser->index;
    unsigned char state = http_parser->state;
    unsigned char header_state = 0;

    const unsigned char *header_field_mark = 0;
    const unsigned char *header_value_mark = 0;
    const unsigned char *url_mark = 0;

    /* in case the received data size is empty */
    if(data_size == 0) {
        /* switches over the state */
        switch(state) {
            case STATE_BODY_IDENTITY_EOF:
                HTTP_CALLBACK(message_complete);

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
    for(pointer = data, pointer_end = data + data_size; pointer != pointer_end; pointer++) {
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
                http_parser->flags = 0;
                http_parser->content_length = -1;

                HTTP_CALLBACK(message_begin);

                if(byte == 'H') {
                    state = STATE_RES_OR_RESP_H;
                } else {
                    http_parser->type = HTTP_REQUEST;
                    goto start_req_method_assign;
                }

                /* breaks the switch */
                break;

            case STATE_RES_OR_RESP_H:
                if (byte == 'T') {
                    http_parser->type = HTTP_RESPONSE;
                    state = STATE_RES_HT;
                } else {
                    if (byte != 'E') {
                        /*SET_ERRNO(HPE_INVALID_CONSTANT); */
                        /*goto error; */
                    }

                    http_parser->type = HTTP_REQUEST;
                    http_parser->method = HTTP_HEAD;
                    index = 2;
                    state = STATE_REQ_METHOD;
                }

                /* breaks the switch */
                break;

            case STATE_START_RES:
                http_parser->flags = 0;
                http_parser->content_length = -1;

                HTTP_CALLBACK(message_begin);

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
                if(byte < '1' || byte  > '9') {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                http_parser->http_major = byte - '0';
                state = STATE_RES_HTTP_MAJOR;

                /* breaks the switch */
                break;

             case STATE_RES_HTTP_MAJOR:
                if(byte == '.') {
                    state = STATE_RES_FIRST_HTTP_MINOR;
                    break;
                }

                if(!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                http_parser->http_major *= 10;
                http_parser->http_major += byte - '0';

                if(http_parser->http_major > 999) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                /* breaks the switch */
                break;

            case STATE_RES_FIRST_HTTP_MINOR:
                if (!IS_NUM(byte)) {
                    /*SET_ERRNO(HPE_INVALID_VERSION);
                    goto error;*/
                }

                http_parser->http_minor = byte - '0';
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

                http_parser->http_minor *= 10;
                http_parser->http_minor += byte - '0';

                if(http_parser->http_minor > 999) {
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

                http_parser->status_code = byte - '0';
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

                http_parser->status_code *= 10;
                http_parser->status_code += byte - '0';

                if(http_parser->status_code > 999) {
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

                http_parser->flags = 0;
                http_parser->content_length = -1;

                HTTP_CALLBACK(message_begin);

                if(!IS_ALPHA(byte)) {
                    /*
                    SET_ERRNO(HPE_INVALID_METHOD);
                    goto error;
                    */
                }

                start_req_method_assign:
                    http_parser->method = (unsigned char) 0;
                    index = 1;

                    switch(byte) {
                        case 'C': http_parser->method = HTTP_CONNECT; /* or COPY, CHECKOUT */ break;
                        case 'D': http_parser->method = HTTP_DELETE; break;
                        case 'G': http_parser->method = HTTP_GET; break;
                        case 'H': http_parser->method = HTTP_HEAD; break;
                        case 'L': http_parser->method = HTTP_LOCK; break;
                        case 'M': http_parser->method = HTTP_MKCOL; /* or MOVE, MKACTIVITY, MERGE, M-SEARCH */ break;
                        case 'N': http_parser->method = HTTP_NOTIFY; break;
                        case 'O': http_parser->method = HTTP_OPTIONS; break;
                        case 'P': http_parser->method = HTTP_POST; /* or PROPFIND or PROPPATCH or PUT or PATCH */ break;
                        case 'R': http_parser->method = HTTP_REPORT; break;
                        case 'S': http_parser->method = HTTP_SUBSCRIBE; break;
                        case 'T': http_parser->method = HTTP_TRACE; break;
                        case 'U': http_parser->method = HTTP_UNLOCK; /* or UNSUBSCRIBE */ break;

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

                matcher = http_method_strings[http_parser->method - 1];

                if(byte == ' ' && matcher[index] == '\0') {
                    state = STATE_REQ_SPACES_BEFORE_URL;
                } else if(byte == matcher[index]) {
                } else if(http_parser->method == HTTP_CONNECT) {
                    if(index == 1 && byte == 'H') {
                        http_parser->method = HTTP_CHECKOUT;
                    } else if(index == 2  && byte == 'P') {
                        http_parser->method = HTTP_COPY;
                    } else {
                        /*goto error;*/
                    }
                } else if(http_parser->method == HTTP_MKCOL) {
                    if(index == 1 && byte == 'O') {
                        http_parser->method = HTTP_MOVE;
                    } else if(index == 1 && byte == 'E') {
                        http_parser->method = HTTP_MERGE;
                    } else if(index == 1 && byte == '-') {
                        http_parser->method = HTTP_MSEARCH;
                    } else if(index == 2 && byte == 'A') {
                        http_parser->method = HTTP_MKACTIVITY;
                    } else {
                        /*goto error;*/
                    }
                } else if(index == 1 && http_parser->method == HTTP_POST) {
                    if(byte == 'R') {
                        http_parser->method = HTTP_PROPFIND; /* or HTTP_PROPPATCH */
                    } else if(byte == 'U') {
                        http_parser->method = HTTP_PUT;
                    } else if(byte == 'A') {
                        http_parser->method = HTTP_PATCH;
                    } else {
                        /*goto error;*/
                    }
                } else if(index == 2 && http_parser->method == HTTP_UNLOCK && byte == 'S') {
                    http_parser->method = HTTP_UNSUBSCRIBE;
                } else if(index == 4 && http_parser->method == HTTP_PROPFIND && byte == 'P') {
                    http_parser->method = HTTP_PROPPATCH;
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
                if(IS_ALPHA(byte) || (http_parser->method == HTTP_CONNECT && IS_NUM(byte))) {
                    HTTP_MARK(url);
                    state = (http_parser->method == HTTP_CONNECT) ? STATE_REQ_HOST : STATE_REQ_SCHEMA;

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
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
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
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
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
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
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
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                    case LF:
                        HTTP_CALLBACK_DATA(url);
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
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
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
                        state = STATE_REQ_LINE_ALMOST_DONE;

                        /* breaks the switch */
                        break;

                  case LF:
                        HTTP_CALLBACK_DATA(url);
                        http_parser->http_major = 0;
                        http_parser->http_minor = 9;
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

                http_parser->http_major = byte - '0';
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

                http_parser->http_major *= 10;
                http_parser->http_major += byte - '0';

                if(http_parser->http_major > 999) {
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

                http_parser->http_minor = byte - '0';
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

                http_parser->http_minor *= 10;
                http_parser->http_minor += byte - '0';

                if (http_parser->http_minor > 999) {
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
                header_field_start:
                    if(byte == CR) {
                        state = STATE_HEADERS_ALMOST_DONE;

                        /* breaks the switch */
                        break;
                    }

                    if(byte == LF) {
                        /* they might be just sending \n instead of \r\n so this would be
                        * the second \n to denote the end of headers*/
                        state = STATE_HEADERS_ALMOST_DONE;

                        goto headers_almost_done;
                    }

                    byte_token = TOKEN(byte);

                    if (!byte_token) {
                        /*SET_ERRNO(HPE_INVALID_HEADER_TOKEN);
                        goto error;*/
                    }

                    HTTP_MARK(header_field);

                    index = 0;
                    state = STATE_HEADER_FIELD;

                    switch(byte_token) {
                        case 'c':
                            header_state = HEADER_STATE_C;

                            /* breaks the switch */
                            break;

                        case 'p':
                            header_state = HEADER_STATE_MATCHING_PROXY_CONNECTION;

                            /* breaks the switch */
                            break;

                        case 't':
                            header_state = HEADER_STATE_MATCHING_TRANSFER_ENCODING;

                            /* breaks the switch */
                            break;

                        case 'u':
                            header_state = HEADER_STATE_MATCHING_UPGRADE;

                            /* breaks the switch */
                            break;

                        default:
                            header_state = HEADER_STATE_GENERAL;

                            /* breaks the switch */
                            break;
                    }

                    /* breaks the switch */
                    break;

            case STATE_HEADER_FIELD:
                byte_token = TOKEN(byte);

                if(byte_token) {
                    switch(header_state) {
                        case HEADER_STATE_GENERAL:
                            /* breaks the switch */
                            break;

                        case HEADER_STATE_C:
                            index++;
                            header_state = (byte_token == 'o' ? HEADER_STATE_CO : HEADER_STATE_GENERAL);

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CO:
                            index++;
                            header_state = (byte_token == 'n' ? HEADER_STATE_CON : HEADER_STATE_GENERAL);

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CON:
                            index++;

                            switch(byte_token) {
                                case 'n':
                                    header_state = HEADER_STATE_MATCHING_CONNECTION;
                                    break;

                                case 't':
                                    header_state = HEADER_STATE_MATCHING_CONTENT_LENGTH;
                                    break;

                                default:
                                    header_state = HEADER_STATE_GENERAL;
                                    break;
                            }

                            /* breaks the switch */
                            break;

                        /* connection */
                        case HEADER_STATE_MATCHING_CONNECTION:
                            index++;

                            if(index > sizeof(HTTP_CONNECTION) - 1 || byte_token != HTTP_CONNECTION[index]) {
                                header_state = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_CONNECTION) - 2) {
                                header_state = HEADER_STATE_CONNECTION;
                            }

                            /* breaks the switch */
                            break;

                        /* proxy-connection */
                        case HEADER_STATE_MATCHING_PROXY_CONNECTION:
                            index++;

                            if(index > sizeof(HTTP_PROXY_CONNECTION) - 1 || byte_token != HTTP_PROXY_CONNECTION[index]) {
                                header_state = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_PROXY_CONNECTION) - 2) {
                                header_state = HEADER_STATE_CONNECTION;
                            }

                            /* breaks the switch */
                            break;

                        /* content-length */
                        case HEADER_STATE_MATCHING_CONTENT_LENGTH:
                            index++;

                            if(index > sizeof(HTTP_CONTENT_LENGTH) - 1 || byte_token != HTTP_CONTENT_LENGTH[index]) {
                                header_state = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_CONTENT_LENGTH) - 2) {
                                header_state = HEADER_STATE_CONTENT_LENGTH;
                            }

                            /* breaks the switch */
                            break;

                        /* transfer-encoding */
                        case HEADER_STATE_MATCHING_TRANSFER_ENCODING:
                            index++;

                            if(index > sizeof(HTTP_TRANSFER_ENCODING) - 1 || byte_token != HTTP_TRANSFER_ENCODING[index]) {
                                header_state = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_TRANSFER_ENCODING) - 2) {
                                header_state = HEADER_STATE_TRANSFER_ENCODING;
                            }

                            /* breaks the switch */
                            break;

                        /* upgrade */
                        case HEADER_STATE_MATCHING_UPGRADE:
                            index++;

                            if(index > sizeof(HTTP_UPGRADE) - 1 || byte_token != HTTP_UPGRADE[index]) {
                                header_state = HEADER_STATE_GENERAL;
                            } else if(index == sizeof(HTTP_UPGRADE) - 2) {
                                header_state = HEADER_STATE_UPGRADE;
                            }

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CONNECTION:
                        case HEADER_STATE_CONTENT_LENGTH:
                        case HEADER_STATE_TRANSFER_ENCODING:
                        case HEADER_STATE_UPGRADE:
                            if(byte != ' ') {
                                header_state = HEADER_STATE_GENERAL;
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
                    HTTP_CALLBACK_DATA(header_field);
                    state = STATE_HEADER_VALUE_START;

                    /* breaks the switch */
                    break;
                }

                if(byte == CR) {
                    state = STATE_HEADER_ALMOST_DONE;
                    HTTP_CALLBACK_DATA(header_field);

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK_DATA(header_field);
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

                HTTP_MARK(header_value);

                state = STATE_HEADER_VALUE;

                index = 0;

                if(byte == CR) {
                    HTTP_CALLBACK_DATA(header_value);
                    header_state = HEADER_STATE_GENERAL;
                    state = STATE_HEADER_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK_DATA(header_value);
                    state = STATE_HEADER_FIELD_START;

                    /* breaks the switch */
                    break;
                }

                byte_token = LOWER(byte);

                switch(header_state) {
                    case HEADER_STATE_UPGRADE:
                        http_parser->flags |= FLAG_UPGRADE;
                        header_state = HEADER_STATE_GENERAL;

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_TRANSFER_ENCODING:
                        if('c' == byte_token) {
                            header_state = HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED;
                        } else {
                            header_state = HEADER_STATE_GENERAL;
                        }

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_CONTENT_LENGTH:
                        if(!IS_NUM(byte)) {
                            /*SET_ERRNO(HPE_INVALID_CONTENT_LENGTH);
                            goto error;*/
                        }

                        http_parser->content_length = byte - '0';

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_CONNECTION:
                        /* looking for 'Connection: keep-alive' */
                        if(byte_token == 'k') {
                            header_state = HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE;
                        /* looking for 'Connection: close' */
                        } else if(byte_token == 'c') {
                            header_state = HEADER_STATE_MATCHING_CONNECTION_CLOSE;
                        } else {
                            header_state = HEADER_STATE_GENERAL;
                        }

                        /* breaks the switch */
                        break;

                    default:
                        header_state = HEADER_STATE_GENERAL;

                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;

            case STATE_HEADER_VALUE:
                if(byte == CR) {
                    HTTP_CALLBACK_DATA(header_value);
                    state = STATE_HEADER_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                if(byte == LF) {
                    HTTP_CALLBACK_DATA(header_value);
                    goto header_almost_done;
                }

                byte_token = LOWER(byte);

                switch(header_state) {
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

                        http_parser->content_length *= 10;
                        http_parser->content_length += byte - '0';

                        /* breaks the switch */
                        break;

                    /* Transfer-Encoding: chunked */
                    case HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED:
                        index++;

                        if(index > sizeof(HTTP_CHUNKED) - 1 || byte_token != HTTP_CHUNKED[index]) {
                            header_state = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_CHUNKED) - 2) {
                            header_state = HEADER_STATE_TRANSFER_ENCODING_CHUNKED;
                        }

                        /* breaks the switch */
                        break;

                    /* looking for 'Connection: keep-alive' */
                    case HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE:
                        index++;

                        if(index > sizeof(HTTP_KEEP_ALIVE) - 1 || byte_token != HTTP_KEEP_ALIVE[index]) {
                            header_state = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_KEEP_ALIVE) - 2) {
                            header_state = HEADER_STATE_CONNECTION_KEEP_ALIVE;
                        }

                        /* breaks the switch */
                        break;

                    /* looking for 'Connection: close' */
                    case HEADER_STATE_MATCHING_CONNECTION_CLOSE:
                        index++;

                        if(index > sizeof(HTTP_CLOSE) - 1 || byte_token != HTTP_CLOSE[index]) {
                            header_state = HEADER_STATE_GENERAL;
                        } else if(index == sizeof(HTTP_CLOSE) - 2) {
                            header_state = HEADER_STATE_CONNECTION_CLOSE;
                        }

                        /* breaks the switch */
                        break;

                    case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                    case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                    case HEADER_STATE_CONNECTION_CLOSE:
                        if(byte != ' ') {
                            header_state = HEADER_STATE_GENERAL;
                        }

                        /* breaks the switch */
                        break;

                    default:
                        state = STATE_HEADER_VALUE;
                        header_state = HEADER_STATE_GENERAL;

                        /* breaks the switch */
                        break;
                }

                /* breaks the switch */
                break;


            case STATE_HEADER_ALMOST_DONE:
                header_almost_done:
                    STRICT_CHECK(byte != LF);

                    state = STATE_HEADER_VALUE_LWS;

                    switch(header_state) {
                        case HEADER_STATE_CONNECTION_KEEP_ALIVE:
                            http_parser->flags |= FLAG_CONNECTION_KEEP_ALIVE;

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_CONNECTION_CLOSE:
                            http_parser->flags |= FLAG_CONNECTION_CLOSE;

                            /* breaks the switch */
                            break;

                        case HEADER_STATE_TRANSFER_ENCODING_CHUNKED:
                            http_parser->flags |= FLAG_CHUNKED;

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
                    goto header_field_start;
                }

                /* breaks the switch */
                break;

            case STATE_HEADERS_ALMOST_DONE:
                headers_almost_done:
                    STRICT_CHECK(byte != LF);

                    if(http_parser->flags & FLAG_TRAILING) {
                        /* End of a chunked request */
                        HTTP_CALLBACK(message_complete);
                        state = NEW_MESSAGE();
                        break;
                    }

                    read_count = 0;

                    if(http_parser->flags & FLAG_UPGRADE || http_parser->method == HTTP_CONNECT) {
                        http_parser->upgrade = 1;
                    }

                    /* here we call the headers_complete callback, this is somewhat
                    * different than other callbacks because if the user returns 1, we
                    * will interpret that as saying that this message has no body. This
                    * is needed for the annoying case of recieving a response to a HEAD
                    * request */
                    if(http_settings->on_headers_complete) {
                        switch (http_settings->on_headers_complete(http_parser)) {
                            case 0:
                                break;

                            case 1:
                                http_parser->flags |= FLAG_SKIPBODY;
                                break;

                            default:
                                http_parser->state = state;
                                /*SET_ERRNO(HPE_CB_headers_complete);*/
                                return pointer - data; /* Error */
                        }
                    }

                    /* Exit, the rest of the connect is in a different protocol. */
                    if(http_parser->upgrade) {
                        HTTP_CALLBACK(message_complete);

                        return (pointer - data) + 1;
                    }

                    if(http_parser->flags & FLAG_SKIPBODY) {
                        HTTP_CALLBACK(message_complete);
                        state = NEW_MESSAGE();
                    } else if(http_parser->flags & FLAG_CHUNKED) {
                        /* chunked encoding - ignore Content-Length header */
                        state = STATE_CHUNK_SIZE_START;
                    } else {
                        if(http_parser->content_length == 0) {
                            /* Content-Length header given but zero: Content-Length: 0\r\n */
                            HTTP_CALLBACK(message_complete);
                            state = NEW_MESSAGE();
                        } else if(http_parser->content_length > 0) {
                            /* Content-Length header given and non-zero */
                            state = STATE_BODY_IDENTITY;

                            /* saves the content length value in the private
                            value so that it can be used later */
                            http_parser->_content_length = http_parser->content_length;
                        } else {
                            if(http_parser->type == HTTP_REQUEST || http_should_keep_alive(http_parser)) {
                                /* Assume content-length 0 - read the next */
                                HTTP_CALLBACK(message_complete);
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
                to_read = MIN(pointer_end - pointer, (long long) http_parser->content_length);

                if(to_read > 0) {
                    CALL_V(http_settings->on_body, http_parser, pointer, to_read);

                    pointer += to_read - 1;

                    http_parser->content_length -= to_read;

                    if(http_parser->content_length == 0) {
                        HTTP_CALLBACK(message_complete);
                        state = NEW_MESSAGE();
                    }
                }

                /* breaks the switch */
                break;

            /* read until EOF */
            case STATE_BODY_IDENTITY_EOF:
                to_read = pointer_end - pointer;

                if(to_read > 0) {
                    CALL_V(http_settings->on_body, http_parser, pointer, to_read);

                    pointer += to_read - 1;
                }

                /* breaks the switch */
                break;

            case STATE_CHUNK_SIZE_START:
                assert(read_count == 1);
                assert(http_parser->flags & FLAG_CHUNKED);

                unhex_value = unhex[byte];

                if(unhex_value == -1) {
                    /*SET_ERRNO(HPE_INVALID_CHUNK_SIZE);
                    goto error;*/
                }

                http_parser->content_length = unhex_value;
                state = STATE_CHUNK_SIZE;

                /* breaks the switch */
                break;

            case STATE_CHUNK_SIZE:
                assert(http_parser->flags & FLAG_CHUNKED);

                if(byte == CR) {
                    state = STATE_CHUNK_SIZE_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                unhex_value = unhex[byte];

                if(unhex_value == -1) {
                    if(byte == ';' || byte == ' ') {
                        state = STATE_CHUNK_PARAMETERS;

                        /* breaks the switch */
                        break;
                    }

                    /*SET_ERRNO(HPE_INVALID_CHUNK_SIZE);
                    goto error;*/
                }

                http_parser->content_length *= 16;
                http_parser->content_length += unhex_value;

                /* breaks the switch */
                break;

            case STATE_CHUNK_PARAMETERS:
                assert(http_parser->flags & FLAG_CHUNKED);

                /* just ignore this shit. TODO check for overflow */
                if(byte == CR) {
                    state = STATE_CHUNK_SIZE_ALMOST_DONE;

                    /* breaks the switch */
                    break;
                }

                /* breaks the switch */
                break;

            case STATE_CHUNK_SIZE_ALMOST_DONE:
                assert(http_parser->flags & FLAG_CHUNKED);
                STRICT_CHECK(byte != LF);

                read_count = 0;

                if(http_parser->content_length == 0) {
                    http_parser->flags |= FLAG_TRAILING;
                    state = STATE_HEADER_FIELD_START;
                } else {
                    state = STATE_CHUNK_DATA;
                }

                /* breaks the switch */
                break;

            case STATE_CHUNK_DATA:
                assert(http_parser->flags & FLAG_CHUNKED);

                to_read = MIN(pointer_end - pointer, (long long) (http_parser->content_length));

                if(to_read > 0) {
                    CALL_V(http_settings->on_body, http_parser, pointer, to_read);

                    pointer += to_read - 1;
                }

                if(to_read == http_parser->content_length) {
                    state = STATE_CHUNK_DATA_ALMOST_DONE;
                }

                http_parser->content_length -= to_read;

                /* breaks the switch */
                break;

            case STATE_CHUNK_DATA_ALMOST_DONE:
                assert(http_parser->flags & FLAG_CHUNKED);
                STRICT_CHECK(byte != CR);
                state = STATE_CHUNK_DATA_DONE;

                /* breaks the switch */
                break;

            case STATE_CHUNK_DATA_DONE:
                assert(http_parser->flags & FLAG_CHUNKED);
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

    if(state == STATE_BODY_IDENTITY_EOF) {
        HTTP_CALLBACK(message_complete);
        state = NEW_MESSAGE();
    }

    HTTP_CALLBACK_DATA(header_field);
    HTTP_CALLBACK_DATA(header_value);
    HTTP_CALLBACK_DATA(url);

    http_parser->state = state;
    http_parser->header_state = header_state;
    http_parser->index = index;
    http_parser->read_count = read_count;

    /* returns the data size (processed data size) */
    return data_size;
}
