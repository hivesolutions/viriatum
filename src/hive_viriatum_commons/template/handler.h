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
#include "../util/util.h"
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

/**
 * Enumeration defining the various
 * types of parameters.
 */
typedef enum TemplateParameterType_e {
    /**
     * The parameter of type string literal.
     */
    TEMPLATE_PARAMETER_STRING = 1,

    /**
     * The parameter of type variable reference
     * it must be dereferenced first.
     */
    TEMPLATE_PARAMETER_REFERENCE,

    /**
     * The parameter of type integer, number with
     * no decimal part.
     */
    TEMPLATE_PARAMETER_INTEGER,

    /**
     * The parameter of type integer, number with
     * decimal part.
     */
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
    unsigned char name[64];

    /**
     * The "raw" and unprocessed parameter value.s
     */
    unsigned char rawValue[128];

    /**
     * The value as a string of the parameter.
     */
    unsigned char string_value[128];

    /**
     * The value as a refernce of the parameter.
     */
    unsigned char referenceValue[64];

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
    unsigned char *name;
    enum TemplateNodeType_e type;
    struct linked_list_t *children;
    struct linked_list_t *parameters;
    struct hash_map_t *parametersMap;
    struct TemplateParameter_t *temporaryParameter;
} TemplateNode;

typedef struct TemplateHandler_t {
    unsigned char *string_value;
    struct TemplateNode_t *currentNode;
    struct TemplateNode_t *temporaryNode;
    struct linked_list_t *nodes;
    struct linked_list_t *contexts;
    struct hash_map_t *names;
    struct string_buffer_t *string_buffer;
    struct linked_list_t *releaseList;
} TemplateHandler;

VIRIATUM_EXPORT_PREFIX void create_template_handler(struct TemplateHandler_t **templateHandlerPointer);
VIRIATUM_EXPORT_PREFIX void delete_template_handler(struct TemplateHandler_t *template_handler);
VIRIATUM_EXPORT_PREFIX void createTemplateNode(struct TemplateNode_t **templateNodePointer, enum TemplateNodeType_e type);
VIRIATUM_EXPORT_PREFIX void deleteTemplateNode(struct TemplateNode_t *templateNode);
VIRIATUM_EXPORT_PREFIX void createTemplateParameter(struct TemplateParameter_t **templateParameterPointer);
VIRIATUM_EXPORT_PREFIX void deleteTemplateParameter(struct TemplateParameter_t *templateParameter);
VIRIATUM_EXPORT_PREFIX void process_template_handler(struct TemplateHandler_t *template_handler, unsigned char *file_path);
VIRIATUM_EXPORT_PREFIX void assignTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, struct type_t *value);
VIRIATUM_EXPORT_PREFIX void assignIntegerTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, int value);
VIRIATUM_EXPORT_PREFIX void assignListTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, struct linked_list_t *value);
VIRIATUM_EXPORT_PREFIX void getTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, struct type_t **value_pointer);
VIRIATUM_EXPORT_PREFIX void traverseNodeDebug(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node, unsigned int indentation);
VIRIATUM_EXPORT_PREFIX void traverseNodeBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node);
VIRIATUM_EXPORT_PREFIX void traverseNodesBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node);
VIRIATUM_EXPORT_PREFIX void _traverseOutBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node);
VIRIATUM_EXPORT_PREFIX void _traverseForEachBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node);
VIRIATUM_EXPORT_PREFIX void _traverseIfBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _openContextTemplateHandler(struct TemplateHandler_t *template_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _closeContextTemplateHandler(struct TemplateHandler_t *template_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _textBeginCallback(struct TemplateEngine_t *templateEngine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _textEndCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tagBeginCallback(struct TemplateEngine_t *templateEngine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tagCloseBeginCallback(struct TemplateEngine_t *templateEngine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tagEndCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tagNameCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _parameterCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _parameterValueCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size);
