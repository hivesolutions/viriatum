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

#include "http_util.h"

size_t write_http_headers(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, char close) {
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
        connection->service->description,
        close_codes[(size_t) close]
    );

    /* returns the number of bytes written into the buffer */
    return count;
}

size_t write_http_headers_c(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, size_t content_length, enum http_cache_e cache, int close) {
    /* writes the header information into the buffer using the basic
    function then uses the count offset to determine the buffer position
    to write the remaining headers for the complete write */
    size_t count = write_http_headers(
        connection,
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

    /* returns the number of bytes written into the buffer */
    return count;
}

size_t write_http_headers_a(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, size_t content_length, enum http_cache_e cache, char *realm, int close) {
    /* writes the header information into the buffer using the basic
    function then uses the count offset to determine the buffer position
    to write the remaining headers for the complete write */
    size_t count = write_http_headers(
        connection,
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
        WWW_AUTHENTICATE_H ": Basic realm=\"%s\"\r\n"
        "%s",
        (long unsigned int) content_length,
        cache_codes[cache - 1],
        realm,
        close_codes[close]
    );

    /* returns the number of bytes written into the buffer */
    return count;
}

size_t write_http_headers_m(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, enum http_keep_alive_e keep_alive, size_t content_length, enum http_cache_e cache, char *message) {
    /* writes the complete set of buffer using the appropriate function and then
    uses the count offset to copy the contents part of the message into the final
    part of the buffer (message writing) */
    size_t count = write_http_headers_c(
        connection,
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

    /* returns the number of bytes written into the buffer */
    return count;
}

ERROR_CODE write_http_message(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int status_code, char *status_message, char *message, connection_data_callback_hu callback, void *callback_parameters) {
    /* allocates the headers buffer (it will be releases automatically by the writter)
    it need to be allocated in the heap so it gets throught the request cycle */
    char *headers_buffer = buffer == NULL ? MALLOC(VIRIATUM_HTTP_SIZE) : buffer;
    size = size == 0 ? VIRIATUM_HTTP_SIZE : size;

    /* writes the http static headers to the response and
    then writes the error description itself */
    write_http_headers_m(
        connection,
        headers_buffer,
        size,
        version,
        status_code,
        status_message,
        KEEP_ALIVE,
        strlen(message),
        NO_CACHE,
        message
    );

    /* writes both the headers to the connection, registers for the appropriate callbacks */
    write_connection(
        connection,
        (unsigned char *) headers_buffer,
        (unsigned int) strlen(headers_buffer),
        (connection_data_callback) callback,
        callback_parameters
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE write_http_error(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int error_code, char *error_message, char *error_description, connection_data_callback_hu callback, void *callback_parameters) {
    return write_http_error_a(
        connection,
        buffer,
        size,
        version,
        error_code,
        error_message,
        error_description,
        NULL,
        callback,
        callback_parameters
    );
}

ERROR_CODE write_http_error_a(struct connection_t *connection, char *buffer, size_t size, enum http_version_e version, int error_code, char *error_message, char *error_description, char *realm, connection_data_callback_hu callback, void *callback_parameters) {
    /* allocates space for the result buffer related
    variables (for both the buffer pointer and size) */
    size_t result_length;
    unsigned char *result_buffer;

    /* allocates space for the references the string
    buffer and template handler structures */
    struct string_buffer_t *string_buffer;
    struct template_handler_t *template_handler;

    /* allocates space for the "possible" locally generated
    error description (format based generation) */
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
    char *headers_buffer = buffer == NULL ? MALLOC(VIRIATUM_HTTP_SIZE) : buffer;
    size = size == 0 ? VIRIATUM_HTTP_SIZE : size;

#ifndef VIRIATUM_DEBUG
    /* sets the error description as null in order to avoid any
    display of the (internal) message, otherwise a possible
    security hole would be created */
    error_description = NULL;
#endif

    /* in case the use template flag is set the error
    should be displayed using the template */
    if(options->use_template) {
        /* creates the complete path to the template file */
        SPRINTF(
            (char *) template_path,
            sizeof(template_path),
            "%s%s",
            VIRIATUM_RESOURCES_PATH,
            VIRIATUM_ERROR_PATH
        );

        /* prints a debug message */
        V_DEBUG_F("Processing template file '%s'\n", template_path);

        /* creates the template handler that will hold the error
        information contents */
        create_template_handler(&template_handler);

        /* assigns the various error related variables into the
        template handler to be used, they may be used to display
        information arround the error, note that the error description
        value is conditional and may not be set */
        assign_integer_template_handler(template_handler, (unsigned char *) "error_code", error_code);
        assign_string_template_handler(template_handler, (unsigned char *) "error_message", error_message);
        if(error_description != NULL) {
            assign_string_template_handler(template_handler, (unsigned char *) "error_description", error_description);
        }

        /* processes the file as a template handler, at this point
        the output buffer of the template engine should be populated
        with the complete header information, the apropriate header
        writing method is chosen based on the existence or not of
        the realm authorization field */
        process_template_handler(template_handler, template_path);
        realm == NULL ? write_http_headers_c(
            connection,
            headers_buffer,
            size,
            version,
            error_code,
            error_message,
            KEEP_ALIVE,
            strlen((char *) template_handler->string_value),
            NO_CACHE,
            TRUE
        ) : write_http_headers_a(
            connection,
            headers_buffer,
            size,
            version,
            error_code,
            error_message,
            KEEP_ALIVE,
            strlen((char *) template_handler->string_value),
            NO_CACHE,
            realm,
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
        write_connection(
            connection,
            result_buffer,
            (unsigned int) result_length,
            (connection_data_callback) callback,
            callback_parameters
        );
    } else {
        /* "stringfies" a possible null error description into a description
        string in order to be correctly displayed then formats the error
        message using the code, message and description */
        error_description = error_description == NULL ? (char *) service->description : error_description;
        SPRINTF(
            _error_description,
            sizeof(_error_description),
            "%d - %s - %s",
            error_code,
            error_message,
            error_description
        );

        /* writes the http static headers to the response and
        then writes the error description itself */
        write_http_headers_m(
            connection,
            headers_buffer,
            size,
            version,
            error_code,
            error_message,
            KEEP_ALIVE,
            strlen(_error_description),
            NO_CACHE,
            _error_description
        );

        /* writes both the headers to the connection, registers for the appropriate callbacks */
        write_connection(
            connection,
            (unsigned char *) headers_buffer,
            (unsigned int) strlen(headers_buffer),
            (connection_data_callback) callback,
            callback_parameters
        );
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE log_http_request(char *host, char *identity, char *user, char *method, char *uri, enum http_version_e version, int error_code, size_t content_length) {
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
        http_version_codes[version - 1],
        error_code,
        (long unsigned int) content_length
    );

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE auth_http(char *auth_file, char *authorization, unsigned char *result) {
    unsigned char is_default = strcmp(auth_file, "$default") == 0 ? TRUE : FALSE;
    if(is_default) { auth_default_http(auth_file, authorization, result); }
    else { auth_file_http(auth_file, authorization, result); }
    RAISE_NO_ERROR;
}

ERROR_CODE auth_default_http(char *auth_file, char *authorization, unsigned char *result) {
    size_t is_valid = strcmp(authorization, DEFAULT_AUTH_HTTP);
    *result = is_valid == 0 ? TRUE : FALSE;
    RAISE_NO_ERROR;
}


typedef enum passwd_state_e {
	PASSWD_USER = 1,
	PASSWD_PASSWORD,
	PASSWD_COMMENT
} passwd_state;



ERROR_CODE process_passwd_file(char *file_path, struct hash_map_t **passwd_pointer) {
	size_t size;
	size_t index;
	size_t length;
	ERROR_CODE return_value;
	unsigned char current;

	unsigned char state;

	unsigned char *mark;
	unsigned char *buffer;
	unsigned char *pointer;

	char name[128];
	char *value;

    struct hash_map_t *passwd;
    
	create_hash_map(&passwd, 0);

	return_value = read_file(file_path, &buffer, &size);
    if(IS_ERROR_CODE(return_value)) {
		RAISE_ERROR_M(
		    RUNTIME_EXCEPTION_ERROR_CODE,
			(unsigned char *) "Problem reading file"
		);
	}

	mark = buffer;
	state = PASSWD_USER;

	for(index = 0; index < size; index++) {
		current = buffer[index];
		pointer = &buffer[index];

		switch(state) {
			case PASSWD_USER:
				switch(current) {
					case ':':
					case '\n':
						length = pointer - mark;
						memcpy(name, mark, length);
						name[length] = '\0';
						mark = pointer + 1;
						state = current == ':' ? PASSWD_PASSWORD : PASSWD_USER;
						break;
				}

				break;

			case PASSWD_PASSWORD:
				switch(current) {
					case ':':
					case '\n':
						length = pointer - mark;
						value = (char *) MALLOC(length + 1);
						memcpy(value, mark, length);
						value[length] = '\0';
						mark = pointer + 1;
						set_value_string_hash_map(passwd, name, value);
						state = current == ':' ? PASSWD_COMMENT : PASSWD_USER;
						break;
				}

				break;

			case PASSWD_COMMENT:
				switch(current) {
					case '\n':
						state = PASSWD_USER;
						mark = pointer + 1;
						break;
				}

				break;
		}
	}

	FREE(buffer);

	*passwd_pointer = passwd;
	RAISE_NO_ERROR;
}

ERROR_CODE auth_file_http(char *auth_file, char *authorization, unsigned char *result) {
	ERROR_CODE return_value;
	struct hash_map_t *passwd;

	char *pointer;
	char *authorization_b64;
	char *authorization_d;
	char *password_pointer;
	char username[128];
	char password[128];
	char *password_v;
	size_t authorization_size;
	size_t username_size;
	size_t password_size;

	process_passwd_file(auth_file, &passwd);

	pointer = strchr(authorization, ' ');
	if(pointer == NULL) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Authorization value not valid"
        );
    }
	authorization_b64 = pointer + 1;

	return_value = decode_base64(
	    authorization_b64,
		strlen(authorization_b64),
		&authorization_d,
		&authorization_size
	);
	if(IS_ERROR_CODE(return_value)) {
		RAISE_ERROR_M(
		    RUNTIME_EXCEPTION_ERROR_CODE,
			(unsigned char *) "Problem decoding base 64 authorization"
		);
	}

	pointer = memchr(authorization_d, ':', authorization_size);
	if(pointer == NULL) {
		FREE(authorization_d);
		RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "No password separator found in authorization"
        );
	}
	password_pointer = pointer + 1;

	username_size = password_pointer - authorization_d - 1;
	password_size = authorization_d + authorization_size - password_pointer;

	memcpy(username, authorization_d, username_size);
	username[username_size] = '\0';

	memcpy(password, password_pointer, password_size);
	password[password_size] = '\0';

	get_value_string_hash_map(passwd, username, &password_v);
	if(password_v != NULL && strcmp(password, password_v) == 0) {
		*result = TRUE;
	} else {
		*result = FALSE;
	}

	/* releases the memory associated with the complete set
	of values in the passwd structure and then releases the
	memory from the hash map structure itself, then releases
	the memory associated with the authorization decoded string */
	delete_values_hash_map(passwd);
	delete_hash_map(passwd);
	FREE(authorization_d);

    RAISE_NO_ERROR;
}
