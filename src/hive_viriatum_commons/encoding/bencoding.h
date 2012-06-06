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
        FOR##_mark = pointer - N;\
    } while(0)

#define BENCODING_CALLBACK(FOR)\
    do {\
        if(bencoding_engine->settings.on_##FOR) {\
            if(bencoding_engine->settings.on_##FOR(bencoding_engine) != 0) {\
                RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
            }\
        }\
    } while(0)

#define BENCODING_CALLBACK_DATA(FOR) BENCODING_CALLBACK_DATA_N(FOR, 0)
#define BENCODING_CALLBACK_DATA_BACK(FOR) BENCODING_CALLBACK_DATA_N(FOR, 1)
#define BENCODING_CALLBACK_DATA_N(FOR, N)\
    do {\
        if(FOR##_mark) {\
            if(bencoding_engine->settings.on_##FOR) {\
                if(bencoding_engine->settings.on_##FOR(bencoding_engine, FOR##_mark, pointer - FOR##_mark - N) != 0) {\
                    RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem handling callback"); \
                }\
            }\
            FOR##_mark = NULL;\
        }\
    } while(0)

struct bencoding_engine_t;

typedef ERROR_CODE (*bencoding_callback) (struct bencoding_engine_t *);
typedef ERROR_CODE (*bencoding_data_callback) (struct bencoding_engine_t *, const unsigned char *, size_t);

typedef enum bencoding_state_e {
    BENCODING_ENGINE_NORMAL = 1,
    BENCODING_ENGINE_INTEGER,
    BENCODING_ENGINE_STRING,
    BENCODING_ENGINE_LIST,
    BENCODING_ENGINE_DICTIONARY,
    BENCODING_ENGINE_STRING_SIZE
} bencoding_state;

typedef struct bencoding_settings_t {
    bencoding_callback on_integer_start;
    bencoding_data_callback on_integer_end;
    bencoding_callback on_string_start;
    bencoding_data_callback on_string_end;
    bencoding_callback on_list_start;
    bencoding_callback on_dictionary_start;
    bencoding_callback on_sequence_end;
} bencoding_settings;

typedef struct bencoding_engine_t {
    enum bencoding_state_e state;
    enum type_e map_type;
    struct bencoding_settings_t settings;
    unsigned char *integer_end_mark;
    unsigned char *string_end_mark;
    void *context;
} bencoding_engine;

typedef struct bencoding_handler_t {
    struct linked_list_t *sequence_stack;
    struct linked_list_t *key_stack;
    struct type_t *sequence;
    struct type_t *top;
    unsigned char *key;
    unsigned char next_key;
} bencoding_handler;

VIRIATUM_EXPORT_PREFIX void create_bencoding_engine(struct bencoding_engine_t **bencoding_engine_pointer, void *context);
VIRIATUM_EXPORT_PREFIX void delete_bencoding_engine(struct bencoding_engine_t *bencoding_engine);
VIRIATUM_EXPORT_PREFIX void create_bencoding_handler(struct bencoding_handler_t **bencoding_handler_pointer);
VIRIATUM_EXPORT_PREFIX void delete_bencoding_handler(struct bencoding_handler_t *bencoding_handler);
VIRIATUM_EXPORT_PREFIX ERROR_CODE encode_bencoding(struct type_t *type, unsigned char **encoded_buffer_pointer, size_t *encoded_buffer_length_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE decode_bencoding(unsigned char *encoded_buffer, size_t encoded_buffer_length, struct type_t **type_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE encode_bencoding_file(char *file_path, struct type_t *type);
VIRIATUM_EXPORT_PREFIX ERROR_CODE decode_bencoding_file(char *file_path, struct type_t **type_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _encode_type(struct type_t *type, struct string_buffer_t *string_buffer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _start_bencoding_engine(struct bencoding_engine_t **bencoding_engine_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _stop_bencoding_engine(struct bencoding_engine_t *bencoding_engine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _run_bencoding_engine(struct bencoding_engine_t *bencoding_engine, unsigned char *buffer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencoding_integer_end_callback(struct bencoding_engine_t *bencoding_engine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencoding_string_end_callback(struct bencoding_engine_t *bencoding_engine, const unsigned char *pointer, size_t size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencoding_list_start_callback(struct bencoding_engine_t *bencoding_engine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencoding_dictionary_start_callback(struct bencoding_engine_t *bencoding_engine);
VIRIATUM_EXPORT_PREFIX ERROR_CODE _bencoding_sequence_end_callback(struct bencoding_engine_t *bencoding_engine);
