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

typedef enum IniState_e {
    INI_ENGINE_NORMAL = 1,
    INI_ENGINE_SECTION,
    INI_ENGINE_KEY,
    INI_ENGINE_VALUE,
    INI_ENGINE_COMMENT
} IniEngineState;


#define INI_MARK(FOR) INI_MARK_N(FOR, 0)
#define INI_MARK_BACK(FOR) INI_MARK_N(FOR, 1)
#define INI_MARK_N(FOR, N)\
    do {\
        FOR##Mark = pointer - N;\
    } while(0)

#define INI_CALLBACK(FOR)\
    do {\
        if(iniSettings->on##FOR) {\
            if(iniSettings->on##FOR(iniEngine) != 0) {\
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
            }\
        }\
    } while(0)

#define INI_CALLBACK_DATA(FOR) INI_CALLBACK_DATA_N(FOR, 0)
#define INI_CALLBACK_DATA_BACK(FOR) INI_CALLBACK_DATA_N(FOR, 1)
#define INI_CALLBACK_DATA_N(FOR, N)\
    do {\
        if(FOR##Mark) {\
            if(iniSettings->on##FOR) {\
                if(iniSettings->on##FOR(iniEngine, FOR##Mark, pointer - FOR##Mark - N) != 0) {\
                    RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
                }\
            }\
            FOR##Mark = NULL;\
        }\
    } while(0)

struct IniEngine_t;

typedef ERROR_CODE (*iniCallback) (struct IniEngine_t *);
typedef ERROR_CODE (*iniDataCallback) (struct IniEngine_t *, const unsigned char *, size_t);



typedef struct IniSettings_t {
    iniCallback onsectionStart;
    iniDataCallback onsectionEnd;
    iniCallback oncommentStart;
    iniDataCallback oncommentEnd;
    iniCallback onkeyStart;
    iniDataCallback onkeyEnd;
    iniCallback onvalueStart;
    iniDataCallback onvalueEnd;
} IniSettings;

typedef struct IniEngine_t {
    void *context;
} IniEngine;

typedef struct IniHandler_t {
    char *section;
    char *key;
    char *value;
    struct HashMap_t *configuration;
} IniHandler;

ERROR_CODE _sectionEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) section */
    struct IniHandler_t *iniHandler = (struct IniHandler_t *) iniEngine->context;
    iniHandler->section = MALLOC(size + 1);
    memcpy(iniHandler->section, pointer, size);
    iniHandler->section[size] = '\0';

    V_PRINT_F("SECTION -> '%s'\n", iniHandler->section);

    RAISE_NO_ERROR;
}

ERROR_CODE _commentEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    char comment[128];

    memcpy(comment, pointer, size);
    comment[size] = '\0';

    V_PRINT_F("COMMENT -> '%s'\n", comment);

    RAISE_NO_ERROR;
}

ERROR_CODE _keyEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) value */
    struct IniHandler_t *iniHandler = (struct IniHandler_t *) iniEngine->context;
    iniHandler->key = MALLOC(size + 1);
    memcpy(iniHandler->key, pointer, size);
    iniHandler->key[size] = '\0';

    V_PRINT_F("KEY -> '%s'\n", iniHandler->key);

    RAISE_NO_ERROR;
}

ERROR_CODE _valueEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size) {
    /* retrieves the ini handler from the template engine context
    then uses it to store the (current) value */
    struct IniHandler_t *iniHandler = (struct IniHandler_t *) iniEngine->context;
    iniHandler->value = MALLOC(size + 1);
    memcpy(iniHandler->value, pointer, size);
    iniHandler->value[size] = '\0';


    setValueStringHashMap(iniHandler->configuration, (unsigned char *) iniHandler->key, iniHandler->value);


    V_PRINT_F("VALUE -> '%s'\n", iniHandler->value);

    RAISE_NO_ERROR;
}


ERROR_CODE readFilessssss(char *filePath, unsigned char **bufferPointer, size_t *fileSizePointer) {
    /* allocates space for the file */
    FILE *file;

    /* allocates space for the file size */
    size_t fileSize;

    /* allocates space for the file buffer */
    unsigned char *fileBuffer;

    /* allocates space for the number of bytes */
    size_t numberBytes;

    /* opens the file */
    FOPEN(&file, filePath, "rb");

    /* in case the file is not found */
    if(file == NULL) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
    }

    /* seeks the file until the end */
    fseek(file, 0, SEEK_END);

    /* retrieves the file size */
    fileSize = ftell(file);

    /* seeks the file until the beginning */
    fseek(file, 0, SEEK_SET);

    /* allocates space for the file buffer */
    fileBuffer = (unsigned char *) MALLOC(fileSize);

    /* reads the file contents */
    numberBytes = fread(fileBuffer, 1, fileSize, file);

    /* in case the number of read bytes is not the
    same as the total bytes in file (error) */
    if(numberBytes != fileSize) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading from file");
    }

    /* closes the file */
    fclose(file);

    /* sets the buffer as the buffer pointer */
    *bufferPointer = fileBuffer;

    /* sets the file size as the file size pointer */
    *fileSizePointer = fileSize;

    /* raise no error */
    RAISE_NO_ERROR;
}



ERROR_CODE processIniFile(char *filePath, struct HashMap_t **configurationPointer) {
    ERROR_CODE returnValue;
    size_t index;
    size_t fileSize;
    unsigned char *fileBuffer;
    unsigned char character;
    enum IniState_e state;

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *sectionEndMark = 0;
    unsigned char *commentEndMark = 0;
    unsigned char *keyEndMark = 0;
    unsigned char *valueEndMark = 0;

    struct IniSettings_t iniSettings_s;
    struct IniEngine_t iniEngine_s;
    struct IniHandler_t iniHandler_s;

    struct IniSettings_t *iniSettings = &iniSettings_s;
    struct IniEngine_t *iniEngine = &iniEngine_s;
    struct IniHandler_t *iniHandler = &iniHandler_s;

    struct HashMap_t *configuration;



    /* creates the hash map that will hold the various
    arguments */
    createHashMap(&configuration, 0);



    iniSettings_s.onsectionStart = NULL;
    iniSettings_s.onsectionEnd = _sectionEndCallback;
    iniSettings_s.oncommentStart = NULL;
    iniSettings_s.oncommentEnd = _commentEndCallback;
    iniSettings_s.onkeyStart = NULL;
    iniSettings_s.onkeyEnd = _keyEndCallback;
    iniSettings_s.onvalueStart = NULL;
    iniSettings_s.onvalueEnd = _valueEndCallback;

    iniHandler_s.configuration = configuration;


    iniEngine_s.context = iniHandler;


    returnValue = readFilessssss(filePath, &fileBuffer, &fileSize);

    /* tests the error code for error, in case there is an
    error prints it to the error stream output */
    if(IS_ERROR_CODE(returnValue)) { V_ERROR_F("Problem reading file (%s)\n", (char *) GET_ERROR()); }

    state = INI_ENGINE_NORMAL;

    for(index = 0; index < fileSize; index++) {
        character = fileBuffer[index];
        pointer = &fileBuffer[index];


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

    /* updates the configuration pointer with the created
    configuration has map */
    *configurationPointer = configuration;

    /* raises no error */
    RAISE_NO_ERROR;
}
