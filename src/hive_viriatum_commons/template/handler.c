/*
 Hive Viriatum Commons
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "handler.h"

void createTemplateHandler(struct TemplateHandler_t **templateHandlerPointer) {
    /* retrieves the template handler size */
    size_t templateHandlerSize = sizeof(struct TemplateHandler_t);

    /* allocates space for the template handler */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) MALLOC(templateHandlerSize);

    /* sets the default values in the template handler */
    templateHandler->nodeCount = 0;
    templateHandler->currentNode = NULL;
    templateHandler->temporaryNode = NULL;
    templateHandler->nodes = NULL;

    /* sets the template engine in the template handler pointer */
    *templateHandlerPointer = templateHandler;
}

void deleteTemplateHandler(struct TemplateHandler_t *templateHandler) {
    /* releases the template handler */
    FREE(templateHandler);
}

void createTemplateNode(struct TemplateNode_t **templateNodePointer, enum TemplateNodeType_t type) {
    /* retrieves the template node size */
    size_t templateNodeSize = sizeof(struct TemplateNode_t);

    /* allocates space for the template node */
    struct TemplateNode_t *templateNode = (struct TemplateNode_t *) MALLOC(templateNodeSize);

    /* sets the (node9 type in the template node */
    templateNode->type = type;

    /* sets the default values in the template node */
    templateNode->childCount = 0;
    templateNode->parameterCount = 0;
    templateNode->name = NULL;
    templateNode->children = NULL;
    templateNode->parameters = NULL;

    /* sets the template engine in the template node pointer */
    *templateNodePointer = templateNode;
}

void deleteTemplateNode(struct TemplateNode_t *templateNode) {
    /* releases the template node */
    FREE(templateNode);
}

ERROR_CODE textBegin(struct TemplateEngine_t *templateEngine) {
    printf("TEXT_BEGIN\n");

    RAISE_NO_ERROR;
}

ERROR_CODE textEnd(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    char buffer[1024];

    memcpy(buffer, pointer, size);
    buffer[size] = '\0';

    printf("TEXT_END: '%s'\n", buffer);

    RAISE_NO_ERROR;
}

ERROR_CODE tagBegin(struct TemplateEngine_t *templateEngine) {
    /* allocates space for the template node */
    struct TemplateNode_t *templateNode;

    /* retrieves the template handler from the template engine context */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;

    /* creates a new template node and sets the template
    node as the temporary node in the template handler */
    createTemplateNode(&templateNode, TEMPLATE_NODE_OPEN);
    templateHandler->temporaryNode = templateNode;

    printf("TAG_BEGIN\n");

    RAISE_NO_ERROR;
}

ERROR_CODE tagCloseBegin(struct TemplateEngine_t *templateEngine) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;

    /* sets the temporary node type as close */
    temporaryNode->type = TEMPLATE_NODE_CLOSE;

    printf("TAG_CLOSE_BEGIN\n");

    RAISE_NO_ERROR;
}

ERROR_CODE tagEnd(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    char buffer[1024];

    memcpy(buffer, pointer, size);
    buffer[size] = '\0';

    printf("TAG_END: '%s'\n", buffer);

    RAISE_NO_ERROR;
}

ERROR_CODE tagName(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    temporaryNode->name = (unsigned char *) malloc(size + 1);
    memcpy(temporaryNode->name, pointer, size);
    temporaryNode->name[size] = '\0';

    printf("TAG_NAME: '%s'\n", temporaryNode->name);

    RAISE_NO_ERROR;
}

ERROR_CODE parameter(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;

	/* allocates space for the parameter to be created */
	struct TemplateParameter_t *templateParameter;


	/* TODO REMOVE THIS */
	char buffer[1024];


	/* in case the children are not defined for the
	temporary node */
	if(temporaryNode->children == NULL) {
		/* creates a new linked list for the children (nodes) */
		createLinkedList(&temporaryNode->children);
	}

	/* creates the template parameter */
	/*createTemplateParameter(templateParameter);*/




    memcpy(buffer, pointer, size);
    buffer[size] = '\0';

    printf("PARAMETER: '%s'\n", buffer);

    RAISE_NO_ERROR;
}

ERROR_CODE parameterValue(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    char buffer[1024];

    memcpy(buffer, pointer, size);
    buffer[size] = '\0';

    printf("VALUE: '%s'\n", buffer);

    RAISE_NO_ERROR;
}

void processTemplateHandler(struct TemplateHandler_t *templateHandler, unsigned char *filePath) {
     /* allocates space for the template engine */
    struct TemplateEngine_t *templateEngine;

    /* allocates space for the template settings */
    struct TemplateSettings_t *templateSettings;

    struct TemplateNode_t *rootNode;

    /* creates the template engine */
    createTemplateEngine(&templateEngine);

    /* creates the template engine */
    createTemplateSettings(&templateSettings);

    /* creates the root node and sets it as the initial
    current node */
    createTemplateNode(&rootNode, TEMPLATE_NODE_ROOT);
    templateHandler->currentNode = rootNode;

    /* sets the context (template handler) in the template engine */
    templateEngine->context = templateHandler;

    /* sets the various callbacks in the template settings */
    templateSettings->ontextBegin = textBegin;
    templateSettings->ontextEnd = textEnd;
    templateSettings->ontagBegin = tagBegin;
    templateSettings->ontagCloseBegin = tagCloseBegin;
    templateSettings->ontagEnd = tagEnd;
    templateSettings->ontagName = tagName;
    templateSettings->onparameter = parameter;
    templateSettings->onparameterValue = parameterValue;

    /* processes the file as a template engine */
    processTemplateEngine(templateEngine, templateSettings, filePath);

    /* deletes the template settings */
    deleteTemplateSettings(templateSettings);

    /* deletes the template engine */
    deleteTemplateEngine(templateEngine);
}
