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

ERROR_CODE createModPhpModule(struct ModPhpModule_t **modPhpModulePointer, struct Module_t *module) {
    /* retrieves the mod php module size */
    size_t modPhpModuleSize = sizeof(struct ModPhpModule_t);

    /* allocates space for the mod php module */
    struct ModPhpModule_t *modPhpModule = (struct ModPhpModule_t *) MALLOC(modPhpModuleSize);

    /* sets the mod php module attributes (default) values */
    modPhpModule->httpHandler = NULL;
    modPhpModule->modPhpHttpHandler = NULL;

    /* sets the mod php module in the (upper) module substrate */
    module->lower = (void *) modPhpModule;

    /* sets the mod php module in the module pointer */
    *modPhpModulePointer = modPhpModule;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteModPhpModule(struct ModPhpModule_t *modPhpModule) {
    /* releases the mod php module */
    FREE(modPhpModule);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE startModule(struct Environment_t *environment, struct Module_t *module) {
    /* allocates the mod php module */
    struct ModPhpModule_t *modPhpModule;

    /* allocates the http handler */
    struct HttpHandler_t *httpHandler;

    /* allocates the mod php http handler */
    struct ModPhpHttpHandler_t *modPhpHttpHandler;

    /* retrieves the name, version and description of
    the current module loaded */
    unsigned char *name = nameViriatumModPhp();
    unsigned char *version = versionViriatumModPhp();
    unsigned char *description = descriptionViriatumModPhp();

    /* retrieves the (environment) service */
    struct Service_t *service = environment->service;

    /* prints a debug message */
    V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);

    /* creates the mod php module */
    createModPhpModule(&modPhpModule, module);

    /* populates the module structure */
    infoModule(module);

    /* loads the php state populating all the erquired values
    for state initialization */
    _loadPhpState();

    /* creates the http handler */
    service->createHttpHandler(service, &httpHandler, (unsigned char *) "php");

    /* creates the mod php http handler */
    createModPhpHttpHandler(&modPhpHttpHandler, httpHandler);

    /* sets the http handler attributes */
    httpHandler->set = setHandlerModule;
    httpHandler->unset = unsetHandlerModule;
    httpHandler->reset = NULL;

    /* sets the mod php handler attributes */
    modPhpHttpHandler->filePath = DEFAULT_FILE_PATH;
    modPhpHttpHandler->fileDirty = 1;

    /* sets the mod php module attributes */
    modPhpModule->httpHandler = httpHandler;
    modPhpModule->modPhpHttpHandler = modPhpHttpHandler;

    /* adds the http handler to the service */
    service->addHttpHandler(service, httpHandler);

    /* loads the service configuration for the http handler
    this should change some of it's behavior */
    _loadConfiguration(service, modPhpHttpHandler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopModule(struct Environment_t *environment, struct Module_t *module) {
    printf("Stoping PHP");

    /*TODO: TENHO DE POR O PHP A FECHAR BEM */


    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE infoModule(struct Module_t *module) {
    /* retrieves the name */
    unsigned char *name = nameViriatumModPhp();

    /* retrieves the version */
    unsigned char *version = versionViriatumModPhp();

    /* populates the module structure */
    module->name = name;
    module->version = version;
    module->type = MODULE_TYPE_HTTP_HANDLER;
    module->start = startModule;
    module->stop = stopModule;
    module->info = infoModule;
    module->error = errorModule;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE errorModule(unsigned char **messagePointer) {
    /* sets the error message in the (error) message pointer */
    *messagePointer = getLastErrorMessage();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _loadConfiguration(struct Service_t *service, struct ModPhpHttpHandler_t *modPhpHttpHandler) {
    /* raises no error */
    RAISE_NO_ERROR;
}

int myapp_php_ub_write(const char *data, unsigned int dataSize TSRMLS_DC) {
    _inputbuffer = MALLOC(dataSize + 1);
    _inputbuffer[dataSize] = '\0';

    _inputbufferSize = dataSize;

    memcpy(_inputbuffer, data, dataSize);

    /*appendValueLinkedList(_outputBuffer, (void **) _data);*/

    /*TODO: MONTES DE LEAKING por causa do malloc */

    return dataSize;
}

ERROR_CODE _loadPhpState() {
    /* creates an array for the default initialization arguments,
    these arguments are going to be sent to the php virtual machine */
    char *args[1] = { "default" };

    /* sets the proper write fucntion for the ouput of the php execution
    this is equivalent to a redirect in the standard output */
    php_embed_module.ub_write = myapp_php_ub_write;

    /* runs the start block for the php interpreter, this should
    be able to start all the internal structures */
    php_embed_init(1, args);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unloadPhpState() {
    /* runs the stop block for the php interpreter, this should
    be able to stop all the internal structures */
    php_embed_shutdown(TSRMLS_C);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _reloadPhpState() {
    _unloadPhpState();
    _loadPhpState();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _startPhpState() {
    /* raises no error */
    RAISE_NO_ERROR;
}
