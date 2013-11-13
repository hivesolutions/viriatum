/*
 Hive Viriatum Commons
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "../stream/stream.h"

/**
 * The size to be used in the huffman interna
 * buffer storage.
 */
#define HUFFMAN_BUFFER_SIZE 4096

/**
 * The maximum number of symbols that are considered
 * for a default huffman table, this value should be
 * the same as the maximum number of symbols represented
 * by an eight bit based byte.
 */
#define HUFFMAN_SYMBOL_SIZE 256

/**
 * Structure that hold the information for the
 * creation of huffman based dictionary/tables.
 */
typedef struct huffman_t {
    size_t freqs[HUFFMAN_SYMBOL_SIZE];
} huffman;

typedef struct huffman_node_t {
    struct huffman_node_t *left;
    struct huffman_node_t *right;
    unsigned char symbol;
    unsigned char byte;
    unsigned char bit_count;
} huffman_node;

VIRIATUM_EXPORT_PREFIX void create_huffman(struct huffman_t **huffman_pointer);
VIRIATUM_EXPORT_PREFIX void delete_huffman(struct huffman_t *huffman);
VIRIATUM_EXPORT_PREFIX void generate_table_huffman(struct huffman_t *huffman, struct stream_t *stream);
