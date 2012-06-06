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
typedef enum template_node_type_e {
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
} template_node_type;

/**
 * Enumeration defining the various
 * types of parameters.
 */
typedef enum template_parameter_type_e {
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
} template_parameter_type;

/**
 * Structure defining the parameter internal
 * parameters and the references to the value.
 */
typedef struct template_parameter_t {
    /**
     * The data type of the current parameter.
     */
    enum template_parameter_type_e type;

    /**
     * The name of the current parameter.
     */
    unsigned char name[64];

    /**
     * The "raw" and unprocessed parameter value.s
     */
    unsigned char raw_value[128];

    /**
     * The value as a string of the parameter.
     */
    unsigned char string_value[128];

    /**
     * The value as a refernce of the parameter.
     */
    unsigned char reference_value[64];

    /**
     * The value as an integer of the parameter.
     */
    int int_value;

    /**
     * The value as a float of the parameter.
     */
    float float_value;
} template_parameter;

typedef struct template_node_t {
    unsigned char *name;
    enum template_node_type_e type;
    struct linked_list_t *children;
    struct linked_list_t *parameters;
    struct hash_map_t *parameters_map;
    struct template_parameter_t *temporary_parameter;
} template_node;

typedef struct template_handler_t {
    unsigned char *string_value;
    struct template_node_t *current_node;
    struct template_node_t *temporary_node;
    struct linked_list_t *nodes;
    struct linked_list_t *contexts;
    struct hash_map_t *names;
    struct string_buffer_t *string_buffer;
    struct linked_list_t *release_list;
} template_handler;

VIRIATUM_EXPORT_PREFIX void create_template_handler(struct template_handler_t **template_handler_pointer);
VIRIATUM_EXPORT_PREFIX void delete_template_handler(struct template_handler_t *template_handler);
VIRIATUM_EXPORT_PREFIX void create_template_node(struct template_node_t **template_node_pointer, enum template_node_type_e type);
VIRIATUM_EXPORT_PREFIX void delete_template_node(struct template_node_t *template_node);
VIRIATUM_EXPORT_PREFIX void create_template_parameter(struct template_parameter_t **template_parameter_pointer);
VIRIATUM_EXPORT_PREFIX void delete_template_parameter(struct template_parameter_t *template_parameter);
VIRIATUM_EXPORT_PREFIX void process_template_handler(struct template_handler_t *template_handler, unsigned char *file_path);
VIRIATUM_EXPORT_PREFIX void assign_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct type_t *value);
VIRIATUM_EXPORT_PREFIX void assign_integer_template_handler(struct template_handler_t *template_handler, unsigned char *name, int value);
VIRIATUM_EXPORT_PREFIX void assign_list_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct linked_list_t *value);
VIRIATUM_EXPORT_PREFIX void get_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct type_t **value_pointer);
VIRIATUM_EXPORT_PREFIX void traverse_node_debug(struct template_handler_t *template_handler, struct template_node_t *node, unsigned int indentation);
VIRIATUM_EXPORT_PREFIX void traverse_node_buffer(struct template_handler_t *template_handler, struct template_node_t *node);
VIRIATUM_EXPORT_PREFIX void traverse_nodes_buffer(struct template_handler_t *template_handler, struct template_node_t *node);
VIRIATUM_EXPORT_PREFIX void _traverse_out_buffer(struct template_handler_t *template_handler, struct template_node_t *node);
VIRIATUM_EXPORT_PREFIX void _traverse_for_each_buffer(struct template_handler_t *template_handler, struct template_node_t *node);
VIRIATUM_EXPORT_PREFIX void _traverse_if_buffer(struct template_handler_t *template_handler, struct template_node_t *node);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _open_context_template_handler(struct template_handler_t *template_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _close_context_template_handler(struct template_handler_t *template_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _text_begin_callback(struct template_engine_t *template_engine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _text_end_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tag_begin_callback(struct template_engine_t *template_engine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tag_close_begin_callback(struct template_engine_t *template_engine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tag_end_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _tag_name_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _parameter_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _parameter_value_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size);
