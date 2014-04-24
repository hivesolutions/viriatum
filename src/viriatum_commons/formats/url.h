/*
 Hive Viriatum Commons
 Copyright (C) 2008-2014 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2014 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../structures/structures.h"

typedef struct url_t {
    struct string_t scheme;
    struct string_t auth;
    struct string_t username;
    struct string_t password;
    struct string_t host_c;
    struct string_t host;
    struct string_t port_s;
    unsigned int port;
    struct string_t path;
    struct string_t query;
    struct string_t fragment;
} url;

typedef struct url_static_t {
    char scheme[64];
    char username[128];
    char password[128];
    char host[256];
    unsigned int port;
    char path[1024];
    char query[1024];
    char fragment[256];
} url_static;

typedef enum url_parse_state_e {
    SCHEME_STATE = 1,
    SLA_STATE,
    SLA_SLA_STATE,
    AUTH_STATE,
    HOST_STATE,
    PORT_STATE,
    PATH_STATE,
    QUERY_STATE,
    FRAGMENT_STATE
} url_parse_state;

VIRIATUM_EXPORT_PREFIX ERROR_CODE parse_url(char *url, size_t url_size, struct url_t *url_s);
VIRIATUM_EXPORT_PREFIX ERROR_CODE parse_url_static(char *url, size_t url_size, struct url_static_t *url_s);
