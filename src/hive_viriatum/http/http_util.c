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

ERROR_CODE get_http_range_limits(unsigned char *range, size_t *initial_byte, size_t *final_byte, size_t size) {
    /* allocates space for the byte to be used in the iteration,
    for the mark value and fot the iteration index counter */
    char byte;
    size_t mark;
    size_t index;

    /* creates the inital and final local values and starts them
    at the invalid (unset) values (intial state) */
    int initial_byte_v = -1;
    int final_byte_v = -1;

    /* retrieves the size of the range string using the normal
    length if a string function */
    size_t range_s = strlen((char *) range);

	/* initializes the mark value with a zero indicating that the
	equals separator may not be found (avoids critical problems) */
	mark = 0;

    /* iterates over the range string character values to try to
    find the various key values in it from the byte */
    for(index = 0; index < range_s; index++) {
        /* retrieves the current byte in iteration and runs the
        switch testing against it */
        byte = range[index];
        switch(byte) {
            case '=':
                mark = index + 1;
                break;

            case '-':
                if(index == 0) { initial_byte_v = 0; }
                else {
                    range[index] = '\0';
                    initial_byte_v = atoi((char *) &range[mark]);
                }
                mark = index + 1;
                break;

            case '\0':
            case ',':
                if(index == mark) { final_byte_v = size - 1; }
                else {
                    range[index] = '\0';
                    final_byte_v = atoi((char *) &range[mark]);
                }
                break;
        }
    }

    /* in case the final byte value hasn't been already
    set time to update its value with the final byte */
    if(final_byte_v == -1) {
        if(index == mark) { final_byte_v = size - 1; }
        else {
            range[index] = '\0';
            final_byte_v = atoi((char *) &range[mark]);
        }
    }

    /* updates both the initial and the final byte pointer
    references with the appropriate values */
    *initial_byte = initial_byte_v;
    *final_byte = final_byte_v;

    /* raises no error as both the initial and the final
    byte values have been computed with success */
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

ERROR_CODE auth_file_http(char *auth_file, char *authorization, unsigned char *result) {
    /* allocates space for the error return value to be
    used in error checking for function calls */
    ERROR_CODE return_value;

    /* allocates space for the pointer to the passwd key
    value structure to be created by parsing the auth file */
    struct hash_map_t *passwd;

    /* allocates space to the various pointer values to be
    used for the separation and treatment of the auth value */
    char *pointer;
    char *authorization_b64;
    char *authorization_d;
    char *password_pointer;

    /* allocates space for the buffers to be used for the username
    and password values extracted from the authorization token */
    char username[128];
    char password[128];
    char *password_v;

    /* allocates the various size relates values for the buffer
    variables creation */
    size_t authorization_size;
    size_t username_size;
    size_t password_size;

    /* tries to find the token that separates the authentication
    type from the authorization base 64 value in case the value
    is not found raises an error indicating the problem */
    pointer = strchr(authorization, ' ');
    if(pointer == NULL) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Authorization value not valid"
        );
    }
    authorization_b64 = pointer + 1;

    /* tries to decode the authorization base 64 value into a plain
    text value in case the decoding fails, re-raises the error to
    the upper levels for caller information */
    return_value = decode_base64(
        (unsigned char *) authorization_b64,
        strlen(authorization_b64),
        (unsigned char **) &authorization_d,
        &authorization_size
    );
    if(IS_ERROR_CODE(return_value)) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem decoding base 64 authorization"
        );
    }

    /* tries to find the token that separates the username part of the
    authorization from the password part in case the value is not found
    raises an error to the upper levels */
    pointer = memchr(authorization_d, ':', authorization_size);
    if(pointer == NULL) {
        FREE(authorization_d);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "No password separator found in authorization"
        );
    }
    password_pointer = pointer + 1;

    /* calculates the size of both the username and the password
    from the diference between the various pointers */
    username_size = password_pointer - authorization_d - 1;
    password_size = authorization_d + authorization_size - password_pointer;

    /* copies both the username and the password values to
    the apropriate internal buffers (to be used in comparision) */
    memcpy(username, authorization_d, username_size);
    username[username_size] = '\0';
    memcpy(password, password_pointer, password_size);
    password[password_size] = '\0';

    /* processes the passwd file using the provided file path
    for it, this is an expensive io driven operation, and must
    be used wth care */
    process_passwd_file(auth_file, &passwd);

    /* retrieves the password verification value for the
    retrieved username and in case it's valid compares it
    and sets the result value accordingly */
    get_value_string_hash_map(
        passwd,
        (unsigned char *) username,
        (void **) &password_v
    );
    if(password_v != NULL && strcmp(password, password_v) == 0) {
        *result = TRUE;
    } else { *result = FALSE; }

    /* releases the memory associated with the complete set
    of values in the passwd structure and then releases the
    memory from the hash map structure itself, then releases
    the memory associated with the authorization decoded string */
    delete_values_hash_map(passwd);
    delete_hash_map(passwd);
    FREE(authorization_d);

    /* raises no error, as everything has been done as possible
    with no problems created in the processing */
    RAISE_NO_ERROR;
}

ERROR_CODE parameters_http(struct hash_map_t *hash_map, unsigned char **buffer_pointer, size_t *buffer_length_pointer) {
    /* allocates space for an iterator object for an hash map element
    and for the string buffer to be used to collect all the partial
    strings that compose the complete url parameters string */
    struct iterator_t *iterator;
    struct hash_map_element_t *element;
    struct string_buffer_t *string_buffer;

    /* allocates space for the string structure to hold the value of
    the element for the string value reference for the joined string,
    for the buffer string from the element and for the corresponding
    string lengths for both cases */
    struct string_t *string;
    unsigned char *string_value;
    unsigned char *_buffer;
    size_t string_length;
    size_t _length;

    /* allocates and sets the initial value on the flag that controls
    if the iteratio to generate key values is the first one */
    char is_first = 1;

    /* creates a new string buffer and a new hash map
    iterator, these structures are going to be used to
    handle the string from the hash map and to iterate
    over the hash map elements */
    create_string_buffer(&string_buffer);
    create_element_iterator_hash_map(hash_map, &iterator);

    /* iterates continuously arround the hash map element
    the iterator is going to stop the iteration */
    while(TRUE) {
        /* retrieves the next element from the iterator
        and in case such element is invalid breaks the loop */
        get_next_iterator(iterator, (void **) &element);
        if(element == NULL) { break; }

        /* checks if this is the first loop in the iteration
        in it's not emits the and character */
        if(is_first) { is_first = 0; }
        else { append_string_buffer(string_buffer, (unsigned char *) "&"); }

        /* retrieves the current element value as a string structure
        then encodes that value using the url encoding (percent encoding)
        and resets the string reference to contain the new buffer as it'
        own contents (avoids extra memory usage) */
        string = (struct string_t *) element->value;
        url_encode(string->buffer, string->length, &_buffer, &_length);
        string->buffer = _buffer;
        string->length = _length;

        /* adds the various elements for the value to the string buffer
        first the key the the attribution operator and then the value */
        append_string_buffer(string_buffer, (unsigned char *) element->key_string);
        append_string_l_buffer(string_buffer, (unsigned char *) "=", sizeof("=") - 1);
        _append_string_t_buffer(string_buffer, string);
    }

    /* "joins" the string buffer values into a single
    value (from the internal string list) and then
    retrieves the length of the string buffer */
    join_string_buffer(string_buffer, &string_value);
    string_length = string_buffer->string_length;

    /* deletes the hash map iterator and string buffer
    structures, to avoid memory leak */
    delete_iterator_hash_map(hash_map, iterator);
    delete_string_buffer(string_buffer);

    /* updates the buffer pointer reference and the
    buffer length pointer reference with the string
    value and the string length values */
    *buffer_pointer = string_value;
    *buffer_length_pointer = string_length;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE parameters_http_c(char *buffer, size_t size, size_t count, ...) {
    /* allocates space for the variable list of arguments
    provided as arguments to this function and tha should
    contain sequences of key, value and length */
    va_list arguments;

    /* allocates space for the various indexes to be
    used in the iteration including the sequence offset
    and the global index counter */
    size_t index;
    size_t index_g;
    size_t offset;

    /* allocates space for the three components of the
    parameter the key, the value and the length of the
    provided value buffer (it's not null terminated) */
    char *key_s;
    char *buffer_s;
    size_t length_s;

    /* allocates space for the pointer to the buffer that
    will hold the created parameters string */
    char *params_buffer;
    size_t params_size;

    /* creates space for the pointer to the map that will
    contain all the arranjed sequence of keys and values
    representing the various parameters */
    struct hash_map_t *parameters_map;

    /* statically allocates space in the stack for the various
    value strings representing the parameter values */
    struct string_t strings[256];

    /* in case the number of tuples provided as arguments is
    more that the space available for the value strings must
    fail with an error otherwise a buffer overflow occurs */
    if(count > 256) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem creating parameters"
        );
    }

    /* sets the inial key string value as null so that the
    value is started initialized, required for safe execution */
    key_s = NULL;

    /* multiplies the count by three as it must contain
    the real number of arguments and not just the number
    of tuples of three in it */
    count *= 3;

    /* creates the hash map that is going to be used to
    temporarly store the various key value associations */
    create_hash_map(&parameters_map, 0);

    /* iterates over the sequence of dynamic arguments
    provided to the function to retrieve them as sequences
    of key, value and length, then sets them in the map
    representing the various parameters */
    va_start(arguments, count);
    for(index = 0; index < count; index++) {
        offset = index % 3;
        index_g = index / 3;

        switch(offset) {
            case 0:
                key_s = va_arg(arguments, char *);
                break;

            case 1:
                buffer_s = va_arg(arguments, char *);
                strings[index_g].buffer = (unsigned char *) buffer_s;
                break;

            case 2:
                length_s = va_arg(arguments, size_t);
                strings[index_g].length = length_s;

                set_value_string_hash_map(
                    parameters_map,
                    (unsigned char *) key_s,
                    (void *) &strings[index_g]
                );

                break;
        }
    }
    va_end(arguments);

    /* generates the (get) parameters for an http request
    from the provided hash map of key values, the returned
    buffer is owned by the caller and must be released */
    parameters_http(
        parameters_map,
        (unsigned char **) &params_buffer,
        &params_size
    );
    delete_hash_map(parameters_map);

    /* in case the amount of bytes to be copied from the
    dynamically created params buffer to the buffer is
    greater than the size provided raises an error */
    if(params_size > size - 1) {
        FREE(params_buffer);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem creating parameters"
        );
    }

    /* copies the generated params buffer into the final
    buffer defined in the parameters structure, then closes
    the string with the final character and releases the
    temporary buffer (params buffer) */
    memcpy(buffer, params_buffer, params_size);
    buffer[params_size] = '\0';
    FREE(params_buffer);

    /* raises no error as the creation of the parameters
    buffer has completed with success */
    RAISE_NO_ERROR;
}
