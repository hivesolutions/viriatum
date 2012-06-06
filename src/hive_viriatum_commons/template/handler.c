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

void create_template_handler(struct TemplateHandler_t **templateHandlerPointer) {
    /* retrieves the template handler size */
    size_t templateHandlerSize = sizeof(struct TemplateHandler_t);

    /* allocates space for the template handler */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) MALLOC(templateHandlerSize);

    /* sets the default values in the template handler */
    template_handler->string_value = NULL;
    template_handler->currentNode = NULL;
    template_handler->temporaryNode = NULL;
    template_handler->nodes = NULL;
    template_handler->contexts = NULL;

    /* creates a new hash map for the names */
    create_hash_map(&template_handler->names, 0);

    /* creates a new string buffer for buffering
    the result of the template processing */
    create_string_buffer(&template_handler->string_buffer);

    /* creates the list to hold the various types
    to have the memory released uppon destruction */
    create_linked_list(&template_handler->releaseList);

    /* sets the template engine in the template handler pointer */
    *templateHandlerPointer = template_handler;
}

void delete_template_handler(struct TemplateHandler_t *template_handler) {
    /* allocates space for the temporary node variable */
    struct TemplateNode_t *node;

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
            deleteTemplateNode(node);
        }

        /* deletes the nodes list */
        delete_linked_list(template_handler->nodes);
    }

    /* iterates continuously for release list
    cleanup (type memory release) */
    while(1) {
        /* pops a node from the release list */
        pop_value_linked_list(template_handler->releaseList, (void **) &type, 1);

        /* in case the value is invalid (empty list) */
        if(type == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the type */
        delete_type(type);
    }

    /* deletes the list of release types from the template handler */
    delete_linked_list(template_handler->releaseList);

    /* deletes the string buffer */
    delete_string_buffer(template_handler->string_buffer);

    /* deletes the names hash map */
    delete_hash_map(template_handler->names);

    /* releases the template handler */
    FREE(template_handler);
}

void createTemplateNode(struct TemplateNode_t **templateNodePointer, enum TemplateNodeType_e type) {
    /* retrieves the template node size */
    size_t templateNodeSize = sizeof(struct TemplateNode_t);

    /* allocates space for the template node */
    struct TemplateNode_t *templateNode = (struct TemplateNode_t *) MALLOC(templateNodeSize);

    /* sets the (node) type in the template node */
    templateNode->type = type;

    /* sets the default values in the template node */
    templateNode->name = NULL;
    templateNode->children = NULL;
    templateNode->parameters = NULL;
    templateNode->parametersMap = NULL;
    templateNode->temporaryParameter = NULL;

    /* sets the template engine in the template node pointer */
    *templateNodePointer = templateNode;
}

void deleteTemplateNode(struct TemplateNode_t *templateNode) {
    /* allocates space for the temporary parameter variable */
    struct TemplateParameter_t *parameter;

    /* in case the template parameters map are defined */
    if(templateNode->parametersMap) {
        /* deletes the parameters map */
        delete_hash_map(templateNode->parametersMap);
    }

    /* in case the template parameters are defined */
    if(templateNode->parameters) {
        /* iterates continuously for parameters list
        cleanup (removal of all parameters) */
        while(1) {
            /* pops a parameter from the parameters list */
            pop_value_linked_list(templateNode->parameters, (void **) &parameter, 1);

            /* in case the value is invalid (empty list) */
            if(parameter == NULL) {
                /* breaks the cycle */
                break;
            }

            /* deletes the template parameter */
            deleteTemplateParameter(parameter);
        }

        /* deletes the parameters list */
        delete_linked_list(templateNode->parameters);
    }

    /* in case the template children are defined */
    if(templateNode->children) {
        /* deletes the children list */
        delete_linked_list(templateNode->children);
    }

    /* in case the name is defined in the template node */
    if(templateNode->name) {
        /* releases the template node name */
        FREE(templateNode->name);
    }

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
    templateParameter->intValue = 0;
    templateParameter->floatValue = 0.0;

    /* sets the template engine in the template parameter pointer */
    *templateParameterPointer = templateParameter;
}

void deleteTemplateParameter(struct TemplateParameter_t *templateParameter) {
    /* releases the template parameter */
    FREE(templateParameter);
}

void process_template_handler(struct TemplateHandler_t *template_handler, unsigned char *file_path) {
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
    template_handler->currentNode = rootNode;

    /* sets the context (template handler) in the template engine */
    templateEngine->context = template_handler;

    /* sets the various callbacks in the template settings */
    templateSettings->ontextBegin = _textBeginCallback;
    templateSettings->ontextEnd = _textEndCallback;
    templateSettings->ontagBegin = _tagBeginCallback;
    templateSettings->ontagCloseBegin = _tagCloseBeginCallback;
    templateSettings->ontagEnd = _tagEndCallback;
    templateSettings->ontagName = _tagNameCallback;
    templateSettings->onparameter = _parameterCallback;
    templateSettings->onparameterValue = _parameterValueCallback;

    /* processes the file as a template engine and then uses the
    created node structure to traverse for string buffer output */
    processTemplateEngine(templateEngine, templateSettings, file_path);
    traverseNodeBuffer(template_handler, template_handler->currentNode);

    /* "joins" the template handler string buffer into the string
    value, retrieving the final template result */
    join_string_buffer(template_handler->string_buffer, &template_handler->string_value);

    /* deletes the now unecessary root node */
    deleteTemplateNode(rootNode);

    /* deletes the template settings */
    deleteTemplateSettings(templateSettings);

    /* deletes the template engine */
    deleteTemplateEngine(templateEngine);
}

void assignTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, struct type_t *value) {
    /* sets the a new name (key) in the names hash map */
    set_value_string_hash_map(template_handler->names, name, value);
}

void assignIntegerTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, int value) {
    /* allocates space for the type */
    struct type_t *type;

    /* create a integer type and sets the value
    to the list to be assigned */
    create_type(&type, INTEGER_TYPE);
    type->value.value_int = value;

    /* sets the value (integer) in the template handler names hash map */
    set_value_string_hash_map(template_handler->names, name, type);

    /* adds the type to the list of types to have the
    memory release uppon template handler destruction (late removal) */
    append_value_linked_list(template_handler->releaseList, type);
}

void assignListTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, struct linked_list_t *value) {
    /* allocates space for the type */
    struct type_t *type;

    /* create a list type and sets the value
    to the list to be assigned */
    create_type(&type, LIST_TYPE);
    type->value.value_list = value;

    /* sets the value (list) in the template handler names hash map */
    set_value_string_hash_map(template_handler->names, name, type);

    /* adds the type to the list of types to have the
    memory release uppon template handler destruction (late removal) */
    append_value_linked_list(template_handler->releaseList, type);
}

void getTemplateHandler(struct TemplateHandler_t *template_handler, unsigned char *name, struct type_t **value_pointer) {
    /* allocates space for the temporary (tokanizable) name variable
    and for the generated token values*/
    unsigned char _name[64];
    unsigned char *nameToken;
    unsigned char *context;

    /* allocates space for the (type) value */
    struct type_t *value;

    /* retrieves the template global names as thee base value */
    struct hash_map_t *_value = template_handler->names;

    /* copies the name reference into a backup value
    to avoid data corruption (from tokenization) */
    memcpy(_name, name, strlen((char *) name) + 1);

    /* tokenizes the name into tokens */
    nameToken = (unsigned char *) STRTOK((char *) _name, ".", context);

    /* iterates continuously */
    while(1) {
        /* in case the name token is invalid
        (no more tokens available) */
        if(nameToken == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrieves the value from the current value with the
        name token reference key value */
        get_value_string_hash_map(_value, nameToken, (void **) &value);

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
        nameToken = (unsigned char *) STRTOK(NULL, ".", context);
    }

    /* sets the value pointer with the internal
    retrieved value */
    *value_pointer = value;
}

void traverseNodeDebug(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node, unsigned int indentation) {
    /* allocates space for the iterator to be used to retrieve
    the various children from the node */
    struct iterator_t *childIterator;

    /* allocates space for the child element */
    struct TemplateNode_t *child;

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
    create_iterator_linked_list(node->children, &childIterator);

    /* iterates continuously for children percolation */
    while(1) {
        /* retrieves the child element from the child iterator */
        get_next_iterator(childIterator, (void **) &child);

        /* in case the child is not valid (no more items available) */
        if(child == NULL) {
            /* breaks the loop */
            break;
        }

        /* traverses the child node (recursion step) */
        traverseNodeDebug(template_handler, child, indentation + 1);
    }

    /* deletes the child iterator */
    delete_iterator_linked_list(node->children, childIterator);
}

void traverseNodeBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node) {
    /* switches over the type of node to be traversed,
    to print the correct value */
    switch(node->type) {
        case TEMPLATE_NODE_ROOT:
            /* traverses all the nodes in the root node */
            traverseNodesBuffer(template_handler, node);

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
                _traverseOutBuffer(template_handler, node);
            } else if(strcmp((char *) node->name, "foreach") == 0) {
                /* traverses the foreach node in buffer mode */
                _traverseForEachBuffer(template_handler, node);
            } else if(strcmp((char *) node->name, "if") == 0) {
                /* traverses the if node in buffer mode */
                _traverseIfBuffer(template_handler, node);
            }

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }
}

void traverseNodesBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node) {
    /* allocates space for the iterator to be used to retrieve
    the various children from the node */
    struct iterator_t *childIterator;

    /* allocates space for the child element */
    struct TemplateNode_t *child;

    /* in case the node contains no children, it should
    be a leaf node (nothing to be done) */
    if(node->children == NULL) {
        /* returns immediately */
        return;
    }

    /* creates a "new" iterator for the children linked list */
    create_iterator_linked_list(node->children, &childIterator);

    /* iterates continuously for children percolation */
    while(1) {
        /* retrieves the child element from the child iterator */
        get_next_iterator(childIterator, (void **) &child);

        /* in case the child is not valid (no more items available) */
        if(child == NULL) {
            /* breaks the loop */
            break;
        }

        /* traverses the child node (recursion step) */
        traverseNodeBuffer(template_handler, child);
    }

    /* deletes the child iterator */
    delete_iterator_linked_list(node->children, childIterator);
}

void _traverseOutBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node) {
    /* allocates space for the value parameter and for
    the value reference */
    struct TemplateParameter_t *valueParameter;
    struct type_t *value;

    /* allocates the buffer to hold the string conversion
    from the source data type of a possible reference */
    unsigned char *buffer;

    /* retrieves value parameter from the parameters map */
    get_value_string_hash_map(node->parametersMap, (unsigned char *) "value", (void **) &valueParameter);

    /* switches over the value parameter type to
    update the string buffer accordingly */
    switch(valueParameter->type) {
        case TEMPLATE_PARAMETER_STRING:
            /* adds the string value of the value to the string buffer */
            append_string_buffer(template_handler->string_buffer, valueParameter->string_value);

            /* breaks the switch */
            break;

        case TEMPLATE_PARAMETER_REFERENCE:
            /* retrievs the value reference from the global names map */
            getTemplateHandler(template_handler, valueParameter->referenceValue, &value);

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
            append_string_buffer(template_handler->string_buffer, valueParameter->rawValue);

            /* breaks the switch */
            break;

        case TEMPLATE_PARAMETER_FLOAT:
            /* adds the raw value of the value to the string buffer */
            append_string_buffer(template_handler->string_buffer, valueParameter->rawValue);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }
}

void _traverseForEachBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node) {
    /* allocates space for the item and the from parameters */
    struct TemplateParameter_t *itemParameter;
    struct TemplateParameter_t *fromParameter;

    /* allocates space for the (type) value, to be used
    in the retrieval of the value from the names map */
    struct type_t *value;

    /* allocates space for the list representing the linked
    list and for the iterator used for percolation */
    struct linked_list_t *list;
    struct iterator_t *iterator;

    /* allocates space for the current vale temporary variable */
    void *_currentValue;

    /* retrieves both the item and from parameters from the parameters map */
    get_value_string_hash_map(node->parametersMap, (unsigned char *) "item", (void **) &itemParameter);
    get_value_string_hash_map(node->parametersMap, (unsigned char *) "from", (void **) &fromParameter);

    /* tries to retrieve the reference value from the map of names in the
    template handler (dereferencing) */
    getTemplateHandler(template_handler, fromParameter->referenceValue, &value);

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
        get_next_iterator(iterator, (void **) &_currentValue);

        /* in case the current value is not set (end of sequence) */
        if(_currentValue == NULL) {
            /* breaks the loop */
            break;
        }

        /* assigns the current iteration value to the global names
        in the template handler (to be used in the current context) and
        then traverses the child nodes of the node */
        assignTemplateHandler(template_handler, itemParameter->referenceValue, _currentValue);
        traverseNodesBuffer(template_handler, node);
    }

    /* deletes the iterator */
    delete_iterator_linked_list(list, iterator);
}

void _traverseIfBuffer(struct TemplateHandler_t *template_handler, struct TemplateNode_t *node) {
    /* allocates space for the item, the value and the operator parameters */
    struct TemplateParameter_t *itemParameter;
    struct TemplateParameter_t *valueParameter;
    struct TemplateParameter_t *operatorParameter;

    /* allocates space for the value to be retrieved */
    struct type_t *value;

    /* retrieves both the from and the item parameters from the parameters map */
    get_value_string_hash_map(node->parametersMap, (unsigned char *) "item", (void **) &itemParameter);
    get_value_string_hash_map(node->parametersMap, (unsigned char *) "value", (void **) &valueParameter);
    get_value_string_hash_map(node->parametersMap, (unsigned char *) "operator", (void **) &operatorParameter);

    /* tries to retrieve the reference value from the map of names in the
    template handler (dereferencing) */
    getTemplateHandler(template_handler, (unsigned char *) "entry.type", &value);

    /* in case the value was not found */
    if(value == NULL) {
        /* returns immediately */
        return;
    }

    if(value->value.value_int != valueParameter->intValue) {
        return;
    }

    /* traverses the child nodes of the node
    (condition validated and verified) */
    traverseNodesBuffer(template_handler, node);
}

ERROR_CODE _openContextTemplateHandler(struct TemplateHandler_t *template_handler) {
    /* allocates space for the current and for the temporary node
    and retieves them from the template handeler */
    struct TemplateNode_t *currentNode = template_handler->currentNode;
    struct TemplateNode_t *temporaryNode = template_handler->temporaryNode;

    /* in case the list of context is not yet created,
    must be created for context memory */
    if(template_handler->contexts == NULL) {
        /* creates a new linked list of context for the template handler */
        create_linked_list(&template_handler->contexts);
    }

    /* adds the current node to the list of (past) contexts and
    sets the temporary node as the currect (context) node */
    append_value_linked_list(template_handler->contexts, currentNode);
    template_handler->currentNode = temporaryNode;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _closeContextTemplateHandler(struct TemplateHandler_t *template_handler) {
    /* pops the last context into the template handler current node */
    pop_top_value_linked_list(template_handler->contexts, (void **) &template_handler->currentNode, 1);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _textBeginCallback(struct TemplateEngine_t *templateEngine) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _textEndCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* allocates space for the template node */
    struct TemplateNode_t *templateNode;

    /* retrieves the template handler from the template engine context
    and retrieves the current node from it */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *currentNode = template_handler->currentNode;

    /* creates a new template node and sets the template
    node as the temporary node in the template handler */
    createTemplateNode(&templateNode, TEMPLATE_NODE_TEXT);
    template_handler->temporaryNode = templateNode;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    templateNode->name = (unsigned char *) MALLOC(size + 1);
    memcpy(templateNode->name, pointer, size);
    templateNode->name[size] = '\0';

    /* in case the children (list) are not defined for the
    current node node */
    if(currentNode->children == NULL) {
        /* creates a new linked list for the children */
        create_linked_list(&currentNode->children);
    }

    /* in case the nodes (list) are not defined for the
    temporary node */
    if(template_handler->nodes == NULL) {
        /* creates a new linked list for the nodes */
        create_linked_list(&template_handler->nodes);
    }

    /* adds the temporary node to the current list of nodes
    in the template handler and to the chilren list of the current node */
    append_value_linked_list(template_handler->nodes, templateNode);
    append_value_linked_list(currentNode->children, templateNode);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tagBeginCallback(struct TemplateEngine_t *templateEngine) {
    /* allocates space for the template node */
    struct TemplateNode_t *templateNode;

    /* retrieves the template handler from the template engine context */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;

    /* creates a new template node and sets the template
    node as the temporary node in the template handler */
    createTemplateNode(&templateNode, TEMPLATE_NODE_OPEN);
    template_handler->temporaryNode = templateNode;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tagCloseBeginCallback(struct TemplateEngine_t *templateEngine) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = template_handler->temporaryNode;

    /* sets the temporary node type as close */
    temporaryNode->type = TEMPLATE_NODE_CLOSE;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tagEndCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = template_handler->temporaryNode;

    /* retrieves the current node from the template handler */
    struct TemplateNode_t *currentNode = template_handler->currentNode;

    /* in case the node does contain the closing
    symbol at the final part of the tag (assumes single node) */
    if(pointer[size - 2] == '/') {
        /* sets the temporary node type as single */
        temporaryNode->type = TEMPLATE_NODE_SINGLE;
    }

    /* switches over the temporary node type */
    switch(temporaryNode->type) {
        case TEMPLATE_NODE_OPEN:
            /* opens a new context for the current node */
            _openContextTemplateHandler(template_handler);

            /* breaks the switch */
            break;

        case TEMPLATE_NODE_CLOSE:
            /* closes the current context (node closed) */
            _closeContextTemplateHandler(template_handler);

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

    /* in case the temporary node is of type close */
    if(temporaryNode->type == TEMPLATE_NODE_CLOSE) {
        /* deletes the temporary node (no need to process it) */
        deleteTemplateNode(temporaryNode);

        /* raise no error */
        RAISE_NO_ERROR;
    }

    /* in case the children (list) are not defined for the
    current node */
    if(currentNode->children == NULL) {
        /* creates a new linked list for the children */
        create_linked_list(&currentNode->children);
    }

    /* in case the nodes (list) are not defined for the
    temporary node */
    if(template_handler->nodes == NULL) {
        /* creates a new linked list for the nodes */
        create_linked_list(&template_handler->nodes);
    }

    /* adds the temporary node to the current list of nodes
    in the template handler and to the chilren list of the temporary node */
    append_value_linked_list(currentNode->children, temporaryNode);
    append_value_linked_list(template_handler->nodes, temporaryNode);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _tagNameCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = template_handler->temporaryNode;

    /* allocates the space for the temporary node name and
    sets it with a memory copy */
    temporaryNode->name = (unsigned char *) MALLOC(size + 1);
    memcpy(temporaryNode->name, pointer, size);
    temporaryNode->name[size] = '\0';

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _parameterCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = template_handler->temporaryNode;

    /* allocates space for the parameter to be created */
    struct TemplateParameter_t *templateParameter;

    /* in case the parameters (list) are not defined for the
    temporary node */
    if(temporaryNode->parameters == NULL) {
        /* creates a new linked list for the parameters */
        create_linked_list(&temporaryNode->parameters);
    }

    /* in case the parameters (map) are not defined for the
    temporary node */
    if(temporaryNode->parametersMap == NULL) {
        /* creates a new hash map for the parameters */
        create_hash_map(&temporaryNode->parametersMap, 0);
    }

    /* creates the template parameter, sets it as the temporary parameter
    in the temporary node and adds it to the list of parameters */
    createTemplateParameter(&templateParameter);
    temporaryNode->temporaryParameter = templateParameter;
    append_value_linked_list(temporaryNode->parameters, templateParameter);

    /* sets the name in the template parameter throught a memory copy */
    memcpy(templateParameter->name, pointer, size);
    templateParameter->name[size] = '\0';

    /* sets the parameter reference in the parameters map */
    set_value_string_hash_map(temporaryNode->parametersMap, templateParameter->name, templateParameter);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _parameterValueCallback(struct TemplateEngine_t *templateEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the template handler from the template engine context
    and retrieves the temporary node from it and then uses it to retrieve
    the temporary parameter */
    struct TemplateHandler_t *template_handler = (struct TemplateHandler_t *) templateEngine->context;
    struct TemplateNode_t *temporaryNode = template_handler->temporaryNode;
    struct TemplateParameter_t *temporaryParameter = temporaryNode->temporaryParameter;

    /* sets the raw value in the template parameter throught a memory copy */
    memcpy(temporaryParameter->rawValue, pointer, size);
    temporaryParameter->rawValue[size] = '\0';

    /* in case the first character of the raw value is
    a start string character the value must be a string */
    if(temporaryParameter->rawValue[0] == '"') {
        memcpy(temporaryParameter->string_value, pointer + 1, size - 2);
        temporaryParameter->string_value[size - 2] = '\0';
        temporaryParameter->type = TEMPLATE_PARAMETER_STRING;
    }
    /* in case the first character of the raw value is a numeric
    value it must be a number */
    else if(temporaryParameter->rawValue[0] > 0x2f && temporaryParameter->rawValue[0] < 0x58) {
        temporaryParameter->intValue = atoi((char *) temporaryParameter->rawValue);
        temporaryParameter->type = TEMPLATE_PARAMETER_INTEGER;
    }
    /* othwerwise it must be a (variable) reference value */
    else {
        memcpy(temporaryParameter->referenceValue, pointer, size);
        temporaryParameter->referenceValue[size] = '\0';
        temporaryParameter->type = TEMPLATE_PARAMETER_REFERENCE;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}
