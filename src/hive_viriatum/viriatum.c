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

#include "system/system.h"

int main(int argc, char *argv[]) {
	/* creates the new socket handle */
	SOCKET_HANDLE socketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

    /* allocates the socket address structure */
    SOCKET_ADDRESS socketAddress;

	/* sets the socket address attributes */
	socketAddress.sin_family = SOCKET_INTERNET_TYPE;
	socketAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
	socketAddress.sin_port = htons(8080);

	/* binds the socket */
	SOCKET_BIND(socketHandle, socketAddress);

	/* iterates continuously */
	while(1) {
		/* listens for a socket change */
		SOCKET_LISTEN(socketHandle);
	}

	/* returns zero (valid) */
	return 0;
}
