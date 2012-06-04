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

zend_function_entry viriatumFunctions[] = {
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

zend_module_entry viriatumModule = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "viriatum",
    viriatumFunctions,
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

sapi_module_struct viriatumSapiModule = {
    "viriatum_handler",
    "Viriatum PHP Handler",
    _moduleStartup,
    php_module_shutdown_wrapper,
    NULL,
    NULL,
    _moduleWrite,
    _moduleFlush,
    _moduleStat,
    _moduleGetenv,
    php_error,
    _moduleHeader,
    _moduleSendHeaders,
    NULL,
    _moduleReadPost,
    _moduleReadCookies,
    _moduleRegister,
    _moduleLog,
    _moduleRequestTime,
    NULL,
    STANDARD_SAPI_MODULE_PROPERTIES
};

int _moduleStartup(sapi_module_struct *module) {
    return php_module_startup(module, &viriatumModule, 1);
}

int _moduleWrite(const char *data, uint dataSize TSRMLS_DC) {
    /* allocates space for the buffer that will hold the write
    data that has just been sent to the write operation */
    char *buffer = MALLOC(dataSize + 1);
    buffer[dataSize] = '\0';

    /* copies the data into the buffer and then adds it to
    the current output linked buffer */
    memcpy(buffer, data, dataSize);
    appendLinkedBuffer(_outputBuffer, buffer, dataSize, 1);

    /* returns the size of the data that has just been
    writen into the internal structures */
    return dataSize;
}

void _moduleFlush(void *context) {
}

struct stat *_moduleStat(TSRMLS_D) {
    return NULL;
}

char *_moduleGetenv(char *name, size_t size TSRMLS_DC) {
    return NULL;
}

int _moduleHeader(sapi_header_struct *header, sapi_header_op_enum operation, sapi_headers_struct *headers TSRMLS_DC) {
    STRCPY(_phpRequest.headers[_phpRequest.headerCount], 1024, header->header);
    _phpRequest.headerCount++;
    return 0;
}

int _moduleSendHeaders(sapi_headers_struct *headers TSRMLS_DC) {
    STRCPY(_phpRequest.mimeType, 1024, headers->mimetype);
    return SAPI_HEADER_SENT_SUCCESSFULLY;
}

int _moduleReadPost(char *buffer, uint size TSRMLS_DC) {
    unsigned char *postData = _phpRequest.phpContext->postData;
    size_t contentLength = _phpRequest.phpContext->contentLength;
    size_t _size = contentLength > size ? size : contentLength;

    if(postData == NULL) { return 0; }

    memcpy(buffer, _phpRequest.phpContext->postData, _size);
    _phpRequest.phpContext->contentLength -= _size;
    _phpRequest.phpContext->postData += _size;

    return _size;
}

char *_moduleReadCookies(TSRMLS_D) {
    return (char *) _phpRequest.phpContext->cookie;
}

void _moduleRegister(zval *_array TSRMLS_DC) {
    /* allocates space for the address string reference
    and then retrieves the current connection's socket
    address structure to be used to retrieve the address
    string value (for exporting) */
    char *addressString;
    SOCKET_ADDRESS address = _connection->socketAddress;

    /* converts the address of the socket into the representing
    string value (for exporting the value) */
    addressString = inet_ntoa(((SOCKET_ADDRESS_INTERNET *) &address)->sin_addr);

    /* registers a series og global wide variable representing the
    current interface (critical for correct php interpreter usage) */
    php_register_variable_safe("PHP_SELF", "-", 1, _array TSRMLS_CC);
    php_register_variable_safe("GATEWAY_INTERFACE", "viriatum", sizeof("viriatum") - 1, _array TSRMLS_CC);
    php_register_variable_safe("QUERY_STRING", (char *) _phpRequest.phpContext->query, _phpRequest.phpContext->_queryString.length, _array TSRMLS_CC);
    php_register_variable_safe("REQUEST_TYPE", (char *) _phpRequest.phpContext->method, strlen(_phpRequest.phpContext->method), _array TSRMLS_CC);
    php_register_variable_safe("REMOTE_ADDR", addressString, strlen(addressString), _array TSRMLS_CC);
}

void _moduleLog(char *message TSRMLS_DC) {
    V_DEBUG_F("%s\n", message);
}

double _moduleRequestTime(TSRMLS_D) {
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
    RETURN_LONG(_service->connectionsList->size);
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
