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

#include "bencoding.h"

void create_bencoding_engine(struct bencoding_engine_t **bencoding_engine_pointer, void *context) {
    /* retrieves the bencoding engine size */
    size_t bencoding_engine_size = sizeof(struct bencoding_engine_t);

    /* allocates space for the bencoding engine */
    struct bencoding_engine_t *bencoding_engine = (struct bencoding_engine_t *) MALLOC(bencoding_engine_size);

    /* initializes the bencoding engine */
    bencoding_engine->state = BENCODING_ENGINE_NORMAL;
    bencoding_engine->map_type = SORT_MAP_TYPE;
    bencoding_engine->integer_end_mark = 0;
    bencoding_engine->string_end_mark = 0;
    bencoding_engine->context = context;

    /* sets the bencoding engine in the bencoding engine pointer */
    *bencoding_engine_pointer = bencoding_engine;
}

void delete_bencoding_engine(struct bencoding_engine_t *bencoding_engine) {
    /* releases the bencoding engine */
    FREE(bencoding_engine);
}

void create_bencoding_handler(struct bencoding_handler_t **bencoding_handler_pointer) {
    /* retrieves the bencoding handler size */
    size_t bencoding_handler_size = sizeof(struct bencoding_handler_t);

    /* allocates space for the bencoding handler */
    struct bencoding_handler_t *bencoding_handler = (struct bencoding_handler_t *) MALLOC(bencoding_handler_size);

    /* initializes the bencoding handler */
    bencoding_handler->sequence = NULL;
    bencoding_handler->top = NULL;
    bencoding_handler->key = NULL;
    bencoding_handler->next_key = 0;

    /* creates the linked lists for both the sequence and key stack
    values (used for runtime) */
    create_linked_list(&bencoding_handler->sequence_stack);
    create_linked_list(&bencoding_handler->key_stack);

    /* sets the bencoding handler in the bencoding handler pointer */
    *bencoding_handler_pointer = bencoding_handler;
}

void delete_bencoding_handler(struct bencoding_handler_t *bencoding_handler) {
    /* in case the sequence stack and the key stack are defined releases
    their memory to avoid any leaks */
    if(bencoding_handler->sequence_stack) { FREE(bencoding_handler->sequence_stack); }
    if(bencoding_handler->key_stack) { FREE(bencoding_handler->key_stack); }

    /* releases the bencoding handler */
    FREE(bencoding_handler);
}

ERROR_CODE encode_bencoding(struct type_t *type, unsigned char **encoded_puffer_pointer, size_t *encoded_buffer_length_pointer) {
    /* allocates space for the encoded buffer reference and
    for the string buffer reference */
    unsigned char *encoded_buffer;
    struct string_buffer_t *string_buffer;

    /* creates the string buffer an then uses it to support
    the encoding of the provided type (top level type) then
    after the encoding is complete joins the string buffer and
    deletes it from memory*/
    create_string_buffer(&string_buffer);
    _encode_type(type, string_buffer);
    join_string_buffer(string_buffer, &encoded_buffer);
    delete_string_buffer(string_buffer);

    /* updates the references to the encoded buffer pointer and
    the reference to the encoded buffer length (from string length) */
    *encoded_puffer_pointer = encoded_buffer;
    *encoded_buffer_length_pointer = strlen((char *) encoded_buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_bencoding(unsigned char *encoded_buffer, size_t encoded_buffer_length, struct type_t **type_pointer) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the buffer */
    struct bencoding_engine_t *bencoding_engine;
    struct bencoding_handler_t *bencoding_handler;

    /* starts the bencoding engine, starting all of its internal
    structures and then runs an engine tick parsing the complete
    file buffer (it's completely available) */
    _start_bencoding_engine(&bencoding_engine);
    _run_bencoding_engine(bencoding_engine, encoded_buffer, encoded_buffer_length);

    /* retieves the handler associated with the engine and then
    uses it to retrieve the top type of the parsed structure */
    bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;
    *type_pointer = bencoding_handler->top;

    /* stops the bencoding engine releasing all of its internal
    structures, the top type is still defined */
    _stop_bencoding_engine(bencoding_engine);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_bencoding_file(char *file_path, struct type_t *type) {
    /* allocates space for the reference to the buffer
    to hold the encoded contents and for the integer to
    hold the size of that buffer */
    unsigned char *encoded_buffer;
    size_t encoded_buffer_length;

    /* runs the encoding process over the provided type structure
    and then uses the resulting buffer to write it to the file */
    encode_bencoding(type, &encoded_buffer, &encoded_buffer_length);
    write_file(file_path, encoded_buffer, encoded_buffer_length);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_bencoding_file(char *file_path, struct type_t **type_pointer) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the file */
    ERROR_CODE return_value;
    size_t file_size;
    unsigned char *file_buffer;
    struct bencoding_engine_t *bencoding_engine;
    struct bencoding_handler_t *bencoding_handler;

    /* reads the file contained in the provided file path
    and then tests the error code for error, in case there is an
    error prints it to the error stream output */
    return_value = read_file(file_path, &file_buffer, &file_size);
    if(IS_ERROR_CODE(return_value)) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading file"); }

    /* starts the bencoding engine, starting all of its internal
    structures and then runs an engine tick parsing the complete
    file buffer (it's completely available) */
    _start_bencoding_engine(&bencoding_engine);
    _run_bencoding_engine(bencoding_engine, file_buffer, file_size);

    /* retieves the handler associated with the engine and then
    uses it to retrieve the top type of the parsed structure */
    bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;
    *type_pointer = bencoding_handler->top;

    /* stops the bencoding engine releasing all of its internal
    structures, the top type is still defined */
    _stop_bencoding_engine(bencoding_engine);

    /* releases the buffer used durring the parsing of
    the configuration file */
    FREE(file_buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _encode_type(struct type_t *type, struct string_buffer_t *string_buffer) {
    char *buffer;
    struct type_t key;
    struct type_t *current;
    struct iterator_t *iterator;
    struct hash_map_element_t *element;

    switch(type->type) {
        case INTEGER_TYPE:
            buffer = MALLOC(16);
            SPRINTF(buffer, 16, "%d", type->value.value_int);

            append_string_buffer(string_buffer, (unsigned char *) "i");
            _append_string_buffer(string_buffer, (unsigned char *) buffer);
            append_string_buffer(string_buffer, (unsigned char *) "e");

            break;

        case STRING_TYPE:
            buffer = MALLOC(16);
            SPRINTF(buffer, 16, "%lu", strlen(type->value.value_string));

            _append_string_buffer(string_buffer, (unsigned char *) buffer);
            append_string_buffer(string_buffer, (unsigned char *) ":");
            append_string_buffer(string_buffer, (unsigned char *) type->value.value_string);

            break;

        case LIST_TYPE:
            append_string_buffer(string_buffer, (unsigned char *) "l");

            create_iterator_linked_list(type->value.value_list, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &current);
                if(current == NULL) { break; }
                _encode_type(current, string_buffer);
            }

            delete_iterator_linked_list(type->value.value_list, iterator);

            append_string_buffer(string_buffer, (unsigned char *) "e");

            break;

        case MAP_TYPE:
            append_string_buffer(string_buffer, (unsigned char *) "d");

            create_element_iterator_hash_map(type->value.value_map, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                key = string_type((char *) element->key_string);
                _encode_type(&key, string_buffer);
                _encode_type((struct type_t *) element->value, string_buffer);
            }

            delete_iterator_hash_map(type->value.value_map, iterator);

            append_string_buffer(string_buffer, (unsigned char *) "e");

            break;

        case SORT_MAP_TYPE:
            append_string_buffer(string_buffer, (unsigned char *) "d");

            create_element_iterator_sort_map(type->value.value_sort_map, &iterator);

            while(1) {
                get_next_iterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                key = string_type((char *) element->key_string);
                _encode_type(&key, string_buffer);
                _encode_type((struct type_t *) element->value, string_buffer);
            }

            delete_iterator_sort_map(type->value.value_sort_map, iterator);

            append_string_buffer(string_buffer, (unsigned char *) "e");

            break;

        default:
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _start_bencoding_engine(struct bencoding_engine_t **bencoding_engine_pointer) {
    /* allocates space for the various bencoding
    related structures for usage in runtime */
    struct bencoding_engine_t *bencoding_engine;
    struct bencoding_handler_t *bencoding_handler;
    struct bencoding_settings_t *bencoding_settings;

    /* creates the bencoding handler structure and then creates
    the bencoding engine setting the handler as the context */
    create_bencoding_handler(&bencoding_handler);
    create_bencoding_engine(&bencoding_engine, (void *) bencoding_handler);

    /* retrieves the memory reference to the engine settings */
    bencoding_settings = &bencoding_engine->settings;

    /* sets the various handlers for the bencoding settings
    parsing, they will be used to correctly update the
    provided configuration hash map */
    bencoding_settings->on_integer_start = NULL;
    bencoding_settings->on_integer_end = _bencoding_integer_end_callback;
    bencoding_settings->on_string_start = NULL;
    bencoding_settings->on_string_end = _bencoding_string_end_callback;
    bencoding_settings->on_list_start = _bencoding_list_start_callback;
    bencoding_settings->on_dictionary_start = _bencoding_dictionary_start_callback;
    bencoding_settings->on_sequence_end = _bencoding_sequence_end_callback;

    /* sets the bencoding engine in the bencoding engine pointer,
    this allows the caller function to use the engine */
    *bencoding_engine_pointer = bencoding_engine;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _stop_bencoding_engine(struct bencoding_engine_t *bencoding_engine) {
    /* deletes both the handler referenced as context in the engine and the
    current context engine (memory cleanup) */
    delete_bencoding_handler((struct bencoding_handler_t *) bencoding_engine->context);
    delete_bencoding_engine(bencoding_engine);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _run_bencoding_engine(struct bencoding_engine_t *bencoding_engine, unsigned char *buffer, size_t size) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the buffer */
    size_t index;
    size_t string_size;
    unsigned char character;
    enum bencoding_state_e state;

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *integer_end_mark = 0;
    unsigned char *string_end_mark = 0;

    /* updates the current state value from the engine
    references (state update) */
    state = bencoding_engine->state;
    integer_end_mark = bencoding_engine->integer_end_mark;
    string_end_mark = bencoding_engine->string_end_mark;

    /* initializes the string size value, avoids
    possible problems with no variable initialization */
    string_size = 0;

    /* iterates over the byte range of the file, all the bytes
    should be contained in the buffer "under" iteration */
    for(index = 0; index < size; index++) {
        /* retrieves the current character from the
        file buffer and the retrieves the pointer to
        its position */
        character = buffer[index];
        pointer = &buffer[index];

        switch(state) {
            case BENCODING_ENGINE_NORMAL:
                switch(character) {
                    case 'i':
                        state = BENCODING_ENGINE_INTEGER;

                        BENCODING_MARK_N(integer_end, -1);
                        BENCODING_CALLBACK(integer_start);

                        break;

                    case 'l':
                        BENCODING_CALLBACK(list_start);

                        break;

                    case 'd':
                        BENCODING_CALLBACK(dictionary_start);

                        break;

                    case 'e':
                        BENCODING_CALLBACK(sequence_end);

                        break;

                    case ' ':
                        break;

                    default:
                        state = BENCODING_ENGINE_STRING_SIZE;
                        string_size = character - '0';

                        break;
                }

                break;

            case BENCODING_ENGINE_INTEGER:
                if(character == 'e') {
                    state = BENCODING_ENGINE_NORMAL;

                    BENCODING_CALLBACK_DATA(integer_end);
                }

                break;

            case BENCODING_ENGINE_STRING:
                if(string_size == 1) {
                    state = BENCODING_ENGINE_NORMAL;

                    BENCODING_CALLBACK_DATA_N(string_end, -1);
                } else {
                    string_size--;
                }

                break;

            case BENCODING_ENGINE_STRING_SIZE:
                if(character == ':') {
                    state = BENCODING_ENGINE_STRING;

                    BENCODING_MARK_N(string_end, -1);
                    BENCODING_CALLBACK(string_start);
                } else {
                    string_size *= 10;
                    string_size += character - '0';
                }

                break;

            default:
                break;
        }
    }

    /* saves the various state related values back
    to the bencoding engine */
    bencoding_engine->state = state;
    bencoding_engine->integer_end_mark = integer_end_mark;
    bencoding_engine->string_end_mark = string_end_mark;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencoding_integer_end_callback(struct bencoding_engine_t *bencoding_engine, const unsigned char *pointer, size_t size) {
    /* allocates space for the integer value and for
    the type that will encapsulate it */
    int _integer;
    struct type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct bencoding_handler_t *bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;

    /* allocates space for the integer string value and then copies
    the data buffer into it and terminates the null string */
    char *integer = MALLOC(size + 1);
    memcpy(integer, pointer, size);
    integer[size] = '\0';

    /* in case the sequence type is defined the integer must
    be a value and must be associated with the sequence */
    if(bencoding_handler->sequence != NULL) {
        /* converts the integer string into an integer value, then creates
        the type structure for the integer and sets it as an integer value */
        _integer = atoi(integer);
        create_type(&type, INTEGER_TYPE);
        *type = integer_type(_integer);

        /* switches over the type of current sequence to
        execute the proper operations */
        switch(bencoding_handler->sequence->type) {
            case LIST_TYPE:
                /* adds the current type to the list sequence */
                append_value_linked_list(bencoding_handler->sequence->value.value_list, (void *) type);

                /* breaks the switch */
                break;

            case MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                set_value_string_hash_map(bencoding_handler->sequence->value.value_map, bencoding_handler->key, (void *) type);
                bencoding_handler->next_key = 1;

                /* breaks the switch */
                break;

            case SORT_MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                set_value_string_sort_map(bencoding_handler->sequence->value.value_sort_map, bencoding_handler->key, (void *) type);
                bencoding_handler->next_key = 1;

                /* breaks the switch */
                break;

            default:
                /* breaks the switch */
                break;
        }
    }

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencoding_handler->top == NULL) { bencoding_handler->top = type; }

    /* releases the memory associated with the intger string value
    in order to avoid memory leaks */
    FREE(integer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencoding_string_end_callback(struct bencoding_engine_t *bencoding_engine, const unsigned char *pointer, size_t size) {
    /* allocates space for the reference to the
    type structure to hold the string */
    struct type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct bencoding_handler_t *bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;

    /* allocates memory for the string and then copies the source pointer
    data into the string buffer for the provided size and then closes it */
    char *string = MALLOC(size + 1);
    memcpy(string, pointer, size);
    string[size] = '\0';

    /* in case the next key flag is active must set the current key as
    the just retrieved string */
    if(bencoding_handler->next_key == 1) {
        /* in case there's a key pending to be release must release it
        to avoid memory leaks, then sets the current string as the key
        and unsets the next key flag to save the value */
        if(bencoding_handler->key != NULL) { FREE(bencoding_handler->key); }
        bencoding_handler->key = (unsigned char *) string;
        bencoding_handler->next_key = 0;
    }
    /* otherwise in case the sequence type is defined the string must
    be a value and must be associated with the sequence */
    else if(bencoding_handler->sequence != NULL) {
        /* creates a new type structure for the string
        and sets it with the correct string value */
        create_type(&type, STRING_TYPE);
        *type = string_type(string);

        /* switches over the type of current sequence to
        execute the proper operations */
        switch(bencoding_handler->sequence->type) {
            case LIST_TYPE:
                /* adds the current type to the list sequence */
                append_value_linked_list(bencoding_handler->sequence->value.value_list, (void *) type);

                /* breaks the switch */
                break;

            case MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                set_value_string_hash_map(bencoding_handler->sequence->value.value_map, bencoding_handler->key, (void *) type);
                bencoding_handler->next_key = 1;

                /* breaks the switch */
                break;

            case SORT_MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                set_value_string_sort_map(bencoding_handler->sequence->value.value_sort_map, bencoding_handler->key, (void *) type);
                bencoding_handler->next_key = 1;

                /* breaks the switch */
                break;

            default:
                /* breaks the switch */
                break;
        }
    }

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencoding_handler->top == NULL) { bencoding_handler->top = type; }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencoding_list_start_callback(struct bencoding_engine_t *bencoding_engine) {
    /* allocates space for the linked list and for the type
    structure that will encapsulate it */
    struct linked_list_t *list;
    struct type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct bencoding_handler_t *bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;

    /* creates a new linked list for the new structure (sequence) context
    and then creates the respective type for the list */
    create_linked_list(&list);
    create_type(&type, LIST_TYPE);
    *type = list_type(list);

    /* adds the sequence type to the sequence stack and then adds the
    current handler key to the key stack, then updates the current sequence
    reference and the current key in the handler */
    append_value_linked_list(bencoding_handler->sequence_stack, (void *) type);
    append_value_linked_list(bencoding_handler->key_stack, (void *) bencoding_handler->key);
    bencoding_handler->sequence = type;
    bencoding_handler->key = NULL;

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencoding_handler->top == NULL) { bencoding_handler->top = type; }

    /* unsets the next key flag to avoid any possible misbehavior, colliding
    with the hash map structure */
    bencoding_handler->next_key = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencoding_dictionary_start_callback(struct bencoding_engine_t *bencoding_engine) {
    /* allocates space for the hash map and for the type
    structure that will encapsulate it */
    struct hash_map_t *hash_map;
    struct sort_map_t *sort_map;
    struct type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct bencoding_handler_t *bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;

    /* switchs over the "target" map type to be used to store conceptual
    maps in the bencoding structure */
    switch(bencoding_engine->map_type) {
        case MAP_TYPE:
            /* creates a new hash map for the new structure (sequence) context
            and then creates the respective type for the map */
            create_hash_map(&hash_map, 0);
            create_type(&type, MAP_TYPE);
            *type = map_type(hash_map);

        case SORT_MAP_TYPE:
            /* creates a new sort map for the new structure (sequence) context
            and then creates the respective type for the map */
            create_sort_map(&sort_map, 0);
            create_type(&type, SORT_MAP_TYPE);
            *type = sort_map_type(sort_map);

        default:
            /* creates a new sort map for the new structure (sequence) context
            and then creates the respective type for the map */
            create_sort_map(&sort_map, 0);
            create_type(&type, SORT_MAP_TYPE);
            *type = sort_map_type(sort_map);
    }

    /* adds the sequence type to the sequence stack and then adds the
    current handler key to the key stack, then updates the current sequence
    reference and the current key in the handler */
    append_value_linked_list(bencoding_handler->sequence_stack, (void *) type);
    append_value_linked_list(bencoding_handler->key_stack, (void *) bencoding_handler->key);
    bencoding_handler->sequence = type;
    bencoding_handler->key = NULL;

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencoding_handler->top == NULL) { bencoding_handler->top = type; }

    /* sets the next key flag so that in the next iteration the string
    is "accepted" as the current key value */
    bencoding_handler->next_key = 1;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencoding_sequence_end_callback(struct bencoding_engine_t *bencoding_engine) {
    /* allocates space for the map key for the sequence
    type and for the current type */
    unsigned char *key;
    struct type_t *sequence;
    struct type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct bencoding_handler_t *bencoding_handler = (struct bencoding_handler_t *) bencoding_engine->context;

    /* pops the current key in the stack and the current sequence in the
    stack (these are the current values), then peeks the current sequence
    in the stack (in case it exists) it should represent the sequence to
    be set in use for the current context */
    pop_top_value_linked_list(bencoding_handler->key_stack, (void **) &key, 1);
    pop_top_value_linked_list(bencoding_handler->sequence_stack, (void **) &sequence, 1);
    peek_top_value_linked_list(bencoding_handler->sequence_stack, (void **) &type);
    bencoding_handler->sequence = type;

    /* in case the current (previous) sequence is a map, must do some
    garbage collection to avoid leeks */
    if(sequence->type == MAP_TYPE || sequence->type == SORT_MAP_TYPE) {
        /* in case there is a key currently defined in the handler
        must release its memory (to avoid any leaks) */
        if(bencoding_handler->key != NULL) { FREE(bencoding_handler->key); }
        bencoding_handler->key = NULL;
    }

    /* in case there is no sequence defined for the current handler context
    no need to continue the processing (nothing to be assiciated) this is
    typical for the top level of parsing */
    if(bencoding_handler->sequence == NULL) { RAISE_NO_ERROR; }

    /* swithces over the sequence type to take the appropriate action of
    setting the lower (previous) context in the upper (current) context */
    switch(bencoding_handler->sequence->type) {
        case LIST_TYPE:
            /* adds the lower value to the upper list appending it to the
            back of the lis */
            append_value_linked_list(bencoding_handler->sequence->value.value_list, (void *) sequence);

            /* unsets the next key flag to avoid any unexpected string parsing
            behavior (hash map only) */
            bencoding_handler->next_key = 0;

            /* breaks the switch */
            break;

        case MAP_TYPE:
            /* sets the lower value in the upper map for the current key
            this is the lower upper layer association */
            set_value_string_hash_map(bencoding_handler->sequence->value.value_map, key, (void *) sequence);

            /* sets the retrieved key as the current key and
            sets the next key flag to force the retrieval of key */
            bencoding_handler->key = key;
            bencoding_handler->next_key = 1;

            /* breaks the switch */
            break;

        case SORT_MAP_TYPE:
            /* sets the lower value in the upper map for the current key
            this is the lower upper layer association */
            set_value_string_sort_map(bencoding_handler->sequence->value.value_sort_map, key, (void *) sequence);

            /* sets the retrieved key as the current key and
            sets the next key flag to force the retrieval of key */
            bencoding_handler->key = key;
            bencoding_handler->next_key = 1;

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}
