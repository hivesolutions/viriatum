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

#include "handler_file.h"

ERROR_CODE create_file_handler(struct file_handler_t **file_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the file handler size */
    size_t file_handler_size = sizeof(struct file_handler_t);

    /* allocates space for the file handler */
    struct file_handler_t *file_handler = (struct file_handler_t *) MALLOC(file_handler_size);

    /* sets the file handler attributes (default) values */
    file_handler->locations = NULL;
    file_handler->locations_count = 0;

    /* sets the file handler in the upper http handler substrate */
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
    handler_file_context->file = NULL;
    handler_file_context->flags = 0;
    handler_file_context->template_handler = NULL;
    handler_file_context->cache_control_status = 0;
    handler_file_context->etag_status = 0;

    /* sets the handler file context in the  pointer */
    *handler_file_context_pointer = handler_file_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_file_context(struct handler_file_context_t *handler_file_context) {
    /* in case there is a file defined in
    the handler file context */
    if(handler_file_context->file != NULL) {
        /* closes the file */
        fclose(handler_file_context->file);
    }

    /* in case there is a template handler defined
    in the handler file context */
    if(handler_file_context->template_handler) {
        /* deletes the template handler (releases memory) */
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

    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates space for the file handler */
    struct file_handler_t *file_handler;

    /* creates the http handler and then uses it to create
    the file handler (lower substrate) */
    service->create_http_handler(service, &http_handler, (unsigned char *) "file");
    create_file_handler(&file_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->resolve_index = 1;
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

        /* tries ro retrieve the script path from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "base_path", &value);
        if(value != NULL) { _location->base_path = (unsigned char *) value; }
    }

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_file(struct service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates space for the file handler */
    struct file_handler_t *file_handler;

    /* retrieves the http handler from the service, then retrieves
    the lower substrate as the file handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "file");
    file_handler = (struct file_handler_t *) http_handler->lower;

    /* deletes the file handler reference */
    delete_file_handler(file_handler);

    /* remove the http handler from the service after
    that deletes the handler reference */
    service->remove_http_handler(service, http_handler);
    service->delete_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_file(struct http_connection_t *http_connection) {
    /* sets the http parser values */
    _set_http_parser_handler_file(http_connection->http_parser);

    /* sets the http settings values */
    _set_http_settings_handler_file(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_file(struct http_connection_t *http_connection) {
    /* unsets the http parser values */
    _unset_http_parser_handler_file(http_connection->http_parser);

    /* unsets the http settings values */
    _unset_http_settings_handler_file(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE reset_handler_file(struct http_connection_t *http_connection) {
    /* resets the http parser values */
    _reset_http_parser_handler_file(http_connection->http_parser);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_file(struct http_parser_t *http_parser) {
    /* retrieves the connection from the http parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* acquires the lock on the http connection, this will avoids further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

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
    SPRINTF(
        (char *) handler_file_context->file_path,
        VIRIATUM_MAX_PATH_SIZE,
        "%s%s%s",
        VIRIATUM_CONTENTS_PATH,
        VIRIATUM_BASE_PATH,
        handler_file_context->url
    );

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* TODO: THIS SHOULD BE A BETTER PARSER STRUCTURE */

    /* checks for the if none match header value */
    switch(data_size) {
        case 13:
            if(data[0] == 'I' && data[1] == 'f' && data[2] == '-' && data[3] == 'N') {
                /* updates the etag status value (is next) */
                handler_file_context->etag_status = 1;

                /* breaks the switch */
                break;
            }

            if(data[0] == 'C' && data[1] == 'a' && data[2] == 'c' && data[3] == 'h') {
                /* updates the cache control status value (is next) */
                handler_file_context->cache_control_status = 1;

                /* breaks the switch */
                break;
            }

            /* breaks the switch */
            break;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* TODO: THIS SHOULD BE A SWITCH */
    if(handler_file_context->etag_status == 1) {
        memcpy(handler_file_context->etag, data, 10);
        handler_file_context->etag[10] = '\0';
        handler_file_context->etag_status = 2;

        RAISE_NO_ERROR;
    }

    if(handler_file_context->cache_control_status == 1) {
        memcpy(handler_file_context->cache_control, data, data_size);
        handler_file_context->cache_control[data_size] = '\0';
        handler_file_context->cache_control_status = 2;

        RAISE_NO_ERROR;
    }

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
    unsigned int is_directory = 0;
    unsigned int is_redirect = 0;

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

    /* allocates the headers buffer (it will be releases automatically by the writter)
    it need to be allocated in the heap so it gets throught the request cycle */
    char *headers_buffer = MALLOC(VIRIATUM_HTTP_SIZE);

    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* retrieves the connection from the http parser parameters */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;

    /* checks if the path being request is in fact a directory */
    is_directory_file((char *) handler_file_context->file_path, &is_directory);

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
            is_redirect = 1;
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
            list_directory_file((char *) handler_file_context->file_path, directory_entries);
            entries_to_map_file(directory_entries, &directory_entries_map);

            /* retrieves the current size of the url and copies into
            the folder path the appropriate part of it, this strategy
            takes into account the size of the url */
            url_size = strlen((char *) handler_file_context->url);
            if(url_size > 2) { memcpy(folder_path, &handler_file_context->url[1], url_size - 2); }
            if(url_size > 2) { folder_path[url_size - 2] = '\0'; }
            else { folder_path[0] = '\0'; }

            /* creates the template handler */
            create_template_handler(&template_handler);

            /* assigns the name of the current folder being listed to
            the template handler (to be set on the template) */
            assign_string_template_handler(template_handler, (unsigned char *) "folder_path", folder_path);

            /* assigns the directory entries to the template handler,
            this variable will be exposed to the template */
            assign_list_template_handler(template_handler, (unsigned char *) "entries", directory_entries_map);
            assign_integer_template_handler(template_handler, (unsigned char *) "items", (int) directory_entries_map->size);

            /* processes the file as a template handler */
            process_template_handler(template_handler, template_path);

            /* sets the template handler in the handler file context and unsets
            the flushed flag */
            handler_file_context->template_handler = template_handler;
            handler_file_context->flushed = 0;

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
        /* counts the total size (in bytes) of the contents in the file path */
        error_code = count_file((char *) handler_file_context->file_path, &file_size);

        /* in case there is no error count the file size, avoids
        extra problems while computing the etag */
        if(!IS_ERROR_CODE(error_code)) {
            /* resets the date time structure to avoid invalid
            date requests */
            memset(&time, 0, sizeof(struct date_time_t));

            /* retrieve the time of the last write in the file path */
            get_write_time_file((char *) handler_file_context->file_path, &time);

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

    /* sets the (http) flags in the handler file context */
    handler_file_context->flags = http_parser->flags;

    /* tests the error code for error */
    if(IS_ERROR_CODE(error_code)) {
        /* prints the error */
        V_DEBUG_F("%s\n", get_last_error_message_safe());

        /* creats the error description string from the error message and then
        sends the error to the connection (with the current format) */
        SPRINTF(
            error_description,
            VIRIATUM_MAX_PATH_SIZE,
            "File not found (%s)",
            handler_file_context->file_path
        );
        write_http_error(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            404,
            "Not Found",
            error_description,
            _cleanup_handler_file,
            handler_file_context
        );
    } else if(is_redirect) {
        /* writes the http static headers to the response */
        count = write_http_headers(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            307,
            "Temporary Redirect",
            KEEP_ALIVE,
            FALSE
        );
        SPRINTF(
            &headers_buffer[count],
            VIRIATUM_HTTP_SIZE - count,
            CONTENT_LENGTH_H ": 0\r\n"
            LOCATION_H ": %s\r\n\r\n",
            location
        );

        /* writes both the headers to the connection, registers for the appropriate callbacks */
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
        /* writes the http static headers to the response */
        write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            200,
            "OK",
            KEEP_ALIVE,
            strlen((char *) handler_file_context->template_handler->string_value),
            NO_CACHE,
            TRUE
        );

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _send_data_handler_file,
            handler_file_context
        );
    }
    else if(handler_file_context->etag_status == 2 && strcmp(etag, (char *) handler_file_context->etag) == 0) {
        /* writes the http static headers to the response */
        write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            304,
            "Not Modified",
            KEEP_ALIVE,
            0,
            NO_CACHE,
            TRUE
        );

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            _cleanup_handler_file,
            handler_file_context
        );
    }
    /* otherwise there was no error in the file and it's a simple
    file situation (no directory) */
    else {
        /* writes the http static headers to the response */
        count = write_http_headers_c(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            200,
            "OK",
            KEEP_ALIVE,
            file_size,
            NO_CACHE,
            FALSE
        );

        /* retrieves the extension part of the file path and then uses
        it to try to retrieve the mime type string for it in case it's
        successfull "puts" the content type in the headers buffer, then
        puts the etag value in the file */
        extension = extension_path((char *) handler_file_context->file_path);
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
            ETAG_H ": %s\r\n\r\n",
            etag
        );

        /* writes both the headers to the connection, registers for the appropriate callbacks */
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
    /* retrieves the handler file context from the http parser
    and uses it to retrieve the reference to the base path in context */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;
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
        SPRINTF(
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
        SPRINTF(
            (char *) handler_file_context->file_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s%s",
            base_path,
            handler_file_context->url
        );
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_file(struct http_parser_t *http_parser, size_t index, size_t offset) {
    /* allocates space for the partial url, resulting from the
    remaining part from the matching of the location */
    unsigned char *partial_url;

    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context =
        (struct handler_file_context_t *) http_parser->context;

    /* retrieves the connection from the parser and then uses it to  the
    the correct file handler reference from the http connection */
    struct connection_t *connection = (struct connection_t *) http_parser->parameters;
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;
    struct file_handler_t *file_handler =
        (struct file_handler_t *) http_connection->http_handler->lower;

    /* retrieves the current location from the location buffer and checks if the
    file path value is correctly set */
    struct file_location_t *location = &file_handler->locations[index];
    if(location->base_path == NULL) { RAISE_NO_ERROR; }

    /* updates the base path reference in the current context, this
    should reflect the one present in the location */
    handler_file_context->base_path = location->base_path;

    /* retrieves the partial url match by "removing" the initial part
    of the url in context */
    partial_url = &handler_file_context->url[offset];

    /* creates the file path using the base viriatum path
    this should be the complete absolute path, note that
    only the partial url is used (offset from location) */
    SPRINTF(
        (char *) handler_file_context->file_path,
        VIRIATUM_MAX_PATH_SIZE,
        "%s%s",
        location->base_path,
        partial_url
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

    /* creates the handler file context */
    create_handler_file_context(&handler_file_context);

    /* sets the handler file context as the context for the http parser */
    http_parser->context = handler_file_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_file(struct http_parser_t *http_parser) {
    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* deletes the handler file context */
    delete_handler_file_context(handler_file_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reset_http_parser_handler_file(struct http_parser_t *http_parser) {
    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* unsets the handler file context file */
    handler_file_context->file = NULL;

    /* unsets the handler file context flags */
    handler_file_context->flags = 0;

    /* resets both the etag and the cache control status */
    handler_file_context->etag_status = 0;
    handler_file_context->cache_control_status = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_file(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
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
    /* unsets the various callback functions from the http settings */
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
    } else {
        /* releases the lock on the http connection, this will allow further
        messages to be processed, an update event should raised following this
        lock releasing call */
        http_connection->release(http_connection);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_chunk_handler_file(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* allocates the number of bytes */
    size_t number_bytes;

    /* casts the parameters as handler file context */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) parameters;

    /* retrieves the file path from the handler file context */
    unsigned char *file_path = handler_file_context->file_path;

    /* retrieves the file from the handler file context */
    FILE *file = handler_file_context->file;

    /* allocates the required buffer for the file */
    unsigned char *file_buffer = MALLOC(FILE_BUFFER_SIZE_HANDLER_FILE);

    /* in case the file is not defined (should be opened) */
    if(file == NULL) {
        /* opens the file */
        FOPEN(&file, (char *) file_path, "rb");

        /* in case the file is not found, must raise an error
        indicating that there was a problem loading the file */
        if(file == NULL) {
            RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
        }

        /* sets the file in the handler file context */
        handler_file_context->file = file;
    }

    /* reads the file contents */
    number_bytes = fread(file_buffer, 1, FILE_BUFFER_SIZE_HANDLER_FILE, file);

    /* in case the number of read bytes is valid */
    if(number_bytes > 0) {
        /* writes both the file buffer to the connection */
        write_connection(
            connection,
            file_buffer,
            (unsigned int) number_bytes,
            _send_chunk_handler_file,
            handler_file_context
        );
    }
    /* otherwise the file "transfer" is complete */
    else {
        /* unsets the file from the handler file context */
        handler_file_context->file = NULL;

        /* runs the cleanup handler file (releases internal structures) */
        _cleanup_handler_file(connection, data, parameters);

        /* closes the file */
        fclose(file);

        /* releases the current file buffer */
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
        handler_file_context->flushed = 1;

        /* unsets the string value in the template handler (avoids double release) */
        template_handler->string_value = NULL;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
