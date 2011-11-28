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

#pragma once

#include "../structures/structures.h"
#include "engine.h"

/**
 * Enumeration defining the types of
 * template nodes that may exist in
 * the template parsing context.
 */
typedef enum TemplateNodeType_e {
    /**
     * The template node to be used as the root
     * node os the node structure.
     */
    TEMPLATE_NODE_ROOT = 1,

    /**
     * The template node representing a "simple"
     * text context.
     */
    TEMPLATE_NODE_TEXT,

    /**
     * The template node representing a single
     * open and closing context.
     */
    TEMPLATE_NODE_SINGLE,

    /**
     * The template node representing an
     * open (tag) context.
     */
    TEMPLATE_NODE_OPEN,

    /**
     * The template node representing an
     * close (tag) context.
     */
    TEMPLATE_NODE_CLOSE,

    /**
     * The template node representing an
     * undefined context.
     */
    TEMPLATE_NODE_UNDEFINED
} TemplateNodeType;

typedef enum TemplateParameterType_e {
    TEMPLATE_PARAMETER_STRING = 1,
    TEMPLATE_PARAMETER_REFERENCE,
    TEMPLATE_PARAMETER_INTEGER,
    TEMPLATE_PARAMETER_FLOAT
} TemplateParameterType;

/**
 * Structure defining the parameter internal
 * parameters and the references to the value.
 */
typedef struct TemplateParameter_t {
    /**
     * The data type of the current parameter.
     */
    enum TemplateParameterType_e type;

    /**
     * The name of the current parameter.
     */
    unsigned char *name;

    /**
     * The "raw" and unprocessed parameter value.s
     */
    unsigned char *rawValue;

    /**
     * The value as a string of the parameter.
     */
    unsigned char *stringValue;

    /**
     * The value as a refernce of the parameter.
     */
    unsigned char *referenceValue;

    /**
     * The value as an integer of the parameter.
     */
    int intValue;

    /**
     * The value as a float of the parameter.
     */
    float floatValue;
} TemplateParameter;

typedef struct TemplateNode_t {
    size_t childCount;
    size_t parameterCount;
    unsigned char *name;
    enum TemplateNodeType_e type;
    struct LinkedList_t *children;
    struct LinkedList_t *parameters;
    struct HashMap_t *parametersMap;
    struct TemplateParameter_t *temporaryParameter;
} TemplateNode;

typedef struct TemplateHandler_t {
    size_t nodeCount;
    unsigned char *stringValue;
    struct TemplateNode_t *currentNode;
    struct TemplateNode_t *temporaryNode;
    struct LinkedList_t *nodes;
    struct LinkedList_t *contexts;
    struct HashMap_t *names;
    struct StringBuffer_t *stringBuffer;
} TemplateHandler;

VIRIATUM_EXPORT_PREFIX void createTemplateHandler(struct TemplateHandler_t **templateHandlerPointer);
VIRIATUM_EXPORT_PREFIX void deleteTemplateHandler(struct TemplateHandler_t *templateHandler);
VIRIATUM_EXPORT_PREFIX void createTemplateNode(struct TemplateNode_t **templateNodePointer, enum TemplateNodeType_e type);
VIRIATUM_EXPORT_PREFIX void deleteTemplateNode(struct TemplateNode_t *templateNode);
VIRIATUM_EXPORT_PREFIX void createTemplateParameter(struct TemplateParameter_t **templateParameterPointer);
VIRIATUM_EXPORT_PREFIX void deleteTemplateParameter(struct TemplateParameter_t *templateParameter);
VIRIATUM_EXPORT_PREFIX void processTemplateHandler(struct TemplateHandler_t *templateHandler, unsigned char *filePath);
VIRIATUM_EXPORT_PREFIX void assignTemplateHandler(struct TemplateHandler_t *templateHandler, unsigned char *key, void *value);
