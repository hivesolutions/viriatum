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

#include "stdafx.h"

#include "stream_httpc.h"


/* THESE ARE TEMPORARY !!!!! */

/* ----------------------------------------- */

ERROR_CODE body_callback_handler_client(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct type_t *type;

    decode_bencoding((unsigned char *) data, data_size, &type);
    print_type(type);
    free_type(type);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_client(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

/* ------------------------------------------- */


ERROR_CODE create_http_client_connection(struct http_client_connection_t **http_client_connection_pointer, struct io_connection_t *io_connection) {
    /* retrieves the http client connection size */
    size_t http_client_connection_size = sizeof(struct http_client_connection_t);

    /* allocates space for the http client connection */
    struct http_client_connection_t *http_client_connection =\
	    (struct http_client_connection_t *) MALLOC(http_client_connection_size);

    /* sets the http handler attributes (default) values */
    http_client_connection->io_connection = io_connection;

    /* creates the http settings and the http parser
    (for a response) these are goint be used in the message */
    create_http_settings(&http_client_connection->http_settings);
    create_http_parser(&http_client_connection->http_parser, 0);

    /* sets the default callback functions in the http settings
    these function are going to be called by the parser */
    http_client_connection->http_settings->on_body = body_callback_handler_client;
    http_client_connection->http_settings->on_message_complete = message_complete_callback_handler_client;

    /* sets the connection as the parser parameter(s) */
    http_client_connection->http_parser->parameters = io_connection->connection;

    /* sets the http client connection in the (upper) io connection substrate */
    io_connection->lower = http_client_connection;

    /* sets the http client connection in the http client connection pointer */
    *http_client_connection_pointer = http_client_connection;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http_client_connection(struct http_client_connection_t *http_client_connection) {
    /* retrieves the various upper layers of the connection in order
    to retrieve the parameters for the connection */
    struct io_connection_t *io_connection = http_client_connection->io_connection;
    struct connection_t *connection = (struct connection_t *) io_connection->connection;

    /* in case the parameters are set they must be released
    with the proper destructor function and then set to invalid
    so that no extra delete operations occur */
    if(connection->parameters != NULL) {
        delete_http_client_parameters(connection->parameters);
        connection->parameters = NULL;
    }

    /* deletes the http parser and the http settings
    structures (avoids memory leaks )*/
    delete_http_parser(http_client_connection->http_parser);
    delete_http_settings(http_client_connection->http_settings);

    /* releases the memory associated with the client connection
    in order to avoid any memory leaks */
    FREE(http_client_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_http_client_parameters(struct http_client_parameters_t **http_client_parameters_pointer) {
    /* retrieves the http client connection size and uses
    it to allocate the required memory for the structure */
    size_t http_client_parameters_size = sizeof(struct http_client_parameters_t);
    struct http_client_parameters_t *http_client_parameters =\
        (struct http_client_parameters_t *) MALLOC(http_client_parameters_size);

    /* sets the various default values in the client paramenters
    structure and then updates the pointer reference */
    http_client_parameters->method = HTTP_GET;
    http_client_parameters->version = HTTP11;
    *http_client_parameters_pointer = http_client_parameters;

    /* raises no error as the creation of the client parameraters
    structure as successful */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_http_client_parameters(struct http_client_parameters_t *http_client_parameters) {
    /* releases the memory associated with the client
    parameters structure */
    FREE(http_client_parameters);

    /* raises no error as the removal of the client parameraters
    structure as successful */
    RAISE_NO_ERROR;
}

ERROR_CODE data_handler_stream_http_client(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    /* retrieves the http client connection as the lower
    part (payload) of the io connection */
    struct http_client_connection_t *http_client_connection =\
        (struct http_client_connection_t *) io_connection->lower;

    /* process the http data for the http parser, this should be
    a partial processing and some data may remain unprocessed (in
    case there are multiple http requests) */
    process_data_http_parser(
        http_client_connection->http_parser,
        http_client_connection->http_settings,
        buffer,
        buffer_size
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_handler_stream_http_client(struct io_connection_t *io_connection) {
    /* allocates the http client connection and retrieves the
    "upper" connection (for parameters retrieval) */
    struct http_client_connection_t *http_client_connection;
    struct connection_t *connection = (struct connection_t *) io_connection->connection;
    struct http_client_parameters_t *parameters = (struct http_client_parameters_t *) connection->parameters;
    char *buffer = MALLOC(VIRIATUM_HTTP_SIZE);

    /* creates the http message header from the provided
    information, note that the parmeters are allways set*/
    SPRINTF(
        buffer,
        VIRIATUM_HTTP_SIZE,
        "%s %s?%s %s\r\n"
        "User-Agent: %s\r\n"
        "Connection: Keep-Alive\r\n\r\n",
        http_method_strings[parameters->method - 1],
        parameters->url,
        parameters->params,
        http_version_strings[parameters->version - 1],
        VIRIATUM_AGENT
    );

    /* creates the http client connection object populating
    all of its internal element for future usage */
    create_http_client_connection(&http_client_connection, io_connection);

    /* writes the created header buffer to the connection
    and waits for the response from the server side */
    write_connection(
        io_connection->connection,
        (unsigned char *) buffer,
        (unsigned int) strlen(buffer),
        NULL,
        NULL
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_handler_stream_http_client(struct io_connection_t *io_connection) {
    /* retrieves the http client connection and deletes it
    releasig all of its memory (avoiding memory leaks) */
    struct http_client_connection_t *http_client_connection =\
        (struct http_client_connection_t *) io_connection->lower;
    delete_http_client_connection(http_client_connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
