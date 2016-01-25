/*
 Hive Viriatum Modules
 Copyright (C) 2008-2016 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "entry.h"

/* starts the memory structures */
START_MEMORY;

ERROR_CODE create_mod_gif_module(struct mod_gif_module_t **mod_gif_module_pointer, struct module_t *module) {
    /* retrieves the mod gif module size and uses it
    to allocate he module structure for it */
    size_t mod_gif_module_size = sizeof(struct mod_gif_module_t);
    struct mod_gif_module_t *mod_gif_module = (struct mod_gif_module_t *) MALLOC(mod_gif_module_size);

    /* sets the mod gif module attributes (default) values
    then updates the lower part of the module with the gif
    module and updates the pointer to the module*/
    mod_gif_module->http_handler = NULL;
    module->lower = (void *) mod_gif_module;
    *mod_gif_module_pointer = mod_gif_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_gif_module(struct mod_gif_module_t *mod_gif_module) {
    /* releases the mod gif module memory (avoiding any memory
    leak) and the returns to the caller method */
    FREE(mod_gif_module);
    RAISE_NO_ERROR;
}

ERROR_CODE start_module_gif(struct environment_t *environment, struct module_t *module) {
    /* allocates the http handler pointer value to be used
    in the creation of the "new" http handler */
    struct http_handler_t *http_handler;

    /* allocates the memory for the referece to mod gid module
    structure used to store the details about this module */
    struct mod_gif_module_t *mod_gif_module;

    /* retrieves the name, version and description of
    the current module loaded, values to be displayed */
    unsigned char *name = name_viriatum_mod_gif();
    unsigned char *version = version_viriatum_mod_gif();
    unsigned char *description = description_viriatum_mod_gif();

    /* retrieves the (environment) service that is going
    to be used for certain creation operations */
    struct service_t *service = environment->service;

    /* prints a debug message about the module that is
    currently being loaded */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* creates the mo gif module with the provided module reference
    and then initializes the new gif module with the currently
    defined state for it including definitions */
    create_mod_gif_module(&mod_gif_module, module);
    info_module_gif(module);

    /* creates a new http handler for the gif module, this
    is the handler structure to be used in the request */
    service->create_http_handler(service, &http_handler, (unsigned char *) "gif");

    /* sets the http handler attributes */
    http_handler->resolve_index = 0;
    http_handler->set = set_handler_gif;
    http_handler->unset = unset_handler_gif;
    http_handler->reset = NULL;

    /* sets the mod gif module attributes */
    mod_gif_module->http_handler = http_handler;

    /* adds the http handler to the service, this operation
    should enable the handling of request using the (empty) gif
    based handler, in case they are correclty routed */
    service->add_http_handler(service, http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_module_gif(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_gif();
    unsigned char *version = version_viriatum_mod_gif();
    unsigned char *description = description_viriatum_mod_gif();

    /* retrieves the (environment) service, that is going to
    be used for certain global operations */
    struct service_t *service = environment->service;

    /* retrieves the mod gif module (from the module) and uses
    it to retrieve the create http handler */
    struct mod_gif_module_t *mod_gif_module = (struct  mod_gif_module_t *) module->lower;
    struct http_handler_t *http_handler = mod_gif_module->http_handler;

    /* prints a debug message about the unloading of the current
    module, for debugging purposes */
    V_DEBUG_F("Stopping the module '%s' (%s) v%s\n", name, description, version);

    /* removes the http handler from the service, it can no longer
    be used to handle any request from this point on, then deletes
    both the http handler and the mod gif module (to avoid memory leaks) */
    service->remove_http_handler(service, http_handler);
    if(http_handler != NULL) { service->delete_http_handler(service, http_handler); }
    delete_mod_gif_module(mod_gif_module);

    /* cleans up the pool based memory allocation system releasing all
    of its memory before the exit (no leaks) then returns the control
    flow to the caller function with success state */
    cleanup_palloc();
    RAISE_NO_ERROR;
}

ERROR_CODE info_module_gif(struct module_t *module) {
    /* retrieves the various elemnts that are going
    to be used in the contruction of the of the module */
    unsigned char *name = name_viriatum_mod_gif();
    unsigned char *name_s = name_s_viriatum_mod_gif();
    unsigned char *version = version_viriatum_mod_gif();

    /* populates the module structure with various
    attributes from the current module */
    module->name = name;
    module->name_s = name_s;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = start_module_gif;
    module->stop = stop_module_gif;
    module->info = info_module_gif;
    module->error = error_module_gif;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE error_module_gif(unsigned char **message_pointer) {
    /* sets the message pointer with the current last
    error message then returns normally */
    *message_pointer = get_last_error_message();
    RAISE_NO_ERROR;
}
