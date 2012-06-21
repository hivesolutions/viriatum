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

#pragma once

#include "../system/system.h"

typedef struct polling_epoll_t {
    /**
     * The polling reference, for top
     * level reference.
     */
    struct polling_t *polling;

    int epoll_fd;

	struct hash_map_t *connections;
} polling_epoll;

void create_polling_epoll(struct polling_epoll_t **polling_epoll_pointer, struct polling_t *polling);
void delete_polling_epoll(struct polling_epoll_t *polling_epoll);
ERROR_CODE open_polling_epoll(struct polling_t *polling);
ERROR_CODE close_polling_epoll(struct polling_t *polling);
ERROR_CODE register_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE register_write_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE unregister_write_polling_epoll(struct polling_t *polling, struct connection_t *connection);
ERROR_CODE poll_polling_epoll(struct polling_t *polling);
ERROR_CODE call_polling_epoll(struct polling_t *polling);
