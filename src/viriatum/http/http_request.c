/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "http_request.h"

void create_http_request(struct http_request_t **http_request_pointer) {
    /* retrieves the HTTP request size */
    size_t http_request_size = sizeof(struct http_request_t);

    /* allocates space for the HTTP request */
    struct http_request_t *http_request = (struct http_request_t *) MALLOC(http_request_size);

    /* sets the HTTP request default attributes, the scheme, the context
    and the parameters are set here and not in the reset operation as
    they are owned by the layers that sit above the message itself */
    http_request->scheme = HTTP_SCHEME;
    http_request->context = NULL;
    http_request->parameters = NULL;

    /* resets the HTTP request so that the transient part of it is
    set to the proper initial values */
    reset_http_request(http_request);

    /* sets the HTTP request in the HTTP request pointer */
    *http_request_pointer = http_request;
}

void delete_http_request(struct http_request_t *http_request) {
    /* releases the HTTP request */
    FREE(http_request);
}

void reset_http_request(struct http_request_t *http_request) {
    /* sets the transient attributes of the HTTP request to their
    initial values, the version defaults to the most common one so
    that a message that never reaches the parsing of the version
    is still writable as a response */
    http_request->version = HTTP11;
    http_request->method = 0;
    http_request->status_code = 0;
    http_request->flags = 0;
    http_request->stream_id = 0;
    http_request->trailers = FALSE;
    http_request->authority[0] = '\0';
    http_request->path[0] = '\0';
    http_request->path_size = 0;
    http_request->content_length = 0;
}

void append_path_http_request(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* calculates the amount of space that is still available in
    the buffer of the path, note that one of the bytes is always
    reserved for the end of string sequence */
    size_t remaining = VIRIATUM_MAX_URL_SIZE - http_request->path_size - 1;

    /* in case the fragment does not fit in the remaining space it
    is truncated, an oversized path is not a reason to fail the
    parsing as the handlers are the ones deciding on the validity
    of the path itself */
    if(data_size > remaining) { data_size = remaining; }

    /* copies the fragment into the end of the path and then closes
    the string with the end of string sequence, the size is kept
    updated so that the next fragment does not measure it again */
    memcpy(&http_request->path[http_request->path_size], data, data_size);
    http_request->path_size += data_size;
    http_request->path[http_request->path_size] = '\0';
}
