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

#pragma once

#include "entry.h"

zend_function_entry viriatum_functions[12];
zend_module_entry viriatum_module;
sapi_module_struct viriatum_sapi_module;

int _module_startup(sapi_module_struct *module);
int _module_write(const char *data, uint data_size TSRMLS_DC);
void _module_flush(void *context);
struct stat *_module_stat(TSRMLS_D);
char *_module_getenv(char *name, size_t size TSRMLS_DC);
int _module_header(sapi_header_struct *header, sapi_header_op_enum operation, sapi_headers_struct *headers TSRMLS_DC);
int _module_send_headers(sapi_headers_struct *headers TSRMLS_DC);
int _module_read_post(char *buffer, uint size TSRMLS_DC);
char *_module_read_cookies(TSRMLS_D);
void _module_register(zval *_array TSRMLS_DC);
void _module_log(char *message TSRMLS_DC);
double _module_request_time(TSRMLS_D);
ZEND_MINFO_FUNCTION(viriatum_information);
PHP_FUNCTION(viriatum_connections);
PHP_FUNCTION(viriatum_name);
PHP_FUNCTION(viriatum_version);
PHP_FUNCTION(viriatum_description);
PHP_FUNCTION(viriatum_observations);
PHP_FUNCTION(viriatum_copyright);
PHP_FUNCTION(viriatum_platform_string);
PHP_FUNCTION(viriatum_platform_cpu);
PHP_FUNCTION(viriatum_compilation_date);
PHP_FUNCTION(viriatum_compilation_time);
PHP_FUNCTION(viriatum_compiler);
