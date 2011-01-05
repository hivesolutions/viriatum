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

#include "service.h"

void createService(struct Service_t **servicePointer) {
    /* retrieves the service size */
    size_t serviceSize = sizeof(struct Service_t);

    /* allocates space for the service */
    struct Service_t *service = (struct Service_t *) malloc(serviceSize);

    /* sets the service in the service pointer */
    *servicePointer = service;
}

void deleteService(struct Service_t *service) {
    /* releases the service */
    free(service);
}

void addConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* adds the connection to the connections list */
    appendValueLinkedList(service->connectionsList, connection);
}

void removeConnectionService(struct Service_t *service, struct Connection_t *connection) {
    /* removes the connection from the connections list */
    removeValueLinkedList(service->connectionsList, connection);
}
