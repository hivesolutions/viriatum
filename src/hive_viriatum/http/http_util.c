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

#include "http_util.h"

ERROR_CODE write_http_headers(struct service_t *service, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, char close) {
    /* writes the header information into the provided buffer, the writing
    into this buffer uses the most of static structures possible in order
    to accelerate the writing speed at the end of the writing the number
    of written bytes is retrieved as the count value */
    size_t count = SPRINTF(
        buffer,
        size,
        "%s %d %s\r\n"
        CONNECTION_H ": %s\r\n"
        SERVER_H ": %s\r\n"
        "%s",
        http_version_codes[version - 1],
        status_code,
        status_message,
        keep_alive_codes[keep_alive - 1],
        service->description,
        close_codes[close]
    );

    /* returns the number of bytes written into the buffer */
    return count;
}

ERROR_CODE write_http_headers_c(struct service_t *service, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, size_t content_length, enum http_cache_e cache, int close) {
    size_t count = write_http_headers(
        service,
        buffer,
        size,
        version,
        status_code,
        status_message,
        keep_alive,
        FALSE
    );
    count += SPRINTF(
        &buffer[count],
        size - count,
        CONTENT_LENGTH_H ": %lu\r\n"
        CACHE_CONTROL_H ": %s\r\n"
        "%s",
        (long unsigned int) content_length,
        cache_codes[cache - 1],
        close_codes[close]
    );

    return count;
}

ERROR_CODE write_http_headers_m(struct service_t *service, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, size_t content_length, enum http_cache_e cache, char *message) {
    size_t count = write_http_headers_c(
        service,
        buffer,
        size,
        version,
        status_code,
        status_message,
        keep_alive,
        content_length,
        cache,
        TRUE
    );
    memcpy(&buffer[count], message, content_length + 1);

    return count;
}

ERROR_CODE write_http_error(struct connection_t *connection, char *header, int error_code, char *error_message, char *error_description, _connection_data_callback callback, void *callback_parameters) {
    /* allocates space for the result buffer related
    variables (for both the buffer pointer and size) */
    size_t result_length;
    unsigned char *result_buffer;

    /* allocates space for the references the string
    buffer and template handler structures */
    struct string_buffer_t *string_buffer;
    struct template_handler_t *template_handler;

    /* allocates space for the "possible" locally generated
    error description (in case none is provided) */
    char _error_description[1024];

    /* allocates space to the path to the template file to
    be used for template generation */
    unsigned char template_path[VIRIATUM_MAX_PATH_SIZE];

    /* retrieves the service from the connection structure and
    then uses it to retrieve its options */
    struct service_t *service = connection->service;
    struct service_options_t *options = service->options;

    /* allocates the headers buffer (it will be releases automatically by the writter)
    it need to be allocated in the heap so it gets throught the request cycle */
    char *headers_buffer = header == NULL ? MALLOC(1024) : header;

    /* in case no error description is sent one must be created from the currently
    staticly allocated buffer and then formatted properly */
    if(error_description == NULL) {
        /* sets the proper error description buffer and then formats the
        string according to the error code and message */
        error_description = _error_description;
        SPRINTF(error_description, 1024, "%d - %s", error_code, error_message);
    }

    /* in case the use template flag is set the error
    should be displayed using the template */
    if(options->use_template) {
        /* creates the complete path to the template file */
        SPRINTF((char *) template_path, 1024, "%s%s", VIRIATUM_RESOURCES_PATH, VIRIATUM_ERROR_PATH);

        /* prints a debug message */
        V_DEBUG_F("Processing template file '%s'\n", template_path);

        /* creates the template handler that will hold the error
        information contents */
        create_template_handler(&template_handler);

        /* assigns the various error related variables into the
        template handler to be used, they may be used to display
        information arround the error */
        assign_integer_template_handler(template_handler, (unsigned char *) "error_code", error_code);
        assign_string_template_handler(template_handler, (unsigned char *) "error_message", error_message);
        assign_string_template_handler(template_handler, (unsigned char *) "error_description", error_description);

        /* processes the file as a template handler, at this point
        the output buffer of the template engine should be populated */
        process_template_handler(template_handler, template_path);
        write_http_headers_c(
            service,
            headers_buffer,
            1024,
            HTTP11,
            error_code,
            error_message,
            KEEP_ALIVE,
            strlen((char *) template_handler->string_value),
            NO_CACHE,
            TRUE
        );

        /* creates a new string buffer to hold the complete set of contents
        to be sent to the client then first writes the buffer containing
        the headers and then the resulting contents from the template handler */
        create_string_buffer(&string_buffer);
        append_string_buffer(string_buffer, (unsigned char *) headers_buffer);
        append_string_buffer(string_buffer, template_handler->string_value);
        join_string_buffer(string_buffer, &result_buffer);
        result_length = string_buffer->string_length;
        delete_string_buffer(string_buffer);

        /* deletes the template handler as all the processing on it
        has been done (buffer generated) */
        delete_template_handler(template_handler);

        /* releases the contents of the headers buffer, no more need to
        continue using them (not requried) */
        FREE(headers_buffer);

        /* writes the resulting buffer into the connection in order to be sent
        to the client (sends the template results) in one chunk */
        write_connection(connection, result_buffer, result_length, (connection_data_callback) callback, callback_parameters);
    } else {
        /* writes the http static headers to the response and
        then writes the error description itself */
        write_http_headers_m(
            service,
            headers_buffer,
            1024,
            HTTP11,
            error_code,
            error_message,
            KEEP_ALIVE,
            strlen(error_description),
            NO_CACHE,
            error_description
        );

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(connection, (unsigned char *) headers_buffer, (unsigned int) strlen(headers_buffer), (connection_data_callback) callback, callback_parameters);
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE log_http_request(char *host, char *identity, char *user, char *method, char *uri, char *protocol, int error_code, size_t content_length) {
    /* allocates space for the buffer to hild the date and time
    information to be logged */
    char date_buffer[1024];

    /* allocates space for the internal date structures to be used
    for the retrieval of the time information and retrieves the
    current time to be used converting it then to the local time*/
    struct tm *_local_time;
    time_t _time = time(NULL);
    LOCAL_TIME(_local_time, &_time);

    /* checks if the converted local time is invalid and in case it
    is raises the apropriate runtime error to be caught */
    if(_local_time == NULL) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem retrieving local time"); }

    /* formats the local time into the data buffer and then uses it and
    the other (sent) variables to format the output buffer */
    strftime(date_buffer, 1024, "%d/%b/%Y %H:%M:%S", _local_time);
    PRINTF_F(
        "%s %s %s [%s] \"%s %s %s\" %d %lu\n",
        host,
        identity,
        user,
        date_buffer,
        method,
        uri,
        protocol,
        error_code,
        (long unsigned int) content_length
    );

    /* raises no error */
    RAISE_NO_ERROR;
}
