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

#include "handler_proxy.h"

ERROR_CODE create_proxy_handler(struct proxy_handler_t **proxy_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the proxy handler size and then allocates the
    required space for the handler object */
    size_t proxy_handler_size = sizeof(struct proxy_handler_t);
    struct proxy_handler_t *proxy_handler = (struct proxy_handler_t *) MALLOC(proxy_handler_size);

    /* sets the proxy handler attributes (default) values and updates
    the top level lower referece to the current handler (layering) */
    proxy_handler->locations = NULL;
    proxy_handler->locations_count = 0;
    http_handler->lower = (void *) proxy_handler;

    /* creates the hash map that is going to be used for the associations
    between the client connections with the viriatum server (proxy) and the
    backend connections to the proxy target servers */
    create_hash_map(&proxy_handler->connections_map, 0);

    /* sets the proxy handler in the proxy handler pointer and returns
    the control flow to the caller function with no error */
    *proxy_handler_pointer = proxy_handler;
    RAISE_NO_ERROR;
}

ERROR_CODE delete_proxy_handler(struct proxy_handler_t *proxy_handler) {
    /* in case the locations buffer is set it must be released
    to avoid any memory leak (not going to be used) */
    if(proxy_handler->locations != NULL) { FREE(proxy_handler->locations); }

    /* deletes the structure that is used for the mapping operation
    of the client connections to the proxy to the backend connections */
    delete_hash_map(proxy_handler->connections_map);

    /* releases the proxy handler, avoiding any memory leaks
    that may be raised from this main structure and then returns
    the control flow to the caller function */
    FREE(proxy_handler);
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_proxy_context(struct handler_proxy_context_t **handler_proxy_context_pointer) {
    /* retrieves the size in bytes of the handler proxy context
    structure and then uses the size in the allocation of the object */
    size_t handler_proxy_context_size = sizeof(struct handler_proxy_context_t);
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) MALLOC(handler_proxy_context_size);

    /* sets the handler proxy default values, should not create
    any problem with the initial handling */
    handler_proxy_context->proxy_pass = NULL;
    handler_proxy_context->buffer = MALLOC(1024);
    handler_proxy_context->buffer_size = 0;
    handler_proxy_context->buffer_max_size = 1024;
    handler_proxy_context->out_buffer = MALLOC(1024);
    handler_proxy_context->out_buffer_size = 0;
    handler_proxy_context->out_buffer_max_size = 1024;
    handler_proxy_context->connection = NULL;
    handler_proxy_context->connection_c = NULL;

    /* creates a new set of http settings and parser for the new
    context that is being created, these are going to be used in
    the backend connection to be able to process its data */
    create_http_settings(&handler_proxy_context->http_settings);
    create_http_parser(&handler_proxy_context->http_parser, 0);

    /* sets the default callback functions in the http settings
    these function are going to be called by the parser */
    handler_proxy_context->http_settings->on_line = line_callback_backend;
    handler_proxy_context->http_settings->on_header_field = header_field_callback_backend;
    handler_proxy_context->http_settings->on_header_value = header_value_callback_backend;
    handler_proxy_context->http_settings->on_headers_complete = headers_complete_callback_backend;
    handler_proxy_context->http_settings->on_body = body_callback_backend;
    handler_proxy_context->http_settings->on_message_complete = message_complete_callback_backend;

    /* updates the currently associated parser context with the context
    that is currentl being created to be able to access it from the callbacks */
    handler_proxy_context->http_parser->context = (void *) handler_proxy_context;

    /* sets the handler proxy context in the  pointer and then returns
    the control flow to the caller method with no error */
    *handler_proxy_context_pointer = handler_proxy_context;
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_proxy_context(struct handler_proxy_context_t *handler_proxy_context) {
    /* in case the buffer for the current proxy context is
    defined it must be properly released to avoid memory leaks */
    if(handler_proxy_context->buffer) { FREE(handler_proxy_context->buffer); }

    /* in case the output buffer for the current proxy context is
    defined it must be properly released to avoid memory leaks */
    if(handler_proxy_context->out_buffer) { FREE(handler_proxy_context->out_buffer); }

    /* in case the http settings structure is defined for the current
    context it must be release in order to avoid any leaks */
    if(handler_proxy_context->http_settings) {
        delete_http_settings(handler_proxy_context->http_settings);
    }

    /* if the parser structure is defined for the current context release
    of its memory is required to avoid problems with leaks */
    if(handler_proxy_context->http_parser) {
        delete_http_parser(handler_proxy_context->http_parser);
    }

    /* releases the handler proxy context object and returns
    the control flow to the caller function */
    FREE(handler_proxy_context);
    RAISE_NO_ERROR;
}

ERROR_CODE register_handler_proxy(struct service_t *service) {
    /* allocates space for the temporary value object and for
    the index counter to be used in the iteration of configurations */
    void *value;
    size_t index;

    /* allocates space for both the location and the configuration
    reference stuctures */
    struct location_t *location;
    struct sort_map_t *configuration;

    /* allocates space for the mod proxy location structure
    reference to be used to resolve the request */
    struct proxy_location_t *_location;

    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates space for the proxy handler */
    struct proxy_handler_t *proxy_handler;

    /* creates the http handler and then uses it to create
    the proxy handler (lower substrate) */
    service->create_http_handler(service, &http_handler, (unsigned char *) "proxy");
    create_proxy_handler(&proxy_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->resolve_index = FALSE;
    http_handler->set = set_handler_proxy;
    http_handler->unset = unset_handler_proxy;
    http_handler->reset = reset_handler_proxy;

    /* allocates space for the various location structures
    that will be used to resolve the proxy request */
    proxy_handler->locations = (struct proxy_location_t *)
        MALLOC(service->locations.count * sizeof(struct proxy_location_t));
    memset(proxy_handler->locations, 0,
        service->locations.count * sizeof(struct proxy_location_t));

    /* updates the locations count variable in the proxy handler so
    that it's possible to iterate over the locations */
    proxy_handler->locations_count = service->locations.count;

    /* iterates over all the locations in the service to create the
    proper configuration structures to the module */
    for(index = 0; index < service->locations.count; index++) {
        /* retrieves the current (service) location and then uses it
        to retrieve the configuration sort map */
        location = &service->locations.values[index];
        configuration = location->configuration;

        /* retrieves the current mod proxy configuration reference from
        the location buffer, this is going ot be populated and sets the
        default values in it */
        _location = &proxy_handler->locations[index];
        _location->proxy_pass = NULL;

        /* tries ro retrieve the proxy pass from the proxy configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "proxy_pass", &value);
        if(value != NULL) { _location->proxy_pass = (unsigned char *) value; }

        /* parses the proxy pass value, unpacking all of its values into a proper
        url describing structure that is allocated statically */
        _location->proxy_pass != NULL && parse_url_static(
            _location->proxy_pass,
            strlen(_location->proxy_pass),
            &_location->url_s
        );
    }

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_proxy(struct service_t *service) {
    /* allocates the http handler and then for the proxy
    handler objects to be released from the current structure */
    struct http_handler_t *http_handler;
    struct proxy_handler_t *proxy_handler;


    /* @todo: tenho de por aki a libertar  a FAZER CLOSE DE TODAS AS CONEXOES
    a clientes !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */



    /* retrieves the http handler from the service, then retrieves
    the lower substrate as the proxy handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "proxy");
    proxy_handler = (struct proxy_handler_t *) http_handler->lower;

    /* deletes the proxy handler reference */
    delete_proxy_handler(proxy_handler);

    /* remove the http handler from the service after
    that deletes the handler reference */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_proxy(struct http_connection_t *http_connection) {
    /* sets both the http parser values and the http
    settings handler for the current proxy handler */
    _set_http_parser_handler_proxy(http_connection->http_parser);
    _set_http_settings_handler_proxy(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_proxy(struct http_connection_t *http_connection) {
    /* unsets both the http parser values and the http
    settings handler from the current proxy handler */
    _unset_http_parser_handler_proxy(http_connection->http_parser);
    _unset_http_settings_handler_proxy(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE reset_handler_proxy(struct http_connection_t *http_connection) {
    /* resets the http parser values */
    _reset_http_parser_handler_proxy(http_connection->http_parser);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_proxy(struct http_parser_t *http_parser) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    handler_proxy_context->buffer_size = 0;
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    write_proxy_buffer(handler_proxy_context, (char *) data, data_size);

    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    write_proxy_buffer(handler_proxy_context, ": ", 2);
    write_proxy_buffer(handler_proxy_context, (char *) data, data_size);
    write_proxy_buffer(handler_proxy_context, "\r\n", 2);
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_proxy(struct http_parser_t *http_parser) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    write_proxy_buffer(handler_proxy_context, "\r\n", 2);
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    write_proxy_buffer(handler_proxy_context, (char *) data, data_size);
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_proxy(struct http_parser_t *http_parser) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection =\
        (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection =\
        (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection =\
        (struct http_connection_t *) io_connection->lower;

    /* acquires the current http connection this avoids the parallel usage of
    the connection, removing any issue arround paralel message handling */
    http_connection->acquire(http_connection);

    write_connection_c(
        handler_proxy_context->connection_c,
        handler_proxy_context->buffer,
        handler_proxy_context->buffer_size,
        NULL,
        NULL,
        FALSE
    );

    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_proxy(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* retrieves the handler proxy context from the http parser */
    struct handler_proxy_context_t *handler_proxy_context =
        (struct handler_proxy_context_t *) http_parser->context;

    /* retrieves the connection from the parser and then uses it to  the
    the correct proxy handler reference from the http connection */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct proxy_handler_t *proxy_handler =
        (struct proxy_handler_t *) http_connection->http_handler->lower;

    /* retrieves the current location from the locations buffer
    to be used to update the current context information */
    struct proxy_location_t *location = &proxy_handler->locations[index];

    /* sets the proxy pass url structure values in the current context
    from the values extracted from the location for the current request */
    handler_proxy_context->proxy_pass = location->proxy_pass;
    handler_proxy_context->url_s = location->url_s;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_proxy(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct custom_parameters_t *parameters;
    struct connection_t *connection_c;

    size_t size_m;

    char buffer[1024];
    char path[1024];

    enum http_version_e version_e = get_http_version(
        http_parser->http_major,
        http_parser->http_minor
    );
    const char *version = get_http_version_string(version_e);
    const char *method = http_method_strings[http_parser->method - 1];

    struct connection_t *connection =\
        (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection =\
        (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection =\
        (struct http_connection_t *) io_connection->lower;
    struct http_handler_t *http_handler = http_connection->http_handler;
    struct proxy_handler_t *proxy_handler =\
        (struct proxy_handler_t *) http_handler->lower;
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;

    memcpy(path, data, data_size);
    path[data_size] = '\0';

    size_m = SPRINTF(buffer, 1024, "%s %s %s\r\n", method, path, version);
    write_proxy_buffer(handler_proxy_context, buffer, size_m);

    /*@todo: can put here the extra headers to be sent to the backend */
    write_proxy_buffer(handler_proxy_context, "X-Viriatum-Proxy:1\r\n", 20);

    /* updates the current proxy context object with the service connection
    pointer so that the internal client handlers are able to communicate "back"
    with the proxy client to send the data back */
    handler_proxy_context->connection = connection;

    /* tries to retrieve a possible backend (client) connection for the current
    proxy client connection, in case it exists it's going to be used (fast solution) */
    get_value_hash_map(
        proxy_handler->connections_map,
        (size_t) connection,
        NULL,
        (void **) &handler_proxy_context->connection_c
    );

    /* creates the parameters object that is going to be passed to
    the connection and used on the open operation to guide it's
    creation, this is a custom protocol connection so the proper
    callback handlers are set for the proxy processing */
    parameters = MALLOC(sizeof(struct custom_parameters_t));
    parameters->on_data = data_backend_handler;
    parameters->on_open = open_backend_handler;
    parameters->on_close = close_backend_handler;
    parameters->parameters = (void *) handler_proxy_context;

    /* in case the connection client reference structure for the current
    context is not defined a new connection must be created */
    if(handler_proxy_context->connection_c == NULL) {
        /* creates a general http client connection structure containing
        all the general attributes for a connection, then sets the
        "local" connection reference from the pointer */
        _create_client_connection(
            &handler_proxy_context->connection_c,
            connection->service,
            handler_proxy_context->url_s.host,
            handler_proxy_context->url_s.port ?\
                handler_proxy_context->url_s.port : 80
        );

        /* retrieves the reference for the connection object that will be used
        for the connection with the final back-end client */
        connection_c = handler_proxy_context->connection_c;

        /* sets the protocol for the "client" connection as custom and sets
        the "custom" parameters structure in it so that the callback functions
        may be controlled for the http proxy behaviour */
        connection_c->protocol = CUSTOM_PROTOCOL;
        connection_c->parameters = (void *) parameters;

        /* opens the connection, starting the chain of events
        further action will only occur in callbacks */
        connection_c->open_connection(connection_c);

        /* updates the connection with the client (backend) value
        so that the current connection is allways going to use the
        same backend connection (avoiding the creation of new ones) */
        set_value_hash_map(
            proxy_handler->connections_map,
            (size_t) connection,
            NULL,
            handler_proxy_context->connection_c
        );
    }
    /* otherwise there's a valid connection to the backend connection and
    it may be re-used, so only the parameters object must be set */
    else {
        /* retrieves the reference for the connection object that will be used
        for the connection with the final back-end client */
        connection_c = handler_proxy_context->connection_c;

        /* sets the protocol for the "client" connection as custom and sets
        the "custom" parameters structure in it so that the callback functions
        may be controlled for the http proxy behaviour */
        connection_c->protocol = CUSTOM_PROTOCOL;
        connection_c->parameters = (void *) parameters;
    }

    /* raises no error to the caller function because no problem
    has occured while handling the virtual url */
    RAISE_NO_ERROR;
}

ERROR_CODE data_backend_handler(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    struct connection_t *connection = io_connection->connection;
    struct custom_parameters_t *custom_parameters =\
        (struct custom_parameters_t *) connection->parameters;
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) custom_parameters->parameters;
    struct connection_t *connection_s =\
        (struct connection_t *) handler_proxy_context->connection;
    struct io_connection_t *io_connection_s =\
        (struct io_connection_t *) connection_s->lower;
    struct http_connection_t *http_connection_s =\
        (struct http_connection_t *) io_connection_s->lower;
    struct http_parser_t *http_parser_s = http_connection_s->http_parser;

    printf("DATA\n");

    process_data_http_parser(
        handler_proxy_context->http_parser,
        handler_proxy_context->http_settings,
        buffer,
        buffer_size
    );

    RAISE_NO_ERROR;
}

ERROR_CODE open_backend_handler(struct io_connection_t *io_connection) {
    printf("CONNECTED\n");
    RAISE_NO_ERROR;
}

ERROR_CODE close_backend_handler(struct io_connection_t *io_connection) {
    /* allocates the space for the various temporary structures that
    are going to be used for the closing operation of the backend connection */
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_s;
    struct io_connection_t *io_connection_s;
    struct http_connection_t *http_connection_s;

    /* unpacks the connection object to retrieve the custom parameters
    and check if they exist (no connection) dropped meanwhile, for such
    cases no connection closing should be done */
    struct connection_t *connection = io_connection->connection;
    struct custom_parameters_t *custom_parameters =\
        (struct custom_parameters_t *) connection->parameters;
    if(custom_parameters == NULL) { RAISE_NO_ERROR; }

    printf("DISCONNECTED\n");

    /* retrieves the proxy context from the parameters and then uses it
    to retrieve the parent http connection to the service (proxy client)
    to be used for closing */
    handler_proxy_context = (struct handler_proxy_context_t *) custom_parameters->parameters;
    connection_s = (struct connection_t *) handler_proxy_context->connection;
    io_connection_s = (struct io_connection_t *) connection_s->lower;
    http_connection_s = (struct http_connection_t *) io_connection_s->lower;

    /* unsets the connection with the client so that no more connection
    processing operation use it (it's no longer usable) */
    handler_proxy_context->connection_c = NULL;

    /* releases the lock present in the connection for the service and then
    closes it releasing all of it's structures, the logic is that if the
    connection with the proxy target (client) closes the connection with the
    proxy client should also be closed, then returns the controll flow to the
    caller function with no error */
    http_connection_s->release(http_connection_s);
    close_connection(connection_s);
    RAISE_NO_ERROR;
}

ERROR_CODE line_callback_backend(struct http_parser_t *http_parser) {
    size_t size_m;
    char buffer[1024];

    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;

    enum http_version_e version_e = get_http_version(
        http_parser->http_major,
        http_parser->http_minor
    );
    const char *version = get_http_version_string(version_e);
    unsigned short status_code = http_parser->status_code;
    const char *status_message = GET_HTTP_STATUS(status_code);

    size_m = SPRINTF(buffer, 1024, "%s %d %s\r\n", version, status_code, status_message);
    write_proxy_out_buffer(handler_proxy_context, buffer, size_m);

    /*@todo: can put here the extra headers to be sent to the backend */
    write_proxy_out_buffer(handler_proxy_context, "X-Viriatum-Proxy:1\r\n", 20);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    write_proxy_out_buffer(handler_proxy_context, (char *) data, data_size);

    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    write_proxy_out_buffer(handler_proxy_context, ": ", 2);
    write_proxy_out_buffer(handler_proxy_context, (char *) data, data_size);
    write_proxy_out_buffer(handler_proxy_context, "\r\n", 2);
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_backend(struct http_parser_t *http_parser) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;

    write_proxy_out_buffer(handler_proxy_context, "\r\n", 2);
    write_connection_c(
        connection,
        handler_proxy_context->out_buffer,
        handler_proxy_context->out_buffer_size,
        NULL,
        NULL,
        FALSE
    );

    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;

    char *buffer = MALLOC(data_size);
    memcpy(buffer, data, data_size);
    write_connection(
        connection,
        buffer,
        data_size,
        NULL,
        NULL
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_backend(struct http_parser_t *http_parser) {
    /* retrieves the current proxy context reference from the parser object
    and uses it to retrieve the connection object to be used in the write operation */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;

    write_connection_c(
        connection,
        "",
        0,
        _cleanup_handler_proxy,
        (void *) (size_t) http_parser->flags,
        FALSE
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_proxy(struct http_parser_t *http_parser) {
    /* allocates space for the handler proxy context */
    struct handler_proxy_context_t *handler_proxy_context;

    /* creates the handler proxy context and then sets the handler
    proxy context as the context for the http parser */
    create_handler_proxy_context(&handler_proxy_context);
    http_parser->context = handler_proxy_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_proxy(struct http_parser_t *http_parser) {
    /* retrieves the handler proxy context from the http parser */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;

    /* in case there's a connection with the client defined the parameters
    of it must be undefined so that no more bottom up operations occur */
    if(handler_proxy_context->connection_c) {
        FREE(handler_proxy_context->connection_c->parameters);
        handler_proxy_context->connection_c->parameters = NULL;
    }

    /* deletes the handler proxy context, it's not going to be possible
    to re-use the context anymore for this session */
    delete_handler_proxy_context(handler_proxy_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reset_http_parser_handler_proxy(struct http_parser_t *http_parser) {
    /* retrieves the handler proxy context from the http parser */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;

    /* restores the buffer size back to the original value, this
    should discard the values in the buffer */
    handler_proxy_context->buffer_size = 0;
    handler_proxy_context->out_buffer_size = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_proxy(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_proxy;
    http_settings->on_url = url_callback_handler_proxy;
    http_settings->on_header_field = header_field_callback_handler_proxy;
    http_settings->on_header_value = header_value_callback_handler_proxy;
    http_settings->on_headers_complete = headers_complete_callback_handler_proxy;
    http_settings->on_body = body_callback_handler_proxy;
    http_settings->on_message_complete = message_complete_callback_handler_proxy;
    http_settings->on_location = location_callback_handler_proxy;
    http_settings->on_virtual_url = virtual_url_callback_handler_proxy;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_proxy(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the http settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;
    http_settings->on_location = NULL;
    http_settings->on_virtual_url = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _cleanup_handler_proxy(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current http flags as the provided parameters
    these flags are going to be used for conditional execution */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* checks if the current connection should be kept alive, this must
    be done prior to the unseting of the connection as the current gif
    context structrue will be destroyed there */
    unsigned char keep_alive = flags & FLAG_CONNECTION_KEEP_ALIVE;

    /* in case there is an http handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current http connection and then sets the reference
        to it in the http connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive must be closed
    in the normal manner (using the close connection function) otherwise
    releases the lock on the http connection, this will allow further
    messages to be processed, an update event should raised following this
    lock releasing call */
    if(!keep_alive) { connection->close_connection(connection); }
    else { http_connection->release(http_connection); }

    /* raise no error */
    RAISE_NO_ERROR;
}
