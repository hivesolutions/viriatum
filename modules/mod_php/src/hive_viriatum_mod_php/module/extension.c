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
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    VIRIATUM_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

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
