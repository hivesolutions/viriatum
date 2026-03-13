/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "entry.h"

extern zend_function_entry viriatum_functions[16];
extern zend_module_entry viriatum_module;
extern sapi_module_struct viriatum_sapi_module;

int _module_startup(sapi_module_struct *module);
size_t _module_write(const char *data, size_t data_size);
void _module_flush(void *context);
struct stat *_module_stat(void);
char *_module_getenv(const char *name, size_t size);
int _module_header(sapi_header_struct *header, sapi_header_op_enum operation, sapi_headers_struct *headers);
int _module_send_headers(sapi_headers_struct *headers);
size_t _module_read_post(char *buffer, size_t size);
char *_module_read_cookies(void);
void _module_register(zval *_array);
void _module_log(const char *message, int syslog_type_int);
zend_result _module_request_time(double *request_time);
ZEND_MINFO_FUNCTION(viriatum_information);
PHP_FUNCTION(viriatum_connections_l);
PHP_FUNCTION(viriatum_connection_info);
PHP_FUNCTION(viriatum_connections);
PHP_FUNCTION(viriatum_uptime);
PHP_FUNCTION(viriatum_name);
PHP_FUNCTION(viriatum_version);
PHP_FUNCTION(viriatum_platform);
PHP_FUNCTION(viriatum_flags);
PHP_FUNCTION(viriatum_compiler);
PHP_FUNCTION(viriatum_compiler_version);
PHP_FUNCTION(viriatum_compilation_date);
PHP_FUNCTION(viriatum_compilation_time);
PHP_FUNCTION(viriatum_compilation_flags);
PHP_FUNCTION(viriatum_modules);
PHP_FUNCTION(viriatum_description);
