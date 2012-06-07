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

#pragma once

#include "../http/http.h"
#include "../system/system.h"

/* forward references (avoids loop) */
struct data_t;
struct connection_t;
struct http_connection_t;

/**
 * The size of the file buffer to be used
 * durring a file transfer.
 * Increasing this value will allow the transfer
 * of bigger chunks.
 */
#define FILE_BUFFER_SIZE_HANDLER_FILE 4096

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
     * The path to the file to be handled by
     * the current file request.
     */
    unsigned char file_path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The reference to the file stream to be
     * used in the file request.
     */
    FILE *file;

    /**
     * The flags to be used during the file
     * handling process.
     */
    unsigned char flags;

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

    unsigned char cache_control_status;
    unsigned char cache_control[128];

    unsigned char etag_status;
    unsigned char etag[11];
} handler_file_context;

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
ERROR_CODE _set_http_parser_handler_file(struct http_parser_t *http_parser);
ERROR_CODE _unset_http_parser_handler_file(struct http_parser_t *http_parser);
ERROR_CODE _reset_http_parser_handler_file(struct http_parser_t *http_parser);
ERROR_CODE _set_http_settings_handler_file(struct http_settings_t *http_settings);
ERROR_CODE _unset_http_settings_handler_file(struct http_settings_t *http_settings);
ERROR_CODE _cleanup_handler_file(struct connection_t *connection, struct data_t *data, void *parameters);
ERROR_CODE _send_chunk_handler_file(struct connection_t *connection, struct data_t *data, void *parameter);
ERROR_CODE _send_data_handler_file(struct connection_t *connection, struct data_t *data, void *parameters);
