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

 __author__    = João Magalhães <joamag@hive.pt>
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
    templateHandler->contexts = NULL;

    /* sets the template engine in the template handler pointer */
    *templateHandlerPointer = templateHandler;
}

void deleteTemplateHandler(struct TemplateHandler_t *templateHandler) {
    /* @TODO: TENHO DE APAGAR TODOS OS PARAMETROS E OS NOS */

    /* releases the template handler */
    FREE(templateHandler);
}

void createTemplateNode(struct TemplateNode_t **templateNodePointer, enum TemplateNodeType_e type) {
    /* retrieves the template node size */
    size_t templateNodeSize = sizeof(struct TemplateNode_t);

    /* allocates space for the template node */
    struct TemplateNode_t *templateNode = (struct TemplateNode_t *) MALLOC(templateNodeSize);

    /* sets the (node) type in the template node */
    templateNode->type = type;

    /* sets the default values in the template node */
    templateNode->childCount = 0;
    templateNode->parameterCount = 0;
    templateNode->name = NULL;
    templateNode->children = NULL;
    templateNode->parameters = NULL;
    templateNode->temporaryParameter = NULL;

    /* sets the template engine in the template node pointer */
    *templateNodePointer = templateNode;
}

void deleteTemplateNode(struct TemplateNode_t *templateNode) {
    /* @TODO Tenho de apagar todos os parametros e todos os nos */


    /* releases the template node */
    FREE(templateNode);
}

void createTemplateParameter(struct TemplateParameter_t **templateParameterPointer) {
    /* retrieves the template parameter size */
    size_t templateParameterSize = sizeof(struct TemplateParameter_t);

    /* allocates space for the template parameter */
    struct TemplateParameter_t *templateParameter = (struct TemplateParameter_t *) MALLOC(templateParameterSize);

    /* sets the default values in the template parameter */
    templateParameter->type = 0;
    templateParameter->name = NULL;
    templateParameter->rawValue = NULL;
    templateParameter->stringValue = NULL;
    templateParameter->referenceValue = NULL;
    templateParameter->intValue = 0;
    templateParameter->floatValue = 0.0;

    /* sets the template engine in the template parameter pointer */
    *templateParameterPointer = templateParameter;
}

void deleteTemplateParameter(struct TemplateParameter_t *templateParameter) {
    /* @TODO: TENHO DE REMOVER OS PARAMETROS SE EXISTIREM */

    /* releases the template parameter */
    FREE(templateParameter);
}

ERROR_CODE textBegin(struct TemplateEngine_t *templateEngine) {
    printf("TEXT_BEGIN\n");

    RAISE_NO_ERROR;
}

ERROR_CODE textEnd(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* allocates space for the template node */
    struct TemplateNode_t *templateNode;

    /* retrieves the template handler from the template engine context
    and retrieves the current node from it */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *currentNode = templateHandler->currentNode;

    /* creates a new template node and sets the template
    node as the temporary node in the template handler */
    createTemplateNode(&templateNode, TEMPLATE_NODE_TEXT);
    templateHandler->temporaryNode = templateNode;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    templateNode->name = (unsigned char *) MALLOC(size + 1);
    memcpy(templateNode->name, pointer, size);
    templateNode->name[size] = '\0';

    if(currentNode->children == NULL) {
        createLinkedList(&currentNode->children);
    }

    /* in case the nodes (list) are not defined for the
    temporary node */
    if(templateHandler->nodes == NULL) {
        /* creates a new linked list for the nodes */
        createLinkedList(&templateHandler->nodes);
    }

    /* adds the temporary node to the current list of nodes
    in the template handler and to the chilren list of the current node */
    appendValueLinkedList(templateHandler->nodes, templateNode);
    appendValueLinkedList(currentNode->children, templateNode);

    printf("TEXT_END: '%s'\n", templateNode->name);

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


ERROR_CODE _openContextTemplateHandler(struct TemplateHandler_t *templateHandler) {
    struct TemplateNode_t *currentNode = templateHandler->currentNode;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;

    if(templateHandler->contexts == NULL) {
        createLinkedList(&templateHandler->contexts);
    }

    appendValueLinkedList(templateHandler->contexts, currentNode);

    templateHandler->currentNode = temporaryNode;

    RAISE_NO_ERROR;
}

ERROR_CODE _closeContextTemplateHandler(struct TemplateHandler_t *templateHandler) {
    popValueLinkedList(templateHandler->contexts, &templateHandler->currentNode, 1);

    RAISE_NO_ERROR;
}


ERROR_CODE tagEnd(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;

    struct TemplateNode_t *currentNode = templateHandler->currentNode;

    /* in case the node does contain the closing
    symbol at the final part of the tag (assumes single node) */
    if(pointer[size - 2] == '/') {
        temporaryNode->type = TEMPLATE_NODE_SINGLE;
    }

    /* switches over the temporary node type */
    switch(temporaryNode->type) {
        case TEMPLATE_NODE_OPEN:
            _openContextTemplateHandler(templateHandler);

            break;

        case TEMPLATE_NODE_CLOSE:
            _closeContextTemplateHandler(templateHandler);

            break;
    }

    if(temporaryNode->type == TEMPLATE_NODE_CLOSE) {
        RAISE_NO_ERROR;
    }

    if(currentNode->children == NULL) {
        createLinkedList(&currentNode->children);
    }

    /* in case the nodes (list) are not defined for the
    temporary node */
    if(templateHandler->nodes == NULL) {
        /* creates a new linked list for the nodes */
        createLinkedList(&templateHandler->nodes);
    }

    /* adds the temporary node to the current list of nodes
    in the template handler and to the chilren list of the temporary node */
    appendValueLinkedList(currentNode->children, temporaryNode);
    appendValueLinkedList(templateHandler->nodes, temporaryNode);

    RAISE_NO_ERROR;
}

ERROR_CODE tagName(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    temporaryNode->name = (unsigned char *) MALLOC(size + 1);
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

    /* in case the parameters (list) are not defined for the
    temporary node */
    if(temporaryNode->parameters == NULL) {
        /* creates a new linked list for the parameters */
        createLinkedList(&temporaryNode->parameters);
    }

    /* creates the template parameter, sets it as the temporary parameter
    in the temporary node and adds it to the list of parameters */
    createTemplateParameter(&templateParameter);
    temporaryNode->temporaryParameter = templateParameter;
    appendValueLinkedList(temporaryNode->parameters, templateParameter);

    /* allocates the space for the template parameter name and
    sets it with a memory copy */
    templateParameter->name = (unsigned char *) MALLOC(size + 1);
    memcpy(templateParameter->name, pointer, size);
    templateParameter->name[size] = '\0';

    printf("PARAMETER: '%s'\n", templateParameter->name);

    RAISE_NO_ERROR;
}

ERROR_CODE parameterValue(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it and then uses it to retrieve
    the temporary parameter */
    struct TemplateHandler_t *templateHandler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = templateHandler->temporaryNode;
    struct TemplateParameter_t *temporaryParameter = temporaryNode->temporaryParameter;

    /* allocates the space for the temporary parameter raw value and
    sets it with a memory copy */
    temporaryParameter->rawValue = (unsigned char *) MALLOC(size + 1);
    memcpy(temporaryParameter->rawValue, pointer, size);
    temporaryParameter->rawValue[size] = '\0';

    /* in case the first character of the raw value is
    a start string character the value must be a string */
    if(temporaryParameter->rawValue[0] == '"') {
        temporaryParameter->stringValue = (unsigned char *) MALLOC(size - 1);
        memcpy(temporaryParameter->stringValue, pointer + 1, size - 2);
        temporaryParameter->stringValue[size - 2] = '\0';
        temporaryParameter->type = TEMPLATE_PARAMETER_STRING;
    }
    /* in case the first character of the raw value is a numeric
    value it must be a number */
    else if(temporaryParameter->rawValue[0] > 0x2f && temporaryParameter->rawValue[0] < 0x58) {
        temporaryParameter->intValue = atoi(temporaryParameter->rawValue);
        temporaryParameter->type = TEMPLATE_PARAMETER_INTEGER;
    }
    /* othwerwise it must be a (variable) reference value */
    else {
        temporaryParameter->referenceValue = (unsigned char *) MALLOC(size + 1);
        memcpy(temporaryParameter->referenceValue, pointer, size);
        temporaryParameter->referenceValue[size] = '\0';
        temporaryParameter->type = TEMPLATE_PARAMETER_REFERENCE;
    }

    printf("VALUE: '%s'\n", temporaryParameter->stringValue);

    RAISE_NO_ERROR;
}

void traverseNode(struct TemplateNode_t *node, unsigned int indentation) {
    /* allocates space for the child element */
    struct TemplateNode_t *child;

    /* allocates space for the index counter */
    unsigned int index;

    /* iterates over the indentation count to print
    the indentation spaces */
    for(index = 0; index < indentation; index++) {
        /* prints an indentation space */
        printf("  ");
    }

    /* prints bot the name and the type in case the name
    is not valid nothing is print */
    if(node->name != NULL) {
        printf("name: '%s' & ", node->name);
    }
    printf("type: '%d'\n", node->type);

    /* in case the node contains no children, it should
    be a leaf node (nothing to be done) */
    if(node->children == NULL) {
        /* returns immediately */
        return;
    }

    /* iterates continuously for children percolation */
    while(1) {
        /* "pops" a value from the children linked list */
        popValueLinkedList(node->children, &child, 1);

        /* in case the child is not valid (no more items available) */
        if(child == NULL) {
            /* breaks the loop */
            break;
        }

        /* traverses the child node (recursion step) */
        traverseNode(child, indentation + 1);
    }
}

void processTemplateHandler(struct TemplateHandler_t *templateHandler, unsigned char *filePath) {
     /* allocates space for the template engine */
    struct TemplateEngine_t *templateEngine;

    /* allocates space for the template settings */
    struct TemplateSettings_t *templateSettings;

    /* allocates space for the root node */
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

    /* traverses the current node recursively, printing
    the names and type of earch of the values */
    traverseNode(templateHandler->currentNode, 0);

    /* deletes the template settings */
    deleteTemplateSettings(templateSettings);

    /* deletes the template engine */
    deleteTemplateEngine(templateEngine);
}
