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

/**
 * Structure representing an http request
 * it contains information about the request
 * including size contents and control flags.
 */
typedef struct HttpRequest_t {
    size_t receivedDataSize;
    unsigned char startLineLoaded;
    unsigned char headersLoaded;
} HttpRequest;

void createHttpRequest(struct HttpRequest_t **httpRequestPointer);
void deleteHttpRequest(struct HttpRequest_t *httpRequest);

/**
 * Called to parse a new data chunk in the context
 * of an http request.
 * This function should be called whenever a new data
 * chunk is received.
 *
 * @param httpRequest The http request object.
 * @param data The data to be parsed.
 * @param dataSize The size of the data to be parsed.
 */
void parseDataHttpRequest(struct HttpRequest_t *httpRequest, unsigned char *data, size_t dataSize);
