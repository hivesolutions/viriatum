/*
 Hive Viriatum Commons
 Copyright (c) 2008-2018 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../stream/stream.h"
#include "../structures/structures.h"

/**
 * The size of a byte unit as defined for the current
 * huffman implementation, remember that this value is
 * not the same for all the architectures.
 */
#define HUFFMAN_BYTE_SIZE 8

/**
 * The range of code values that are allowed for
 * an huffman structure, this value should match
 * a basic size of a data type (eg: 8 bits, 16 bits).
 */
#define HUFFMAN_CODE_RANGE 65536

/**
 * The size to be used in the huffman internal
 * buffer storage, all the buffer to be allocated
 * under the huffman infra-structure use this size.
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
 * Structure that describes an element (node) that
 * is going to be used in the construction of the
 * huffman tree used in the encoding and decoding.
 */
typedef struct huffman_node_t {
    struct huffman_node_t *left;
    struct huffman_node_t *right;
    size_t value;
    unsigned char symbol;
    unsigned short code;
    unsigned char bit_count;
} huffman_node;

/**
 * Structure that hold the information for the
 * creation of huffman based dictionary/tables.
 */
typedef struct huffman_t {
    /**
     * The number of bits contained in the encoded
     * buffer resulting from the current huffman
     * structure, in case there's one.
     */
    long long bit_count;

    /**
     * The number of valid symbols that are going to
     * be used in the current huffman context. A symbol
     * may or may not be an ascii based 8 bit value.
     */
    unsigned char symbol_count;

    /**
     * The size in bits of the maximum sized prefix
     * value for the current huffman structure. This
     * value may be used when constructing the prefix
     * tables to know the number of bits to be used.
     */
    unsigned char prefix_size;

    /**
     * The reference to the root node of the huffman
     * table for the current state, this may be used
     * when percolating the tree.
     */
    struct huffman_node_t *root;

    /**
     * Array that associates a code (symbol) to the
     * appropriate node in the huffman tree, this may
     * be used as an alternative method of accessing
     * a node in the huffman tree.
     */
    struct huffman_node_t *nodes[HUFFMAN_SYMBOL_SIZE];

    /**
     * The array that associates a code (symbol) index
     * to the ammount of times it occurred in the target
     * data file, this is going to be used in the
     * contruction of the huffman tree.
     */
    size_t freqs[HUFFMAN_SYMBOL_SIZE];

    /**
     * Table containing the association between the various
     * prefixes and the code that they represent, this table
     * together with the prefix extra table may be used to
     * provide a much faster decoder performance.
     */
    unsigned char *prefix_code;

    /**
     * Table that associates the prefix code with the number
     * of extra bits contained in it, so that it's possible
     * to discard some extra bits contained in it.
     */
    unsigned char *prefix_extra;
} huffman;

VIRIATUM_EXPORT_PREFIX void create_huffman(struct huffman_t **huffman_pointer);
VIRIATUM_EXPORT_PREFIX void delete_huffman(struct huffman_t *huffman);
VIRIATUM_EXPORT_PREFIX void create_huffman_node(struct huffman_node_t **huffman_node_pointer);
VIRIATUM_EXPORT_PREFIX void delete_huffman_node(struct huffman_node_t *huffman_node);
VIRIATUM_EXPORT_PREFIX void delete_tree_huffman(struct huffman_node_t *node);
VIRIATUM_EXPORT_PREFIX void encode_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out);
VIRIATUM_EXPORT_PREFIX void decode_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out);
VIRIATUM_EXPORT_PREFIX void decode_table_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out);
VIRIATUM_EXPORT_PREFIX void decode_tree_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out);
VIRIATUM_EXPORT_PREFIX void generate_prefix_huffman(struct huffman_t *huffman);
VIRIATUM_EXPORT_PREFIX void generate_table_huffman(struct huffman_t *huffman, struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX void calc_freqs_huffman(struct huffman_t *huffman, struct stream_t *stream);
VIRIATUM_EXPORT_PREFIX void allocate_tree_huffman(
    struct huffman_t *huffman,
    struct huffman_node_t *node,
    unsigned short code,
    unsigned char bit_count
);
VIRIATUM_EXPORT_PREFIX void _fill_prefix_huffman(
    struct huffman_t *huffman,
    unsigned char symbol,
    unsigned short code,
    unsigned char extra,
    unsigned char _extra
);
VIRIATUM_EXPORT_PREFIX int _compare_huffman(void *first, void *second);
