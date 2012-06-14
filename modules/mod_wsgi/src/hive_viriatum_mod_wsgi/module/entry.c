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

#include "entry.h"

ERROR_CODE create_mod_wsgi_module(struct mod_wsgi_module_t **mod_wsgi_module_pointer, struct module_t *module) {
    /* retrieves the mod wsgi module size */
    size_t mod_wsgi_module_size = sizeof(struct mod_wsgi_module_t);

    /* allocates space for the mod wsgi module */
    struct mod_wsgi_module_t *mod_wsgi_module = (struct mod_wsgi_module_t *) MALLOC(mod_wsgi_module_size);

    /* sets the mod wsgi module attributes (default) values */
    mod_wsgi_module->http_handler = NULL;
    mod_wsgi_module->mod_wsgi_http_handler = NULL;

    /* sets the mod wsgi module in the (upper) module substrate */
    module->lower = (void *) mod_wsgi_module;

    /* sets the mod wsgi module in the module pointer */
    *mod_wsgi_module_pointer = mod_wsgi_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_wsgi_module(struct mod_wsgi_module_t *mod_wsgi_module) {
    /* releases the mod wsgi module */
    FREE(mod_wsgi_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE start_module(struct environment_t *environment, struct module_t *module) {
    /* allocates the mod wsgi module */
    struct mod_wsgi_module_t *mod_wsgi_module;

    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates the mod wsgi http handler */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_wsgi();
    unsigned char *version = version_viriatum_mod_wsgi();
    unsigned char *description = description_viriatum_mod_wsgi();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* sets the global service reference to be used in the
    externalized function for the interpreter */
    _service = service;

    /* creates the mod wsgi module */
    create_mod_wsgi_module(&mod_wsgi_module, module);

    /* populates the module structure */
    info_module(module);

    /* loads the wsgi state populating all the required values
    for state initialization */
    _load_wsgi_state();

    /* creates the http handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "wsgi");

    /* creates the mod wsgi http handler */
    create_mod_wsgi_http_handler(&mod_wsgi_http_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->set = set_handler_module;
    http_handler->unset = unset_handler_module;
    http_handler->reset = NULL;

    /* sets the mod wsgi module attributes */
    mod_wsgi_module->http_handler = http_handler;
    mod_wsgi_module->mod_wsgi_http_handler = mod_wsgi_http_handler;

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* loads the service configuration for the http handler
    this should change some of it's behavior */
    _load_configuration(service, mod_wsgi_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_module(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_wsgi();
    unsigned char *version = version_viriatum_mod_wsgi();
    unsigned char *description = description_viriatum_mod_wsgi();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* retrieves the mod wsgi module (from the module) */
    struct mod_wsgi_module_t *mod_wsgi_module = (struct  mod_wsgi_module_t *) module->lower;

    /* retrieves the http handler from the mod wsgi module */
    struct http_handler_t *http_handler = mod_wsgi_module->http_handler;

    /* retrieves the mod wsgi http handler from the mod wsgi module */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler = mod_wsgi_module->mod_wsgi_http_handler;

    /* prints a debug message */
    V_DEBUG_F("Stoping the module '%s' (%s) v%s\n", name, description, version);

    /* removes the http handler from the service */
    service->remove_http_handler(service, http_handler);

    /* in case the mod wsgi http handler is valid and
    initialized (correct state) */
    if(mod_wsgi_http_handler != NULL) {
        /* deletes the mod wsgi http handler */
        delete_mod_wsgi_http_handler(mod_wsgi_http_handler);
    }

    /* in case the http handler is valid and
    initialized (correct state) */
    if(http_handler != NULL) {
        /* deletes the http handler */
        service->delete_http_handler(service, http_handler);
    }

    /* unloads the wsgi state destroying all the required values
    for state destroyed */
    _unload_wsgi_state();

    /* deletes the mod wsgi module */
    delete_mod_wsgi_module(mod_wsgi_module);

    /* sets the global service reference this is no longer necessary
    because the module has been unloaded */
    _service = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE info_module(struct module_t *module) {
    /* retrieves the name */
    unsigned char *name = name_viriatum_mod_wsgi();

    /* retrieves the version */
    unsigned char *version = version_viriatum_mod_wsgi();

    /* populates the module structure */
    module->name = name;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = start_module;
    module->stop = stop_module;
    module->info = info_module;
    module->error = error_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE error_module(unsigned char **message_pointer) {
    /* sets the error message in the (error) message pointer */
    *message_pointer = get_last_error_message();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_configuration(struct service_t *service, struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_wsgi_state() {
    /* starts the python interpreter initializing all the resources
    related with the virtual machine */
    Py_Initialize();

    /* starts the wsgi state updating the major global value in
    the current interpreter state */
    _start_wsgi_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unload_wsgi_state() {
    /* shutsdown the python interpreter, releasing all the resources
    associated with it (everything is destroyed) */
    Py_Finalize();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reload_wsgi_state() {
    _unload_wsgi_state();
    _load_wsgi_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _start_wsgi_state() {
	/* sets the current program name in the python interpreter
	the name used is the same as the process running the service */
	Py_SetProgramName((char *) _service->program_name);

    /* registers the viriatum wsgi module in the python interpreter
    this module may be used to provide wsgi functions */
    Py_InitModule("viriatum_wsgi", wsgi_methods);

    /* raises no error */
    RAISE_NO_ERROR;
}
