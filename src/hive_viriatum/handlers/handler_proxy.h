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

#pragma once

#include "../http/http.h"
#include "../system/system.h"

/**
 * The structure that holds the internal
 * structure to support the proxy handler
 * should contain configuration info.
 */
typedef struct proxy_handler_t {
    /**
     * The various locations loaded from the configuration
     * they refer the cofiruation attributes associated
     * with the proxy structures.
     */
    struct proxy_location_t *locations;

    /**
     * The number of locations currently loaded in the handler
     * this value is used for iteration arround the locations
     * buffer.
     */
    size_t locations_count;
} proxy_handler;

/**
 * The context structure to be used allong
 * the interpretation of the request for
 * the peoxy handler.
 */
typedef struct handler_proxy_context_t {
    char *name;  /* this is dummy */
} handler_proxy_context;

ERROR_CODE create_proxy_handler(struct proxy_handler_t **proxy_handler_pointer, struct http_handler_t *http_handler);
ERROR_CODE delete_proxy_handler(struct proxy_handler_t *proxy_handler);
ERROR_CODE create_handler_proxy_context(struct handler_proxy_context_t **handler_proxy_context_pointer);
ERROR_CODE delete_handler_proxy_context(struct handler_proxy_context_t *handler_proxy_context);
