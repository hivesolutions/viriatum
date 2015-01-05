/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
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
 * durring a file transfer.
 * Increasing this value will allow the transfer
 * of bigger chunks, note that this is just the
 * maximum value for the buffer smaller files will
 * use smaller buffers.
 */
#define FILE_BUFFER_SIZE_HANDLER_FILE 262144

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
} file_location_t;

/**
 * The structure that holds the internal
 * structure to support the context
 * of the file (handler).
 */
typedef struct file_handler_t {
    /**
     * The various locations loaded from the configuration
     * they refer the cofiruation attributes associated
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
     * the contruction of the full path for the file
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
     * The reference to the file stream to be
     * used in the file request.
     */
    FILE *file;

    /**
     * The size of the file that is being opened
     * in bytes.
     * This value is limited by the proess architecutre
     * which may cause problems in 32 bit or less.
     */
    size_t file_size;

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

ERROR_CODE create_file_handler(struct file_handler_t **file_handler_pointer, struct http_handler_t *http_handler);
ERROR_CODE delete_file_handler(struct file_handler_t *file_handler);
ERROR_CODE create_handler_file_context(struct handler_file_context_t **handler_file_context_pointer);
ERROR_CODE delete_handler_file_context(struct handler_file_context_t *handler_file_context);
ERROR_CODE register_handler_file(struct service_t *service);
ERROR_CODE unregister_handler_file(struct service_t *service);
ERROR_CODE set_handler_file(struct http_connection_t *http_connection);
ERROR_CODE unset_handler_file(struct http_connection_t *http_connection);
ERROR_CODE reset_handler_file(struct http_connection_t *http_connection);
ERROR_CODE message_begin_callback_handler_file(struct http_parser_t *http_parser);
ERROR_CODE url_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_field_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE header_value_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE headers_complete_callback_handler_file(struct http_parser_t *http_parser);
ERROR_CODE body_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE message_complete_callback_handler_file(struct http_parser_t *http_parser);
ERROR_CODE path_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE location_callback_handler_file(struct http_parser_t *http_parser, size_t index, size_t offset);
ERROR_CODE virtual_url_callback_handler_file(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size);
ERROR_CODE _set_http_parser_handler_file(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_file(struct http_parser_t *http_parser);
ERROR_CODE _reset_http_parser_handler_file(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_file(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_file(struct http_settings_t *http_settings);
ERROR_CODE _cleanup_handler_file(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _send_chunk_handler_file(struct connection_t *connection, struct data_t *data, void *parameter);
ERROR_CODE _send_data_handler_file(struct connection_t *connection, struct data_t *data, void *parameters);
