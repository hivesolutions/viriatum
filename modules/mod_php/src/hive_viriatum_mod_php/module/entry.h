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

#include "handler.h"
#include "extension.h"

/**
 * The path to the file to be used as default
 * in case no other definition exists (configuration).
 */
#define DEFAULT_FILE_PATH "default.php"

/**
 * Structure describing the internal
 * structures and information for the
 * mod php module.
 */
typedef struct ModPhpModule_t {
    /**
     * The http handler associated with the
     * module (upper layer).
     */
    struct HttpHandler_t *httpHandler;

    /**
     * The mod php http handler associated
     * with the module.
     */
    struct ModPhpHttpHandler_t *modPhpHttpHandler;
} ModPhpModule;

/**
 * The global reference to the currently loaded service
 * this is the reference required for the basic interaction
 * with the service.
 */
struct Service_t *_service;

/**
 * The global reference to the linked buffer to
 * be used to hold the various strings resulting
 * from the php default execution output.
 */
struct LinkedBuffer_t *_outputBuffer;

VIRIATUM_EXPORT_PREFIX ERROR_CODE createModPhpModule(struct ModPhpModule_t **modPhpModulePointer, struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE deleteModPhpModule(struct ModPhpModule_t *modPhpModule);
VIRIATUM_EXPORT_PREFIX ERROR_CODE startModule(struct Environment_t *environment, struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stopModule(struct Environment_t *environment, struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE infoModule(struct Module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE errorModule(unsigned char **messagePointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _loadConfiguration(struct Service_t *service, struct ModPhpHttpHandler_t *modPhpHttpHandler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _loadPhpState();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _unloadPhpState();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _reloaPhpState();
VIRIATUM_EXPORT_PREFIX ERROR_CODE _startPhpState();
VIRIATUM_EXPORT_PREFIX int _writePhpState(const char *data, unsigned int dataSize TSRMLS_DC);
VIRIATUM_EXPORT_PREFIX void _logPhpState(char *message);
VIRIATUM_EXPORT_PREFIX void _errorPhpState(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
