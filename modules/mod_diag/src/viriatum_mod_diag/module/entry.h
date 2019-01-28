/*
 Hive Viriatum Modules
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "handler.h"

/**
 * Structure describing the internal
 * structures and information for the
 * mod diag module.
 */
typedef struct mod_diag_module_t {
    /**
     * The HTTP handler associated with the
     * module (upper layer).
     */
    struct http_handler_t *http_handler;
} mod_diag_module;

VIRIATUM_EXPORT_PREFIX ERROR_CODE create_mod_diag_module(struct mod_diag_module_t **mod_diag_module_pointer, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_mod_diag_module(struct mod_diag_module_t *mod_diag_module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE start_module_diag(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE stop_module_diag(struct environment_t *environment, struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE info_module_diag(struct module_t *module);
VIRIATUM_EXPORT_PREFIX ERROR_CODE error_module_diag(unsigned char **message_pointer);
