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

#include "service_select.h"

void createServiceSelect(struct ServiceSelect_t **serviceSelectPointer) {
    /* retrieves the service select size */
    size_t serviceSelectSize = sizeof(struct ServiceSelect_t);

    /* retrieves the service size */
    size_t serviceSize = sizeof(struct Service_t);

    /* allocates space for the service select */
    struct ServiceSelect_t *serviceSelect = (struct ServiceSelect_t *) malloc(serviceSelectSize);

    /* allocates space for the service */
    serviceSelect->service = (struct Service_t *) malloc(serviceSize);

    /* zeros the sockets read set */
    FD_ZERO(&serviceSelect->socketsReadSet);

    /* zeros the sockets write set */
    FD_ZERO(&serviceSelect->socketsWriteSet);

    /* sets the service select in the service select pointer */
    *serviceSelectPointer = serviceSelect;
}

void deleteServiceSelect(struct ServiceSelect_t *serviceSelect) {
    /* releases the service */
    free(serviceSelect->service);

    /* releases the service select */
    free(serviceSelect);
}

void addConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* calls the add connection service (super) */
    addConnectionService(serviceSelect->service, connection);

    /* adds the socket handle to the sockets read set */
    FD_SET(connection->socketHandle, &serviceSelect->socketsReadSet);

    /* adds the socket handle to the sockets write set */
    FD_SET(connection->socketHandle, &serviceSelect->socketsWriteSet);
}

void removeConnectionServiceSelect(struct ServiceSelect_t *serviceSelect, struct Connection_t *connection) {
    /* calls the remove connection service (super) */
    removeConnectionService(serviceSelect->service, connection);

    /* removes the socket handle from the sockets read set */
    FD_CLR(connection->socketHandle, &serviceSelect->socketsReadSet);

    /* removes the socket handle from the sockets write set */
    FD_CLR(connection->socketHandle, &serviceSelect->socketsWriteSet);
}
