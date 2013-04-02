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

#include "torrent.h"

ERROR_CODE _create_tracker_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port, char *file_path) {
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
    struct string_t strings[9];
    struct hash_map_t *parameters_map;

    char *params_buffer;
    size_t params_size;

    /* alocates dynamic space for the parameters to the
    http stream (http client) this structure will be able
    to guide the stream of http client */
    struct http_client_parameters_t *parameters;
    create_http_client_parameters(&parameters);

    /* populates the parameters structure with the
    required values for the http client request */
    parameters->method = HTTP_GET;

    memcpy(parameters->url, "/ptorrent/announce.php", 23);

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

    /* tenho de fazer gerador de get parameters !!!! */
    /* pega nas chaves e nos valores do hash map e gera a get string para um string buffer */
    create_hash_map(&parameters_map, 0);
    strings[0].buffer = info_hash;
    strings[0].length = 20;
    strings[1].buffer = peer_id;
    strings[1].length = 20;
    strings[2].buffer = (unsigned char *) "9090";
    strings[2].length = sizeof("9090") - 1;
    strings[3].buffer = (unsigned char *) "0";
    strings[3].length = sizeof("0") - 1;
    strings[4].buffer = (unsigned char *) "0";
    strings[4].length = sizeof("0") - 1;
    strings[5].buffer = (unsigned char *) "222904320"; /* must calculate this value */
    strings[5].length = sizeof("222904320") - 1; /* must calculate this value */
    strings[6].buffer = (unsigned char *) "0";
    strings[6].length = sizeof("0") - 1;
    strings[7].buffer = (unsigned char *) "0";
    strings[7].length = sizeof("0") - 1;
    strings[8].buffer = (unsigned char *) "started";
    strings[8].length = sizeof("started") - 1;
    set_value_string_hash_map(parameters_map, (unsigned char *) "info_hash", (void *) &strings[0]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "peer_id", (void *) &strings[1]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "port", (void *) &strings[2]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "uploaded", (void *) &strings[3]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "downloaded", (void *) &strings[4]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "left", (void *) &strings[5]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "compact", (void *) &strings[6]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "no_peer_id", (void *) &strings[7]);
    set_value_string_hash_map(parameters_map, (unsigned char *) "event", (void *) &strings[8]);
    parameters_http(parameters_map, (unsigned char **) &params_buffer, &params_size);
    delete_hash_map(parameters_map);

    memcpy(parameters->params, params_buffer, params_size);
    parameters->params[params_size] = '\0';
    FREE(params_buffer);

    /* creates a general http client connection structure containing
    all the general attributes for a connection, then sets the
    "local" connection reference from the pointer */
    _create_client_connection(connection_pointer, service, hostname, port);
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

ERROR_CODE _create_torrent_connection(struct connection_t **connection_pointer, struct service_t *service, char *hostname, unsigned int port) {
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
