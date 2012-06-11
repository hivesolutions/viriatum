/*
 Hive Viriatum Modules
 Copyright (C) 2008 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "extension.h"

zend_function_entry viriatum_functions[] = {
    PHP_FE(viriatum_connections, NULL)
    PHP_FE(viriatum_name, NULL)
    PHP_FE(viriatum_version, NULL)
    PHP_FE(viriatum_description, NULL)
    PHP_FE(viriatum_observations, NULL)
    PHP_FE(viriatum_copyright, NULL)
    PHP_FE(viriatum_platform_string, NULL)
    PHP_FE(viriatum_platform_cpu, NULL)
    PHP_FE(viriatum_compilation_date, NULL)
    PHP_FE(viriatum_compilation_time, NULL)
    PHP_FE(viriatum_compiler, NULL)
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
    STRCPY(_php_request.headers[_php_request.header_count], 1024, header->header);
    _php_request.header_count++;
    return 0;
}

int _module_send_headers(sapi_headers_struct *headers TSRMLS_DC) {
    STRCPY(_php_request.mime_type, 1024, headers->mimetype);
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
	/* allocates space for the strinf to hold the service
	port value (to be passed to the interpreter) */
	char port_string[128];

    /* allocates space for the address string reference
    and then retrieves the current connection's socket
    address structure to be used to retrieve the address
    string value (for exporting) */
    char *address_string;
    SOCKET_ADDRESS address = _connection->socket_address;

	/* converts the service port value into a string value
	using a base ten encoding */
	SPRINTF(port_string, 128, "%d", _connection->service->options->port);

    /* converts the address of the socket into the representing
    string value (for exporting the value) */
    address_string = inet_ntoa(((SOCKET_ADDRESS_INTERNET *) &address)->sin_addr);

    /* registers a series og global wide variable representing the
    current interface (critical for correct php interpreter usage) */
    php_register_variable_safe("PHP_SELF", "-", 1, _array TSRMLS_CC);
    php_register_variable_safe("GATEWAY_INTERFACE", "viriatum", sizeof("viriatum") - 1, _array TSRMLS_CC);
	php_register_variable_safe("SERVER_NAME", (char *) _php_request.php_context->server_name, _php_request.php_context->_server_name_string.length, _array TSRMLS_CC);
	php_register_variable_safe("SERVER_PORT", (char *) port_string, strlen(port_string), _array TSRMLS_CC);
	php_register_variable_safe("SCRIPT_NAME", (char *)_php_request.php_context->file_name, _php_request.php_context->_file_name_string.length, _array TSRMLS_CC);
	php_register_variable_safe("SCRIPT_FILENAME", (char *)_php_request.php_context->file_path, _php_request.php_context->_file_path_string.length, _array TSRMLS_CC);
	php_register_variable_safe("QUERY_STRING", (char *) _php_request.php_context->query, _php_request.php_context->_query_string.length, _array TSRMLS_CC);
	php_register_variable_safe("REQUEST_METHOD", (char *) _php_request.php_context->method, strlen(_php_request.php_context->method), _array TSRMLS_CC);
    php_register_variable_safe("REMOTE_ADDR", address_string, strlen(address_string), _array TSRMLS_CC);
}

void _module_log(char *message TSRMLS_DC) {
    V_DEBUG_F("%s\n", message);
}

double _module_request_time(TSRMLS_D) {
    return 0;
}

ZEND_MINFO_FUNCTION(viriatum_information) {
    php_info_print_table_start();
    php_info_print_table_row(2, "Name", VIRIATUM_NAME);
    php_info_print_table_row(2, "Version", VIRIATUM_VERSION);
    php_info_print_table_row(2, "Compiler", VIRIATUM_COMPILER);
    php_info_print_table_row(2, "Compilation Date", VIRIATUM_COMPILATION_DATE);
    php_info_print_table_end();
    php_info_print_table_start();
    php_info_print_table_header(2, "Name", "Path");
    php_info_print_table_row(2, "Modules", VIRIATUM_MODULES_PATH);
    php_info_print_table_row(2, "Contents", VIRIATUM_CONTENTS_PATH);
    php_info_print_table_row(2, "Resources", VIRIATUM_RESOURCES_PATH);
    php_info_print_table_row(2, "Configuration", VIRIATUM_CONFIG_PATH);
    php_info_print_table_end();
}

PHP_FUNCTION(viriatum_connections) {
    RETURN_LONG(_service->connections_list->size);
}

PHP_FUNCTION(viriatum_name) {
    RETURN_STRING(VIRIATUM_NAME, 1);
}

PHP_FUNCTION(viriatum_version) {
    RETURN_STRING(VIRIATUM_VERSION, 1);
}

PHP_FUNCTION(viriatum_description) {
    RETURN_STRING(VIRIATUM_DESCRIPTION, 1);
}

PHP_FUNCTION(viriatum_observations) {
    RETURN_STRING(VIRIATUM_OBSERVATIONS, 1);
}

PHP_FUNCTION(viriatum_copyright) {
    RETURN_STRING(VIRIATUM_COPYRIGHT, 1);
}

PHP_FUNCTION(viriatum_platform_string) {
    RETURN_STRING(VIRIATUM_PLATFORM_STRING, 1);
}

PHP_FUNCTION(viriatum_platform_cpu) {
    RETURN_STRING(VIRIATUM_PLATFORM_CPU, 1);
}

PHP_FUNCTION(viriatum_compilation_date) {
    RETURN_STRING(VIRIATUM_COMPILATION_DATE, 1);
}

PHP_FUNCTION(viriatum_compilation_time) {
    RETURN_STRING(VIRIATUM_COMPILATION_TIME, 1);
}

PHP_FUNCTION(viriatum_compiler) {
    RETURN_STRING(VIRIATUM_COMPILER, 1);
}
