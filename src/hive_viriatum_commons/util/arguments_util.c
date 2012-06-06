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

#include "arguments_util.h"

ERROR_CODE process_arguments(int argc, char *argv[], struct hash_map_t **arguments_pointer) {
    /* allocates space for the index counter */
    unsigned int index;

    /* allocates the space for the current argument string
    in the iteration */
    char *current_argument;

    /* allocates space for both the argument holder, to
    be created every iteration of the argument retrieval */
    struct argument_t *argument;
    struct hash_map_t *arguments;

    /* creates the hash map that will hold the various
    arguments */
    create_hash_map(&arguments, 0);

    /* iterates over all the argument except the
    first (file name value) */
    for(index = 1; index < (unsigned int) argc; index++) {
        /* allocates space for the "new" argument to be parsed */
        argument = (struct argument_t *) MALLOC(sizeof(struct argument_t));

        /* retrievs the current argument, then processes it
        using the default (simple) parser and sets it in
        the arguments map */
        current_argument = argv[index];
        _process_argument(current_argument, argument);
        set_value_string_hash_map(arguments, (unsigned char *) argument->key, (void *) argument);
    }

    /* sets the hash map of arguments as the value pointed
    by the arguments pointer */
    *arguments_pointer = arguments;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_arguments(struct hash_map_t *arguments) {
    /* allocates space for the pointer to the element and
    for the argument to be retrieved */
    struct hash_map_element_t *element;
    struct argument_t *argument;

    /* allocates space for the iterator for the arguments */
    struct iterator_t *arguments_iterator;

    /* creates an iterator for the arguments hash map */
    create_element_iterator_hash_map(arguments, &arguments_iterator);

    /* iterates continuously */
    while(1) {
        /* retrieves the next value from the arguments iterator */
        get_next_iterator(arguments_iterator, (void **) &element);

        /* in case the current module is null (end of iterator) */
        if(element == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrievs the hash map value for the key */
        get_value_hash_map(arguments, element->key, element->key_string, (void **) &argument);

        /* releases the argument memory */
        FREE(argument);
    }

    /* deletes the iterator for the arguments hash map */
    delete_iterator_hash_map(arguments, arguments_iterator);

    /* deletes the hash map that holds the arguments */
    delete_hash_map(arguments);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE print_arguments(struct hash_map_t *arguments) {
    /* allocates space for the pointer to the element and
    for the argument to be retrieved */
    struct hash_map_element_t *element;
    struct argument_t *argument;

    /* allocates space for the iterator for the arguments */
    struct iterator_t *arguments_iterator;

    /* creates an iterator for the arguments hash map */
    create_iterator_hash_map(arguments, &arguments_iterator);

    /* prints the initial arguments header */
    PRINTF("Arguments\n");

    /* iterates continuously */
    while(1) {
        /* retrieves the next value from the arguments iterator */
        get_next_iterator(arguments_iterator, (void **) &element);

        /* in case the current module is null (end of iterator) */
        if(element == NULL) {
            /* breaks the loop */
            break;
        }

        /* retrievs the hash map value for the key */
        get_value_hash_map(arguments, element->key, element->key_string, (void **) &argument);

        /* in case the argument is of type value */
        if(argument->type == VALUE_ARGUMENT) {
            /* prints the value with the key and argument */
            PRINTF_F(" %s => %s\n", argument->key, argument->value);
        }
        /* otherwise it must be a key only argument */
        else {
            /* prints just the key and a null reference */
            PRINTF_F(" %s => NULL\n", argument->key);
        }
    }

    /* deletes the iterator for the arguments hash map */
    delete_iterator_hash_map(arguments, arguments_iterator);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _process_argument(char *argument_value, struct argument_t *argument) {
    /* allocates space for the index counter */
    unsigned int index;

    /* allocates space for the current value in the
    parsing loop (iteration item) */
    char current;

    /* starts the state value with the initial value */
    unsigned int state = ARGUMENT_INITIAL;

    /* starts the mark value wth the base index */
    size_t mark = 0;

    /* retrieves the argument size for processing */
    size_t argument_size = strlen(argument_value);

    /* sets the argument to type key (just a name) */
    argument->type = SINGLE_ARGUMENT;

    /* iterates over all the characters present in
    the argument string to parse the argument */
    for(index = 0; index < argument_size; index++) {
        /* retrieves the current iteration values, the
        current character of the argument */
        current = argument_value[index];

        /* switch over the current (parsing) state */
        switch(state) {
            case ARGUMENT_INITIAL:
                /* in case the current character is not a
                separator character there is an error */
                if(current != '-') {
                    /* raises an error */
                    RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem parsing option");
                }

                /* changes the state to the first */
                state = ARGUMENT_FIRST;

                /* breaks the switch */
                break;

            case ARGUMENT_FIRST:
                if(current == '-') {
                    /* marks the next index as the start point
                    for the key string */
                    mark = index + 1;

                    /* changes the state to the second */
                    state = ARGUMENT_SECOND;
                } else {
                    /* marks the current index as the start point
                    for the key string */
                    mark = index;

                    /* changes the state to the key (parsing) */
                    state = ARGUMENT_KEY;
                }

                /* breaks the switch */
                break;

            case ARGUMENT_SECOND:
            case ARGUMENT_KEY:
                if(current == '=') {
                    /* sets the argument to type key and value */
                    argument->type = VALUE_ARGUMENT;

                    /* copies the key value into the appropriate variable
                    in the argument */
                    memcpy(argument->key, argument_value + mark, index - mark);
                    argument->key[index - mark] = '\0';

                    /* marks the current index as the start point
                    for the value string */
                    mark = index + 1;

                    /* changes the state to the value (parsing) */
                    state = ARGUMENT_VALUE;
                } else {
                }

                /* breaks the switch */
                break;
        }
    }

    /* switches one final time over the state to close
    the pending values */
    switch(state) {
        case ARGUMENT_SECOND:
        case ARGUMENT_KEY:
            /* copies the key value into the appropriate variable
            in the argument */
            memcpy(argument->key, argument_value + mark, index - mark);
            argument->key[index - mark] = '\0';

            /* breaks the switch */
            break;

        case ARGUMENT_VALUE:
            /* copies the value value into the appropriate variable
            in the argument */
            memcpy(argument->value, argument_value + mark, index - mark);
            argument->value[index - mark] = '\0';

            /* breaks the switch */
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}
