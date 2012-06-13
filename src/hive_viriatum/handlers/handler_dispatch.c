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

#include "handler_dispatch.h"

ERROR_CODE create_dispatch_handler(struct dispatch_handler_t **dispatch_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the dispatch handler size */
    size_t dispatch_handler_size = sizeof(struct dispatch_handler_t);

    /* allocates space for the dispatch handler */
    struct dispatch_handler_t *dispatch_handler = (struct dispatch_handler_t *) MALLOC(dispatch_handler_size);

    /* sets the dispatch handler attributes (default) values */
#ifdef VIRIATUM_PCRE
    dispatch_handler->regex = NULL;
#endif
    dispatch_handler->names = NULL;
    dispatch_handler->regex_count = 0;

    /* sets the dispatch handler in the upper http handler substrate */
    http_handler->lower = (void *) dispatch_handler;

    /* sets the dispatch handler in the mod lua http handler pointer */
    *dispatch_handler_pointer = dispatch_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_dispatch_handler(struct dispatch_handler_t *dispatch_handler) {
#ifdef VIRIATUM_PCRE
    /* allocates space for the index counter */
    size_t index;
#endif

#ifdef VIRIATUM_PCRE
    /* iterates over all the regular expressions to release their
    internal memory contents */
    for(index = 0; index < dispatch_handler->regex_count; index++ ) {  pcre_free(dispatch_handler->regex[index]); }
#endif

    /* in case the names buffer is defined releases it */
    if(dispatch_handler->names != NULL) { FREE(dispatch_handler->names); }

#ifdef VIRIATUM_PCRE
    /* in case the regex buffer is defined releases it */
    if(dispatch_handler->regex != NULL) { FREE(dispatch_handler->regex); }
#endif

    /* releases the dispatch handler */
    FREE(dispatch_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_handler_dispatch(struct service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates space for the dispatch handler */
    struct dispatch_handler_t *dispatch_handler;

#ifdef VIRIATUM_PCRE
    /* allocates space for the regex related variables */
    const char *error;
    int error_offset;
#endif

    /* creates the http handler and then uses it to create
    the dispatch handler (lower substrate) */
    service->create_http_handler(service, &http_handler, (unsigned char *) "dispatch");
    create_dispatch_handler(&dispatch_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->set = set_handler_dispatch;
    http_handler->unset = unset_handler_dispatch;
    http_handler->reset = NULL;


    dispatch_handler->regex_count = 4;
#ifdef VIRIATUM_PCRE
    dispatch_handler->regex = (pcre **) MALLOC(sizeof(pcre *) * dispatch_handler->regex_count);
    dispatch_handler->regex[0] = pcre_compile("[.]*\\.lua", 0, &error, &error_offset, NULL);
    dispatch_handler->regex[1] = pcre_compile("[.]*\\.php", 0, &error, &error_offset, NULL);
	dispatch_handler->regex[2] = pcre_compile("[.]*\\.wsgi", 0, &error, &error_offset, NULL);
    dispatch_handler->regex[3] = pcre_compile("[.]*\\.default", 0, &error, &error_offset, NULL);
#endif
    dispatch_handler->names = (unsigned char **) MALLOC(sizeof(unsigned char *) * dispatch_handler->regex_count);
    dispatch_handler->names[0] = (unsigned char *) "lua";
    dispatch_handler->names[1] = (unsigned char *) "php";
	dispatch_handler->names[2] = (unsigned char *) "wsgi";
    dispatch_handler->names[3] = (unsigned char *) "dedault";



    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_dispatch(struct service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates space for the dispatch handler */
    struct dispatch_handler_t *dispatch_handler;

    /* retrieves the http handler from the service, then retrieves
    the lower substrate as the dispatch handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "dispatch");
    dispatch_handler = (struct dispatch_handler_t *) http_handler->lower;

    /* deletes the dispatch handler reference */
    delete_dispatch_handler(dispatch_handler);

    /* remove the http handler from the service after
    that deletes the handler reference */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_dispatch(struct http_connection_t *http_connection) {
    /* sets the http parser values */
    _set_http_parser_handler_dispatch(http_connection->http_parser);

    /* sets the http settings values */
    _set_http_settings_handler_dispatch(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_dispatch(struct http_connection_t *http_connection) {
    /* unsets the http parser values */
    _unset_http_parser_handler_dispatch(http_connection->http_parser);

    /* unsets the http settings values */
    _unset_http_settings_handler_dispatch(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_dispatch(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
#ifdef VIRIATUM_PCRE
    int matching;
    size_t index;
#endif

    /* allocates space for the name of the handler to be
    used for the dispatching operation (target handler) */
    unsigned char *handler_name;

    /* allocates the required space for the url, this
    is done through static allocation */
    unsigned char url[VIRIATUM_MAX_URL_SIZE];

    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct service_t *service = connection->service;
    struct http_handler_t *handler = http_connection->http_handler;
#ifdef VIRIATUM_PCRE
    struct dispatch_handler_t *dispatch_handler = (struct dispatch_handler_t *) handler->lower;
#endif

    /* copies the memory from the data to the url, then
    puts the end of string in the url */
    memcpy(url, data, data_size);
    url[data_size] = '\0';





    /* THIS IS EXTREMLY SLOW !!! WARNING may be a better
    idea to compile all the regex into a single regex must
    refer to the nginx documentation for that */

    /* sets the default handler, this is considered to be
    the fallback in case no handler is found */
    handler_name = (unsigned char *) DISPATCH_DEFAULT_HANDLER;

#ifdef VIRIATUM_PCRE
    /* iterates over all the regular expressions so that they
    may be tested agains the current url */
    for(index = 0; index < dispatch_handler->regex_count; index++) {
        /* tries to match the current url agains the registered
        regular expression in case it fails continues the loop */
        matching = pcre_exec(dispatch_handler->regex[index], NULL, (char *) url, data_size, 0, 0, NULL, 0);
        if(matching != 0) { continue; }

        /* sets the name of the handler as the name in the current index
        the breaks the loop to process it */
        handler_name = dispatch_handler->names[index];
        break;
    }
#endif

    /* END OF WARNING */

    /* sets the current http handler accoring to the current options
    in the service, the http handler must be loaded in the handlers map
    in case the handler is not currently available an error is printed */
    get_value_string_hash_map(service->http_handlers_map, handler_name, (void **) &handler);
    if(handler) {
        /* retrieves the current handler and then unsets it
        from the connection (detach) then sets the the prper
        handler in the connection and notifies it of the url */
        http_connection->http_handler->unset(http_connection);
        handler->set(http_connection);
        http_connection->http_handler = handler;
        http_connection->http_settings->on_message_begin(http_parser);
        http_connection->http_settings->on_url(http_parser, data, data_size);
    } else {
        /* prints an error message to the output */
        V_ERROR_F("Error retrieving '%s' handler reference\n", handler_name);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_dispatch(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_dispatch(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_dispatch(struct http_parser_t *http_parser) {
    /* sends (and creates) the reponse */
    _send_response_handler_dispatch(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_dispatch(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_dispatch(struct http_parser_t *http_parser) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_dispatch(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_dispatch;
    http_settings->on_url = url_callback_handler_dispatch;
    http_settings->on_header_field = header_field_callback_handler_dispatch;
    http_settings->on_header_value = header_value_callback_handler_dispatch;
    http_settings->on_headers_complete = headers_complete_callback_handler_dispatch;
    http_settings->on_body = body_callback_handler_dispatch;
    http_settings->on_message_complete = message_complete_callback_handler_dispatch;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_dispatch(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the http settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_handler_dispatch(struct http_parser_t *http_parser) {
    /* allocates the response buffer */
    char *response_buffer = MALLOC(256);

    /* retrieves the connection from the http parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* writes the response to the connection, registers for the appropriate callbacks
    this method uses the http error util to correctly format the error message */
    write_http_error(connection, response_buffer, "500", "Internal Server Error", DISPATCH_ERROR_MESSAGE, _send_response_callback_handler_dispatch, (void *) (size_t) http_parser->flags);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_dispatch(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* retrieves the current http flags */
    unsigned char flags = (unsigned char) (size_t) parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* in case there is an http handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current http connection and then sets the reference
        to it in the http connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive */
    if(!(flags & FLAG_CONNECTION_KEEP_ALIVE)) {
        /* closes the connection */
        connection->close_connection(connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
