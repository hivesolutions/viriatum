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

#include "../http/http.h"
#include "../handlers/handlers.h"
#include "stream_io.h"

typedef struct HttpConnection_t {
    struct IoConnection_t *ioConnection;
    struct HttpSettings_t *httpSettings;
    struct HttpParser_t *httpParser;
} HttpConnection;

void createHttpConnection(struct HttpConnection_t **httpConnectionPointer, struct IoConnection_t *ioConnection);
void deleteHttpConnection(struct HttpConnection_t *httpConnection);
ERROR_CODE dataHandlerStreamHttp(struct IoConnection_t *ioConnection, unsigned char *buffer, size_t bufferSize);
ERROR_CODE openHandlerStreamHttp(struct IoConnection_t *ioConnection);
ERROR_CODE closeHandlerStreamHttp(struct IoConnection_t *ioConnection);
