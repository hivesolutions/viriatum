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

#include "stream_http.h"

void dataHandlerStreamHttp(struct Connection_t *connection, unsigned char *buffer, size_t bufferSize) {
    /* allocates the http settings */
    struct HttpSettings_t *httpSettings;

    /* allocates the http parser */
    struct HttpParser_t *httpParser;

    /* creates the http settings */
    createHttpSettings(&httpSettings);

    /* creates the http parser */
    createHttpParser(&httpParser);

    /* sets the connection as the parser parameter(s) */
    httpParser->parameters = connection;

    /* sets the structures for the handler */
    setHandlerDefault(httpParser, httpSettings);


    // TODO: tenho de testar quantos bytes processei !!!


    /* process the http data for the http parser */
    processDataHttpParser(httpParser, httpSettings, buffer, bufferSize);



    /* unsets the structures for the handler */
    unsetHandlerDefault(httpParser, httpSettings);

    /* deletes the http parser */
    deleteHttpParser(httpParser);

    /* deletes the http settings */
    deleteHttpSettings(httpSettings);
}
