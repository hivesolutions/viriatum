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
#include "viriatum.h"

#include "handler_dispatch_test.h"

const char *test_dispatch_handler_context_keepalive() {
    /* allocates space for the HTTP parser and for a
    handler file context to simulate keep-alive reuse */
    struct http_parser_t *http_parser;
    struct handler_file_context_t *handler_file_context;

    /* creates the HTTP parser and the handler file context,
    wiring them together through the context pointer to
    simulate a file handler that has just processed a request */
    create_http_parser(&http_parser, TRUE);
    create_handler_file_context(&handler_file_context);
    http_parser->context = handler_file_context;

    /* simulates the file handler unset which deletes the
    context structure and nullifies the pointer, matching
    the corrected behavior of _unset_http_parser_handler_file */
    delete_handler_file_context(handler_file_context);
    http_parser->context = NULL;

    /* simulates the dispatch handler being re-set for the
    next keep-alive request, context should be NULL at this
    point because the file handler properly cleaned up */
    _set_http_parser_handler_dispatch(http_parser);

    /* calls the dispatch handler unset, which is triggered
    when the dispatch handler finds a target handler and
    switches to it, context is NULL so no free should occur */
    _unset_http_parser_handler_dispatch(http_parser);

    /* verifies that the context pointer remains NULL after
    the dispatch handler unset (no corruption occurred) */
    V_ASSERT(http_parser->context == NULL);

    /* cleans up the parser */
    delete_http_parser(http_parser);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
