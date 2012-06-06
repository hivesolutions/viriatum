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

ERROR_CODE processIniFile(char *file_path, struct hash_map_t **configurationPointer) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the file */
    ERROR_CODE returnValue;
    size_t index;
    size_t file_size;
    unsigned char *file_buffer;
    unsigned char character;
    enum IniState_e state;

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *sectionEndMark = 0;
    unsigned char *commentEndMark = 0;
    unsigned char *keyEndMark = 0;
    unsigned char *valueEndMark = 0;

    /* allocates space for the settings to be used by
    the engine for the engine instance itself and for
    the handler to be used to "catch" the events, then
    retrieves the pointers to these structures*/
    struct IniSettings_t iniSettings_s;
    struct IniEngine_t iniEngine_s;
    struct IniHandler_t iniHandler_s;
    struct IniSettings_t *iniSettings = &iniSettings_s;
    struct IniEngine_t *iniEngine = &iniEngine_s;
    struct IniHandler_t *iniHandler = &iniHandler_s;

    /* allocates space for the hash map to be used for
    the configuration to be created */
    struct hash_map_t *configuration;

    /* creates the hash map that will hold the various
    arguments, then updates the configuration pointer
    with the created configuration hash map */
    create_hash_map(&configuration, 0);
    *configurationPointer = configuration;

    /* sets the various handlers for the ini settings
    parsing, they will be used to correctly update the
    provided configuration hash map */
    iniSettings_s.onsectionStart = NULL;
    iniSettings_s.onsectionEnd = _iniSectionEndCallback;
    iniSettings_s.oncommentStart = NULL;
    iniSettings_s.oncommentEnd = _iniCommentEndCallback;
    iniSettings_s.onkeyStart = NULL;
    iniSettings_s.onkeyEnd = _iniKeyEndCallback;
    iniSettings_s.onvalueStart = NULL;
    iniSettings_s.onvalueEnd = _iniValueEndCallback;

    /* sets the configuration reference in the ini handler
    so that it may be updatd and then sets the handler in
    the ini engine instance to be used for parsing */
    iniHandler_s.configuration = configuration;
    iniEngine_s.context = iniHandler;

    /* reads the file contained in the provided file path
    and then tests the error code for error, in case there is an
    error prints it to the error stream output */
    returnValue = read_file(file_path, &file_buffer, &file_size);
    if(IS_ERROR_CODE(returnValue)) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading file"); }

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

                    INI_MARK_N(sectionEnd, -1);
                    INI_CALLBACK(sectionStart);
                } else if(character == ';') {
                    state = INI_ENGINE_COMMENT;

                    INI_MARK_N(commentEnd, -1);
                    INI_CALLBACK(commentStart);
                } else if(character == '\n' || character == '\r') {
                    state = INI_ENGINE_NORMAL;
                } else {
                    state = INI_ENGINE_KEY;

                    INI_MARK(keyEnd);
                    INI_CALLBACK(keyStart);
                }

                break;

            case INI_ENGINE_SECTION:
                if(character == ']') {
                    state = INI_ENGINE_NORMAL;

                    INI_CALLBACK_DATA(sectionEnd);
                }

                break;

            case INI_ENGINE_COMMENT:
                if(character == '\n' || character == '\r') {
                    state = INI_ENGINE_NORMAL;

                    INI_CALLBACK_DATA(commentEnd);
                }

                break;

            case INI_ENGINE_KEY:
                if(character == '=') {
                    state = INI_ENGINE_VALUE;

                    INI_MARK_N(valueEnd, -1);
                    INI_CALLBACK_DATA(keyEnd);
                    INI_CALLBACK(valueStart);
                }

                break;

            case INI_ENGINE_VALUE:
                if(character == '\n' || character == '\r') {
                    state = INI_ENGINE_NORMAL;

                    INI_CALLBACK_DATA(valueEnd);
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

ERROR_CODE _iniSectionEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) section */
    struct IniHandler_t *iniHandler = (struct IniHandler_t *) iniEngine->context;
    memcpy(iniHandler->section, pointer, size);
    iniHandler->section[size] = '\0';

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _iniCommentEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _iniKeyEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) value */
    struct IniHandler_t *iniHandler = (struct IniHandler_t *) iniEngine->context;
    memcpy(iniHandler->key, pointer, size);
    iniHandler->key[size] = '\0';

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _iniValueEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* allocates space for the hash map reference to
    hold the reference to the current section configuration */
    struct hash_map_t *sectionConfiguration;

    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) value */
    struct IniHandler_t *iniHandler = (struct IniHandler_t *) iniEngine->context;
    char *value = MALLOC(size + 1);
    memcpy(value, pointer, size);
    value[size] = '\0';

    /* tries to retrieve the current section configuration from the
    configuration in case it's not possible to retrieve it a new
    configuration map must be created */
    get_value_string_hash_map(iniHandler->configuration, (unsigned char *) iniHandler->section, (void **) &sectionConfiguration);
    if(sectionConfiguration == NULL) {
        /* creates a new hash map to contain the section configuration
        and sets it under the current configuration variable in the ini
        handler for the current section name */
        create_hash_map(&sectionConfiguration, 0);
        set_value_string_hash_map(iniHandler->configuration, (unsigned char *) iniHandler->section, sectionConfiguration);
    }

    /* sets the current key and value under the current section configuration
    this is the main handler action */
    set_value_string_hash_map(sectionConfiguration, (unsigned char *) iniHandler->key, value);

    /* raises no error */
    RAISE_NO_ERROR;
}
