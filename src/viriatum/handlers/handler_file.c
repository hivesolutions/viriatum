/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "handler_file.h"

ERROR_CODE create_file_handler(struct file_handler_t **file_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the file handler size */
    size_t file_handler_size = sizeof(struct file_handler_t);

    /* allocates space for the file handler */
    struct file_handler_t *file_handler = (struct file_handler_t *) MALLOC(file_handler_size);

    /* sets the file handler attributes (default) values */
    file_handler->locations = NULL;
    file_handler->locations_count = 0;

    /* sets the file handler in the upper HTTP handler substrate */
    http_handler->lower = (void *) file_handler;

    /* sets the file handler in the file handler pointer */
    *file_handler_pointer = file_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_file_handler(struct file_handler_t *file_handler) {
    /* in case the locations buffer is set it must be released
    to avoid any memory leak (not going to be used) */
    if(file_handler->locations != NULL) { FREE(file_handler->locations); }

    /* releases the file handler */
    FREE(file_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_file_context(struct handler_file_context_t **handler_file_context_pointer) {
    /* retrieves the handler file context size */
    size_t handler_file_context_size = sizeof(struct handler_file_context_t);

    /* allocates space for the handler file context */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) MALLOC(handler_file_context_size);

    /* sets the handler file default values */
    handler_file_context->base_path = NULL;
    handler_file_context->auth_basic = NULL;
    handler_file_context->auth_file = NULL;
    handler_file_context->file = NULL;
    handler_file_context->file_size = 0;
    handler_file_context->initial_byte = 0;
    handler_file_context->final_byte = 0;
    handler_file_context->flags = 0;
    handler_file_context->next_header = UNDEFINED_HEADER;
    handler_file_context->template_handler = NULL;
    handler_file_context->cache_control_status = 0;
    handler_file_context->etag_status = 0;
    handler_file_context->authorization_status = 0;
    handler_file_context->range_status = 0;

    /* sets the handler file context in the  pointer */
    *handler_file_context_pointer = handler_file_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_file_context(struct handler_file_context_t *handler_file_context) {
    /* in case there is a file defined in the handler
    file context closes it (avoiding a memory leak) */
    if(handler_file_context->file != NULL) {
        fclose(handler_file_context->file);
    }

    /* in case there is a template handler defined
    in the handler file context, deletes it relasing
    all of its currently allocated resources */
    if(handler_file_context->template_handler) {
        delete_template_handler(handler_file_context->template_handler);
    }

    /* releases the handler file context */
    FREE(handler_file_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE register_handler_file(struct service_t *service) {
    /* allocates space for the temporary value object and for
    the index counter to be used in the iteration of configurations */
    void *value;
    size_t index;

    /* allocates space for both the location and the configuration
    reference stuctures */
    struct location_t *location;
    struct sort_map_t *configuration;

    /* allocates space for the mod file location structure
    reference to be used to resolve the request */
    struct file_location_t *_location;

    /* allocates the HTTP handler */
    struct http_handler_t *http_handler;

    /* allocates space for the file handler */
    struct file_handler_t *file_handler;

    /* creates the HTTP handler and then uses it to create
    the file handler (lower substrate) */
    service->create_http_handler(service, &http_handler, (unsigned char *) "file");
    create_file_handler(&file_handler, http_handler);

    /* sets the HTTP handler attributes */
    http_handler->resolve_index = TRUE;
    http_handler->set = set_handler_file;
    http_handler->unset = unset_handler_file;
    http_handler->reset = reset_handler_file;

    /* allocates space for the various location structures
    that will be used to resolve the file request */
    file_handler->locations = (struct file_location_t *)
        MALLOC(service->locations.count * sizeof(struct file_location_t));
    memset(file_handler->locations, 0,
        service->locations.count * sizeof(struct file_location_t));

    /* updates the locations count variable in the file handler so
    that it's possible to iterate over the locations */
    file_handler->locations_count = service->locations.count;

    /* iterates over all the locations in the service to create the
    proper configuration structures to the module */
    for(index = 0; index < service->locations.count; index++) {
        /* retrieves the current (service) location and then uses it
        to retrieve the configuration sort map */
        location = &service->locations.values[index];
        configuration = location->configuration;

        /* retrieves the current mod file configuration reference from
        the location buffer, this is going ot be populated and sets the
        default values in it */
        _location = &file_handler->locations[index];
        _location->base_path = NULL;
        _location->auth_basic = NULL;
        _location->auth_file = NULL;

        /* tries ro retrieve the base path from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "base_path", &value);
        if(value != NULL) { _location->base_path = (unsigned char *) value; }

        /* tries ro retrieve the auth basic from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "auth_basic", &value);
        if(value != NULL) { _location->auth_basic = (unsigned char *) value; }

        /* tries ro retrieve the auth file from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "auth_file", &value);
        if(value != NULL) { _location->auth_file = (unsigned char *) value; }
    }

    /* adds the HTTP handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_file(struct service_t *service) {
    /* allocates the HTTP handler */
    struct http_handler_t *http_handler;

    /* allocates space for the file handler */
    struct file_handler_t *file_handler;

    /* retrieves the HTTP handler from the service, then retrieves
    the lower substrate as the file handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "file");
    file_handler = (struct file_handler_t *) http_handler->lower;

    /* deletes the file handler reference */
    delete_file_handler(file_handler);

    /* remove the HTTP handler from the service after
    that deletes the handler reference */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_file(struct http_connection_t *http_connection) {
    /* sets both the HTTP parser values and the HTTP
    settings handler for the current file handler */
    _set_http_parser_handler_file(http_connection->http_parser);
    _set_http_settings_handler_file(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_file(struct http_connection_t *http_connection) {
    /* unsets both the HTTP parser values and the HTTP
    settings handler from the current file handler */
    _unset_http_parser_handler_file(http_connection->http_parser);
    _unset_http_settings_handler_file(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE reset_handler_file(struct http_connection_t *http_connection) {
    /* resets the HTTP parser values */
    _reset_http_parser_handler_file(http_connection->http_parser);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_file(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates memory for the variable that will hold the
    size of the decoded file path */
    size_t decoded_size;

    /* the complete size of the string that contains the file
    path "calculated" from the url */
    size_t file_path_size;

    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context =\
        (struct handler_file_context_t *) http_parser->context;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;

    /* copies the memory from the data to the url and then
    puts the end of string in the url, note that only the path
    part of the string is used for the url */
    memcpy(handler_file_context->url, data, path_size);
    handler_file_context->url[path_size] = '\0';

    /* prints a debug message */
    V_INFO_F("%s %s\n", get_http_method_string(http_parser->method), handler_file_context->url);

    /* creates the file path using the base viriatum path
    this should be the complete absolute path */
    file_path_size = SPRINTF(
        (char *) handler_file_context->file_path,
        VIRIATUM_MAX_PATH_SIZE,
        "%s%s%s",
        VIRIATUM_CONTENTS_PATH,
        VIRIATUM_BASE_PATH,
        handler_file_context->url
    );

    /* decodes the url and file path for the percent encoding, this method
    uses constant (pre-allocated) memory for fast performance the resulting
    value is stored as a simple string in utf-8 encoding */
    decode_percent(
        handler_file_context->url,
        path_size,
        handler_file_context->url_d,
        &decoded_size
    );
    decode_percent(
        handler_file_context->file_path,
        file_path_size,
        handler_file_context->file_path_d,
        &decoded_size
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* switches over the size of the header name (field)
    that was provided (used for faster parsing) */
    switch(data_size) {
        case 5:
            if(data[0] == 'R' && data[1] == 'a' && data[2] == 'n' && data[3] == 'g') {
                /* updates the range status value, it's the next
                value to be parsed and put in context */
                handler_file_context->range_status = 1;
                handler_file_context->next_header = RANGE;
                break;
            }

            /* breaks the switch, this is the
            fallback in case no match exists */
            break;

        case 13:
            if(data[0] == 'I' && data[1] == 'f' && data[2] == '-' && data[3] == 'N') {
                /* updates the etag status value, it's the next
                value to be parsed and put in context */
                handler_file_context->etag_status = 1;
                handler_file_context->next_header = ETAG;
                break;
            }

            if(data[0] == 'C' && data[1] == 'a' && data[2] == 'c' && data[3] == 'h') {
                /* updates the cache control status value, it's the next
                value to be parsed and put in context */
                handler_file_context->cache_control_status = 1;
                handler_file_context->next_header = CACHE_CONTROL;
                break;
            }

            if(data[0] == 'A' && data[1] == 'u' && data[2] == 't' && data[3] == 'h') {
                /* updates the authorization status value, it's the next
                value to be parsed and put in context */
                handler_file_context->authorization_status = 1;
                handler_file_context->next_header = AUTHORIZATION;
                break;
            }

            /* breaks the switch, this is the
            fallback in case no match exists */
            break;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* switches over the kind of next header to be
    processed, note that in case the value is set
    to undefined no parsing will exist */
    switch(handler_file_context->next_header) {
        case ETAG:
            memcpy(handler_file_context->etag, data, 10);
            handler_file_context->etag[10] = '\0';
            handler_file_context->etag_status = 2;
            break;

        case CACHE_CONTROL:
            memcpy(handler_file_context->cache_control, data, data_size);
            handler_file_context->cache_control[data_size] = '\0';
            handler_file_context->cache_control_status = 2;
            break;

        case AUTHORIZATION:
            memcpy(handler_file_context->authorization, data, data_size);
            handler_file_context->authorization[data_size] = '\0';
            handler_file_context->authorization_status = 2;
            break;

        case RANGE:
            memcpy(handler_file_context->range, data, data_size);
            handler_file_context->range[data_size] = '\0';
            handler_file_context->range_status = 2;
            break;

        default:
            break;
    }

    /* sets the next heder value to the "default" undefined
    value so that no extra processing occurs on next header value */
    handler_file_context->next_header = UNDEFINED_HEADER;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_file(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_file(struct http_parser_t *http_parser) {
    /* allocates the file size and for the temporary count
    variable used to count the written bytes */
    size_t file_size;
    size_t count;

    /* allocates space for the directory entries and for
    the template handler */
    struct linked_list_t *directory_entries;
    struct linked_list_t *directory_entries_map;
    struct template_handler_t *template_handler;

    /* allocates space for the is directory and the is redirect flags */
    unsigned int is_directory = FALSE;
    unsigned int is_redirect = FALSE;

    /* allocates space for the new location value for
    redirect request cases and for the path to the
    template (for directory listing) */
    unsigned char location[VIRIATUM_MAX_PATH_SIZE];
    unsigned char template_path[VIRIATUM_MAX_PATH_SIZE];

    /* allocates space for the computation of the time
    and of the time string, then allocates space for the
    etag calculation structure (crc32 value), for the etag
    value and for the (file) extension and mime type */
    struct date_time_t time;
    char time_string[20];
    unsigned long crc32_value;
    char etag[11];
    char *extension;
    const char *mime_type;

    /* allocates space for the variable that will be set in case
    the connection is meant to be kep alive at the end of the
    HTTP message processing or not */
    unsigned char keep_alive;

    /* allocates space for the size of the url string to
    be calculates and for the folder path variable */
    size_t url_size;
    char folder_path[VIRIATUM_MAX_PATH_SIZE];

    /* allocates space for the temporary buffer that will hold
    the description of the error to be sent to the client in case
    there is a problem (eg: file not found) */
    char error_description[VIRIATUM_MAX_PATH_SIZE];

    /* allocates the space for the "read" result
    error code (valid by default) */
    ERROR_CODE error_code = 0;

    /* allocates the space required for the authentication result
    boolean value to be used by the basic authentication system */
    unsigned char auth_result = TRUE;

    /* allocates the headers buffer (it will be releases automatically by the writter)
    it need to be allocated in the heap so it gets throught the request cycle */
    char *headers_buffer = MALLOC(VIRIATUM_HTTP_SIZE);

    /* retrieves the handler file context from the HTTP parser and uses
    it to retrieve the respective flags value */
    struct handler_file_context_t *handler_file_context =\
        (struct handler_file_context_t *) http_parser->context;
    unsigned char flags = http_parser->flags;

    /* retrieves the connection from the HTTP parser parameters,
    the connection object is going to be used for the input and
    outpu operations associated with the file handling */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for register */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* verifies if the currently set flag grant "permission" to keep
    the connection alive at the end of the HTTP message processing,
    this values is going to be used for headers generation */
    keep_alive = flags & FLAG_KEEP_ALIVE;

    /* acquires the lock on the HTTP connection, this will avoids further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* checks if the path being request is in fact a directory */
    is_directory_file((char *) handler_file_context->file_path_d, &is_directory);

    /* in case the auth basic value is set in the current file
    context must proceed with the authentication process for
    the current authorization value */
    if(handler_file_context->auth_basic != NULL) {
        /* in case the authorization status is defined as set
        must proceed with the authorization, otherwise invalidates
        the authorization result immediately (no information has
        been provided from the client side) */
        if(handler_file_context->authorization_status == 2) {
            auth_http(
                (char *) handler_file_context->auth_file,
                (char *) handler_file_context->authorization,
                &auth_result
            );
        } else { auth_result = FALSE; }
    }

    /* in case the file path being request referes a directory
    it must be checked and the entries retrieved to be rendered */
    if(is_directory) {
        /* in case the current url does not ends with the trailing slash must
        redirect the user agent to the same location but with the trauling slash */
        if(handler_file_context->url[strlen((char *) handler_file_context->url) - 1] != '/') {
            /* creates the new location by adding the slash character to the current
            handler file context url (avoids directory confusion) */
            memcpy(location, handler_file_context->url, strlen((char *) handler_file_context->url));
            location[strlen((char *) handler_file_context->url)] = '/';
            location[strlen((char *) handler_file_context->url) + 1] = '\0';

            /* sets the is redirect flag (forces temporary redirect) */
            is_redirect = TRUE;
        }
        /* otherwise it's the correct directory location and must present the
        listing of the directory to the user agent */
        else {
            /* creates the complete path to the template file */
            SPRINTF(
                (char *) template_path,
                sizeof(template_path),
                "%s%s",
                VIRIATUM_RESOURCES_PATH,
                VIRIATUM_LISTING_PATH
            );

            /* prints a debug message */
            V_DEBUG_F("Processing template file '%s'\n", template_path);

            /* creates the directory entries (linked list) */
            create_linked_list(&directory_entries);

            /* lists the directory file into the directory
            entries linked list and then converts them into maps */
            list_directory_file((char *) handler_file_context->file_path_d, directory_entries);
            entries_to_map_file(directory_entries, &directory_entries_map);

            /* retrieves the current size of the url and copies into
            the folder path the appropriate part of it, this strategy
            takes into account the size of the url */
            url_size = strlen((char *) handler_file_context->url_d);
            if(url_size > 2) { memcpy(folder_path, &handler_file_context->url_d[1], url_size - 2); }
            if(url_size > 2) { folder_path[url_size - 2] = '\0'; }
            else { folder_path[0] = '\0'; }

            /* creates the template handler */
            create_template_handler(&template_handler);

            /* assigns the name of the current folder being listed to
            the template handler (to be set on the template) */
            assign_string_template_handler(
                template_handler,
                (unsigned char *) "folder_path",
                folder_path
            );

            /* assigns the directory entries to the template handler,
            this variable will be exposed to the template */
            assign_list_template_handler(
                template_handler,
                (unsigned char *) "entries",
                directory_entries_map
            );
            assign_integer_template_handler(
                template_handler,
                (unsigned char *) "items",
                (int) directory_entries_map->size
            );

            /* processes the file as a template handler */
            process_template_handler(template_handler, template_path);

            /* sets the template handler in the handler file context and unsets
            the flushed flag */
            handler_file_context->template_handler = template_handler;
            handler_file_context->flushed = FALSE;

            /* deletes the directory entries map and the
            directory entries */
            delete_directory_entries_map_file(directory_entries_map);
            delete_directory_entries_file(directory_entries);

            /* deletes the directory entries (linked list) and
            the entries map (linked list) */
            delete_linked_list(directory_entries);
            delete_linked_list(directory_entries_map);
        }
    }
    /* otherwise the file path must refered a "normal" file path and
    it must be checked */
    else {
        /* counts the total size (in bytes) of the contents
        in the file path, this also the call used for checking
        the existence of the file */
        error_code = count_file(
            (char *) handler_file_context->file_path_d,
            &file_size
        );

        /* in case there is no error count the file size, avoids
        extra problems while computing the etag */
        if(!IS_ERROR_CODE(error_code)) {
            /* resets the date time structure to avoid invalid
            date requests */
            memset(&time, 0, sizeof(struct date_time_t));

            /* retrieve the time of the last write in the file path */
            get_write_time_file((char *) handler_file_context->file_path_d, &time);

            /* creates the date time string for the file entry */
            SPRINTF(
                time_string,
                20,
                "%04d-%02d-%02d %02d:%02d:%02d",
                time.year,
                time.month,
                time.day,
                time.hour,
                time.minute,
                time.second
            );

            /* creates the crc32 value and prints it into the
            etag as an heexadecimal string value */
            crc32_value = crc_32((unsigned char *) time_string, 19);
            SPRINTF(etag, 11, "\"%08x\"", (unsigned int) crc32_value);
        }
    }

    /* sets the (HTTP) flags in the handler file context */
    handler_file_context->flags = http_parser->flags;

    /* tests the error code for error, in case there's an error
    the file is considered to be not found (normal error) */
    if(IS_ERROR_CODE(error_code)) {
        /* prints the information about the current error */
        V_DEBUG_F("%s\n", get_last_error_message_safe());

        /* creates the error description string from the error message and then
        sends the error to the connection (with the current format) */
        SPRINTF(
            error_description,
            VIRIATUM_MAX_PATH_SIZE,
            "File not found (%s)",
            handler_file_context->file_path_d
        );
        write_http_error(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            404,
            "Not Found",
            error_description,
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            _cleanup_handler_file,
            handler_file_context
        );
    } else if(!auth_result) {
        /* prints some debug information about the problem in
        the authentication of the request */
        V_DEBUG("Request not authorized\n");

        /* sends the message containing the error definition for
        the authorization failed operation, note that the realm
        is also passed as an argument (required for extra header) */
        write_http_error_a(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            401,
            "Unauthorized",
            "Invalid password or user not found",
            (char *) handler_file_context->auth_basic,
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            _cleanup_handler_file,
            handler_file_context
        );
    } else if(is_redirect) {
        /* writes the HTTP static headers to the response */
        count = write_http_headers(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            307,
            "Temporary Redirect",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            FALSE
        );
        SPRINTF(
            &headers_buffer[count],
            VIRIATUM_HTTP_SIZE - count,
            CONTENT_LENGTH_H ": 0\r\n"
            LOCATION_H ": %s\r\n\r\n",
            location
        );

        /* writes both the headers to the connection, registers
        for the appropriate callbacks */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _cleanup_handler_file,
            handler_file_context
        );
    }
    /* in case the current situation is a directory list */
    else if(is_directory) {
        /* writes the HTTP static headers to the response */
        write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            200,
            "OK",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            strlen((char *) handler_file_context->template_handler->string_value),
            NO_CACHE,
            TRUE
        );

        /* writes both the headers to the connection, register
        for the appropriate callbacks */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _send_data_handler_file,
            handler_file_context
        );
    }
    /* in case there's an etag value defined and the values matched
    the one defined for the file, time to return a not modified value
    to the client indicating that cache should be used */
    else if(handler_file_context->etag_status == 2 &&\
        strcmp(etag, (char *) handler_file_context->etag) == 0) {
        /* writes the HTTP static headers to the response */
        write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            304,
            "Not Modified",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            0,
            NO_CACHE,
            TRUE
        );

        /* writes both the headers to the connection, registers for
        the appropriate callbacks */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _cleanup_handler_file,
            handler_file_context
        );
    }
    /* in case the range value is set for the current file request
    must calculate the range and retrive the associated file chunk */
    else if(handler_file_context->range_status == 2) {
        /* retrieves the initial and final bytes for the requested
        range, the final range is set in accordance with the file
        size that is provided */
        get_http_range_limits(
            handler_file_context->range,
            &handler_file_context->initial_byte,
            &handler_file_context->final_byte,
            file_size
        );

        /* writes the HTTP static headers to the response indicating
        that only a part of the file is going to be retrieved, then
        writes also the content range header indicating which bytes
        are going to be retrieved */
        count = write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            206,
            "Partial content",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            handler_file_context->final_byte -\
            handler_file_context->initial_byte + 1,
            NO_CACHE,
            FALSE
        );
        SPRINTF(
            &headers_buffer[count],
            VIRIATUM_HTTP_SIZE - count,
            CONTENT_RANGE_H ": bytes %ld-%ld/%ld\r\n\r\n",
            (long int) handler_file_context->initial_byte,
            (long int) handler_file_context->final_byte,
            (long int) file_size
        );

        /* writes both the headers to the connection, registers for the
        appropriate callbacks, this is going to trigger the chain of write
        to callback behavior expected for the file sending */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _send_chunk_handler_file,
            handler_file_context
        );
    }
    /* otherwise there was no error in the file and it's a simple
    file situation (no directory) */
    else {
        /* sets the default (complete file) values for the reading
        of the file, these are not required for a complete file
        reading but only for partial (ranged) contents */
        handler_file_context->initial_byte = 0;
        handler_file_context->final_byte = file_size - 1;

        /* writes the HTTP static headers to the response indicating
        that the file is going to be served normally */
        count = write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            200,
            "OK",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            file_size,
            NO_CACHE,
            FALSE
        );

        /* retrieves the extension part of the file path and then uses
        it to try to retrieve the mime type string for it in case it's
        successfull "puts" the content type in the headers buffer, then
        puts the etag value in the file */
        extension = extension_path((char *) handler_file_context->file_path_d);
        mime_type = connection->service->get_mime_type(connection->service, extension);
        if(mime_type != NULL) {
            count += SPRINTF(
                &headers_buffer[count],
                VIRIATUM_HTTP_SIZE - count,
                CONTENT_TYPE_H ": %s\r\n",
                mime_type
            );
        }
        SPRINTF(
            &headers_buffer[count],
            VIRIATUM_HTTP_SIZE - count,
            ACCEPT_RANGES_H ": bytes\r\n"
            ETAG_H ": %s\r\n\r\n",
            etag
        );

        /* writes both the headers to the connection, registers for the
        appropriate callbacks, this is going to trigger the chain of write
        to callback behavior expected for the file sending */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _send_chunk_handler_file,
            handler_file_context
        );
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates memory for the variable that will hold the
    size of the decoded file path */
    size_t decoded_size;

    /* the complete size of the string that contains the file
    path "calculated" from the url */
    size_t file_path_size;

    /* retrieves the handler file context from the HTTP parser
    and uses it to retrieve the reference to the base path in context */
    struct handler_file_context_t *handler_file_context =\
        (struct handler_file_context_t *) http_parser->context;
    unsigned char *base_path = handler_file_context->base_path;

    /* copies the memory from the data to the url and then
    puts the end of string in the url, note that only the path
    part of the string is used for the url */
    memcpy(handler_file_context->url, data, data_size);
    handler_file_context->url[data_size] = '\0';

    /* prints a debug message */
    V_INFO_F("%s %s\n", get_http_method_string(http_parser->method), handler_file_context->url);

    /* in case a base path is not defined the contant values
    for the contents and base path must be used */
    if(base_path == NULL) {
        /* creates the file path using the base viriatum path
        this should be the complete absolute path */
        file_path_size = SPRINTF(
            (char *) handler_file_context->file_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s%s%s",
            VIRIATUM_CONTENTS_PATH,
            VIRIATUM_BASE_PATH,
            handler_file_context->url
        );
    }
    /* otherwise the currently set base path is used instead for
    the resolution of the file path */
    else {
        /* creates the file path using the base viriatum path
        this should be the complete absolute path */
        file_path_size = SPRINTF(
            (char *) handler_file_context->file_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s%s",
            base_path,
            handler_file_context->url
        );
    }

    /* decodes the url and file path for the percent encoding, this method
    uses constant (pre-allocated) memory for fast performance the resulting
    value is stored as a simple string in utf-8 encoding */
    decode_percent(
        handler_file_context->url,
        data_size,
        handler_file_context->url_d,
        &decoded_size
    );
    decode_percent(
        handler_file_context->file_path,
        file_path_size,
        handler_file_context->file_path_d,
        &decoded_size
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_file(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* allocates memory for the variable that will hold the
    size of the decoded file path */
    size_t decoded_size;

    /* the complete size of the string that contains the file
    path "calculated" from the url */
    size_t file_path_size;

    /* allocates space for the partial url, resulting from the
    remaining part from the matching of the location */
    unsigned char *partial_url;

    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context =
        (struct handler_file_context_t *) http_parser->context;

    /* retrieves the connection from the parser and then uses it to  the
    the correct file handler reference from the HTTP connection */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct file_handler_t *file_handler =
        (struct file_handler_t *) http_connection->http_handler->lower;

    /* retrieves the current location from the locations buffer
    to be used to update the current context information */
    struct file_location_t *location = &file_handler->locations[index];

    /* updates the various references in the current context, this
    should reflect the one present in the location */
    handler_file_context->base_path = location->base_path;
    handler_file_context->auth_basic = location->auth_basic;
    handler_file_context->auth_file = location->auth_file;

    /* verifies if the location's base path is defined in case it's
    not returns immediately as no file path changing is required */
    if(location->base_path == NULL) { RAISE_NO_ERROR; }

    /* retrieves the partial url match by "removing" the initial part
    of the url in context */
    partial_url = &handler_file_context->url[offset];

    /* creates the file path using the base viriatum path
    this should be the complete absolute path, note that
    only the partial url is used (offset from location) */
    file_path_size = SPRINTF(
        (char *) handler_file_context->file_path,
        VIRIATUM_MAX_PATH_SIZE,
        "%s%s",
        location->base_path,
        partial_url
    );

    /* decodes the file path for the percent encoding, this method
    uses constant (pre-allocated) memory for fast performance */
    decode_percent(
        handler_file_context->file_path,
        file_path_size,
        handler_file_context->file_path_d,
        &decoded_size
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_file(struct http_parser_t *http_parser) {
    /* allocates space for the handler file context */
    struct handler_file_context_t *handler_file_context;

    /* creates the handler file context and then sets the handler
    file context as the context for the HTTP parser */
    create_handler_file_context(&handler_file_context);
    http_parser->context = handler_file_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_file(struct http_parser_t *http_parser) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* deletes the handler file context */
    delete_handler_file_context(handler_file_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reset_http_parser_handler_file(struct http_parser_t *http_parser) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context =\
        (struct handler_file_context_t *) http_parser->context;

    /* unsets the handler file context file */
    handler_file_context->file = NULL;

    /* resets the file size to be processed and the various
    range associated values so that the new file may be
    retrieved without any size related side problem */
    handler_file_context->file_size = 0;
    handler_file_context->initial_byte = 0;
    handler_file_context->final_byte = 0;

    /* unsets the handler file context flags, setting
    the value of them to zerified value */
    handler_file_context->flags = 0;

    /* resets the various flag based variables that are
    going to be used to control the parsing of headers */
    handler_file_context->etag_status = 0;
    handler_file_context->cache_control_status = 0;
    handler_file_context->authorization_status = 0;
    handler_file_context->range_status = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_file(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the HTTP settings
    structure, these callbacks are going to be used in the runtime
    processing of HTTP parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_file;
    http_settings->on_url = url_callback_handler_file;
    http_settings->on_header_field = header_field_callback_handler_file;
    http_settings->on_header_value = header_value_callback_handler_file;
    http_settings->on_headers_complete = headers_complete_callback_handler_file;
    http_settings->on_body = body_callback_handler_file;
    http_settings->on_message_complete = message_complete_callback_handler_file;
    http_settings->on_path = path_callback_handler_file;
    http_settings->on_location = location_callback_handler_file;
    http_settings->on_virtual_url = virtual_url_callback_handler_file;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_file(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the HTTP settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;
    http_settings->on_path = NULL;
    http_settings->on_location = NULL;
    http_settings->on_virtual_url = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _cleanup_handler_file(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* casts the parameters as handler file context and then
    retrieves the flags argument for checking of connection */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) parameters;
    unsigned char flags = handler_file_context->flags;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* in case there is an HTTP handler in the current connection must
    unset it (remove temporary information) */
    if(http_connection->http_handler) {
        /* unsets the current HTTP connection and then sets the reference
        to it in the HTTP connection as unset */
        http_connection->http_handler->unset(http_connection);
        http_connection->http_handler = NULL;
    }

    /* in case the connection is not meant to be kept alive */
    if(!(flags & FLAG_KEEP_ALIVE)) {
        /* closes the connection, no need to continue keeping
        it open as there's no intention to keep it open from
        the client side (active closing) */
        connection->close_connection(connection);
    } else {
        /* releases the lock on the HTTP connection, this will allow further
        messages to be processed, an update event should raised following this
        lock releasing call */
        http_connection->release(http_connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_chunk_handler_file(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* allocates the number of bytes value to be used in
    the read operation (read bytes) */
    size_t number_bytes;

    /* reserves space for the offset, reamingin and buffer
    size values to be used in the calculus of the "optimal"
    buffer size for allocation */
    size_t offset;
    size_t remaining;
    size_t buffer_size;

    /* allocates space for the pointer to the buffer to be
    used in the reading operation from the file */
    unsigned char *file_buffer;

    /* casts the parameters as handler file context and uses it
    to retrieve the proper file path for the sending */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) parameters;
    unsigned char *file_path = handler_file_context->file_path_d;

    /* retrieves the file from the handler file context */
    FILE *file = handler_file_context->file;

    /* in case the file is not defined (should be opened) */
    if(file == NULL) {
        /* opens the file in the most secure manner making sure
        that the proper encoding is set for the path */
        open_file((char *) file_path, "rb", &file);

        /* seeks to the end of the file and then to the
        beginig in order to correctly retrieve the size
        of the current file for reading */
        fseek(file, 0, SEEK_END);
        handler_file_context->file_size = ftell(file);
        fseek(file, handler_file_context->initial_byte, SEEK_SET);

        /* sets the file in the handler file context, this is
        the pointer that will be used for the sending of the
        various chunks of the file */
        handler_file_context->file = file;
    }

    /* retrieves the current offset position in the file reading
    and uses it to calculate the remaining bytes to be read, then
    calculates the "target" buffer size from the minimum value
    between the (maximum) file buffer size and the remaining number
    of bytes to be read from the file (optimal buffer sizing) */
    offset = ftell(file);
    remaining = handler_file_context->final_byte - offset + 1;
    buffer_size = remaining < FILE_BUFFER_SIZE_HANDLER_FILE ?\
        remaining : FILE_BUFFER_SIZE_HANDLER_FILE;
    file_buffer = MALLOC(buffer_size);

    /* reads the file contents, should read either the size
    of a chunk or the size of the complete file in case it's
    shorter than the chunk size */
    number_bytes = fread(file_buffer, 1, buffer_size, file);

    /* in case the number of read bytes is valid, there's
    data to be sent to the client side */
    if(number_bytes > 0) {
        /* writes the complete set of contents in the file
        buffer to the current connection (send operation) */
        write_connection(
            connection,
            file_buffer,
            (unsigned int) number_bytes,
            _send_chunk_handler_file,
            handler_file_context
        );
    }
    /* otherwise the file "transfer" is complete and the control
    flow should proceed to the cleanup operations */
    else {
        /* unsets the file from the handler file context */
        handler_file_context->file = NULL;

        /* runs the cleanup handler file (releases internal structures) */
        _cleanup_handler_file(connection, data, parameters);

        /* closes the file and the releases the currently allocated
        file buffer (avoids memory leaking) */
        fclose(file);
        FREE(file_buffer);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_data_handler_file(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* casts the parameters as handler file context and then
    retrieves the templat handler from it */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) parameters;
    struct template_handler_t *template_handler = handler_file_context->template_handler;

    /* in case the handler file context is already flushed
    time to clenaup pending structures */
    if(handler_file_context->flushed) {
        /* deletes the template handler (releases memory) and
        unsets the reference in the handler file context */
        delete_template_handler(template_handler);
        handler_file_context->template_handler = NULL;

        /* runs the cleanup handler file (releases internal structures) */
        _cleanup_handler_file(connection, data, parameters);
    }
    /* otherwise the "normal" write connection applies */
    else {
        /* writes the (file) data to the connection and sets the handler
        file context as flushed */
        write_connection(
            connection,
            template_handler->string_value,
            (unsigned int) strlen((char *) template_handler->string_value),
            _send_data_handler_file,
            handler_file_context
        );
        handler_file_context->flushed = TRUE;

        /* unsets the string value in the template handler (avoids double release) */
        template_handler->string_value = NULL;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
