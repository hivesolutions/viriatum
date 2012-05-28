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

#include "../io/io.h"
#include "../debug/debug.h"
#include "../structures/structures.h"

/**
 * The maximum size for a key contained
 * inside and ini file.
 */
#define INI_KEY_MAX_SIZE 1024

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

/**
 * Enumeration describing the various states
 * occuring during the parsing of an ini file.
 */
typedef enum IniState_e {
    INI_ENGINE_NORMAL = 1,
    INI_ENGINE_SECTION,
    INI_ENGINE_KEY,
    INI_ENGINE_VALUE,
    INI_ENGINE_COMMENT
} IniEngineState;

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
    char section[INI_KEY_MAX_SIZE];
    char key[INI_KEY_MAX_SIZE];
    struct HashMap_t *configuration;
} IniHandler;

VIRIATUM_EXPORT_PREFIX ERROR_CODE processIniFile(char *filePath, struct HashMap_t **configurationPointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _iniSectionEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _iniCommentEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _iniKeyEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _iniValueEndCallback(struct IniEngine_t *iniEngine, const unsigned char *pointer, size_t size);
