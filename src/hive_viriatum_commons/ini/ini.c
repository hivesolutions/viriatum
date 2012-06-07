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

#include "ini.h"

ERROR_CODE process_ini_file(char *file_path, struct hash_map_t **configuration_pointer) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the file */
    ERROR_CODE return_value;
    size_t index;
    size_t file_size;
    unsigned char *file_buffer;
    unsigned char character;
    enum ini_state_e state;

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *section_end_mark = 0;
    unsigned char *comment_end_mark = 0;
    unsigned char *key_end_mark = 0;
    unsigned char *value_end_mark = 0;

    /* allocates space for the settings to be used by
    the engine for the engine instance itself and for
    the handler to be used to "catch" the events, then
    retrieves the pointers to these structures*/
    struct ini_settings_t ini_settings_s;
    struct ini_engine_t ini_engine_s;
    struct ini_handler_t ini_handler_s;
    struct ini_settings_t *ini_settings = &ini_settings_s;
    struct ini_engine_t *ini_engine = &ini_engine_s;
    struct ini_handler_t *ini_handler = &ini_handler_s;

    /* allocates space for the hash map to be used for
    the configuration to be created */
    struct hash_map_t *configuration;

    /* creates the hash map that will hold the various
    arguments, then updates the configuration pointer
    with the created configuration hash map */
    create_hash_map(&configuration, 0);
    *configuration_pointer = configuration;

    /* sets the various handlers for the ini settings
    parsing, they will be used to correctly update the
    provided configuration hash map */
    ini_settings_s.on_section_start = NULL;
    ini_settings_s.on_section_end = _ini_section_end_callback;
    ini_settings_s.on_comment_start = NULL;
    ini_settings_s.on_comment_end = _ini_comment_end_callback;
    ini_settings_s.on_key_start = NULL;
    ini_settings_s.on_key_end = _ini_key_end_callback;
    ini_settings_s.on_value_start = NULL;
    ini_settings_s.on_value_end = _ini_value_end_callback;

    /* sets the configuration reference in the ini handler
    so that it may be updatd and then sets the handler in
    the ini engine instance to be used for parsing */
    ini_handler_s.configuration = configuration;
    ini_engine_s.context = ini_handler;

    /* reads the file contained in the provided file path
    and then tests the error code for error, in case there is an
    error prints it to the error stream output */
    return_value = read_file(file_path, &file_buffer, &file_size);
    if(IS_ERROR_CODE(return_value)) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading file"); }

    /* sets the initial state for the parsing process this
    is considered to be the "general loop" state */
    state = INI_ENGINE_NORMAL;

    /* iterates over the byte range of the file, all the bytes
    should be contained in the buffer "under" iteration */
    for(index = 0; index < file_size; index++) {
        /* retrieves the current character from the
        file buffer and the retrieves the pointer to
        its position */
        character = file_buffer[index];
        pointer = &file_buffer[index];

        switch(state) {
            case INI_ENGINE_NORMAL:
                if(character == '[') {
                    state = INI_ENGINE_SECTION;

                    INI_MARK_N(section_end, -1);
                    INI_CALLBACK(section_start);
                } else if(character == ';') {
                    state = INI_ENGINE_COMMENT;

                    INI_MARK_N(comment_end, -1);
                    INI_CALLBACK(comment_start);
                } else if(character == '\n' || character == '\r') {
                    state = INI_ENGINE_NORMAL;
                } else {
                    state = INI_ENGINE_KEY;

                    INI_MARK(key_end);
                    INI_CALLBACK(key_start);
                }

                break;

            case INI_ENGINE_SECTION:
                if(character == ']') {
                    state = INI_ENGINE_NORMAL;

                    INI_CALLBACK_DATA(section_end);
                }

                break;

            case INI_ENGINE_COMMENT:
                if(character == '\n' || character == '\r') {
                    state = INI_ENGINE_NORMAL;

                    INI_CALLBACK_DATA(comment_end);
                }

                break;

            case INI_ENGINE_KEY:
                if(character == '=') {
                    state = INI_ENGINE_VALUE;

                    INI_MARK_N(value_end, -1);
                    INI_CALLBACK_DATA(key_end);
                    INI_CALLBACK(value_start);
                }

                break;

            case INI_ENGINE_VALUE:
                if(character == '\n' || character == '\r') {
                    state = INI_ENGINE_NORMAL;

                    INI_CALLBACK_DATA(value_end);
                }

                break;
        }
    }

    /* releases the buffer used durring the parsing of
    the configuration file */
    FREE(file_buffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _ini_section_end_callback(struct ini_engine_t *ini_engine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) section */
    struct ini_handler_t *ini_handler = (struct ini_handler_t *) ini_engine->context;
    memcpy(ini_handler->section, pointer, size);
    ini_handler->section[size] = '\0';

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _ini_comment_end_callback(struct ini_engine_t *ini_engine, const unsigned char *pointer, size_t size) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _ini_key_end_callback(struct ini_engine_t *ini_engine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) value */
    struct ini_handler_t *ini_handler = (struct ini_handler_t *) ini_engine->context;
    memcpy(ini_handler->key, pointer, size);
    ini_handler->key[size] = '\0';

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _ini_value_end_callback(struct ini_engine_t *ini_engine, const unsigned char *pointer, size_t size) {
    /* allocates space for the hash map reference to
    hold the reference to the current section configuration */
    struct hash_map_t *section_configuration;

    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) value */
    struct ini_handler_t *ini_handler = (struct ini_handler_t *) ini_engine->context;
    char *value = MALLOC(size + 1);
    memcpy(value, pointer, size);
    value[size] = '\0';

    /* tries to retrieve the current section configuration from the
    configuration in case it's not possible to retrieve it a new
    configuration map must be created */
    get_value_string_hash_map(ini_handler->configuration, (unsigned char *) ini_handler->section, (void **) &section_configuration);
    if(section_configuration == NULL) {
        /* creates a new hash map to contain the section configuration
        and sets it under the current configuration variable in the ini
        handler for the current section name */
        create_hash_map(&section_configuration, 0);
        set_value_string_hash_map(ini_handler->configuration, (unsigned char *) ini_handler->section, section_configuration);
    }

    /* sets the current key and value under the current section configuration
    this is the main handler action */
    set_value_string_hash_map(section_configuration, (unsigned char *) ini_handler->key, value);

    /* raises no error */
    RAISE_NO_ERROR;
}
