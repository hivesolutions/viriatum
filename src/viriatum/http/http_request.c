/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2016 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "http_request.h"

void create_http_request(struct http_request_t **http_request_pointer) {
    /* retrieves the http request size */
    size_t http_request_size = sizeof(struct http_request_t);

    /* allocates space for the http request */
    struct http_request_t *http_request = (struct http_request_t *) MALLOC(http_request_size);

    /* sets the http request default attributes */
    http_request->method = GET_REQUEST_METHOD;
    http_request->data = NULL;

    /* sets the http request in the http request pointer */
    *http_request_pointer = http_request;
}

void delete_http_request(struct http_request_t *http_request) {
    /* releases the http request */
    FREE(http_request);
}
