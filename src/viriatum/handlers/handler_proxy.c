/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
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
    backend connections to the proxy target servers and then creates the
    reverved version of the same map associating the backend connection with
    the proxy client connections */
    create_hash_map(&proxy_handler->connections_map, 0);
    create_hash_map(&proxy_handler->reverse_map, 0);

    /* creates the internal structures for the map that is going to be used
    to resolve the proxy connection to the old set on close handler so that
    it's possible to propagate the on close callback call */
    create_hash_map(&proxy_handler->on_close_map, 0);

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
    of the client connections to the proxy to the backend connections
    and the exact oposite of that structure (reverse mapping) */
    delete_hash_map(proxy_handler->connections_map);
    delete_hash_map(proxy_handler->reverse_map);

    /* removes the hash map structure that associates a client connection
    with the proxy with the upper on close handler that has been replaced
    in order to provide the backend connection removal */
    delete_hash_map(proxy_handler->on_close_map);

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
    handler_proxy_context->pending = FALSE;
    handler_proxy_context->pending_write = 0;

    /* creates a new set of HTTP settings and parser for the new
    context that is being created, these are going to be used in
    the backend connection to be able to process its data */
    create_http_settings(&handler_proxy_context->http_settings);
    create_http_parser(&handler_proxy_context->http_parser, 0);

    /* sets the default callback functions in the HTTP settings
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

    /* in case the HTTP settings structure is defined for the current
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

    /* allocates the HTTP handler */
    struct http_handler_t *http_handler;

    /* allocates space for the proxy handler */
    struct proxy_handler_t *proxy_handler;

    /* creates the HTTP handler and then uses it to create
    the proxy handler (lower substrate) */
    service->create_http_handler(service, &http_handler, (unsigned char *) "proxy");
    create_proxy_handler(&proxy_handler, http_handler);

    /* sets the HTTP handler attributes */
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
        if(_location->proxy_pass != NULL) {
            parse_url_static(
                (char *) _location->proxy_pass,
                strlen((char *) _location->proxy_pass),
                &_location->url_s
            );
        }
    }

    /* adds the HTTP handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_proxy(struct service_t *service) {
    /* allocates the HTTP handler and then for the proxy
    handler objects to be released from the current structure */
    struct http_handler_t *http_handler;
    struct proxy_handler_t *proxy_handler;

    /* retrieves the HTTP handler from the service, then retrieves
    the lower substrate as the proxy handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "proxy");
    proxy_handler = (struct proxy_handler_t *) http_handler->lower;

    /* deletes the proxy handler reference */
    delete_proxy_handler(proxy_handler);

    /* remove the HTTP handler from the service after
    that deletes the handler reference */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE close_proxy_connection(struct io_connection_t *io_connection) {
    /* allocates space for the various structures that are going
    to be used as temporary variables in the close operation */
    struct http_handler_t *http_handler;
    struct proxy_handler_t *proxy_handler;
    struct connection_t *connection_c;
    io_connection_callback on_close;

    /* retrieves the uper abstract connection object from the
    io connection and then uses it to retrieve the service reference */
    struct connection_t *connection = io_connection->connection;
    struct service_t *service = connection->service;

    /* retrieves the reference to the HTTP handler from the service
    using the handler map and then  retrieves the lower proxy handler */
    get_value_string_hash_map(
        service->http_handlers_map,
        (unsigned char *) "proxy",
        (void **) &http_handler
    );
    proxy_handler = (struct proxy_handler_t *) http_handler->lower;

    /* retrieves the client (backend) connection associated with the
    current proxy client connection to be used for closing, if the
    client closes the backend connection must also be closed */
    get_value_hash_map(
        proxy_handler->connections_map,
        (size_t) connection,
        NULL,
        (void **) &connection_c
    );

    /* retrieves the top level on close handler to be called in order
    to propagate the closing operation to the upper layers */
    get_value_hash_map(
        proxy_handler->on_close_map,
        (size_t) connection,
        NULL,
        (void **) &on_close
    );

    /* in case an on close handler is define in the associative map must
    restore it and then call the top layer for the close event propagation */
    if(on_close != NULL) {
        io_connection->on_close = on_close;
        on_close(io_connection);
    }

    /* in case the client (backend) connection is defined must close it also
    because no backend connection should be defined for a non existing proxy
    client connection, it's the default strategy */
    if(connection_c != NULL) {
        connection_c->close_connection(connection_c);
    }

    /* undefines the complete set of values associated both with the proxy client
    connection and with the backend connection, no longer required */
    set_value_hash_map(proxy_handler->connections_map, (size_t) connection, NULL, NULL);
    set_value_hash_map(proxy_handler->reverse_map, (size_t) connection_c, NULL, NULL);
    set_value_hash_map(proxy_handler->on_close_map, (size_t) connection, NULL, NULL);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_proxy(struct http_connection_t *http_connection) {
    /* sets both the HTTP parser values and the HTTP
    settings handler for the current proxy handler */
    _set_http_parser_handler_proxy(http_connection->http_parser);
    _set_http_settings_handler_proxy(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_proxy(struct http_connection_t *http_connection) {
    /* unsets both the HTTP parser values and the HTTP
    settings handler from the current proxy handler */
    _unset_http_parser_handler_proxy(http_connection->http_parser);
    _unset_http_settings_handler_proxy(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE reset_handler_proxy(struct http_connection_t *http_connection) {
    /* resets the HTTP parser values */
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
    /* reserves space fot the pointer to the buffer that may
    be created in case there's a problem in the gateway */
    char *buffer;

    /* retrieves the parser's context as the proxy context and then
    uses it's parameters to retrieve the connection and then retrieves
    it's underlying layers of the connection */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection =\
        (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection =\
        (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection =\
        (struct http_connection_t *) io_connection->lower;

    /* acquires the current HTTP connection this avoids the parallel usage of
    the connection, removing any issue arround paralel message handling */
    http_connection->acquire(http_connection);

    /* checks if the client connection with the backend server is available
    in case it's not there must have been an error and it must be sent to the
    proxy client connection (service conneciont) */
    if(handler_proxy_context->connection_c == NULL) {
        buffer = MALLOC(VIRIATUM_HTTP_SIZE);
        write_http_error(
            connection,
            buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            502,
            "Bad gateway",
            "Invalid proxy connection",
            http_parser->flags & FLAG_KEEP_ALIVE ? KEEP_ALIVE : KEEP_CLOSE,
            _cleanup_handler_proxy,
            (void *) (size_t) http_parser->flags
        );
    } else {
        write_connection_c(
            handler_proxy_context->connection_c,
            (unsigned char *) handler_proxy_context->buffer,
            handler_proxy_context->buffer_size,
            NULL,
            NULL,
            FALSE
        );
    }

    /* raises no error as no problem has arised from the
    final handling of the proxy message */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_proxy(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* retrieves the handler proxy context from the HTTP parser */
    struct handler_proxy_context_t *handler_proxy_context =
        (struct handler_proxy_context_t *) http_parser->context;

    /* retrieves the connection from the parser and then uses it to  the
    the correct proxy handler reference from the HTTP connection */
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
    /* reserves space for a return error code to be used in various
    operation in the current function call */
    ERROR_CODE return_value;

    /* allocates space for the pointers to both the parameters and the
    client connection (to the backend) structures to be used/created*/
    struct custom_parameters_t *parameters;
    struct connection_t *connection_c;

    /* sets space for the resulting size of the buffer that will hold
    the status line to be sent to the backend connection */
    size_t size_m;

    /* allocates two buffers for temporary and local operations to
    in the current callback function */
    char buffer[1024];
    char path[1024];

    /* allocates space fot the temporary variable that will store the
    on close handler that will be set in the associative map */
    io_connection_callback on_close;

    /* retrieves the enumeration representation of the version and then
    uses it to retrieve the string value out of it, then retrieves the
    proper HTTP method used in the current call */
    enum http_version_e version_e = get_http_version(
        http_parser->http_major,
        http_parser->http_minor
    );
    const char *version = get_http_version_string(version_e);
    const char *method = http_method_strings[http_parser->method - 1];

    /* retrieves the connection from the parser parameters and then uses
    it to retrieve its lower connection layers (substrates) then uses the
    HTTP connection to retrieve the handler and the handler to retrieve the
    current proxy context structure */
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

    /* copies the virtual url value into the local path buffer and then
    closes it as a null terminated string */
    memcpy(path, data, data_size);
    path[data_size] = '\0';

    /* writes the initial line of the HTTP message into the temporary buffer
    and then sends this value to the current buffer for writing */
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

    /* updates the current proxy context so that the pending flag is set
    this means that the message to be sent to the proxy client is in the
    middle of the processing, any connection dropped in this state should
    also drop the connection to the proxy client */
    handler_proxy_context->pending = TRUE;

    /* sets the initial values for the number of bytes pending in the write
    buffer to be writen this value will be used to control the read stream
    of the backend server, in order to avoid flooding */
    handler_proxy_context->pending_write = 0;

    /* in case the connection client reference structure for the current
    context is not defined a new connection must be created */
    if(handler_proxy_context->connection_c == NULL) {
        /* creates a general HTTP client connection structure containing
        all the general attributes for a connection, then sets the
        "local" connection reference from the pointer */
        return_value = _create_client_connection(
            &handler_proxy_context->connection_c,
            connection->service,
            handler_proxy_context->url_s.host,
            handler_proxy_context->url_s.port ?\
                handler_proxy_context->url_s.port : 80
        );
        if(IS_ERROR_CODE(return_value)) {
            V_ERROR_F("Problem connecting to backend (%s)\n", (char *) GET_ERROR());
            FREE(parameters);
            RAISE_NO_ERROR;
        }

        /* saves the currently set on close handler for the io connection
        in the temporary local variable and then updates the reference to
        the close handler with the callback function that closes the proxy */
        on_close = io_connection->on_close;
        io_connection->on_close = close_proxy_connection;

        /* retrieves the reference for the connection object that will be used
        for the connection with the final back-end client */
        connection_c = handler_proxy_context->connection_c;

        /* sets the protocol for the "client" connection as custom and sets
        the "custom" parameters structure in it so that the callback functions
        may be controlled for the HTTP proxy behaviour */
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

        /* sets the reverse connection map, associating the client
        (backend) connection with its own client proxy to be used
        in the backend operation callbacks */
        set_value_hash_map(
            proxy_handler->reverse_map,
            (size_t) handler_proxy_context->connection_c,
            NULL,
            connection
        );

        /* updates the on close map that associates the client connection
        with the previous (old) on close handler function with the respective
        on cloese function for the current connection, to be used in callback
        propagation and in the closing of the backend connection */
        set_value_hash_map(
            proxy_handler->on_close_map,
            (size_t) connection,
            NULL,
            (void *) (size_t) on_close
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
        may be controlled for the HTTP proxy behaviour */
        connection_c->protocol = CUSTOM_PROTOCOL;
        connection_c->parameters = (void *) parameters;
    }

    /* raises no error to the caller function because no problem
    has occured while handling the virtual url */
    RAISE_NO_ERROR;
}

ERROR_CODE data_backend_handler(struct io_connection_t *io_connection, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the proxy context that may be retrieved
    latter in the function */
    struct handler_proxy_context_t *handler_proxy_context;

    /* retrieves the top level connection object from the io connection
    and then uses it to retrieve its parameters */
    struct connection_t *connection = io_connection->connection;
    struct custom_parameters_t *custom_parameters =\
        (struct custom_parameters_t *) connection->parameters;

    /* in case there're no custom parameters available for the current
    backend connection, the connection is already disabled (but not closed)
    and so the data operation should be ignored */
    if(custom_parameters == NULL) { RAISE_NO_ERROR; }

    /* retrieves the current proxy context structure from the custom parameters
    to be used for the processing of the current data received from the backend */
    handler_proxy_context =\
        (struct handler_proxy_context_t *) custom_parameters->parameters;

    /* runs the process operation (parser iteration) using the buffer that contains
    the data that has been retrieved from the backend server, this should trigger a
    series of callbacks for the various stages of the parsing, then returns the control
    flow to the caller function (everything went well) */
    process_data_http_parser(
        handler_proxy_context->http_parser,
        handler_proxy_context->http_settings,
        buffer,
        buffer_size
    );
    RAISE_NO_ERROR;
}

ERROR_CODE open_backend_handler(struct io_connection_t *io_connection) {
    RAISE_NO_ERROR;
}

ERROR_CODE close_backend_handler(struct io_connection_t *io_connection) {
    /* allocates the space for the various temporary structures that
    are going to be used for the closing operation of the backend connection */
    struct handler_proxy_context_t *handler_proxy_context;
    struct connection_t *connection_s;
    struct io_connection_t *io_connection_s;

    /* reserves space for both the top level HTTP handler structure and the
    concrete proxy handler structure containing the specifics of the proxy and
    then allocates space for the on close function pointer */
    struct http_handler_t *http_handler;
    struct proxy_handler_t *proxy_handler;
    io_connection_callback on_close;

    /* unpacks the connection object to retrieve the custom parameters
    from it, these parameters are only set if the connection is in the
    middle of processing a proxy connection */
    struct connection_t *connection = io_connection->connection;
    struct service_t *service = connection->service;
    struct custom_parameters_t *custom_parameters =\
        (struct custom_parameters_t *) connection->parameters;

    /* uses the "global" service structure to retrieve the handle to the
    proper proxy handler structure so that it may be used to retrieve the
    associated connection related information */
    get_value_string_hash_map(
        service->http_handlers_map,
        (unsigned char *) "proxy",
        (void **) &http_handler
    );
    proxy_handler = (struct proxy_handler_t *) http_handler->lower;

    /* retrieves the service connection (with the proxy client) from the
    current backend connection using the reversed map in the proxy handler */
    get_value_hash_map(
        proxy_handler->reverse_map,
        (size_t) connection,
        NULL,
        (void **) &connection_s
    );

    /* uses the service connection to retrieve the currently associated
    on close function callback in order to restore it */
    get_value_hash_map(
        proxy_handler->on_close_map,
        (size_t) connection_s,
        NULL,
        (void **) &on_close
    );

    /* resolves the service connection (proxy client) into the proper io
    and HTTP connection and then restores the close handler to them */
    io_connection_s = (struct io_connection_t *) connection_s->lower;
    io_connection_s->on_close = on_close;

    /* unsets the complete set of map asociating the data from the frontend
    connections to the backend so that no more resolution is possible, this
    is the expected behavior because no more backend connections are available */
    set_value_hash_map(proxy_handler->connections_map, (size_t) connection_s, NULL, NULL);
    set_value_hash_map(proxy_handler->reverse_map, (size_t) connection, NULL, NULL);
    set_value_hash_map(proxy_handler->on_close_map, (size_t) connection_s, NULL, NULL);

    /* in case the custom parameters are set the connection is talking
    with the backend server and so the connection must be correctly and
    gracefully handled */
    if(custom_parameters != NULL) {
        /* retrieves the proxy context from the parameters and then
        unsets the connection with the client so that no more connection
        processing operation uses it (it's no longer usable) */
        handler_proxy_context =\
            (struct handler_proxy_context_t *) custom_parameters->parameters;
        handler_proxy_context->connection_c = NULL;

        /* in case there's data currently pending then the final connection cleanup
        operation has not been yet scheduled, so one must be scheduled to close
        the service (proxy client) connection */
        if(handler_proxy_context->pending) {
            write_connection_c(
                connection,
                (unsigned char *) "",
                0,
                _cleanup_handler_proxy,
                NULL,
                FALSE
            );
        }
    }

    /* raises no error as the processing of the close opearion
    in the backend connection has been done correctly */
    RAISE_NO_ERROR;
}

ERROR_CODE line_callback_backend(struct http_parser_t *http_parser) {
    /* allocates space for both the size of the message and
    for the buffer to be used in the status line construction */
    size_t size_m;
    char buffer[1024];

    /* retrieves the current parser's context as the proxy context
    structure to be used for buffer handling */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;

    /* retrieves the enumeration representation of the version
    value in order to be used in the string retrieval */
    enum http_version_e version_e = get_http_version(
        http_parser->http_major,
        http_parser->http_minor
    );

    /* retrieves the string value of the version, the status code
    and the equivalent status message from the pre-defined ones */
    const char *version = get_http_version_string(version_e);
    unsigned short status_code = http_parser->status_code;
    const char *status_message = GET_HTTP_STATUS(status_code);

    /* writes the status line (should mimic the received one) into a local
    buffer and then writes this buffer into the output buffer */
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
    /* retrieves the parser's context as the proxy context and then uses
    it to retrieve both the proxy client connection and the backend client
    connection to be used in the processing of the header's final */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;
    struct connection_t *connection_c = handler_proxy_context->connection_c;

    /* writes the final header to body separator in the current output
    buffer of the proxy and then flushes (writes) the output buffer to
    the proxy client connection (propagation) */
    write_proxy_out_buffer(handler_proxy_context, "\r\n", 2);
    write_connection_c(
        connection,
        (unsigned char *) handler_proxy_context->out_buffer,
        handler_proxy_context->out_buffer_size,
        _pending_handler_proxy,
        (void *) handler_proxy_context,
        FALSE
    );

    /* adds the size of the current data to the counter of bytes
    that are in the write buffer and then in case this values exceeds
    the maximum allowed and the read enabled in the client connection
    disables the read operations in the backend connection (avoids flooding) */
    handler_proxy_context->pending_write += handler_proxy_context->out_buffer_size;
    if(connection_c != NULL && connection_c->read_registered == TRUE &&\
        handler_proxy_context->pending_write >= VIRIATUM_MAX_READ) {
        connection_c->unregister_read(connection_c);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_backend(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the proxy context object from the parser and then
    uses it to retrieve both the (proxy client) connection and the
    client connection (backend connection) */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;
    struct connection_t *connection_c = handler_proxy_context->connection_c;

    /* allocates a buffer of the same size of the received body
    data and then copies the data into this new buffer, then
    writes the buffer into the current (proxy client) connection */
    char *buffer = MALLOC(data_size);
    memcpy(buffer, data, data_size);
    write_connection(
        connection,
        (unsigned char *) buffer,
        data_size,
        _pending_handler_proxy,
        (void *) handler_proxy_context
    );

    /* adds the size of the current data to the counter of bytes
    that are in the write buffer and then in case this values exceeds
    the maximum allowed and the read enabled in the client connection
    disables the read operations in the backend connection (avoids flooding) */
    handler_proxy_context->pending_write += data_size;
    if(connection_c->read_registered == TRUE &&\
        handler_proxy_context->pending_write >= VIRIATUM_MAX_READ) {
        connection_c->unregister_read(connection_c);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_backend(struct http_parser_t *http_parser) {
    /* retrieves the current proxy context reference from the parser object
    and uses it to retrieve the connection object to be used in the write operation */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;
    struct connection_t *connection = handler_proxy_context->connection;

    /* writes the final (empty value) to the connection stream so that
    the cleanup handler callback is called at the end of all the pending
    write operations, this is a required "empty" operation */
    write_connection_c(
        connection,
        (unsigned char *) "",
        0,
        _cleanup_handler_proxy,
        (void *) (size_t) http_parser->flags,
        FALSE
    );

    /* unsets the pending flag in the current context meaning that
    the message has been completely sent to the proxy client, any
    disconnect operation from the backend connection comming after
    will not afect the client of the proxy */
    handler_proxy_context->pending = FALSE;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_proxy(struct http_parser_t *http_parser) {
    /* allocates space for the handler proxy context */
    struct handler_proxy_context_t *handler_proxy_context;

    /* creates the handler proxy context and then sets the handler
    proxy context as the context for the HTTP parser */
    create_handler_proxy_context(&handler_proxy_context);
    http_parser->context = handler_proxy_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_proxy(struct http_parser_t *http_parser) {
    /* retrieves the handler proxy context from the HTTP parser */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;

    /* in case there's a connection with the client defined the parameters
    of it must be undefined so that no more bottom up operations occur,
    otherwise invalid data would be sent to the proxy client */
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
    /* retrieves the handler proxy context from the HTTP parser */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) http_parser->context;

    /* restores the buffer size back to the original value, this
    should discard the values in the buffer */
    handler_proxy_context->buffer_size = 0;
    handler_proxy_context->out_buffer_size = 0;
    handler_proxy_context->pending = FALSE;
    handler_proxy_context->pending_write = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_proxy(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the HTTP settings
    structure, these callbacks are going to be used in the runtime
    processing of HTTP parser (runtime execution) */
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
    /* unsets the various callback functions from the HTTP settings */
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

ERROR_CODE _pending_handler_proxy(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* loads the parameters pointer as the proxy context structure and then
    retrieves the client connection from it to be used for its state update */
    struct handler_proxy_context_t *handler_proxy_context =\
        (struct handler_proxy_context_t *) parameters;
    struct connection_t *connection_c = handler_proxy_context->connection_c;

    /* updates the amount of byts pending in the write buffer and then in
    case the client connection is not registered for writing and the amount
    of bytes pending has reached a resonable level re-enabled thre reading */
    handler_proxy_context->pending_write -= data->size_base;
    if(connection_c != NULL && connection_c->read_registered == FALSE &&\
        handler_proxy_context->pending_write < VIRIATUM_TRE_READ) {
        connection_c->register_read(connection_c);
    }

    /* returns with no error as the processing of the data write callback
    has been a success, no problem arised */
    RAISE_NO_ERROR;
}

ERROR_CODE _cleanup_handler_proxy(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current HTTP flags as the provided parameters
    these flags are going to be used for conditional execution */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* checks if the current connection should be kept alive, this must
    be done prior to the unseting of the connection as the current gif
    context structrue will be destroyed there */
    unsigned char keep_alive = flags & FLAG_KEEP_ALIVE;

    /* in case there is an HTTP handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current HTTP connection and then sets the reference
        to it in the HTTP connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive must be closed
    in the normal manner (using the close connection function) otherwise
    releases the lock on the HTTP connection, this will allow further
    messages to be processed, an update event should raised following this
    lock releasing call */
    if(!keep_alive) { connection->close_connection(connection); }
    else { http_connection->release(http_connection); }

    /* raise no error */
    RAISE_NO_ERROR;
}
