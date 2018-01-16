/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2018 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "torrent.h"

ERROR_CODE _create_tracker_connection(
    struct connection_t **connection_pointer,
    struct service_t *service,
    char *url,
    char *file_path
) {
    /* allocates space for the connection reference */
    struct connection_t *connection;

    /* allocates space for the temporary error variable to
    be used to detect errors in calls */
    ERROR_CODE error;

    /* allocates the http client connection and retrieves the
    "upper" connection (for parameters retrieval) */
    struct type_t *type;
    struct type_t *_type;
    unsigned char *_buffer;
    size_t _buffer_size;
    unsigned char info_hash[SHA1_DIGEST_SIZE + 1];
    unsigned char random[12];
    unsigned char peer_id[21];

    /* sets space for the static structure tht will hold the
    various url components parsed from the url string */
    struct url_static_t url_static;

    /* alocates dynamic space for the parameters to the
    http stream (http client) this structure will be able
    to guide the stream of http client */
    struct http_client_parameters_t *parameters;
    create_http_client_parameters(&parameters);

    /* parses the provided url populating a structure with
    the details of such url */
    parse_url_static(url, strlen(url), &url_static);

    /* populates the parameters structure with the
    required values for the http client request */
    parameters->method = HTTP_GET;

    /* copies the "parsed url from the static url structure
    into the parameters structure, this is the url that will
    be used for the client connection */
    memcpy(parameters->url, url_static.path, strlen(url_static.path) + 1);

    /* creates the peer id from the current client version information
    plus a random number represnting the unique "visit" as defined by
    the bittorrent specification */
    SPRINTF(
        (char *) peer_id,
        20,
        "-%s%d%d%d0-",
        VIRIATUM_PREFIX,
        VIRIATUM_MAJOR,
        VIRIATUM_MINOR,
        VIRIATUM_MICRO
    );
    random_buffer(random, 12);
    memcpy(peer_id + 8, random, 12);

    /* tries to decode the bencoded torrent file an in case
    thre's an error propagates it to the calling function */
    error = decode_bencoding_file(file_path, &type);
    if(error) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem reading torrent file"
        );
    }

    /* retries the info part of the torrent description, this
    value contains "valuable" information describing the file
    and may be used to generate the info hash */
    get_value_string_sort_map(
        type->value.value_sort_map,
        (unsigned char *) "info",
        (void **) &_type
    );

    /* encodes the info dictionary into bencoding and calculates
    the sha1 value of the encoded value as the info hash value
    that uniquely identifies the file over the grid */
    encode_bencoding(_type, &_buffer, &_buffer_size);
    sha1(_buffer, (unsigned int) _buffer_size, info_hash);
    free_type(type);
    FREE(_buffer);

    /* creates the params (paramters) string from the provided
    sequence of key values, this is an automator utility function
    for the creation of the string and the provided buffer should
    have been previously allocated with enough space */
    parameters_http_c(
        parameters->params, VIRIATUM_MAX_URL_SIZE, 9,
        "info_hash", info_hash, 20,
        "peer_id", peer_id, 20,
        "port", "9090", sizeof("9090") - 1,
        "uploaded", "0", sizeof("0") - 1,
        "downloaded", "0", sizeof("0") - 1,
        "left", "222904320", sizeof("222904320") - 1,
        "compact", "0", sizeof("0") - 1,
        "no_peer_id", "0", sizeof("0") - 1,
        "event", "started", sizeof("started") - 1
    );

    /* creates a general http client connection structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(
        connection_pointer,
        service,
        url_static.host,
        url_static.port
    );
    connection = *connection_pointer;

    /* sets the http client protocol as the protocol to be
    "respected" for this client connection, this should
    be able to set the apropriate handlers in io then sets
    the parameters structure in the connection so that the
    lower layers "know" what to do */
    connection->protocol = HTTP_CLIENT_PROTOCOL;
    connection->parameters = (void *) parameters;

    /* opens the connection, starting the chain of events
    further action will only occur in callbacks */
    connection->open_connection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _create_torrent_connection(
    struct connection_t **connection_pointer,
    struct service_t *service,
    char *hostname,
    unsigned int port
) {
    /* allocates space for the connection reference */
    struct connection_t *connection;

    /* creates a general client conneciton structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(connection_pointer, service, hostname, port);
    connection = *connection_pointer;

    /* sets the torrent protocol as the protocol to be
    "respected" for this client connection, this should
    be able to set the apropriate handlers in io */
    connection->protocol = TORRENT_PROTOCOL;

    /* opens the connection */
    connection->open_connection(connection);

    /* raises no error */
    RAISE_NO_ERROR;
}
