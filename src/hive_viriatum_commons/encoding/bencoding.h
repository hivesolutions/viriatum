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

#define BENCODING_MARK(FOR) BENCODING_MARK_N(FOR, 0)
#define BENCODING_MARK_BACK(FOR) BENCODING_MARK_N(FOR, 1)
#define BENCODING_MARK_N(FOR, N)\
    do {\
        FOR##Mark = pointer - N;\
    } while(0)

#define BENCODING_CALLBACK(FOR)\
    do {\
        if(bencodingEngine->settings.on##FOR) {\
            if(bencodingEngine->settings.on##FOR(bencodingEngine) != 0) {\
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
            }\
        }\
    } while(0)

#define BENCODING_CALLBACK_DATA(FOR) BENCODING_CALLBACK_DATA_N(FOR, 0)
#define BENCODING_CALLBACK_DATA_BACK(FOR) BENCODING_CALLBACK_DATA_N(FOR, 1)
#define BENCODING_CALLBACK_DATA_N(FOR, N)\
    do {\
        if(FOR##Mark) {\
            if(bencodingEngine->settings.on##FOR) {\
                if(bencodingEngine->settings.on##FOR(bencodingEngine, FOR##Mark, pointer - FOR##Mark - N) != 0) {\
                    RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
                }\
            }\
            FOR##Mark = NULL;\
        }\
    } while(0)

struct BencodingEngine_t;

typedef ERROR_CODE (*bencodingCallback) (struct BencodingEngine_t *);
typedef ERROR_CODE (*bencodingDataCallback) (struct BencodingEngine_t *, const unsigned char *, size_t);

typedef enum BencodingState_e {
    BENCODING_ENGINE_NORMAL = 1,
    BENCODING_ENGINE_INTEGER,
    BENCODING_ENGINE_STRING,
    BENCODING_ENGINE_LIST,
    BENCODING_ENGINE_DICTIONARY,
    BENCODING_ENGINE_STRING_SIZE
} BencodingState;

typedef struct BencodingSettings_t {
    bencodingCallback onintegerStart;
    bencodingDataCallback onintegerEnd;
    bencodingCallback onstringStart;
    bencodingDataCallback onstringEnd;
    bencodingCallback onlistStart;
    bencodingCallback ondictionaryStart;
    bencodingCallback onsequenceEnd;
} BencodingSettings;

typedef struct BencodingEngine_t {
    enum BencodingState_e state;
    enum Type_e mapType;
    struct BencodingSettings_t settings;
    unsigned char *integerEndMark;
    unsigned char *stringEndMark;
    void *context;
} BencodingEngine;

typedef struct BencodingHandler_t {
    struct LinkedList_t *sequenceStack;
    struct LinkedList_t *keyStack;
    struct Type_t *sequence;
    struct Type_t *top;
    unsigned char *key;
    unsigned char nextKey;
} BencodingHandler;

VIRIATUM_EXPORT_PREFIX void createBencodingEngine(struct BencodingEngine_t **bencodingEnginePointer, void *context);
VIRIATUM_EXPORT_PREFIX void deleteBencodingEngine(struct BencodingEngine_t *bencodingEngine);
VIRIATUM_EXPORT_PREFIX void createBencodingHandler(struct BencodingHandler_t **bencodingHandlerPointer);
VIRIATUM_EXPORT_PREFIX void deleteBencodingHandler(struct BencodingHandler_t *bencodingHandler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE encodeBencoding(struct Type_t *type, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE decodeBencoding(unsigned char *encodedBuffer, size_t encodedBufferLength, struct Type_t **typePointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE encodeBencodingFile(char *filePath, struct Type_t *type);
VIRIATUM_EXPORT_PREFIX ERROR_CODE decodeBencodingFile(char *filePath, struct Type_t **typePointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _encodeType(struct Type_t *type, struct StringBuffer_t *stringBuffer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _startBencodingEngine(struct BencodingEngine_t **bencodingEnginePointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _stopBencodingEngine(struct BencodingEngine_t *bencodingEngine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _runBencodingEngine(struct BencodingEngine_t *bencodingEngine, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencodingIntegerEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencodingStringEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencodingListStartCallback(struct BencodingEngine_t *bencodingEngine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencodingDictionaryStartCallback(struct BencodingEngine_t *bencodingEngine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencodingSequenceEndCallback(struct BencodingEngine_t *bencodingEngine);
