/*
 Hive Viriatum Commons
 Copyright (c) 2008-2016 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "url.h"

ERROR_CODE parse_url(char *url, size_t url_size, struct url_t *url_s) {
    /* allocates space for the index and length counters to be
    used on the parse of the provided url */
    size_t index;
    size_t length;

    /* sets space for the variables to be set with the current
    (byte) in parsing and for the current state of the parse */
    unsigned char current;
    enum url_parse_state_e state;

    /* allocates space for the internal markers and pointers to
    be used durring the parsin main loop */
    unsigned char *mark;
    unsigned char *pointer;

    /* creates space for a "local" buffer that is going to
    be used as temporary for some of the conversion operations */
    unsigned char buffer[128];

    /* resets the url structure by setting all of its contents
    to the default zero (reset) value */
    memset(url_s, 0, sizeof(struct url_t));

    /* sets the url pointer as the initial mark value and also
    sets the initial state of the parsing as the scheme state */
    mark = (unsigned char *) url;
    state = SCHEME_STATE;

    /* iterate over the complete url buffer to try to gather
    the range of the various parts of the url */
    for(index = 0; index < url_size; index++) {
        /* retrieves the current byte in iteration and the pointer
        address to the current buffer position */
        current = (unsigned char) url[index];
        pointer = (unsigned char *) &url[index];

        switch(state) {
            case SCHEME_STATE:
                switch(current) {
                    case ':':
                        length = pointer - mark;
                        url_s->scheme.buffer = mark;
                        url_s->scheme.length = length;
                        state = SLA_STATE;
                        break;
                }

                break;

            case SLA_STATE:
                switch(current) {
                    case '/':
                        state = SLA_SLA_STATE;
                        break;

                    default:
                        goto error;
                        break;
                }

                break;

            case SLA_SLA_STATE:
                switch(current) {
                    case '/':
                        state = AUTH_STATE;
                        mark = pointer + 1;
                        break;

                    default:
                        goto error;
                        break;
                }

                break;

            case AUTH_STATE:
                switch(current) {
                    case '/':
                        length = pointer - mark;
                        url_s->host_c.buffer = mark;
                        url_s->host_c.length = length;
                        state = PATH_STATE;
                        mark = pointer;
                        break;

                    case '@':
                        length = pointer - mark;
                        url_s->auth.buffer = mark;
                        url_s->auth.length = length;
                        state = HOST_STATE;
                        mark = pointer + 1;
                        break;
                }

                break;

            case HOST_STATE:
                switch(current) {
                    case '/':
                        length = pointer - mark;
                        url_s->host.buffer = mark;
                        url_s->host.length = length;
                        state = PATH_STATE;
                        mark = pointer;
                        break;

                    case ':':
                        length = pointer - mark;
                        url_s->host.buffer = mark;
                        url_s->host.length = length;
                        state = PORT_STATE;
                        mark = pointer + 1;
                        break;
                }

                break;

            case PORT_STATE:
                switch(current) {
                    case '/':
                        length = pointer - mark;
                        url_s->port_s.buffer = mark;
                        url_s->port_s.length = length;
                        state = PATH_STATE;
                        mark = pointer;
                        break;
                }

                break;

            case PATH_STATE:
                switch(current) {
                    case '?':
                        length = pointer - mark;
                        url_s->path.buffer = mark;
                        url_s->path.length = length;
                        state = QUERY_STATE;
                        mark = pointer + 1;
                        break;
                }

                break;

            case QUERY_STATE:
                switch(current) {
                    case '#':
                        length = pointer - mark;
                        url_s->query.buffer = mark;
                        url_s->query.length = length;
                        state = FRAGMENT_STATE;
                        mark = pointer + 1;
                        break;
                }

                break;

            case FRAGMENT_STATE:
                break;
        }
    }
    pointer++;

    /* runs the final swith operation to clone the still
    opened parse step (the final step) */
    switch(state) {
        case PATH_STATE:
            length = pointer - mark;
            url_s->path.buffer = mark;
            url_s->path.length = length;
            break;

        case QUERY_STATE:
            length = pointer - mark;
            url_s->query.buffer = mark;
            url_s->query.length = length;
            break;

        case FRAGMENT_STATE:
            length = pointer - mark;
            url_s->fragment.buffer = mark;
            url_s->fragment.length = length;
            break;

        default:
            goto error;
            break;
    }

    if(url_s->host_c.buffer != NULL) {
        pointer = memchr(url_s->host_c.buffer, ':', url_s->host_c.length);
        if(pointer == NULL) {
            url_s->host.buffer = url_s->host_c.buffer;
            url_s->host.length = url_s->host_c.length;
        } else {
            url_s->host.buffer = url_s->host_c.buffer;
            url_s->host.length = pointer - url_s->host_c.buffer;

            url_s->port_s.buffer = pointer + 1;
            url_s->port_s.length = url_s->host_c.buffer +\
                url_s->host_c.length - (pointer + 1);
        }
    }

    if(url_s->auth.buffer != NULL) {
        pointer = memchr(url_s->auth.buffer, ':', url_s->auth.length);
        if(pointer == NULL) { goto error; }

        url_s->username.buffer = url_s->auth.buffer;
        url_s->username.length = pointer - url_s->auth.buffer;

        url_s->password.buffer = pointer + 1;
        url_s->password.length = url_s->auth.buffer +\
            url_s->auth.length - (pointer + 1);
    }

    if(url_s->port_s.buffer != NULL) {
        memcpy(buffer, url_s->port_s.buffer, url_s->port_s.length);
        buffer[url_s->port_s.length] = '\0';

        url_s->port = atoi((char *) buffer);
    }

    RAISE_NO_ERROR;

error:
    RAISE_ERROR_M(
        RUNTIME_EXCEPTION_ERROR_CODE,
        (unsigned char *) "Problem parsing url string"
    );
}

ERROR_CODE parse_url_static(char *url, size_t url_size, struct url_static_t *url_s) {
    /* allocates space for the temporary error variable to
    be used to detect errors in calls */
    ERROR_CODE error;

    /* allocates the space for the temporary (dynamic) url structure
    to be used as basis for the construction of the static one */
    struct url_t _url_s;

    error = parse_url(url, url_size, &_url_s);
    if(error) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem parsing the url"
        );
    }

    memset(url_s, 0, sizeof(struct url_static_t));

    if(_url_s.scheme.buffer != NULL) {
        memcpy(url_s->scheme, _url_s.scheme.buffer, _url_s.scheme.length);
        url_s->scheme[_url_s.scheme.length] = '\0';
    }

    if(_url_s.username.buffer != NULL) {
        memcpy(url_s->username, _url_s.username.buffer, _url_s.username.length);
        url_s->username[_url_s.username.length] = '\0';
    }

    if(_url_s.password.buffer != NULL) {
        memcpy(url_s->password, _url_s.password.buffer, _url_s.password.length);
        url_s->password[_url_s.password.length] = '\0';
    }

    if(_url_s.host.buffer != NULL) {
        memcpy(url_s->host, _url_s.host.buffer, _url_s.host.length);
        url_s->password[_url_s.host.length] = '\0';
    }

    if(_url_s.port != 0) { url_s->port = _url_s.port; }

    if(_url_s.path.buffer != NULL) {
        memcpy(url_s->path, _url_s.path.buffer, _url_s.path.length);
        url_s->password[_url_s.path.length] = '\0';
    }

    if(_url_s.query.buffer != NULL) {
        memcpy(url_s->query, _url_s.query.buffer, _url_s.query.length);
        url_s->password[_url_s.query.length] = '\0';
    }

    if(_url_s.fragment.buffer != NULL) {
        memcpy(url_s->fragment, _url_s.fragment.buffer, _url_s.fragment.length);
        url_s->password[_url_s.fragment.length] = '\0';
    }

    RAISE_NO_ERROR;
}
