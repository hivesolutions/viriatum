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

ERROR_CODE create_mod_lua_module(struct mod_lua_module_t **mod_lua_module_pointer, struct module_t *module) {
    /* retrieves the mod lua module size */
    size_t mod_lua_module_size = sizeof(struct mod_lua_module_t);

    /* allocates space for the mod lua module */
    struct mod_lua_module_t *mod_lua_module = (struct mod_lua_module_t *) MALLOC(mod_lua_module_size);

     /* sets the mod lua module attributes (default) values */
    mod_lua_module->lua_state = NULL;
    mod_lua_module->http_handler = NULL;
    mod_lua_module->mod_lua_http_handler = NULL;

    /* sets the mod lua module in the (upper) module substrate */
    module->lower = (void *) mod_lua_module;

    /* sets the mod lua module in the module pointer */
    *mod_lua_module_pointer = mod_lua_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_lua_module(struct mod_lua_module_t *mod_lua_module) {
    /* releases the mod lua module */
    FREE(mod_lua_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE start_module(struct environment_t *environment, struct module_t *module) {
    /* allocates the lua state */
    lua_State *lua_state;

    /* allocates the mod lua module */
    struct mod_lua_module_t *mod_lua_module;

    /* allocates the http handler */
    struct http_handler_t *http_handler;

    /* allocates the mod lua http handler */
    struct mod_lua_http_handler_t *mod_lua_http_handler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_lua();
    unsigned char *version = version_viriatum_mod_lua();
    unsigned char *description = description_viriatum_mod_lua();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* creates the mod lua module */
    create_mod_lua_module(&mod_lua_module, module);

    /* populates the module structure */
    info_module(module);

    /* loads the lua state populating all the erquired values
    for state initialization */
    _load_lua_state(&lua_state);

    /* creates the http handler */
    service->create_http_handler(service, &http_handler, (unsigned char *) "lua");

    /* creates the mod lua http handler */
    create_mod_lua_http_handler(&mod_lua_http_handler, http_handler);

    /* sets the http handler attributes */
    http_handler->set = set_handler_module;
    http_handler->unset = unset_handler_module;
    http_handler->reset = NULL;

    /* sets the mod lua handler attributes */
    mod_lua_http_handler->lua_state = lua_state;
    mod_lua_http_handler->file_path = DEFAULT_FILE_PATH;
    mod_lua_http_handler->file_dirty = 1;

    /* sets the mod lua module attributes */
    mod_lua_module->lua_state = lua_state;
    mod_lua_module->http_handler = http_handler;
    mod_lua_module->mod_lua_http_handler = mod_lua_http_handler;

    /* adds the http handler to the service */
    service->add_http_handler(service, http_handler);

    /* loads the service configuration for the http handler
    this should change some of it's behavior */
    _load_configuration(service, mod_lua_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stop_module(struct environment_t *environment, struct module_t *module) {
    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = name_viriatum_mod_lua();
    unsigned char *version = version_viriatum_mod_lua();
    unsigned char *description = description_viriatum_mod_lua();

    /* retrieves the (environment) service */
    struct service_t *service = environment->service;

    /* retrieves the mod lua module (from the module) */
    struct mod_lua_module_t *mod_lua_module = (struct  mod_lua_module_t *) module->lower;

    /* retrieves the http handler from the mod lua module */
    struct http_handler_t *http_handler = mod_lua_module->http_handler;

    /* retrieves the mod lua http handler from the mod lua module */
    struct mod_lua_http_handler_t *mod_lua_http_handler = mod_lua_module->mod_lua_http_handler;

    /* retrieves the lua state from the mod lua http handler */
    lua_State *lua_state = mod_lua_http_handler->lua_state;

    /* prints a debug message */
    V_DEBUG_F("Stopping the module '%s' (%s) v%s\n", name, description, version);

    /* removes the http handler from the service */
    service->remove_http_handler(service, http_handler);

    /* in case the mod lua http handler is valid and
    initialized (correct state) */
    if(mod_lua_http_handler != NULL) {
        /* deletes the mod lua http handler */
        delete_mod_lua_http_handler(mod_lua_http_handler);
    }

    /* in case the http handler is valid and
    initialized (correct state) */
    if(http_handler != NULL) {
        /* deletes the http handler */
        service->delete_http_handler(service, http_handler);
    }

    /* in case the lua state is valid and
    initialized (correct state) */
    if(lua_state != NULL) {
        /* cleanup lua */
        lua_close(lua_state);
    }

    /* deletes the mod lua module */
    delete_mod_lua_module(mod_lua_module);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE info_module(struct module_t *module) {
    /* retrieves the name */
    unsigned char *name = name_viriatum_mod_lua();

    /* retrieves the version */
    unsigned char *version = version_viriatum_mod_lua();

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

ERROR_CODE _load_configuration(struct service_t *service, struct mod_lua_http_handler_t *mod_lua_http_handler) {
    /* allocates space for both a configuration item reference
    (value) and for the configuration to be retrieved */
    void *value;
    struct sort_map_t *configuration;

    /* in case the current service configuration is not set
    must return immediately (not possible to load it) */
    if(service->configuration == NULL) { RAISE_NO_ERROR; }

    /* tries to retrieve the mod lua section configuration from the configuration
    map in case none is found returns immediately no need to process anything more */
    get_value_string_sort_map(service->configuration, (unsigned char *) "mod_lua", (void **) &configuration);
    if(configuration == NULL) { RAISE_NO_ERROR; }

    /* tries ro retrieve the script path from the lua configuration and in
    case it exists sets it in the mod lua handler (attribute reference change) */
    get_value_string_sort_map(configuration, (unsigned char *) "script_path", &value);
    if(value != NULL) { mod_lua_http_handler->file_path = (char *) value; }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _load_lua_state(lua_State **lua_state_pointer) {
    /* initializes the lua interpreter, then loads
    various (default) lua libraries and then starts
    the global values in the environment (symbol injection) */
    lua_State *lua_state = lua_open();
    luaL_openlibs(lua_state);
    _start_lua_state(lua_state);

    /* sets the lua state in the pointer reference */
    *lua_state_pointer = lua_state;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unload_lua_state(lua_State **lua_state_pointer) {
    lua_State *lua_state = *lua_state_pointer;

    lua_close(lua_state);
    *lua_state_pointer = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reload_lua_state(lua_State **lua_state_pointer) {
    _unload_lua_state(lua_state_pointer);
    _load_lua_state(lua_state_pointer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _start_lua_state(lua_State *lua_state) {
    /* registers the viriatum name in lua */
    lua_pushstring(lua_state, VIRIATUM_NAME);
    lua_setglobal(lua_state, "VIRIATUM_NAME");

    /* registers the viriatum version in lua */
    lua_pushstring(lua_state, VIRIATUM_VERSION);
    lua_setglobal(lua_state, "VIRIATUM_VERSION");

    /* registers the viriatum description in lua */
    lua_pushstring(lua_state, VIRIATUM_DESCRIPTION);
    lua_setglobal(lua_state, "VIRIATUM_DESCRIPTION");

    /* registers the viriatum observations in lua */
    lua_pushstring(lua_state, VIRIATUM_OBSERVATIONS);
    lua_setglobal(lua_state, "VIRIATUM_OBSERVATIONS");

    /* registers the viriatum copyright in lua */
    lua_pushstring(lua_state, VIRIATUM_COPYRIGHT);
    lua_setglobal(lua_state, "VIRIATUM_COPYRIGHT");

    /* registers the viriatum platform string in lua */
    lua_pushstring(lua_state, VIRIATUM_PLATFORM_STRING);
    lua_setglobal(lua_state, "VIRIATUM_PLATFORM_STRING");

    /* registers the viriatum platform cpu in lua */
    lua_pushstring(lua_state, VIRIATUM_PLATFORM_CPU);
    lua_setglobal(lua_state, "VIRIATUM_PLATFORM_CPU");

    /* raises no error */
    RAISE_NO_ERROR;
}
