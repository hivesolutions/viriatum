/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "handler_proxy.h"

ERROR_CODE create_proxy_handler(struct proxy_handler_t **proxy_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the proxy handler size and then allocates the
    required space for the handler object */
    size_t proxy_handler_size = sizeof(struct proxy_handler_t);
    struct proxy_handler_t *proxy_handler = (struct proxy_handler_t *) MALLOC(proxy_handler_size);

    /* sets the proxy handler attributes (default) values and updates
    the top level lower referece to the current handler (layering) */
    proxy_handler->locations = NULL;
    proxy_handler->locations_count = 0;
    http_handler->lower = (void *) proxy_handler;

    /* sets the proxy handler in the proxy handler pointer and returns
    the control flow to the caller function with no error */
    *proxy_handler_pointer = proxy_handler;
    RAISE_NO_ERROR;
}

ERROR_CODE delete_proxy_handler(struct proxy_handler_t *proxy_handler) {
    /* in case the locations buffer is set it must be released
    to avoid any memory leak (not going to be used) */
    if(proxy_handler->locations != NULL) { FREE(proxy_handler->locations); }

    /* releases the proxy handler, avoiding any memory leaks
    that may be raised from this main structure and then returns
    the control flow to the caller function */
    FREE(proxy_handler);
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_proxy_context(struct handler_proxy_context_t **handler_proxy_context_pointer) {
    /* retrieves the size in bytes of the handler proxy context
    structure and then uses the size in the allocation of the object */
    size_t handler_proxy_context_size = sizeof(struct handler_proxy_context_t);
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) MALLOC(handler_proxy_context_size);

    /* sets the handler proxy default values, should not create
    any problem with the initial handling */
    handler_proxy_context->name = NULL;

    /* sets the handler proxy context in the  pointer and then returns
    the control flow to the caller method with no error */
    *handler_proxy_context_pointer = handler_proxy_context;
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_proxy_context(struct handler_proxy_context_t *handler_proxy_context) {
    /* releases the handler proxy context object and returns
    the control flow to the caller function */
    FREE(handler_proxy_context);
    RAISE_NO_ERROR;
}
