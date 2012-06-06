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

#include "handler_file.h"

ERROR_CODE create_handler_file_context(struct handler_file_context_t **handler_file_context_pointer) {
    /* retrieves the handler file context size */
    size_t handler_file_context_size = sizeof(struct handler_file_context_t);

    /* allocates space for the handler file context */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) MALLOC(handler_file_context_size);

    /* sets the handler file default values */
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

ERROR_CODE register_handler_file(struct Service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* creates the http handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "file");

    /* sets the http handler attributes */
    http_handler->set = set_handler_file;
    http_handler->unset = unset_handler_file;
    http_handler->reset = reset_handler_file;

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unregister_handler_file(struct Service_t *service) {
    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* retrieves the http handler from the service, then
    remove it from the service after that delete the handler */
    service->get_http_handler(service, &http_handler, (unsigned char *) "file");
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
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* allocates the required space for the url, this
    is done through static allocation */
    unsigned char url[VIRIATUM_MAX_URL_SIZE];

    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* retrieves the connection from the http parser parameters and then
    uses it to access the service options using the service */
    struct Connection_t *connection = (struct Connection_t *) http_parser->parameters;
    struct Service_t *service = connection->service;
    struct ServiceOptions_t *options = service->options;

    /* copies the memory from the data to the url and then
    puts the end of string in the url */
    memcpy(url, data, data_size);
    url[data_size] = '\0';

    /* prints a debug message */
    V_INFO_F("%s %s\n", get_http_method_string(http_parser->method), url);

    /* in case the string refers the base path (default handler must be used)
    the selection of the index file as defautl is conditioned by the default
    index configuration option */
    if(options->default_index && (strcmp((char *) url, "/") == 0 || strcmp((char *) url, "") == 0)) {
        /* copies the index reference as the url */
        memcpy(url, "/index.html", 12);
    }

    /* copies the url to the url reference in the handler file context */
    memcpy(handler_file_context->url, url, data_size + 1);

    /* creates the file path from using the base viriatum path */
    SPRINTF((char *) handler_file_context->file_path, VIRIATUM_MAX_PATH_SIZE, "%s%s%s", VIRIATUM_CONTENTS_PATH, VIRIATUM_BASE_PATH, url);

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
    /* allocates the file size */
    size_t file_size;

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
    unsigned char location[1024];
    unsigned char template_path[1024];

    /* allocates space for the computation of the time
    and of the time string, then allocates space for the
    etag calculation structure (crc32 value) and for the etag*/
    struct date_time_t time;
    char time_string[20];
    unsigned long crc32_value;
    char etag[11];

    /* allocates the space for the "read" result
    error code (valid by default) */
    ERROR_CODE error_code = 0;

    /* allocates the headers buffer (it will be releases automatically by the writter)
    it need to be allocated in the heap so it gets throught the request cycle */
    char *headers_buffer = MALLOC(1024);

    /* retrieves the handler file context from the http parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_parser->context;

    /* retrieves the connection from the http parser parameters */
    struct Connection_t *connection = (struct Connection_t *) http_parser->parameters;

    /* checks if the path being request is in fact a directory */
    is_directory_file((char *) handler_file_context->file_path, &is_directory);

    /* in case the file path being request referes a directory
    it must be checked and the entries retrieved to be rendered */
    if(is_directory) {
        if(handler_file_context->url[strlen((char *) handler_file_context->url) - 1] != '/') {
            /* creates the new location by adding the slash character to the current
            handler file context url (avoids directory confusion) */
            memcpy(location, handler_file_context->url, strlen((char *) handler_file_context->url));
            location[strlen((char *) handler_file_context->url)] = '/';
            location[strlen((char *) handler_file_context->url) + 1] = '\0';

            /* sets the is redirect flag (forces temporary redirect) */
            is_redirect = 1;
        } else {
            /* creates the complete path to the template file */
            SPRINTF((char *) template_path, 1024, "%s%s", VIRIATUM_RESOURCES_PATH, VIRIATUM_LISTING_PATH);

            /* pritns a debug message */
            V_DEBUG_F("Processing template file '%s'\n", template_path);

            /* creates the directory entries (linked list) */
            create_linked_list(&directory_entries);

            /* lists the directory file into the directory
            entries linked list and then converts them into maps */
            list_directory_file((char *) handler_file_context->file_path, directory_entries);
            entries_to_map_file(directory_entries, &directory_entries_map);

            /* creates the template handler */
            create_template_handler(&template_handler);

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
            SPRINTF(time_string, 20, "%04d-%02d-%02d %02d:%02d:%02d", time.year, time.month, time.day, time.hour, time.minute, time.second);

            /* creates the crc32 value and prints it into the
            etag as an heexadecimal string value */
            crc32_value = crc32((unsigned char *) time_string, 19);
            SPRINTF(etag, 11, "\"%08x\"", (unsigned int) crc32_value);
        }
    }

    /* sets the (http) flags in the handler file context */
    handler_file_context->flags = http_parser->flags;

    /* tests the error code for error */
    if(IS_ERROR_CODE(error_code)) {
        /* prints the error */
        V_DEBUG_F("%s\n", get_last_error_message_safe());

        /* writes the http static headers to the response */
        SPRINTF(headers_buffer, 1024, "HTTP/1.1 404 Not Found\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: %lu\r\n\r\n404 - Not Found (%s)", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) strlen((char *) handler_file_context->file_path) + 18, handler_file_context->file_path);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) strlen(headers_buffer), _cleanup_handler_file, handler_file_context);
    } else if(is_redirect) {
        /* writes the http static headers to the response */
        SPRINTF(headers_buffer, 1024, "HTTP/1.1 307 Temporary Redirect\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nContent-Length: 0\r\nLocation: %s\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, location);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) strlen(headers_buffer), _cleanup_handler_file, handler_file_context);
    }
    /* in case the current situation is a directory list */
    else if(is_directory) {
        /* writes the http static headers to the response */
        SPRINTF(headers_buffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, (long unsigned int) strlen((char *) handler_file_context->template_handler->string_value));

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) strlen(headers_buffer), _send_data_handler_file, handler_file_context);
    }
    else if(handler_file_context->etag_status == 2 && strcmp(etag, (char *) handler_file_context->etag) == 0) {
        /* writes the http static headers to the response */
        SPRINTF(headers_buffer, 1024, "HTTP/1.1 304 Not Modified\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nContent-Length: 0\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) strlen(headers_buffer), _cleanup_handler_file, handler_file_context);
    }
    /* otherwise there was no error in the file and it's a simple
    file situation (no directory) */
    else {
        /* writes the http static headers to the response */
        SPRINTF(headers_buffer, 1024, "HTTP/1.1 200 OK\r\nServer: %s/%s (%s - %s)\r\nConnection: Keep-Alive\r\nCache-Control: no-cache, must-revalidate\r\nETag: %s\r\nContent-Length: %lu\r\n\r\n", VIRIATUM_NAME, VIRIATUM_VERSION, VIRIATUM_PLATFORM_STRING, VIRIATUM_PLATFORM_CPU, etag, (long unsigned int) file_size);

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(connection, (unsigned char *) headers_buffer, strlen(headers_buffer), _send_chunk_handler_file, handler_file_context);
    }

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

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _cleanup_handler_file(struct Connection_t *connection, struct Data_t *data, void *parameters) {
    /* casts the parameters as handler file context and then
    retrieves the flags argument for checking of connection */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) parameters;
    unsigned char flags = handler_file_context->flags;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for unregister */
    struct IoConnection_t *io_connection = (struct IoConnection_t *) connection->lower;
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

ERROR_CODE _send_chunk_handler_file(struct Connection_t *connection, struct Data_t *data, void *parameters) {
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

        /* in case the file is not found */
        if(file == NULL) {
            /* raises an error */
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
        write_connection(connection, file_buffer, number_bytes, _send_chunk_handler_file, handler_file_context);
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

ERROR_CODE _send_data_handler_file(struct Connection_t *connection, struct Data_t *data, void *parameters) {
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
        write_connection(connection, template_handler->string_value, (unsigned int) strlen((char *) template_handler->string_value), _send_data_handler_file, handler_file_context);
        handler_file_context->flushed = 1;

        /* unsets the string value in the template handler (avoids double release) */
        template_handler->string_value = NULL;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
