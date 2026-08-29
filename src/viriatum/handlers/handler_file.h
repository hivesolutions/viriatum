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

#pragma once

#include "../http/http.h"
#include "../system/system.h"

/* forward references (avoids loop) */
struct data_t;
struct connection_t;
struct http_handler_t;
struct http_connection_t;

/**
 * The (maximum) size of the file buffer to be used
 * during a file transfer.
 * Increasing this value will allow the transfer
 * of bigger chunks, note that this is just the
 * maximum value for the buffer smaller files will
 * use smaller buffers.
 */
#define FILE_BUFFER_SIZE_HANDLER_FILE 262144

/**
 * The number of files that the cache of the handler is
 * allowed to keep open at the same time, past which the
 * one that has gone longest without being asked for is
 * closed to make room for the one being asked for now.
 */
#define CACHE_SIZE_HANDLER_FILE 128

/**
 * The number of seconds that an entry of the cache is
 * trusted for, once past it the file it describes is
 * looked at again and the entry is either renewed or
 * thrown away in favour of the file as it now stands.
 */
#define CACHE_VALID_HANDLER_FILE 4

/**
 * Structure describing a file that the handler has
 * open, so that the serving of it again costs neither
 * the opening of it nor the describing of it, which
 * together are the greater part of what a request for
 * a small file costs.
 */
typedef struct file_cache_entry_t {
    /**
     * The path of the file that this entry describes,
     * which is also the key it is stored under.
     */
    unsigned char path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The descriptor of the file, kept open for as long
     * as the entry lives, the requests read through a
     * duplicate of it so that none of them is ever left
     * holding one that the cache has closed.
     */
    int descriptor;

    /**
     * The size in bytes of the file, as it stood when the
     * entry was last renewed.
     */
    size_t size;

    /**
     * The time of the last write to the file, as it stood
     * when the entry was last renewed.
     */
    struct date_time_t time;

    /**
     * The moment at which the file was last looked at, an
     * entry older than the validity is looked at again
     * before it is trusted.
     */
    unsigned int checked;
} file_cache_entry;

/**
 * Structure describing the set of files that the handler
 * is keeping open, one per process as the workers are
 * forked and each of them serves on its own, so that
 * nothing here is ever reached by two at once.
 *
 * A path falls on exactly one of the entries, decided by
 * the hash of it, and takes that entry over from whatever
 * was sitting there before, which is what keeps both the
 * lookup and the making of room down to a single step.
 */
typedef struct file_cache_t {
    /**
     * The entries of the cache, one slot per position that
     * the hash of a path is able to fall on.
     */
    struct file_cache_entry_t *entries;
} file_cache;

/**
 * Structure describing the internal parameters
 * for a location in the file context.
 */
typedef struct file_location_t {
    /**
     * The path to be used as the base for the
     * "computation" of the files to be retrieved.
     */
    unsigned char *base_path;

    /**
     * The name of the realm to be used in a
     * basic authentication for the location.
     * In case the value is not set no basic
     * authentication is used for location.
     */
    unsigned char *auth_basic;

    /**
     * The path to the passwd file to be used for
     * the basic authentication process, must
     * confirm with the defined standard.
     */
    unsigned char *auth_file;

    /**
     * The resources that are promised to a peer together with a
     * request that matches this location, given as a list of paths
     * separated by spaces.
     * Only ever acted upon under HTTP/2, the older version of the
     * protocol carries no way of promising anything.
     */
    unsigned char *push;
} file_location_t;

/**
 * The structure that holds the internal
 * structure to support the context
 * of the file (handler).
 */
typedef struct file_handler_t {
    /**
     * The various locations loaded from the configuration
     * they refer the configuration attributes associated
     * with the file structures.
     */
    struct file_location_t *locations;

    /**
     * The number of locations currently loaded in the handler
     * this value is used for iteration arround the locations
     * buffer.
     */
    size_t locations_count;
} file_handler;

/**
 * The context structure to be used allong
 * the interpretation of the request for
 * the file handler.
 */
typedef struct handler_file_context_t {
    /**
     * The url to be used for retrieving the file.
     */
    unsigned char url[VIRIATUM_MAX_URL_SIZE];

    /**
     * The url (percent) decoded verson of the url
     * to be used for the retrieval of the file.
     */
    unsigned char url_d[VIRIATUM_MAX_URL_SIZE];

    /**
     * The path to the file to be handled by
     * the current file request.
     */
    unsigned char file_path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The url (percent) decoded version of the path
     * to the file to be handled by the current request.
     */
    unsigned char file_path_d[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The base path of the directory to be used for
     * the construction of the full path for the file
     * this value may be unset and in such case the
     * the default value is used instead.
     */
    unsigned char *base_path;

    /**
     * The string describing the realm for which the
     * basic authentication procedure will be used.
     * In case this value is not set no basic
     * authentication is used for the context.
     */
    unsigned char *auth_basic;

    /**
     * The path to the authentication file to be used
     * in a basic authentication request.
     * This file must conform with the standard for
     * the passwd file.
     */
    unsigned char *auth_file;

    /**
     * The resources that are promised to a peer together with the
     * response of this request, taken from the location that has
     * matched it.
     */
    unsigned char *push;

    /**
     * The descriptor of the file to be used in the file
     * request, a duplicate of the one the cache holds so
     * that the request owns it and closes it on its own,
     * set to minus one while there is none.
     */
    int descriptor;

    /**
     * The offset the reading of the file has reached, kept
     * here rather than in the descriptor so that the reads
     * may name the position they want and never have to
     * seek the descriptor towards it.
     */
    size_t offset;

    /**
     * The initial byte value to be used for the retrieval of the
     * file, a complete file request should have this value set to
     * a zero value (initial byte).
     */
    size_t initial_byte;

    /**
     * The final byte value to be used for the finish of the retrieval
     * of the file, a normal (and complete) file request should have
     * this value set to the size of the file request minus one.
     */
    size_t final_byte;

    /**
     * The flags to be used during the file
     * handling process.
     */
    unsigned char flags;

    /**
     * The "internal" next header value that controls the
     * next value that is going to be parsed as header.
     */
    enum http_header_e next_header;

    /**
     * The template handler to be used for requests
     * that provide dynamic data, that must be
     * processed in the beginning of the workflows
     * (eg: listing the entries of a directory).
     */
    struct template_handler_t *template_handler;

    /**
     * The flag that controlls the flushing of the
     * internal structures of the file handler.
     */
    unsigned int flushed;

    /**
     * The flag that controls if the cache control value
     * has already been retrieved (and parsed).
     */
    unsigned char cache_control_status;

    /**
     * The value of the cache control header processed for
     * the file retrieval.
     */
    unsigned char cache_control[128];

    /**
     * The flag that controls if the etag (control) value
     * has already been retrieved (and parsed).
     */
    unsigned char etag_status;

    /**
     * The value of the authorization header processed for
     * the file retrieval.
     */
    unsigned char authorization[128];

    /**
     * The flag that controls if the authorization value
     * has already been retrieved (and parsed).
     */
    unsigned char authorization_status;

    /**
     * The value of the etag header processed for the file
     * retrieval. This value is going to be used to control
     * if a new file must be sent to the client.
     */
    unsigned char etag[11];

    /**
     * The flag that controls if the range value
     * has already been retrieved (and parsed).
     */
    unsigned char range_status;

    /**
     * The string based field for the range value, this is meant
     * to be parsed latter in order to set the initial and final
     * byte values (as integers).
     */
    unsigned char range[128];
} handler_file_context;

ERROR_CODE create_file_cache(struct file_cache_t **file_cache_pointer);
ERROR_CODE delete_file_cache(struct file_cache_t *file_cache);
ERROR_CODE clear_file_cache(struct file_cache_t *file_cache);
ERROR_CODE acquire_file_cache(struct file_cache_t *file_cache, unsigned char *file_path, struct file_cache_entry_t **file_cache_entry_pointer);
ERROR_CODE open_file_cache(struct file_cache_t *file_cache, unsigned char *file_path, int *descriptor_pointer);
ERROR_CODE create_file_handler(struct file_handler_t **file_handler_pointer, struct http_handler_t *http_handler);
ERROR_CODE delete_file_handler(struct file_handler_t *file_handler);
ERROR_CODE create_handler_file_context(struct handler_file_context_t **handler_file_context_pointer);
ERROR_CODE delete_handler_file_context(struct handler_file_context_t *handler_file_context);
ERROR_CODE register_handler_file(struct service_t *service);
ERROR_CODE unregister_handler_file(struct service_t *service);
ERROR_CODE set_handler_file(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_file(struct http_connection_t *http_connection);
ERROR_CODE reset_handler_file(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_file(struct http_request_t *http_request);
ERROR_CODE url_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_file(struct http_request_t *http_request);
ERROR_CODE body_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_file(struct http_request_t *http_request);
ERROR_CODE path_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_file(struct http_request_t *http_request, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_file(struct http_request_t *http_request, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_request_handler_file(struct http_request_t *http_request);
ERROR_CODE _unset_http_request_handler_file(struct http_request_t *http_request);
ERROR_CODE _reset_http_request_handler_file(struct http_request_t *http_request);
ERROR_CODE _set_http_settings_handler_file(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_file(struct http_settings_t *http_settings);
ERROR_CODE _cleanup_handler_file(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _send_chunk_handler_file(struct connection_t *connection, struct data_t *data, void *parameter);
ERROR_CODE _send_data_handler_file(struct connection_t *connection, struct data_t *data, void *parameters);
