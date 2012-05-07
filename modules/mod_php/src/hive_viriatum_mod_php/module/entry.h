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
 * The global reference to the string buffer to
 * be used to hold the various strings resulting
 * from the php default execution output.
 */
struct LinkedList_t *_outputBuffer;

char *_inputbuffer;
size_t _inputbufferSize;





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
