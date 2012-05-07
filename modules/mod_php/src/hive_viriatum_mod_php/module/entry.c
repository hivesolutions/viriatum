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
	modPhpModule->tobias = 2;

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
    /* prints a debug message */
    /*V_DEBUG_F("Starting the module '%s' (%s) v%s\n", name, description, version);*/
	char *args[1] = { "default" };

	printf("Starting PHP...");

	PHP_EMBED_START_BLOCK(1, args)
    zend_eval_string("echo 'Hello World';", NULL, "Embedded Code" TSRMLS_CC);
    PHP_EMBED_END_BLOCK()

	printf("started\n");

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE stopModule(struct Environment_t *environment, struct Module_t *module) {
    printf("Stoping PHP");
	
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
