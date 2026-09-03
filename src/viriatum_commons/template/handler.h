/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../io/io.h"
#include "../structures/structures.h"
#include "../util/util.h"
#include "engine.h"

/**
 * The number of templates that the cache is allowed
 * to hold parsed at the same time, a path falls on
 * exactly one of the entries and takes it over from
 * whatever was sitting there before.
 */
#define CACHE_SIZE_TEMPLATE_HANDLER 32

/**
 * The number of seconds that an entry of the cache is
 * trusted for, once past it the template it holds is
 * opened again through its path and parsed again, the
 * very same period the cache of the served files trusts
 * an entry for, so that a file put in the place of
 * another is picked up by the two of them alike.
 */
#define CACHE_VALID_TEMPLATE_HANDLER 4

/**
 * The number of pages that an entry of the cache holds
 * rendered, each of them under a key of the caller's own
 * choosing, a page falls on exactly one of the slots and
 * takes it over from whatever was sitting there before.
 */
#define CACHE_PAGES_TEMPLATE_HANDLER 8

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
     * The value as a reference of the parameter.
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

/**
 * Structure describing a page that has been rendered out
 * of a held template and is being held itself, under the
 * key it was rendered for, so that a page whose names
 * never change, the one of an error among them, costs
 * not even its rendering the second time around.
 */
typedef struct template_cache_page_t {
    /**
     * The key the page is held under, which is what a
     * caller asks for it by, unset while there is no page.
     */
    unsigned char *key;

    /**
     * The contents of the page, as they came out of the
     * rendering, null terminated.
     */
    unsigned char *contents;

    /**
     * The size in bytes of the contents of the page.
     */
    size_t size;
} template_cache_page;

/**
 * Structure describing a template that has been parsed
 * and is being held, so that the building of a page out
 * of it costs only the rendering and never the reading
 * and the parsing of the file, which together are the
 * greater part of what such a page costs.
 */
typedef struct template_cache_entry_t {
    /**
     * The path of the template that this entry describes,
     * which is also the key it is stored under.
     */
    unsigned char path[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The descriptor of the file, kept open for as long as
     * the entry lives so that it may be asked about the file
     * on every request, a file written over in place keeps
     * the very same descriptor and is told from the one that
     * was parsed by what the descriptor reports about it.
     */
    int descriptor;

    /**
     * The size in bytes of the file, as it stood when the
     * tree was parsed out of it.
     */
    size_t size;

    /**
     * The moment of the last write to the file, as it stood
     * when the tree was parsed out of it.
     */
    time_t written;

    /**
     * The moment at which the file was last looked at through
     * its path, an entry older than the validity is opened
     * and parsed again before it is trusted.
     */
    unsigned int checked;

    /**
     * The root node of the tree that was parsed out of the
     * file, the one the rendering of a page starts from.
     */
    struct template_node_t *root;

    /**
     * The complete set of nodes of the tree, kept as a flat
     * list so that every one of them is released together
     * with the entry, the very way a handler keeps its own.
     */
    struct linked_list_t *nodes;

    /**
     * The pages that have been rendered out of the tree and
     * are held under the keys they were rendered for, they
     * go away with the tree, a page of a template that has
     * changed underneath it describes nothing any longer.
     */
    struct template_cache_page_t pages[CACHE_PAGES_TEMPLATE_HANDLER];
} template_cache_entry;

/**
 * Structure describing the set of templates that are being
 * held parsed, one per process as the workers are forked and
 * each of them serves on its own, so that nothing here is
 * ever reached by two of them at once.
 *
 * A path falls on exactly one of the entries, decided by the
 * hash of it, and takes that entry over from whatever was
 * sitting there before, the very shape the cache of the
 * served files has, so that the tree carries one answer to
 * the holding of a file rather than two.
 */
typedef struct template_cache_t {
    /**
     * The entries of the cache, one slot per position that
     * the hash of a path is able to fall on.
     */
    struct template_cache_entry_t *entries;
} template_cache;

VIRIATUM_EXPORT_PREFIX void create_template_handler(struct template_handler_t **template_handler_pointer);
VIRIATUM_EXPORT_PREFIX void delete_template_handler(struct template_handler_t *template_handler);
VIRIATUM_EXPORT_PREFIX void create_template_node(struct template_node_t **template_node_pointer, enum template_node_type_e type);
VIRIATUM_EXPORT_PREFIX void delete_template_node(struct template_node_t *template_node);
VIRIATUM_EXPORT_PREFIX void create_template_parameter(struct template_parameter_t **template_parameter_pointer);
VIRIATUM_EXPORT_PREFIX void delete_template_parameter(struct template_parameter_t *template_parameter);
VIRIATUM_EXPORT_PREFIX void create_template_cache(struct template_cache_t **template_cache_pointer);
VIRIATUM_EXPORT_PREFIX void delete_template_cache(struct template_cache_t *template_cache);
VIRIATUM_EXPORT_PREFIX void clear_template_cache(struct template_cache_t *template_cache);
VIRIATUM_EXPORT_PREFIX ERROR_CODE acquire_template_cache(struct template_cache_t *template_cache, unsigned char *file_path, struct template_cache_entry_t **template_cache_entry_pointer);
VIRIATUM_EXPORT_PREFIX void process_template_handler(struct template_handler_t *template_handler, unsigned char *file_path);
VIRIATUM_EXPORT_PREFIX void process_cache_template_handler(struct template_handler_t *template_handler, struct template_cache_t *template_cache, unsigned char *file_path);
VIRIATUM_EXPORT_PREFIX void process_page_template_handler(struct template_handler_t *template_handler, struct template_cache_t *template_cache, unsigned char *file_path, unsigned char *key);
VIRIATUM_EXPORT_PREFIX void assign_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct type_t *value);
VIRIATUM_EXPORT_PREFIX void assign_integer_template_handler(struct template_handler_t *template_handler, unsigned char *name, int value);
VIRIATUM_EXPORT_PREFIX void assign_string_template_handler(struct template_handler_t *template_handler, unsigned char *name, char *value);
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
