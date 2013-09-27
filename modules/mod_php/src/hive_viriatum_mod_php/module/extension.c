/*
 Hive Viriatum Modules
 Copyright (C) 2008-2012 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "extension.h"

zend_function_entry viriatum_functions[] = {
    PHP_FE(viriatum_connections_l, NULL)
    PHP_FE(viriatum_connection_info, NULL)
    PHP_FE(viriatum_connections, NULL)
    PHP_FE(viriatum_uptime, NULL)
    PHP_FE(viriatum_name, NULL)
    PHP_FE(viriatum_version, NULL)
    PHP_FE(viriatum_platform, NULL)
    PHP_FE(viriatum_flags, NULL)
    PHP_FE(viriatum_modules, NULL)
    PHP_FE(viriatum_description, NULL)
    PHP_FE(viriatum_compiler, NULL)
    PHP_FE(viriatum_compiler_version, NULL)
    PHP_FE(viriatum_compilation_date, NULL)
    PHP_FE(viriatum_compilation_time, NULL)
    { NULL, NULL, NULL }
};

zend_module_entry viriatum_module = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "viriatum",
    viriatum_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    ZEND_MINFO(viriatum_information),
#if ZEND_MODULE_API_NO >= 20010901
    VIRIATUM_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

sapi_module_struct viriatum_sapi_module = {
    "viriatum_handler",
    "Viriatum PHP Handler",
    _module_startup,
    php_module_shutdown_wrapper,
    NULL,
    NULL,
    _module_write,
    _module_flush,
    _module_stat,
    _module_getenv,
    php_error,
    _module_header,
    _module_send_headers,
    NULL,
    _module_read_post,
    _module_read_cookies,
    _module_register,
    _module_log,
    _module_request_time,
    NULL,
    STANDARD_SAPI_MODULE_PROPERTIES
};

int _module_startup(sapi_module_struct *module) {
    return php_module_startup(module, &viriatum_module, 1);
}

int _module_write(const char *data, uint data_size TSRMLS_DC) {
    /* allocates space for the buffer that will hold the write
    data that has just been sent to the write operation */
    char *buffer = MALLOC(data_size + 1);
    buffer[data_size] = '\0';

    /* copies the data into the buffer and then adds it to
    the current output linked buffer */
    memcpy(buffer, data, data_size);
    append_linked_buffer(_output_buffer, buffer, data_size, 1);

    /* returns the size of the data that has just been
    writen into the internal structures */
    return data_size;
}

void _module_flush(void *context) {
}

struct stat *_module_stat(TSRMLS_D) {
    return NULL;
}

char *_module_getenv(char *name, size_t size TSRMLS_DC) {
    return NULL;
}

int _module_header(sapi_header_struct *header, sapi_header_op_enum operation, sapi_headers_struct *headers TSRMLS_DC) {
    STRCPY(_php_request.headers[_php_request.header_count], VIRIATUM_MAX_HEADER_C_SIZE, header->header);
    _php_request.header_count++;
    return 0;
}

int _module_send_headers(sapi_headers_struct *headers TSRMLS_DC) {
    STRCPY(_php_request.mime_type, VIRIATUM_MAX_HEADER_V_SIZE, headers->mimetype);
    return SAPI_HEADER_SENT_SUCCESSFULLY;
}

int _module_read_post(char *buffer, uint size TSRMLS_DC) {
    unsigned char *post_data = _php_request.php_context->post_data;
    size_t content_length = _php_request.php_context->content_length;
    size_t _size = content_length > size ? size : content_length;

    if(post_data == NULL) { return 0; }

    memcpy(buffer, _php_request.php_context->post_data, _size);
    _php_request.php_context->content_length -= _size;
    _php_request.php_context->post_data += _size;

    return _size;
}

char *_module_read_cookies(TSRMLS_D) {
    return (char *) _php_request.php_context->cookie;
}

void _module_register(zval *_array TSRMLS_DC) {
    /* allocates space for the iterator to be used to "go around"
    the various header elements and for the header (element) */
    struct iterator_t *headers_iterator;
    struct http_header_value_t *header;

    /* allocates space for the "new" name of the header element
    this anme should be uppercased and (slash) transformed */
    char name[VIRIATUM_MAX_HEADER_SIZE];

    /* allocates space for the address string reference
    and then retrieves the current connection's socket
    address structure to be used to retrieve the address
    string value (for exporting) */
    char *address_string;
    SOCKET_ADDRESS address = _connection->socket_address;

    /* retrieves the current port buffer value and the
    string structure that controls it */
    unsigned char *port = _connection->service->options->_port;
    struct string_t *port_string = &_connection->service->options->_port_string;

    /* converts the address of the socket into the representing
    string value (for exporting the value) */
    address_string = inet_ntoa(((SOCKET_ADDRESS_INTERNET *) &address)->sin_addr);

    /* creates the iterator to the headers linked list so that it's possible
    to "go arround" the headers to expose them to the php interpreter */
    create_iterator_linked_list(_php_request.php_context->headers, &headers_iterator);

    /* iterates continuously over all the headers to export them
    to the php interpreter */
    while(1) {
        /* retrieves the next header element and in case it's null
        finishes the iteration otherwise converts the name to uppercase,
        prepends the prefix value and exposes it to the php interpreter */
        get_next_iterator(headers_iterator, (void **) &header);
        if(header == NULL) { break; }
        uppercase(header->name);
        SPRINTF(name, 1024, "HTTP_%s", header->name);
        php_register_variable_safe(name, (char *) header->value, header->value_size, _array TSRMLS_CC);
    }

    /* deletes the headers iterator (it's no longer required) */
    delete_iterator_linked_list(_php_request.php_context->headers, headers_iterator);

    /* sets the self server variable with the path to the file to be
    executed, this is of major importance for execution (conditional
    usage of this variable is set based on the file name existence) */
    if(_php_request.php_context->file_name) { php_register_variable_safe("PHP_SELF", (char *) _php_request.php_context->file_name, _php_request.php_context->_file_name_string.length, _array TSRMLS_CC); }
    else { php_register_variable_safe("PHP_SELF", "-", 1, _array TSRMLS_CC); }

    /* registers a series og global wide variable representing the
    current interface (critical for correct php interpreter usage) */
    php_register_variable_safe("SERVER_SOFTWARE", "viriatum", sizeof("viriatum") - 1, _array TSRMLS_CC);
    php_register_variable_safe("GATEWAY_INTERFACE", "CGI/1.1", sizeof("CGI/1.1") - 1, _array TSRMLS_CC);
    php_register_variable_safe("SERVER_PROTOCOL", "HTTP/1.1", sizeof("HTTP/1.1") - 1, _array TSRMLS_CC);
    php_register_variable_safe("SERVER_NAME", (char *) _php_request.php_context->server_name, _php_request.php_context->_server_name_string.length, _array TSRMLS_CC);
    php_register_variable_safe("SERVER_PORT", (char *) port, port_string->length, _array TSRMLS_CC);
    php_register_variable_safe("REQUEST_URI", (char *) _php_request.php_context->url, _php_request.php_context->_url_string.length, _array TSRMLS_CC);
    php_register_variable_safe("SCRIPT_NAME", (char *) _php_request.php_context->file_name, _php_request.php_context->_file_name_string.length, _array TSRMLS_CC);
    php_register_variable_safe("SCRIPT_FILENAME", (char *) _php_request.php_context->file_path, _php_request.php_context->_file_path_string.length, _array TSRMLS_CC);
    php_register_variable_safe("QUERY_STRING", (char *) _php_request.php_context->query, _php_request.php_context->_query_string.length, _array TSRMLS_CC);
    php_register_variable_safe("REQUEST_METHOD", (char *) _php_request.php_context->method, strlen(_php_request.php_context->method), _array TSRMLS_CC);
    php_register_variable_safe("REMOTE_ADDR", address_string, strlen(address_string), _array TSRMLS_CC);
    if(_php_request.php_context->_content_type_string.length > 0) { php_register_variable_safe("CONTENT_TYPE", (char *)  _php_request.php_context->content_type, _php_request.php_context->_content_type_string.length, _array TSRMLS_CC); }
    if(_php_request.php_context->_content_length_string.length > 0) { php_register_variable_safe("CONTENT_LENGTH", (char *) _php_request.php_context->content_length_, _php_request.php_context->_content_length_string.length, _array TSRMLS_CC); }
}

void _module_log(char *message TSRMLS_DC) {
    V_DEBUG_F("%s\n", message);
}

double _module_request_time(TSRMLS_D) {
    return 0;
}

ZEND_MINFO_FUNCTION(viriatum_information) {
    php_info_print_table_start();
    php_info_print_table_row(2, "Name", _service->name);
    php_info_print_table_row(2, "Version", _service->version);
    php_info_print_table_row(2, "Platform", _service->platform);
    php_info_print_table_row(2, "Flags", _service->flags);
    php_info_print_table_row(2, "Modules", _service->modules);
    php_info_print_table_row(2, "Compiler", _service->compiler);
    php_info_print_table_row(2, "Compiler Version", _service->compiler_version);
    php_info_print_table_row(2, "Compilation Date", _service->compilation_date);
    php_info_print_table_row(2, "Compilation Time", _service->compilation_time);
    php_info_print_table_end();
    php_info_print_table_start();
    php_info_print_table_header(2, "Name", "Path");
    php_info_print_table_row(2, "Modules", VIRIATUM_MODULES_PATH);
    php_info_print_table_row(2, "Contents", VIRIATUM_CONTENTS_PATH);
    php_info_print_table_row(2, "Resources", VIRIATUM_RESOURCES_PATH);
    php_info_print_table_row(2, "Configuration", VIRIATUM_CONFIG_PATH);
    php_info_print_table_end();
}

PHP_FUNCTION(viriatum_connections_l) {
    /* allocates space for the local variables that are
    going to be used in the construction of the connections */
    size_t index;
    char is_empty;
    struct iterator_t *iterator;
    struct connection_t *connection;
    zval *connection_array;
    unsigned long long delta;
    char uptime[128];

    /* intializes the return value of the function as an
    associative array to be used y the zend system */
    array_init(return_value);

    /* creates an iterator object for the current list of connection
    available in the viriatum engine */
    create_iterator_linked_list(_service->connections_list, &iterator);

    /* starts the index counter value that is going to be
    used in the appending of the values to the array */
    index = 0;

    /* iterates continyously over the complete set of connection
    in the viriatum running instance */
    while(TRUE) {
        /* retrieves the next connection value from the iterator
        and verifies if its value is defined in case it's not this
        is the end of iteration and so the cycle must be break */
        get_next_iterator(iterator, (void **) &connection);
        if(connection == NULL) { break; }

        /* retrieves the delta value by calculating the diference between
        the current time and the creation time then uses it to calculate
        the uptime for the connection as a string description */
        delta = (unsigned long long) time(NULL) - connection->creation;
        format_delta(uptime, sizeof(uptime), delta, 2);

        /* creates the zend value for the connection array and then uses
        the value to be initialized as an array */
        MAKE_STD_ZVAL(connection_array);
        array_init(connection_array);

        /* verifies if the current host is empty, this is a special
        case where no resolution of the value was possible */
        is_empty = connection->host[0] == '\0';

        /* populates the connection array with the complete set of attributes
        that describe the connection, for latter usage by the api client */
        if(is_empty) { add_assoc_string(connection_array, "host", "N/A", 1); }
        else { add_assoc_string(connection_array, "host", (char *) connection->host, 1); }
        add_assoc_long(connection_array, "id", (long) connection->id);
        add_assoc_string(connection_array, "uptime", uptime, 1);

        /* adds the connection array as the current value for the index in
        iteration, this will allow a correct iteration over the connections
        list that is going to be returned in this function */
        add_index_zval(return_value, index, connection_array);

        /* increments the index value as the iteration cycle has
        been completed and a new index must be used (next one) */
        index++;
    }

    /* deletes the iterator for the connections list in order to
    avoid any memory leak that could arise  from this */
    delete_iterator_linked_list(_service->connections_list, iterator);
}

PHP_FUNCTION(viriatum_connection_info) {
    /* allocates space for the local variables that are
    going to be used in the construction of the connection info */
    long id;
    char is_empty;
    struct iterator_t *iterator;
    struct connection_t *connection;
    unsigned long long delta;
    char uptime[128];
    char received[64];
    char sent[64];
    char *family;
    char *protocol;

    /* verifies if the number of arguments received is the expected
    one in case it's not fails with the proper error */
    if(ZEND_NUM_ARGS() != 1) { WRONG_PARAM_COUNT; }

    /* parses the arguments that were provided to the function as
    expected by the php c api infra-structure */
    zend_parse_parameters(1, "l", &id);

    /* intializes the return value of the function as an
    associative array to be used y the zend system */
    array_init(return_value);

    /* creates an iterator object for the current list of connection
    available in the viriatum engine */
    create_iterator_linked_list(_service->connections_list, &iterator);

    /* iterates continyously over the complete set of connection
    in the viriatum running instance */
    while(TRUE) {
        /* retrieves the next connection value from the iterator
        and verifies if its value is defined in case it's not this
        is the end of iteration and so the cycle must be break */
        get_next_iterator(iterator, (void **) &connection);
        if(connection == NULL) { break; }

        /* in case the id of the connection currently in iteration is
        not the smae as the one we're trying to find continues the loop */
        if((long) connection->id != id) { continue; }

        /* retrieves the delta value by calculating the diference between
        the current time and the creation time then uses it to calculate
        the uptime for the connection as a string description */
        delta = (unsigned long long) time(NULL) - connection->creation;
        format_delta(uptime, sizeof(uptime), delta, 2);

        /* formats both the ammount of bytes that have been sent and the
        the ammount that has been received as "readable" strings */
        format_bytes(sent, sizeof(sent), connection->sent);
        format_bytes(received, sizeof(received), connection->received);

        /* retrieves the string that identifies the family for the
        connection from the provided family enumeration value */
        family = (char *) connection_family_strings[connection->family - 1];

        /* retrieves the string that identifies the protocol for the
        connection from the provided protocol enumeration value */
        protocol = (char *) connection_protocol_strings[connection->protocol - 1];

        /* verifies if the current host is empty, this is a special
        case where no resolution of the value was possible */
        is_empty = connection->host[0] == '\0';

        /* populates the connection array with the complete set of attributes
        that describe the connection, for latter usage by the api client */
        if(is_empty) { add_assoc_string(return_value, "host", "N/A", 1); }
        else { add_assoc_string(return_value, "host", (char *) connection->host, 1); }
        add_assoc_long(return_value, "port", (long) connection->port);
        add_assoc_long(return_value, "id", (long) connection->id);
        add_assoc_string(return_value, "family", family, 1);
        add_assoc_string(return_value, "protocol", protocol, 1);
        add_assoc_string(return_value, "uptime", uptime, 1);
        add_assoc_string(return_value, "sent", sent, 1);
        add_assoc_string(return_value, "received", received, 1);

        /* breaks the loop as the proper connection has been found and the
        return value array has been correctly populated */
        break;
    }

    /* deletes the iterator for the connections list in order to
    avoid any memory leak that could arise  from this */
    delete_iterator_linked_list(_service->connections_list, iterator);
}

PHP_FUNCTION(viriatum_connections) {
    RETURN_LONG(_service->connections_list->size);
}

PHP_FUNCTION(viriatum_uptime) {
    RETURN_STRING(_service->get_uptime(_service, 2), 1);
}

PHP_FUNCTION(viriatum_name) {
    RETURN_STRING((char *) _service->name, 1);
}

PHP_FUNCTION(viriatum_version) {
    RETURN_STRING((char *) _service->version, 1);
}

PHP_FUNCTION(viriatum_platform) {
    RETURN_STRING((char *) _service->platform, 1);
}

PHP_FUNCTION(viriatum_flags) {
    RETURN_STRING((char *) _service->flags, 1);
}

PHP_FUNCTION(viriatum_compiler) {
    RETURN_STRING((char *) _service->compiler, 1);
}

PHP_FUNCTION(viriatum_compiler_version) {
    RETURN_STRING((char *) _service->compiler_version, 1);
}

PHP_FUNCTION(viriatum_compilation_date) {
    RETURN_STRING((char *) _service->compilation_date, 1);
}

PHP_FUNCTION(viriatum_compilation_time) {
    RETURN_STRING((char *) _service->compilation_time, 1);
}

PHP_FUNCTION(viriatum_modules) {
    RETURN_STRING((char *) _service->modules, 1);
}

PHP_FUNCTION(viriatum_description) {
    RETURN_STRING((char *) _service->description, 1);
}
