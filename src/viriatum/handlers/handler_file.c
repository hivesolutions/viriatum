/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "handler_file.h"

/* the files that the handler is keeping open, one set per process as
the workers are forked and each of them serves on its own, so that
nothing here is ever reached by two of them at the same time and no
locking of any kind is called for around it */
static struct file_cache_t *_file_cache = NULL;

static struct file_cache_t *_get_file_cache(void) {
    /* creates the cache the first time that one is asked for, which
    happens inside the worker that is serving and never before the
    forking of it, so that each of them ends up with one of its own
    and none of them is ever left without one to reach for */
    if(_file_cache == NULL) { create_file_cache(&_file_cache); }
    return _file_cache;
}

static void _time_file_cache(STAT_TYPE *file_stat, struct date_time_t *date_time) {
    /* allocates space for the structure that carries the parts of
    the moment and for the moment itself as the system reports it */
    struct tm time;
    time_t written = (time_t) file_stat->st_mtime;

    /* breaks the moment of the last write into its parts, the very
    same way the commons does it for the time of a file, so that the
    tag that travels with a response does not change over this */
    GM_TIME(&time, &written);

    /* populates the date time structure with the information
    on the file various parts */
    date_time->year = time.tm_year + 1900;
    date_time->month = time.tm_mon + 1;
    date_time->day = time.tm_mday;
    date_time->hour = time.tm_hour;
    date_time->minute = time.tm_min;
    date_time->second = time.tm_sec;
}

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
    handler_file_context->push = NULL;
    handler_file_context->descriptor = -1;
    handler_file_context->offset = 0;
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
    /* in case there is a file being read through in the handler file
    context closes it, the cache holds one of its own and this is
    only ever the duplicate that this request was handed */
    if(handler_file_context->descriptor != -1) {
        CLOSE_READ(handler_file_context->descriptor);
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
    reference structures */
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
    memset(file_handler->locations, 0, service->locations.count * sizeof(struct file_location_t));

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
        the location buffer, this is going to be populated and sets the
        default values in it */
        _location = &file_handler->locations[index];
        _location->base_path = NULL;
        _location->auth_basic = NULL;
        _location->auth_file = NULL;
        _location->push = NULL;

        /* tries to retrieve the base path from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "base_path", &value);
        if(value != NULL) { _location->base_path = (unsigned char *) value; }

        /* tries to retrieve the auth basic from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "auth_basic", &value);
        if(value != NULL) { _location->auth_basic = (unsigned char *) value; }

        /* tries to retrieve the auth file from the file configuration and in
        case it exists sets it in the location (attribute reference change) */
        get_value_string_sort_map(configuration, (unsigned char *) "auth_file", &value);
        if(value != NULL) { _location->auth_file = (unsigned char *) value; }

        /* tries to retrieve the resources to be promised from the file
        configuration and in case they exist sets them in the location */
        get_value_string_sort_map(configuration, (unsigned char *) "push", &value);
        if(value != NULL) { _location->push = (unsigned char *) value; }
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

    /* closes every file that the cache was keeping open and releases
    it, a descriptor that is never closed is a descriptor leaked */
    if(_file_cache != NULL) {
        delete_file_cache(_file_cache);
        _file_cache = NULL;
    }

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
    _set_http_request_handler_file(http_connection->request);
    _set_http_settings_handler_file(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_file(struct http_connection_t *http_connection) {
    /* unsets both the HTTP parser values and the HTTP
    settings handler from the current file handler */
    _unset_http_request_handler_file(http_connection->request);
    _unset_http_settings_handler_file(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE reset_handler_file(struct http_connection_t *http_connection) {
    /* resets the HTTP parser values */
    _reset_http_request_handler_file(http_connection->request);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_file(struct http_request_t *http_request) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* allocates memory for the variable that will hold the
    size of the decoded file path */
    size_t decoded_size;

    /* the complete size of the string that contains the file
    path "calculated" from the url */
    size_t file_path_size;

    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context =
        (struct handler_file_context_t *) http_request->context;

    /* retrieves the connection from the HTTP parser parameters and
    uses the pre-resolved contents path from service options */
    struct connection_t *connection = (struct connection_t *) http_request->parameters;
    char *contents_path = (char *) connection->service->options->contents_path;

    /* checks the position of the get parameters divisor position
    and then uses it to calculate the size of the (base) path */
    char *pointer = (char *) memchr((char *) data, '?', data_size);
    size_t path_size = pointer == NULL ? data_size : pointer - (char *) data;

    /* copies the memory from the data to the url and then
    puts the end of string in the url, note that only the path
    part of the string is used for the url */
    memcpy(handler_file_context->url, data, path_size);
    handler_file_context->url[path_size] = '\0';

    /* prints the line that describes the request, the writing of it
    is a call into the kernel that every request pays for, so it is
    only ever written when the service has been asked for it */
    if(connection->service->options->access_log) {
        V_INFO_F("%s %s\n", get_http_method_string(http_request->method), handler_file_context->url);
    }

    /* creates the file path using the base viriatum path
    this should be the complete absolute path */
    file_path_size = SPRINTF(
        (char *) handler_file_context->file_path,
        VIRIATUM_MAX_PATH_SIZE,
        "%s%s%s",
        contents_path,
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

    /* validates the decoded url for path traversal attempts,
    rejects requests containing ".." sequences that could escape
    the web root directory */
    if(!is_path_safe(handler_file_context->url_d)) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Path traversal detected in URL"
        );
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_request->context;

    /* allocates space for the first bytes of the name once they have
    been lowered, the name of a field is never case sensitive and the
    most recent version of the protocol carries them in lower case */
    unsigned char lowered[4];
    size_t index;

    /* lowers the bytes that the matching below looks at, a name that
    is shorter than them is none of the ones being looked for */
    if(data_size < sizeof(lowered)) { RAISE_NO_ERROR; }
    for(index = 0; index < sizeof(lowered); index++) {
        lowered[index] = (unsigned char) tolower((unsigned char) data[index]);
    }

    /* switches over the size of the header name (field)
    that was provided (used for faster parsing) */
    switch(data_size) {
        case 5:
            if(lowered[0] == 'r' && lowered[1] == 'a' && lowered[2] == 'n' && lowered[3] == 'g') {
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
            if(lowered[0] == 'i' && lowered[1] == 'f' && lowered[2] == '-' && lowered[3] == 'n') {
                /* updates the etag status value, it's the next
                value to be parsed and put in context */
                handler_file_context->etag_status = 1;
                handler_file_context->next_header = ETAG;
                break;
            }

            if(lowered[0] == 'c' && lowered[1] == 'a' && lowered[2] == 'c' && lowered[3] == 'h') {
                /* updates the cache control status value, it's the next
                value to be parsed and put in context */
                handler_file_context->cache_control_status = 1;
                handler_file_context->next_header = CACHE_CONTROL;
                break;
            }

            if(lowered[0] == 'a' && lowered[1] == 'u' && lowered[2] == 't' && lowered[3] == 'h') {
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

ERROR_CODE header_value_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_request->context;

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

ERROR_CODE headers_complete_callback_handler_file(struct http_request_t *http_request) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

/**
 * Writes the status and the fields that every response of the file
 * handler carries, through the operations of the connection so that
 * the encoding of them follows the protocol in use.
 *
 * @param http_connection The connection the response belongs to.
 * @param connection The connection the response is written on.
 * @param buffer The buffer the response is built in.
 * @param size The size in bytes of the provided buffer.
 * @param version The version of the protocol in use.
 * @param status_code The status code of the response.
 * @param status_message The message that describes the status.
 * @param keep_alive The mode the connection is left in.
 * @param content_length The size in bytes of the payload.
 * @return The number of bytes the buffer holds.
 */
static size_t _write_headers_handler_file(
    struct http_connection_t *http_connection,
    struct connection_t *connection,
    char *buffer,
    size_t size,
    enum http_version_e version,
    int status_code,
    char *status_message,
    enum http_keep_alive_e keep_alive,
    size_t content_length
) {
    /* allocates space for the position in the buffer and for the
    text of the length of the payload */
    size_t count;
    char length[32];

    count = http_connection->write_status(
        connection,
        buffer,
        size,
        version,
        status_code,
        status_message,
        keep_alive
    );
    SPRINTF(length, sizeof(length), "%lu", (long unsigned int) content_length);
    count = http_connection->write_field(connection, buffer, size, count, CONTENT_LENGTH_H, length);
    count = http_connection->write_field(
        connection,
        buffer,
        size,
        count,
        CACHE_CONTROL_H,
        cache_codes[NO_CACHE - 1]
    );

    /* the permissive cross origin fields travel on the response only
    when they have been asked for, a page served from another origin
    being allowed to read what is answered here */
    if(connection->service->options->cors) {
        count = http_connection->write_field(
            connection,
            buffer,
            size,
            count,
            ACCESS_CONTROL_ORIGIN_H,
            "*"
        );
        count = http_connection->write_field(
            connection,
            buffer,
            size,
            count,
            ACCESS_CONTROL_METHODS_H,
            "GET, HEAD, POST, PUT, DELETE, OPTIONS"
        );
        count = http_connection->write_field(
            connection,
            buffer,
            size,
            count,
            ACCESS_CONTROL_HEADERS_H,
            "*"
        );
    }

    /* returns the number of bytes that the buffer holds */
    return count;
}

/**
 * Promises the resources that the location of the request lists to
 * the peer, each one of them opening a stream of its own that is
 * served as though the peer had asked for it.
 * Nothing at all happens under HTTP/1.1, which carries no way of
 * promising a resource.
 *
 * @param http_connection The connection the request belongs to.
 * @param http_request The message being served.
 * @param push The list of the paths to be promised.
 */
static void _push_handler_file(struct http_connection_t *http_connection, struct http_request_t *http_request, unsigned char *push) {
#ifdef VIRIATUM_HTTP2
    /* allocates space for the path being taken out of the list and
    for the walk over the list itself */
    char path[VIRIATUM_MAX_URL_SIZE];
    size_t size = 0;
    unsigned char *pointer = push;
    struct http2_stream_t *http2_stream;

    /* the promising of a resource only exists under HTTP/2, so under
    anything else there's nothing at all to be done */
    if(http_connection->http2_connection == NULL) { return; }
    if(push == NULL) { return; }

    /* retrieves the stream of the request, it is the one the promises
    are made on and the one they are going to hang from */
    http2_stream = find_stream_http2_connection(
        http_connection->http2_connection,
        http_request->stream_id
    );
    if(http2_stream == NULL) { return; }

    /* walks the list, every sequence of characters that is not a space
    is one of the paths that gets promised */
    while(TRUE) {
        if(*pointer != ' ' && *pointer != '\0') {
            if(size < sizeof(path) - 1) { path[size++] = (char) *pointer; }
            pointer++;
            continue;
        }

        if(size > 0) {
            path[size] = '\0';
            push_stream_http2_connection(http_connection->http2_connection, http2_stream, path);
            size = 0;

            /* the promising of a resource may have moved the streams
            inside the table, so the one of the request is taken again */
            http2_stream = find_stream_http2_connection(
                http_connection->http2_connection,
                http_request->stream_id
            );
            if(http2_stream == NULL) { return; }
        }

        if(*pointer == '\0') { break; }
        pointer++;
    }
#endif
}

ERROR_CODE message_complete_callback_handler_file(struct http_request_t *http_request) {
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

    /* allocates space for the flag that marks a listing that has been
    asked for while the producing of one is turned off */
    unsigned int is_forbidden = FALSE;

    /* allocates space for the new location value for
    redirect request cases and for the path to the
    template (for directory listing) */
    unsigned char location[VIRIATUM_MAX_PATH_SIZE];
    unsigned char template_path[VIRIATUM_MAX_PATH_SIZE];

    /* allocates space for the name of the index file that answers for a
    directory and for the complete path that is built out of it */
    char *index_;
    unsigned char index_path[VIRIATUM_MAX_PATH_SIZE];

    /* allocates space for the computation of the time
    and of the time string, then allocates space for the
    etag calculation structure (crc32 value), for the etag
    value and for the (file) extension and mime type */
    struct date_time_t time;

    /* allocates space for the entry of the cache that describes the
    file, both its size and the time of the last write to it */
    struct file_cache_entry_t *file_cache_entry;
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
    struct handler_file_context_t *handler_file_context =
        (struct handler_file_context_t *) http_request->context;
    unsigned char flags = http_request->flags;

    /* retrieves the connection from the HTTP parser parameters,
    the connection object is going to be used for the input and
    outpu operations associated with the file handling */
    struct connection_t *connection = (struct connection_t *) http_request->parameters;

    /* retrieves the underlying connection references in order to be
    able to operate over them, for register */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* allocates space for the value of the range of the contents, it
    is written as a field of its own */
    char range_value[64];

    /* verifies if the currently set flag grant "permission" to keep
    the connection alive at the end of the HTTP message processing,
    this values is going to be used for headers generation */
    keep_alive = flags & FLAG_KEEP_ALIVE;

    /* acquires the lock on the HTTP connection, this will avoids further
    messages to be processed, no parallel request handling problems */
    http_connection->acquire(http_connection);

    /* promises the resources that the location of the request lists,
    this happens before the response so that the peer learns of them
    before it has a chance to ask for them itself */
    _push_handler_file(http_connection, http_request, handler_file_context->push);

    /* checks if the path being request is in fact a directory */
    is_directory_file((char *) handler_file_context->file_path_d, &is_directory);

    /* a request for a directory that already carries the trailing slash is
    served with the index file of it whenever one of the named ones is
    around, which is what a request for a directory means to every server
    of this class, the listing being only what answers for a directory
    that carries none of them */
    url_size = strlen((char *) handler_file_context->url);
    if(is_directory && url_size > 0 && handler_file_context->url[url_size - 1] == '/') {
        index_ = validate_file(
            (char *) handler_file_context->file_path_d,
            (char *) connection->service->options->index,
            connection->service->options->index_count,
            128
        );

        /* the index file takes the place of the directory in the path that
        is going to be served, from this point on it is a normal file and
        neither the listing nor the status that stands in for it applies */
        if(index_ != NULL) {
            SPRINTF(
                (char *) index_path,
                VIRIATUM_MAX_PATH_SIZE,
                "%s" VIRIATUM_PATH_SEPARATOR "%s",
                (char *) handler_file_context->file_path_d,
                index_
            );
            SPRINTF(
                (char *) handler_file_context->file_path_d,
                VIRIATUM_MAX_PATH_SIZE,
                "%s",
                (char *) index_path
            );
            is_directory = FALSE;
        }
    }

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
        } else {
            auth_result = FALSE;
        }
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
        /* otherwise the listing of the directory has been turned off and
        so the contents of it may not be revealed to the user agent */
        else if(!connection->service->options->listing) {
            is_forbidden = TRUE;
        }
        /* otherwise it's the correct directory location and must present the
        listing of the directory to the user agent */
        else {
            /* creates the complete path to the template file using
            the pre-resolved resources path from service options */
            char *resources_path = (char *) connection->service->options->resources_path;
            SPRINTF(
                (char *) template_path,
                sizeof(template_path),
                "%s%s",
                resources_path,
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
            if(url_size > 2) {
                folder_path[url_size - 2] = '\0';
            } else {
                folder_path[0] = '\0';
            }

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

            /* warns if the template file was not found or produced no output
            this will allow proper debugging of the situation */
            if(template_handler->string_value == NULL || template_handler->string_value[0] == '\0') {
                V_WARNING_F("Listing template file not found or empty '%s'\n", template_path);
            } else {
                V_DEBUG_F(
                    "Sending listing template contents from '%s' (%lu bytes)\n",
                    template_path,
                    (unsigned long) strlen((char *) template_handler->string_value)
                );
            }

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
    /* otherwise the file path must refer to a "normal" file path and
    it must be checked */
    else {
        /* counts the total size (in bytes) of the contents
        in the file path, this also the call used for checking
        the existence of the file */
        error_code = acquire_file_cache(
            _get_file_cache(),
            handler_file_context->file_path_d,
            &file_cache_entry
        );

        /* a path that resolves to nothing is served with the index file
        instead of an error whenever the routing of a single page
        application has been asked for, the application itself being
        what decides what the path it was given means */
        if(IS_ERROR_CODE(error_code) && connection->service->options->spa) {
            RESET_ERROR;
            SPRINTF(
                (char *) handler_file_context->file_path_d,
                VIRIATUM_MAX_PATH_SIZE,
                "%s" VIRIATUM_PATH_SEPARATOR "%s",
                (char *) connection->service->options->contents_path,
                (char *) connection->service->options->index[0]
            );
            error_code = acquire_file_cache(
                _get_file_cache(),
                handler_file_context->file_path_d,
                &file_cache_entry
            );
        }

        /* in case there is no error count the file size, avoids
        extra problems while computing the etag */
        if(!IS_ERROR_CODE(error_code)) {
            /* both the size and the time of the last write come out
            of the entry that has just been acquired, which learnt
            them from the very descriptor it is holding open, so that
            neither of them costs a look at the file system of its own */
            file_size = file_cache_entry->size;
            time = file_cache_entry->time;

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
    handler_file_context->flags = http_request->flags;

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
    } else if(is_forbidden) {
        /* prints some debug information about the listing that was
        asked for while the producing of one is turned off */
        V_DEBUG("Listing of the directory is not allowed\n");

        /* sends the message containing the error definition for the
        listing that may not be produced, the contents of a directory
        are never revealed once the listing of it is turned off */
        write_http_error(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            HTTP11,
            403,
            "Forbidden",
            "Directory listing is not allowed",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            _cleanup_handler_file,
            handler_file_context
        );
    } else if(is_redirect) {
        /* writes the HTTP static headers to the response */
        count = http_connection->write_status(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            http_request->version,
            307,
            "Temporary Redirect",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE
        );
        count = http_connection->write_field(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            CONTENT_LENGTH_H,
            "0"
        );
        count = http_connection->write_field(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            LOCATION_H,
            (const char *) location
        );
        count = http_connection->write_end(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            TRUE
        );

        /* writes both the headers to the connection, registers
        for the appropriate callbacks */
        http_connection->write_flush(
            connection,
            (unsigned char *) headers_buffer,
            count,
            _cleanup_handler_file,
            handler_file_context
        );
    }
    /* in case the current situation is a directory list */
    else if(is_directory) {
        /* writes the HTTP static headers to the response */
        count = _write_headers_handler_file(
            http_connection,
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            http_request->version,
            200,
            "OK",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            strlen((char *) handler_file_context->template_handler->string_value)
        );
        count = http_connection->write_end(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            FALSE
        );

        /* writes both the headers to the connection, register
        for the appropriate callbacks */
        http_connection->write_flush(
            connection,
            (unsigned char *) headers_buffer,
            count,
            _send_data_handler_file,
            handler_file_context
        );
    }
    /* in case there's an etag value defined and the values matched
    the one defined for the file, time to return a not modified value
    to the client indicating that cache should be used */
    else if(handler_file_context->etag_status == 2 && strcmp(etag, (char *) handler_file_context->etag) == 0) {
        /* writes the HTTP static headers to the response */
        count = _write_headers_handler_file(
            http_connection,
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            http_request->version,
            304,
            "Not Modified",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            0
        );
        count = http_connection->write_end(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            TRUE
        );

        /* writes both the headers to the connection, registers for
        the appropriate callbacks */
        http_connection->write_flush(
            connection,
            (unsigned char *) headers_buffer,
            count,
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
        count = _write_headers_handler_file(
            http_connection,
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            http_request->version,
            206,
            "Partial content",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            handler_file_context->final_byte -
                handler_file_context->initial_byte + 1
        );
        SPRINTF(
            range_value,
            sizeof(range_value),
            "bytes %ld-%ld/%ld",
            (long int) handler_file_context->initial_byte,
            (long int) handler_file_context->final_byte,
            (long int) file_size
        );
        count = http_connection->write_field(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            CONTENT_RANGE_H,
            range_value
        );
        count = http_connection->write_end(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            FALSE
        );

        /* writes both the headers to the connection, registers for the
        appropriate callbacks, this is going to trigger the chain of write
        to callback behavior expected for the file sending */
        http_connection->write_flush(
            connection,
            (unsigned char *) headers_buffer,
            count,
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
        count = _write_headers_handler_file(
            http_connection,
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            http_request->version,
            200,
            "OK",
            keep_alive ? KEEP_ALIVE : KEEP_CLOSE,
            file_size
        );

        /* retrieves the extension part of the file path and then uses
        it to try to retrieve the mime type string for it in case it's
        successfull "puts" the content type in the headers buffer, then
        puts the etag value in the file */
        extension = extension_path((char *) handler_file_context->file_path_d);
        mime_type = connection->service->get_mime_type(connection->service, extension);
        if(mime_type != NULL) {
            count = http_connection->write_field(
                connection,
                headers_buffer,
                VIRIATUM_HTTP_SIZE,
                count,
                CONTENT_TYPE_H,
                mime_type
            );
        }
        count = http_connection->write_field(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            ACCEPT_RANGES_H,
            "bytes"
        );
        count = http_connection->write_field(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            ETAG_H,
            etag
        );
        count = http_connection->write_end(
            connection,
            headers_buffer,
            VIRIATUM_HTTP_SIZE,
            count,
            FALSE
        );

        /* writes both the headers to the connection, registers for the
        appropriate callbacks, this is going to trigger the chain of write
        to callback behavior expected for the file sending */
        http_connection->write_flush(
            connection,
            (unsigned char *) headers_buffer,
            count,
            _send_chunk_handler_file,
            handler_file_context
        );
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE path_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* allocates memory for the variable that will hold the
    size of the decoded file path */
    size_t decoded_size;

    /* the complete size of the string that contains the file
    path "calculated" from the url */
    size_t file_path_size;

    /* retrieves the handler file context from the HTTP parser
    and uses it to retrieve the reference to the base path in context */
    struct handler_file_context_t *handler_file_context =
        (struct handler_file_context_t *) http_request->context;
    unsigned char *base_path = handler_file_context->base_path;

    /* retrieves the connection from the HTTP parser parameters and
    uses the pre-resolved contents path from service options */
    struct connection_t *connection = (struct connection_t *) http_request->parameters;
    char *contents_path = (char *) connection->service->options->contents_path;

    /* copies the memory from the data to the url and then
    puts the end of string in the url, note that only the path
    part of the string is used for the url */
    memcpy(handler_file_context->url, data, data_size);
    handler_file_context->url[data_size] = '\0';

    /* prints the line that describes the request, the writing of it
    is a call into the kernel that every request pays for, so it is
    only ever written when the service has been asked for it */
    if(connection->service->options->access_log) {
        V_INFO_F("%s %s\n", get_http_method_string(http_request->method), handler_file_context->url);
    }

    /* in case a base path is not defined the contant values
    for the contents and base path must be used */
    if(base_path == NULL) {
        /* creates the file path using the base viriatum path
        this should be the complete absolute path */
        file_path_size = SPRINTF(
            (char *) handler_file_context->file_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s%s%s",
            contents_path,
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

    /* validates the decoded url for path traversal attempts,
    rejects requests containing ".." sequences that could escape
    the web root directory */
    if(!is_path_safe(handler_file_context->url_d)) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Path traversal detected in URL"
        );
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE location_callback_handler_file(struct http_request_t *http_request, size_t index, size_t offset) {
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
        (struct handler_file_context_t *) http_request->context;

    /* retrieves the connection from the parser and then uses it to  the
    the correct file handler reference from the HTTP connection */
    struct connection_t *connection = (struct connection_t *) http_request->parameters;
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
    handler_file_context->push = location->push;
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

    /* validates the decoded file path for path traversal attempts,
    rejects requests containing ".." sequences that could escape
    the web root directory */
    if(!is_path_safe(handler_file_context->file_path_d)) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Path traversal detected in URL"
        );
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE virtual_url_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_request_handler_file(struct http_request_t *http_request) {
    /* allocates space for the handler file context */
    struct handler_file_context_t *handler_file_context;

    /* creates the handler file context and then sets the handler
    file context as the context for the HTTP parser */
    create_handler_file_context(&handler_file_context);
    http_request->context = handler_file_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_request_handler_file(struct http_request_t *http_request) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) http_request->context;

    /* deletes the handler file context and nullifies the
    reference to avoid dangling pointer on keep-alive reuse */
    delete_handler_file_context(handler_file_context);
    http_request->context = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reset_http_request_handler_file(struct http_request_t *http_request) {
    /* retrieves the handler file context from the HTTP parser */
    struct handler_file_context_t *handler_file_context =
        (struct handler_file_context_t *) http_request->context;

    /* closes whatever the request before this one was reading through
    and had not finished with, a transfer that was cut short would
    otherwise leave its descriptor behind for the whole connection */
    if(handler_file_context->descriptor != -1) {
        CLOSE_READ(handler_file_context->descriptor);
        handler_file_context->descriptor = -1;
    }

    /* resets the offset the reading had reached and the various
    range associated values so that the new file may be
    retrieved without any size related side problem */
    handler_file_context->offset = 0;
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

ERROR_CODE create_file_cache(struct file_cache_t **file_cache_pointer) {
    /* allocates space for the cache itself and for the entries that
    it is made of, which are as many as a hash is able to fall on */
    size_t index;
    size_t file_cache_size = sizeof(struct file_cache_t);
    size_t entries_size = sizeof(struct file_cache_entry_t) * CACHE_SIZE_HANDLER_FILE;
    struct file_cache_t *file_cache = (struct file_cache_t *) MALLOC(file_cache_size);
    file_cache->entries = (struct file_cache_entry_t *) MALLOC(entries_size);

    /* empties every one of the entries, a descriptor of minus one
    being what says that the slot holds no file at all */
    for(index = 0; index < CACHE_SIZE_HANDLER_FILE; index++) {
        file_cache->entries[index].descriptor = -1;
        file_cache->entries[index].path[0] = '\0';
        file_cache->entries[index].size = 0;
        file_cache->entries[index].checked = 0;
    }

    /* sets the cache in the cache pointer */
    *file_cache_pointer = file_cache;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_file_cache(struct file_cache_t *file_cache) {
    /* closes every file that is still being held and then releases
    both the entries and the cache that carried them */
    clear_file_cache(file_cache);
    FREE(file_cache->entries);
    FREE(file_cache);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE clear_file_cache(struct file_cache_t *file_cache) {
    /* allocates space for the index to be used in the iteration
    over the complete set of entries of the cache */
    size_t index;

    /* closes the file of every entry that is holding one, a
    descriptor that is never closed is a descriptor leaked */
    for(index = 0; index < CACHE_SIZE_HANDLER_FILE; index++) {
        if(file_cache->entries[index].descriptor == -1) { continue; }
        CLOSE_READ(file_cache->entries[index].descriptor);
        file_cache->entries[index].descriptor = -1;
        file_cache->entries[index].path[0] = '\0';
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE acquire_file_cache(struct file_cache_t *file_cache, unsigned char *file_path, struct file_cache_entry_t **file_cache_entry_pointer) {
    /* allocates space for the structure that describes the file and
    for the moment at which this is all happening */
    STAT_TYPE file_stat;
    ERROR_CODE error_code;
    unsigned int now = (unsigned int) time(NULL);

    /* the entry that the provided path falls on, a path always falls
    on the very same one of them and takes it over from whatever was
    sitting there before, which is what makes both the finding of it
    and the making of room for it a single step */
    size_t index = _calculate_string_hash_map(file_path) % CACHE_SIZE_HANDLER_FILE;
    struct file_cache_entry_t *entry = &file_cache->entries[index];

    /* in case the entry is holding the very file that is being asked
    for it may be handed back, once what is known about it has been
    made to agree with the file that the descriptor actually reaches */
    if(entry->descriptor != -1 && strcmp((char *) entry->path, (char *) file_path) == 0) {
        /* the descriptor is asked about itself on every single one of
        the requests, which costs nothing against the opening it saves
        and is what keeps the size of the entry honest, a file written
        over in place keeps the very same descriptor and would
        otherwise be answered with the length it used to have and a
        body cut short to match it */
        if(STAT_READ(entry->descriptor, file_stat) != 0) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Problem loading file"
            );
        }
        entry->size = (size_t) file_stat.st_size;
        _time_file_cache(&file_stat, &entry->time);

        /* within the time that an entry is trusted for there is
        nothing else to be asked, the descriptor has just answered */
        if(now - entry->checked < CACHE_VALID_HANDLER_FILE) {
            *file_cache_entry_pointer = entry;
            RAISE_NO_ERROR;
        }

        /* past that the file is opened again through its path, the
        only way of telling that another one has been put in its
        place, the descriptor that is held would go on answering
        about the file that used to be there and a replacement of
        the very same length is told from it by nothing else */
    }

    /* whatever the entry was holding is of no use, either because it
    describes another file or because the one it describes has moved
    on, and it is closed before the slot is taken over */
    if(entry->descriptor != -1) {
        CLOSE_READ(entry->descriptor);
        entry->descriptor = -1;
        entry->path[0] = '\0';
    }

    /* a path longer than an entry is able to carry is never cached,
    the copying of it would run past the end of the buffer */
    if(strlen((char *) file_path) >= VIRIATUM_MAX_PATH_SIZE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem loading file"
        );
    }

    /* opens the file and describes it through the descriptor that
    was just obtained, which answers about the very file that was
    opened and never about one that took its place in between */
    error_code = open_read_file((char *) file_path, &entry->descriptor);
    if(IS_ERROR_CODE(error_code)) {
        entry->descriptor = -1;
        RAISE_AGAIN(error_code);
    }

    if(STAT_READ(entry->descriptor, file_stat) != 0) {
        CLOSE_READ(entry->descriptor);
        entry->descriptor = -1;
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem loading file"
        );
    }

    /* fills the entry with what has just been learnt about the file,
    the time of the last write to it included as the tag that travels
    with the response is built out of it */
    STRCPY((char *) entry->path, VIRIATUM_MAX_PATH_SIZE, (char *) file_path);
    entry->size = (size_t) file_stat.st_size;
    entry->checked = now;
    _time_file_cache(&file_stat, &entry->time);

    /* sets the entry in the entry pointer */
    *file_cache_entry_pointer = entry;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE open_file_cache(struct file_cache_t *file_cache, unsigned char *file_path, int *descriptor_pointer) {
    /* allocates space for the entry of the cache and for the error
    that the acquiring of it may raise */
    ERROR_CODE error_code;
    struct file_cache_entry_t *entry;

    /* acquires the entry for the path, which opens the file when the
    cache is not already holding it open */
    error_code = acquire_file_cache(file_cache, file_path, &entry);
    if(IS_ERROR_CODE(error_code)) { RAISE_AGAIN(error_code); }

    /* hands back a duplicate rather than the descriptor of the entry
    itself, so that the request owns what it reads through and the
    cache is free to close its own whenever the slot is taken over */
    *descriptor_pointer = DUPLICATE(entry->descriptor);
    if(*descriptor_pointer == -1) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem loading file"
        );
    }

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
    the read operation (read bytes), together with the signed
    result of the read that a failure of it is told apart by */
    size_t number_bytes;
    long read_bytes;

    /* reserves space for the offset, reamingin and buffer
    size values to be used in the calculus of the "optimal"
    buffer size for allocation */
    size_t offset;
    size_t remaining;
    size_t buffer_size;

    /* allocates space for the pointer to the buffer to be
    used in the reading operation from the file */
    unsigned char *file_buffer;

    /* allocates the space for the result of the opening of the
    file, which is no longer certain to have succeeded */
    ERROR_CODE error_code;

    /* casts the parameters as handler file context and uses it
    to retrieve the proper file path for the sending */
    struct handler_file_context_t *handler_file_context = (struct handler_file_context_t *) parameters;
    unsigned char *file_path = handler_file_context->file_path_d;

    /* retrieves the underlying connection references in order to be
    able to write the payload through the protocol in use */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

    /* retrieves the descriptor from the handler file context */
    int descriptor = handler_file_context->descriptor;

    /* in case the file is not defined (should be opened) */
    if(descriptor == -1) {
        /* asks the cache for the file, which hands back a duplicate
        of the one it is holding open and only ever opens it when it
        is not already, a file that does not open at this point was
        there when the response was decided upon and is not any
        longer, and there is then nothing at all left to send */
        error_code = open_file_cache(_get_file_cache(), file_path, &descriptor);
        if(IS_ERROR_CODE(error_code)) {
            /* the cleanup the exhausting of a file runs is the one that
            applies here as well, there is nothing left to send either
            way, and returning without it would leave the connection
            locked with the handler of it still in place */
            _cleanup_handler_file(connection, data, parameters);
            RAISE_AGAIN(error_code);
        }

        /* sets the descriptor in the handler file context together
        with the position the reading of it starts from, which is the
        one the range of the request asked to start at */
        handler_file_context->descriptor = descriptor;
        handler_file_context->offset = handler_file_context->initial_byte;
    }

    /* retrieves the current offset position in the file reading
    and uses it to calculate the remaining bytes to be read, then
    calculates the "target" buffer size from the minimum value
    between the (maximum) file buffer size and the remaining number
    of bytes to be read from the file (optimal buffer sizing) */
    offset = handler_file_context->offset;
    remaining = handler_file_context->final_byte - offset + 1;
    buffer_size = remaining < FILE_BUFFER_SIZE_HANDLER_FILE ? remaining : FILE_BUFFER_SIZE_HANDLER_FILE;
    file_buffer = MALLOC(buffer_size);

    /* reads the file contents from the position that is wanted rather
    than seeking the descriptor towards it first, should read either
    the size of a chunk or the size of the complete file in case it's
    shorter than the chunk size */
    read_bytes = READ_AT(descriptor, file_buffer, buffer_size, offset);
    number_bytes = read_bytes < 0 ? 0 : (size_t) read_bytes;
    handler_file_context->offset += number_bytes;

    /* in case the number of read bytes is valid, there's
    data to be sent to the client side */
    if(number_bytes > 0) {
        /* writes the complete set of contents in the file buffer to
        the current connection, the fragment that exhausts the range
        is the one that closes the message */
        http_connection->write_chunk(
            connection,
            file_buffer,
            number_bytes,
            number_bytes == remaining ? TRUE : FALSE,
            _send_chunk_handler_file,
            handler_file_context
        );
    }
    /* otherwise the file "transfer" is complete and the control
    flow should proceed to the cleanup operations */
    else {
        /* unsets the descriptor from the handler file context */
        handler_file_context->descriptor = -1;

        /* runs the cleanup handler file (releases internal structures) */
        _cleanup_handler_file(connection, data, parameters);

        /* closes the duplicate that this request was reading through
        and releases the currently allocated file buffer, the cache
        keeps the one of its own open for whoever asks next */
        CLOSE_READ(descriptor);
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

    /* retrieves the underlying connection references in order to be
    able to write the payload through the protocol in use */
    struct io_connection_t *io_connection = (struct io_connection_t *) connection->lower;
    struct http_connection_t *http_connection = (struct http_connection_t *) io_connection->lower;

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
        file context as flushed, the data is the complete payload of the
        response and so it closes the message */
        http_connection->write_chunk(
            connection,
            template_handler->string_value,
            strlen((char *) template_handler->string_value),
            TRUE,
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
