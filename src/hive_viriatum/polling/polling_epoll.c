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

#include "polling_epoll.h"

void create_polling_epoll(struct polling_epoll_t **polling_epoll_pointer, struct polling_t *polling) {
    /* retrieves the polling epoll size */
    size_t polling_epoll_size = sizeof(struct polling_epoll_t);

    /* allocates space for the polling epoll */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) MALLOC(polling_epoll_size);

	/* creates the epoll file descriptor with an arbitrary
	size (this no longer used by the kernel) */
	polling_epoll->epoll_fd = epoll_create(1024);

    /* resets the polling in the polling epoll */
    polling_epoll->polling = polling;

    /* sets the polling epoll in the polling epoll pointer */
    *polling_epoll_pointer = polling_epoll;
}

void delete_polling_epoll(struct polling_epoll_t *polling_epoll) {
	/* in case the polling fd is defined it must be closed
	in order to avoid any memory leak problem */
	if(polling_epoll->epoll_fd) { close(polling_epoll->epoll_fd); } 

    /* releases the polling epoll */
    FREE(polling_epoll);
}

ERROR_CODE open_polling_epoll(struct polling_t *polling) {
    /* allocates the polling epoll */
    struct polling_epoll_t *polling_epoll;

    /* creates the polling epoll */
    create_polling_epoll(&polling_epoll, polling);

    /* sets the polling epoll in the polling as
    the lower substrate */
    polling->lower = (void *) polling_epoll;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_polling_epoll(struct polling_t *polling) {
    /* retrieves the polling epoll */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

    /* deletes the polling epoll */
    delete_polling_epoll(polling_epoll);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection) {
	/* allocates space for teh new event to be inserted into
	the epoll polling system (this is an internal kernel structure) */
	struct epoll_event _event;

    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;

	/* populates the event structure with the appropriate
	structures in order to register for the right events 
	and then inserts the event request into the epoll fd */
	_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    _event.data.fd = connection->socket_handle;
	epoll_ctl(polling_epoll->epoll_fd, EPOLL_CTL_ADD, connection->socket_handle, &_event);

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_connection_polling_epoll(struct polling_t *polling, struct connection_t *connection)  {
    /* retrieves the polling epoll structure from the upper
	polling control structure */
    struct polling_epoll_t *polling_epoll = (struct polling_epoll_t *) polling->lower;


	/*   !!!! PENDING IMPLEMENTATION    !!!!!! */


    /* raises no error */
    RAISE_NO_ERROR;
}