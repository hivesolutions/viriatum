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

void create_template_handler(struct template_handler_t **template_handler_pointer) {
    /* retrieves the template handler size */
    size_t template_handler_size = sizeof(struct template_handler_t);

    /* allocates space for the template handler */
    struct template_handler_t *template_handler = (struct template_handler_t *) MALLOC(template_handler_size);

    /* sets the default values in the template handler */
    template_handler->string_value = NULL;
    template_handler->current_node = NULL;
    template_handler->temporary_node = NULL;
    template_handler->nodes = NULL;
    template_handler->contexts = NULL;

    /* creates a new hash map for the names */
    create_hash_map(&template_handler->names, 0);

    /* creates a new string buffer for buffering
    the result of the template processing */
    create_string_buffer(&template_handler->string_buffer);

    /* creates the list to hold the various types
    to have the memory released uppon destruction */
    create_linked_list(&template_handler->release_list);

    /* sets the template engine in the template handler pointer */
    *template_handler_pointer = template_handler;
}

void delete_template_handler(struct template_handler_t *template_handler) {
    /* allocates space for the temporary node variable */
    struct template_node_t *node;

    /* allocates space for the temporary type */
    struct type_t *type;

    /* in case the string value is set */
    if(template_handler->string_value) {
        /* releases the string value from the template handler */
        FREE(template_handler->string_value);
    }

    /* in case the template contexts are defined */
    if(template_handler->contexts) {
        /* deletes the contexts list */
        delete_linked_list(template_handler->contexts);
    }

    /* in case the template nodes are defined */
    if(template_handler->nodes) {
        /* iterates continuously for nodes list
        cleanup (removal of all nodes) */
        while(1) {
            /* pops a node from the nodes list */
            pop_value_linked_list(template_handler->nodes, (void **) &node, 1);

            /* in case the value is invalid (empty list) */
            if(node == NULL) {
                /* breaks the cycle */
                break;
            }

            /* deletes the template node */
            delete_template_node(node);
        }

        /* deletes the nodes list */
        delete_linked_list(template_handler->nodes);
    }

    /* iterates continuously for release list
    cleanup (type memory release) */
    while(1) {
        /* pops a node from the release list */
        pop_value_linked_list(template_handler->release_list, (void **) &type, 1);

        /* in case the value is invalid (empty list) */
        if(type == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the type */
        delete_type(type);
    }

    /* deletes the list of release types from the template handler */
    delete_linked_list(template_handler->release_list);

    /* deletes the string buffer */
    delete_string_buffer(template_handler->string_buffer);

    /* deletes the names hash map */
    delete_hash_map(template_handler->names);

    /* releases the template handler */
    FREE(template_handler);
}

void create_template_node(struct template_node_t **template_node_pointer, enum template_node_type_e type) {
    /* retrieves the template node size */
    size_t template_node_size = sizeof(struct template_node_t);

    /* allocates space for the template node */
    struct template_node_t *template_node = (struct template_node_t *) MALLOC(template_node_size);

    /* sets the (node) type in the template node */
    template_node->type = type;

    /* sets the default values in the template node */
    template_node->name = NULL;
    template_node->children = NULL;
    template_node->parameters = NULL;
    template_node->parameters_map = NULL;
    template_node->temporary_parameter = NULL;

    /* sets the template engine in the template node pointer */
    *template_node_pointer = template_node;
}

void delete_template_node(struct template_node_t *template_node) {
    /* allocates space for the temporary parameter variable */
    struct template_parameter_t *parameter;

    /* in case the template parameters map are defined */
    if(template_node->parameters_map) {
        /* deletes the parameters map */
        delete_hash_map(template_node->parameters_map);
    }

    /* in case the template parameters are defined */
    if(template_node->parameters) {
        /* iterates continuously for parameters list
        cleanup (removal of all parameters) */
        while(1) {
            /* pops a parameter from the parameters list */
            pop_value_linked_list(template_node->parameters, (void **) &parameter, 1);

            /* in case the value is invalid (empty list) */
            if(parameter == NULL) {
                /* breaks the cycle */
                break;
            }

            /* deletes the template parameter */
            delete_template_parameter(parameter);
        }

        /* deletes the parameters list */
        delete_linked_list(template_node->parameters);
    }

    /* in case the template children are defined */
    if(template_node->children) {
        /* deletes the children list */
        delete_linked_list(template_node->children);
    }

    /* in case the name is defined in the template node */
    if(template_node->name) {
        /* releases the template node name */
        FREE(template_node->name);
    }

    /* releases the template node */
    FREE(template_node);
}

void create_template_parameter(struct template_parameter_t **template_parameter_pointer) {
    /* retrieves the template parameter size */
    size_t template_parameter_size = sizeof(struct template_parameter_t);

    /* allocates space for the template parameter */
    struct template_parameter_t *template_parameter = (struct template_parameter_t *) MALLOC(template_parameter_size);

    /* sets the default values in the template parameter */
    template_parameter->type = 0;
    template_parameter->int_value = 0;
    template_parameter->float_value = 0.0;

    /* sets the template engine in the template parameter pointer */
    *template_parameter_pointer = template_parameter;
}

void delete_template_parameter(struct template_parameter_t *template_parameter) {
    /* releases the template parameter */
    FREE(template_parameter);
}

void process_template_handler(struct template_handler_t *template_handler, unsigned char *file_path) {
     /* allocates space for the template engine */
    struct template_engine_t *template_engine;

    /* allocates space for the template settings */
    struct template_settings_t *template_settings;

    /* allocates space for the root node */
    struct template_node_t *root_node;

    /* creates the template engine */
    create_template_engine(&template_engine);

    /* creates the template engine */
    create_template_settings(&template_settings);

    /* creates the root node and sets it as the initial
    current node */
    create_template_node(&root_node, TEMPLATE_NODE_ROOT);
    template_handler->current_node = root_node;

    /* sets the context (template handler) in the template engine */
    template_engine->context = template_handler;

    /* sets the various callbacks in the template settings */
    template_settings->on_text_begin = _text_begin_callback;
    template_settings->on_text_end = _text_end_callback;
    template_settings->on_tag_begin = _tag_begin_callback;
    template_settings->on_tag_close_begin = _tag_close_begin_callback;
    template_settings->on_tag_end = _tag_end_callback;
    template_settings->on_tag_name = _tag_name_callback;
    template_settings->on_parameter = _parameter_callback;
    template_settings->on_parameter_value = _parameter_value_callback;

    /* processes the file as a template engine and then uses the
    created node structure to traverse for string buffer output */
    process_template_engine(template_engine, template_settings, file_path);
    traverse_node_buffer(template_handler, template_handler->current_node);

    /* "joins" the template handler string buffer into the string
    value, retrieving the final template result */
    join_string_buffer(template_handler->string_buffer, &template_handler->string_value);

    /* deletes the now unecessary root node */
    delete_template_node(root_node);

    /* deletes the template settings */
    delete_template_settings(template_settings);

    /* deletes the template engine */
    delete_template_engine(template_engine);
}

void assign_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct type_t *value) {
    /* sets the a new name (key) in the names hash map */
    set_value_string_hash_map(template_handler->names, name, value);
}

void assign_integer_template_handler(struct template_handler_t *template_handler, unsigned char *name, int value) {
    /* allocates space for the type */
    struct type_t *type;

    /* create a integer type and sets the value
    to the list to be assigned */
    create_type(&type, INTEGER_TYPE);
    type->value.value_int = value;

    /* sets the value (integer) in the template handler names hash map */
    set_value_string_hash_map(template_handler->names, name, type);

    /* adds the type to the list of types to have the
    memory released uppon template handler destruction (late removal) */
    append_value_linked_list(template_handler->release_list, type);
}

void assign_string_template_handler(struct template_handler_t *template_handler, unsigned char *name, char *value) {
    /* allocates space for the type */
    struct type_t *type;

    /* create a string type and sets the value
    to the list to be assigned */
    create_type(&type, STRING_TYPE);
    type->value.value_string = value;

    /* sets the value (string) in the template handler names hash map */
    set_value_string_hash_map(template_handler->names, name, type);

    /* adds the type to the list of types to have the
    memory released uppon template handler destruction (late removal) */
    append_value_linked_list(template_handler->release_list, type);
}

void assign_list_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct linked_list_t *value) {
    /* allocates space for the type */
    struct type_t *type;

    /* create a list type and sets the value
    to the list to be assigned */
    create_type(&type, LIST_TYPE);
    type->value.value_list = value;

    /* sets the value (list) in the template handler names hash map */
    set_value_string_hash_map(template_handler->names, name, type);

    /* adds the type to the list of types to have the
    memory released uppon template handler destruction (late removal) */
    append_value_linked_list(template_handler->release_list, type);
}

void get_template_handler(struct template_handler_t *template_handler, unsigned char *name, struct type_t **value_pointer) {
    /* allocates space for the temporary (tokanizable) name variable
    and for the generated token values*/
    unsigned char _name[64];
    unsigned char *name_token;
    unsigned char *context;

    /* allocates space for the (type) value */
    struct type_t *value;

    /* retrieves the template global names as thee base value */
    struct hash_map_t *_value = template_handler->names;

    /* copies the name reference into a backup value
    to avoid data corruption (from tokenization) */
    memcpy(_name, name, strlen((char *) name) + 1);

    /* tokenizes the name into tokens */
    name_token = (unsigned char *) STRTOK((char *) _name, ".", context);

    /* iterates continuously */
    while(1) {
        /* in case the name token is invalid
        (no more tokens available) */
        if(name_token == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrieves the value from the current value with the
        name token reference key value */
        get_value_string_hash_map(_value, name_token, (void **) &value);

        /* in case the retrieve value is not valid
        (not found), must break immediately */
        if(value == NULL) {
            /* breaks the loop */
            break;
        }

        /* updates the (hash map) value reference with the "newly"
        retrieved value */
        _value = value->value.value_map;

        /* retrieves the next token */
        name_token = (unsigned char *) STRTOK(NULL, ".", context);
    }

    /* sets the value pointer with the internal
    retrieved value */
    *value_pointer = value;
}

void traverse_node_debug(struct template_handler_t *template_handler, struct template_node_t *node, unsigned int indentation) {
    /* allocates space for the iterator to be used to retrieve
    the various children from the node */
    struct iterator_t *child_iterator;

    /* allocates space for the child element */
    struct template_node_t *child;

    /* allocates space for the index counter */
    unsigned int index;

    /* iterates over the indentation count to print
    the indentation spaces */
    for(index = 0; index < indentation; index++) {
        /* prints an indentation space */
        V_DEBUG("  ");
    }

    /* prints both the name and the type in case the name
    is not valid nothing is print */
    if(node->name != NULL) {
        V_DEBUG_F("name: '%s' & ", node->name);
    }
    V_DEBUG_F("type: '%d'\n", node->type);

    /* in case the node contains no children, it should
    be a leaf node (nothing to be done) */
    if(node->children == NULL) {
        /* returns immediately */
        return;
    }

    /* creates a "new" iterator for the children linked list */
    create_iterator_linked_list(node->children, &child_iterator);

    /* iterates continuously for children percolation */
    while(1) {
        /* retrieves the child element from the child iterator */
        get_next_iterator(child_iterator, (void **) &child);

        /* in case the child is not valid (no more items available) */
        if(child == NULL) {
            /* breaks the loop */
            break;
        }

        /* traverses the child node (recursion step) */
        traverse_node_debug(template_handler, child, indentation + 1);
    }

    /* deletes the child iterator */
    delete_iterator_linked_list(node->children, child_iterator);
}

void traverse_node_buffer(struct template_handler_t *template_handler, struct template_node_t *node) {
    /* switches over the type of node to be traversed,
    to print the correct value */
    switch(node->type) {
        case TEMPLATE_NODE_ROOT:
            /* traverses all the nodes in the root node */
            traverse_nodes_buffer(template_handler, node);

            /* breaks the switch */
            break;

        case TEMPLATE_NODE_TEXT:
            /* adds the node name (text value) to the string buffer */
            append_string_buffer(template_handler->string_buffer, node->name);

            /* breaks the switch */
            break;

        case TEMPLATE_NODE_SINGLE:
        case TEMPLATE_NODE_OPEN:
            if(strcmp((char *) node->name, "out") == 0) {
                /* traverses the out node in buffer mode */
                _traverse_out_buffer(template_handler, node);
            } else if(strcmp((char *) node->name, "foreach") == 0) {
                /* traverses the foreach node in buffer mode */
                _traverse_for_each_buffer(template_handler, node);
            } else if(strcmp((char *) node->name, "if") == 0) {
                /* traverses the if node in buffer mode */
                _traverse_if_buffer(template_handler, node);
            }

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }
}

void traverse_nodes_buffer(struct template_handler_t *template_handler, struct template_node_t *node) {
    /* allocates space for the iterator to be used to retrieve
    the various children from the node */
    struct iterator_t *child_iterator;

    /* allocates space for the child element */
    struct template_node_t *child;

    /* in case the node contains no children, it should
    be a leaf node (nothing to be done) */
    if(node->children == NULL) {
        /* returns immediately */
        return;
    }

    /* creates a "new" iterator for the children linked list */
    create_iterator_linked_list(node->children, &child_iterator);

    /* iterates continuously for children percolation */
    while(1) {
        /* retrieves the child element from the child iterator */
        get_next_iterator(child_iterator, (void **) &child);

        /* in case the child is not valid (no more items available) */
        if(child == NULL) {
            /* breaks the loop */
            break;
        }

        /* traverses the child node (recursion step) */
        traverse_node_buffer(template_handler, child);
    }

    /* deletes the child iterator */
    delete_iterator_linked_list(node->children, child_iterator);
}

void _traverse_out_buffer(struct template_handler_t *template_handler, struct template_node_t *node) {
    /* allocates space for the value parameter and for
    the value reference */
    struct template_parameter_t *value_parameter;
    struct type_t *value;

    /* allocates the buffer to hold the string conversion
    from the source data type of a possible reference */
    unsigned char *buffer;

    /* retrieves value parameter from the parameters map */
    get_value_string_hash_map(node->parameters_map, (unsigned char *) "value", (void **) &value_parameter);

    /* switches over the value parameter type to
    update the string buffer accordingly */
    switch(value_parameter->type) {
        case TEMPLATE_PARAMETER_STRING:
            /* adds the string value of the value to the string buffer */
            append_string_buffer(template_handler->string_buffer, value_parameter->string_value);

            /* breaks the switch */
            break;

        case TEMPLATE_PARAMETER_REFERENCE:
            /* retrievs the value reference from the global names map */
            get_template_handler(template_handler, value_parameter->reference_value, &value);

            /* in case the value was successfully found */
            if(value != NULL) {
                /* converts the value into a string representation, to
                be used in the template generation */
                to_string_type(value, &buffer);

                /* adds the value (string) to the string buffer */
                _append_string_buffer(template_handler->string_buffer, buffer);
            }

            /* breaks the switch */
            break;

        case TEMPLATE_PARAMETER_INTEGER:
            /* adds the raw value of the value to the string buffer */
            append_string_buffer(template_handler->string_buffer, value_parameter->raw_value);

            /* breaks the switch */
            break;

        case TEMPLATE_PARAMETER_FLOAT:
            /* adds the raw value of the value to the string buffer */
            append_string_buffer(template_handler->string_buffer, value_parameter->raw_value);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }
}

void _traverse_for_each_buffer(struct template_handler_t *template_handler, struct template_node_t *node) {
    /* allocates space for the item and the from parameters */
    struct template_parameter_t *item_parameter;
    struct template_parameter_t *from_parameter;

    /* allocates space for the (type) value, to be used
    in the retrieval of the value from the names map */
    struct type_t *value;

    /* allocates space for the list representing the linked
    list and for the iterator used for percolation */
    struct linked_list_t *list;
    struct iterator_t *iterator;

    /* allocates space for the current vale temporary variable */
    void *_current_value;

    /* retrieves both the item and from parameters from the parameters map */
    get_value_string_hash_map(node->parameters_map, (unsigned char *) "item", (void **) &item_parameter);
    get_value_string_hash_map(node->parameters_map, (unsigned char *) "from", (void **) &from_parameter);

    /* tries to retrieve the reference value from the map of names in the
    template handler (dereferencing) */
    get_template_handler(template_handler, from_parameter->reference_value, &value);

    /* in case the value was not found */
    if(value == NULL) {
        /* returns immediately */
        return;
    }

    /* sets the list as the value represented by the type */
    list = value->value.value_list;

    /* creates the iterator for the value, assumes it
    is iterable (fails otherwise) */
    create_iterator_linked_list(list, &iterator);

    /* iterates over all the available elements in the
    iterator element */
    while(1) {
        /* retrieves the next (current) value from the iterator */
        get_next_iterator(iterator, (void **) &_current_value);

        /* in case the current value is not set (end of sequence) */
        if(_current_value == NULL) {
            /* breaks the loop */
            break;
        }

        /* assigns the current iteration value to the global names
        in the template handler (to be used in the current context) and
        then traverses the child nodes of the node */
        assign_template_handler(template_handler, item_parameter->reference_value, _current_value);
        traverse_nodes_buffer(template_handler, node);
    }

    /* deletes the iterator */
    delete_iterator_linked_list(list, iterator);
}

void _traverse_if_buffer(struct template_handler_t *template_handler, struct template_node_t *node) {
    /* allocates space for the item, the value and the operator parameters */
    struct template_parameter_t *item_parameter;
    struct template_parameter_t *value_parameter;
    struct template_parameter_t *operator_parameter;

    /* allocates space for the value to be retrieved */
    struct type_t *value;

    /* retrieves both the from and the item parameters from the parameters map */
    get_value_string_hash_map(node->parameters_map, (unsigned char *) "item", (void **) &item_parameter);
    get_value_string_hash_map(node->parameters_map, (unsigned char *) "value", (void **) &value_parameter);
    get_value_string_hash_map(node->parameters_map, (unsigned char *) "operator", (void **) &operator_parameter);

    /* tries to retrieve the reference value from the map of names in the
    template handler (dereferencing) */
    get_template_handler(template_handler, (unsigned char *) "entry.type", &value);

    /* in case the value was not found */
    if(value == NULL) {
        /* returns immediately */
        return;
    }

    if(value->value.value_int != value_parameter->int_value) {
        return;
    }

    /* traverses the child nodes of the node
    (condition validated and verified) */
    traverse_nodes_buffer(template_handler, node);
}

ERROR_CODE _open_context_template_handler(struct template_handler_t *template_handler) {
    /* allocates space for the current and for the temporary node
    and retieves them from the template handeler */
    struct template_node_t *current_node = template_handler->current_node;
    struct template_node_t *temporary_node = template_handler->temporary_node;

    /* in case the list of context is not yet created,
    must be created for context memory */
    if(template_handler->contexts == NULL) {
        /* creates a new linked list of context for the template handler */
        create_linked_list(&template_handler->contexts);
    }

    /* adds the current node to the list of (past) contexts and
    sets the temporary node as the currect (context) node */
    append_value_linked_list(template_handler->contexts, current_node);
    template_handler->current_node = temporary_node;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _close_context_template_handler(struct template_handler_t *template_handler) {
    /* pops the last context into the template handler current node */
    pop_top_value_linked_list(template_handler->contexts, (void **) &template_handler->current_node, 1);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _text_begin_callback(struct template_engine_t *template_engine) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _text_end_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    /* allocates space for the template node */
    struct template_node_t *template_node;

    /* retrieves the template handler from the template engine context
    and retrieves the current node from it */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;
    struct template_node_t *current_node = template_handler->current_node;

    /* creates a new template node and sets the template
    node as the temporary node in the template handler */
    create_template_node(&template_node, TEMPLATE_NODE_TEXT);
    template_handler->temporary_node = template_node;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    template_node->name = (unsigned char *) MALLOC(size + 1);
    memcpy(template_node->name, pointer, size);
    template_node->name[size] = '\0';

    /* in case the children (list) are not defined for the
    current node node */
    if(current_node->children == NULL) {
        /* creates a new linked list for the children */
        create_linked_list(&current_node->children);
    }

    /* in case the nodes (list) are not defined for the
    temporary node */
    if(template_handler->nodes == NULL) {
        /* creates a new linked list for the nodes */
        create_linked_list(&template_handler->nodes);
    }

    /* adds the temporary node to the current list of nodes
    in the template handler and to the chilren list of the current node */
    append_value_linked_list(template_handler->nodes, template_node);
    append_value_linked_list(current_node->children, template_node);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tag_begin_callback(struct template_engine_t *template_engine) {
    /* allocates space for the template node */
    struct template_node_t *template_node;

    /* retrieves the template handler from the template engine context */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;

    /* creates a new template node and sets the template
    node as the temporary node in the template handler */
    create_template_node(&template_node, TEMPLATE_NODE_OPEN);
    template_handler->temporary_node = template_node;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tag_close_begin_callback(struct template_engine_t *template_engine) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;
    struct template_node_t *temporary_node = template_handler->temporary_node;

    /* sets the temporary node type as close */
    temporary_node->type = TEMPLATE_NODE_CLOSE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tag_end_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;
    struct template_node_t *temporary_node = template_handler->temporary_node;

    /* retrieves the current node from the template handler */
    struct template_node_t *current_node = template_handler->current_node;

    /* in case the node does contain the closing
    symbol at the final part of the tag (assumes single node) */
    if(pointer[size - 2] == '/') {
        /* sets the temporary node type as single */
        temporary_node->type = TEMPLATE_NODE_SINGLE;
    }

    /* switches over the temporary node type */
    switch(temporary_node->type) {
        case TEMPLATE_NODE_OPEN:
            /* opens a new context for the current node */
            _open_context_template_handler(template_handler);

            /* breaks the switch */
            break;

        case TEMPLATE_NODE_CLOSE:
            /* closes the current context (node closed) */
            _close_context_template_handler(template_handler);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

    /* in case the temporary node is of type close */
    if(temporary_node->type == TEMPLATE_NODE_CLOSE) {
        /* deletes the temporary node (no need to process it) */
        delete_template_node(temporary_node);

        /* raise no error */
        RAISE_NO_ERROR;
    }

    /* in case the children (list) are not defined for the
    current node */
    if(current_node->children == NULL) {
        /* creates a new linked list for the children */
        create_linked_list(&current_node->children);
    }

    /* in case the nodes (list) are not defined for the
    temporary node */
    if(template_handler->nodes == NULL) {
        /* creates a new linked list for the nodes */
        create_linked_list(&template_handler->nodes);
    }

    /* adds the temporary node to the current list of nodes
    in the template handler and to the chilren list of the temporary node */
    append_value_linked_list(current_node->children, temporary_node);
    append_value_linked_list(template_handler->nodes, temporary_node);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tag_name_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;
    struct template_node_t *temporary_node = template_handler->temporary_node;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    temporary_node->name = (unsigned char *) MALLOC(size + 1);
    memcpy(temporary_node->name, pointer, size);
    temporary_node->name[size] = '\0';

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _parameter_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;
    struct template_node_t *temporary_node = template_handler->temporary_node;

    /* allocates space for the parameter to be created */
    struct template_parameter_t *template_parameter;

    /* in case the parameters (list) are not defined for the
    temporary node */
    if(temporary_node->parameters == NULL) {
        /* creates a new linked list for the parameters */
        create_linked_list(&temporary_node->parameters);
    }

    /* in case the parameters (map) are not defined for the
    temporary node */
    if(temporary_node->parameters_map == NULL) {
        /* creates a new hash map for the parameters */
        create_hash_map(&temporary_node->parameters_map, 0);
    }

    /* creates the template parameter, sets it as the temporary parameter
    in the temporary node and adds it to the list of parameters */
    create_template_parameter(&template_parameter);
    temporary_node->temporary_parameter = template_parameter;
    append_value_linked_list(temporary_node->parameters, template_parameter);

    /* sets the name in the template parameter throught a memory copy */
    memcpy(template_parameter->name, pointer, size);
    template_parameter->name[size] = '\0';

    /* sets the parameter reference in the parameters map */
    set_value_string_hash_map(temporary_node->parameters_map, template_parameter->name, template_parameter);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _parameter_value_callback(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it and then uses it to retrieve
    the temporary parameter */
    struct template_handler_t *template_handler = (struct template_handler_t *) template_engine->context;
    struct template_node_t *temporary_node = template_handler->temporary_node;
    struct template_parameter_t *temporary_parameter = temporary_node->temporary_parameter;

    /* sets the raw value in the template parameter throught a memory copy */
    memcpy(temporary_parameter->raw_value, pointer, size);
    temporary_parameter->raw_value[size] = '\0';

    /* in case the first character of the raw value is
    a start string character the value must be a string */
    if(temporary_parameter->raw_value[0] == '"') {
        memcpy(temporary_parameter->string_value, pointer + 1, size - 2);
        temporary_parameter->string_value[size - 2] = '\0';
        temporary_parameter->type = TEMPLATE_PARAMETER_STRING;
    }
    /* in case the first character of the raw value is a numeric
    value it must be a number */
    else if(temporary_parameter->raw_value[0] > 0x2f && temporary_parameter->raw_value[0] < 0x58) {
        temporary_parameter->int_value = atoi((char *) temporary_parameter->raw_value);
        temporary_parameter->type = TEMPLATE_PARAMETER_INTEGER;
    }
    /* othwerwise it must be a (variable) reference value */
    else {
        memcpy(temporary_parameter->reference_value, pointer, size);
        temporary_parameter->reference_value[size] = '\0';
        temporary_parameter->type = TEMPLATE_PARAMETER_REFERENCE;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
