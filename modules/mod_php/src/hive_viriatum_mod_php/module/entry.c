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

unsigned char local = 0;

ERROR_CODE create_mod_php_module(struct mod_php_module_t **mod_php_module_pointer, struct module_t *module) {
    /* retrieves the mod php module size */
    size_t mod_php_module_size = sizeof(struct mod_php_module_t);

    /* allocates space for the mod php module */
    struct mod_php_module_t *mod_php_module = (struct mod_php_module_t *) MALLOC(mod_php_module_size);

    /* sets the mod php module attributes (default) values */
    mod_php_module->http_handler = NULL;
    mod_php_module->mod_php_http_handler = NULL;

    /* sets the mod php module in the (upper) module substrate */
    module->lower = (void *) mod_php_module;

    /* sets the mod php module in the module pointer */
    *mod_php_module_pointer = mod_php_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_php_module(struct mod_php_module_t *mod_php_module) {
    /* releases the mod php module */
    FREE(mod_php_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE start_module(struct environment_t *environment, struct module_t *module) {
    /* allocates the mod php module */
    struct mod_php_module_t *mod_php_module;

    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates the mod php http handler */
    struct mod_php_http_handler_t *mod_php_http_handler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_php();
    unsigned char *version = version_viriatum_mod_php();
    unsigned char *description = description_viriatum_mod_php();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* sets the global service reference to be used in the
    externalized function for the interpreter then updates
    the global local value according to the service options */
    _service = service;
    local = service->options->local;

    /* creates the mod php module */
    create_mod_php_module(&mod_php_module, module);

    /* populates the module structure */
    info_module(module);

    /* loads the php state populating all the required values
    for state initialization */
    _load_php_state();

    /* creates the http handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "php");

    /* creates the mod php http handler */
    create_mod_php_http_handler(&mod_php_http_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->resolve_index = 0;
    http_handler->set = set_handler_module;
    http_handler->unset = unset_handler_module;
    http_handler->reset = NULL;

    /* sets the mod php handler attributes */
    mod_php_http_handler->base_path = DEFAULT_BASE_PATH;

    /* sets the mod php module attributes */
    mod_php_module->http_handler = http_handler;
    mod_php_module->mod_php_http_handler = mod_php_http_handler;

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* loads the service configuration for the http handler
    this should change some of it's behavior */
    _load_configuration(service, mod_php_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_module(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_php();
    unsigned char *version = version_viriatum_mod_php();
    unsigned char *description = description_viriatum_mod_php();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* retrieves the mod php module (from the module) */
    struct mod_php_module_t *mod_php_module = (struct  mod_php_module_t *) module->lower;

    /* retrieves the http handler from the mod php module */
    struct http_handler_t *http_handler = mod_php_module->http_handler;

    /* retrieves the mod php http handler from the mod php module */
    struct mod_php_http_handler_t *mod_php_http_handler = mod_php_module->mod_php_http_handler;

    /* prints a debug message */
    V_DEBUG_F("Stopping the module '%s' (%s) v%s\n", name, description, version);

    /* removes the http handler from the service */
    service->remove_http_handler(service, http_handler);

    /* in case the mod php http handler is valid and
    initialized (correct state) */
    if(mod_php_http_handler != NULL) {
        /* deletes the mod php http handler */
        delete_mod_php_http_handler(mod_php_http_handler);
    }

    /* in case the http handler is valid and
    initialized (correct state) */
    if(http_handler != NULL) {
        /* deletes the http handler */
        service->delete_http_handler(service, http_handler);
    }

    /* unloads the php state destroying all the required values
    for state destroyed */
    _unload_php_state();

    /* deletes the mod php module */
    delete_mod_php_module(mod_php_module);

    /* sets the global service reference this is no longer necessary
    because the module has been unloaded */
    _service = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE info_module(struct module_t *module) {
    /* retrieves the name */
    unsigned char *name = name_viriatum_mod_php();

    /* retrieves the version */
    unsigned char *version = version_viriatum_mod_php();

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

ERROR_CODE _load_configuration(struct service_t *service, struct mod_php_http_handler_t *mod_php_http_handler) {
    /* allocates space for both a configuration item reference
    (value) and for the configuration to be retrieved */
    void *value;
    struct sort_map_t *configuration;

    /* in case the current service configuration is not set
    must return immediately (not possible to load it) */
    if(service->configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the mod php section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(service->configuration, (unsigned char *) "mod_php", (void **) &configuration);
    if(configuration == NULL) { RAISE_NO_ERROR; }

    /* tries ro retrieve the base path from the php configuration and in
    case it exists sets it in the mod php handler (attribute reference change) */
    get_value_string_sort_map(configuration, (unsigned char *) "base_path", &value);
    if(value != NULL) { mod_php_http_handler->base_path = (char *) value; }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_php_state() {
    /* sets the proper functions for the ouput of the php execution
    this is equivalent to a redirect in the standard output and error */
    php_embed_module.ub_write = _write_php_state;
    php_embed_module.log_message = _log_php_state;
    php_embed_module.sapi_error = _error_php_state;

    /* sets a series of default handlers (callbacks) for the viriatum
    sapi module (required for stability issues) */
    viriatum_sapi_module.default_post_reader = php_default_post_reader;
    viriatum_sapi_module.treat_data = php_default_treat_data;
    viriatum_sapi_module.input_filter = php_default_input_filter;

    /* runs the start block for the php interpreter, this should
    be able to start all the internal structures, then loads the
    viriatum module to export the proper features */
    sapi_startup(&viriatum_sapi_module);
    viriatum_sapi_module.startup(&viriatum_sapi_module);

    /* forrces the logging of the error for the execution in the
    current php environment */
    zend_alter_ini_entry("display_errors", sizeof("display_errors"), "0", sizeof("0") - 1, PHP_INI_SYSTEM, PHP_INI_STAGE_RUNTIME);
    zend_alter_ini_entry("log_errors", sizeof("log_errors"), "1", sizeof("1") - 1, PHP_INI_SYSTEM, PHP_INI_STAGE_RUNTIME);

    /* starts the php state updating the major global value in
    the current interpreter state */
    _start_php_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unload_php_state() {
    /* runs the stop block for the php interpreter, this should
    be able to stop all the internal structures */
    php_module_shutdown(TSRMLS_C);
    sapi_shutdown();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reload_php_state() {
    _unload_php_state();
    _load_php_state();

    /* raises no error */
    RAISE_NO_ERROR;
}

#ifdef _MSC_VER
#pragma warning(disable:4700)
#endif
ERROR_CODE _start_php_state() {
    /* raises no error */
    RAISE_NO_ERROR;
}
#ifdef _MSC_VER
#pragma warning(default:4700)
#endif

int _write_php_state(const char *data, unsigned int data_size TSRMLS_DC) {
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

void _log_php_state(char *message) {
    /* logs the error message (critical error) */
    V_ERROR_F("%s\n", message);
}

void _error_php_state(int type, const char *message, ...) {
    /* check if the kind of error is of type critical in such case it should
    return immediately */
    if (type != E_ERROR && type != E_USER_ERROR && type != E_CORE_ERROR && type != E_PARSE && type != E_COMPILE_ERROR) {
        /* returns immediately no need to continue */
        return;
    }

    /* logs the error message (critical error) */
    V_ERROR("Critical error in user code\n");

    /* exits the current code (code jump) */
    zend_bailout();
}
