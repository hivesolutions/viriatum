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

#include "stdafx.h"

#include "huffman.h"

void create_huffman(struct huffman_t **huffman_pointer) {
    /* calculates the size of the huffman structure and uses
    it to allocate the appropriate structure, then returns the
    allocated value to the caller function */
    size_t huffman_size = sizeof(struct huffman_t);
    struct huffman_t *huffman = (struct huffman_t *) MALLOC(huffman_size);
    huffman->bit_count = 0;
    huffman->symbol_count = 0;
    huffman->prefix_size = 0;
    huffman->root = NULL;
    huffman->prefix_code = NULL;
    huffman->prefix_extra = NULL;
    memset(huffman->nodes, 0, sizeof(huffman->nodes));
    memset(huffman->freqs, 0, sizeof(huffman->freqs));
    *huffman_pointer = huffman;
}

void delete_huffman(struct huffman_t *huffman) {
    /* releases the huffman structure avoiding any kind
    of memory leak (could create problems )*/
    if(huffman->root) { delete_tree_huffman(huffman->root); }
    if(huffman->prefix_code) { FREE(huffman->prefix_code); }
    if(huffman->prefix_extra) { FREE(huffman->prefix_extra); }
    FREE(huffman);
}

void create_huffman_node(struct huffman_node_t **huffman_node_pointer) {
    /* calculates the size of an huffman node structure and then
    uses the value for the allocation of the appropriate memory */
    size_t huffman_node_size = sizeof(struct huffman_node_t);
    struct huffman_node_t *huffman_node =\
        (struct huffman_node_t *) MALLOC(huffman_node_size);

    /* initializes the complete set of attribute of an huffman
    node to the default values of each of them */
    huffman_node->left = NULL;
    huffman_node->right = NULL;
    huffman_node->value = 0;
    huffman_node->symbol = 0;
    huffman_node->code = 0;
    huffman_node->bit_count = 0;

    /* returns the huffman node to the caller method by setting
    the reference of the allocated structure in the parameter */
    *huffman_node_pointer = huffman_node;
}

void delete_huffman_node(struct huffman_node_t *huffman_node) {
    /* releases the memory associated with the huffman node in
    order to avoid any memory leak (could corrupt data) */
    FREE(huffman_node);
}

void delete_tree_huffman(struct huffman_node_t *node) {
    /* deletes the huffman tree structure taking a recursive approach
    and traversing the complete set of nodes in the tree, in order to
    have the complete tree removed the root node should be passed */
    if(node->left != NULL) { delete_tree_huffman(node->left); }
    if(node->right != NULL) { delete_tree_huffman(node->right); }
    delete_huffman_node(node);
}

void encode_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    /* allocates space in the stack for the various temporary
    variables that are going to be used in the encoding operation
    of the stream using the current huffman state */
    size_t count;
    size_t index;
    unsigned char byte;
    struct huffman_node_t *node;
    struct bit_stream_t *bit_stream;
    unsigned char buffer[HUFFMAN_BUFFER_SIZE];

    /* creates the but stream that is going to be used as
    the encoded output (bit stream is required for huffman) */
    create_bit_stream(&bit_stream, out);

    /* opens the input stream and the just created bit stream
    that is going to be used as the output */
    in->open(in);
    open_bit_stream(bit_stream);

    /* initializes the bit counter of the huffman structure to
    zero, this value will count the total amount of bits of the
    huffman encoded strema (to be used in decoding) */
    huffman->bit_count = 0;

    /* iterates continuously to read new data from the input
    stream and then run the encoder using the current nodes map */
    while(TRUE) {
        /* reads some data from the input stream in case the returned
        count of bytes is zero the end of stream is found, and so it
        must break the current loop (all bytes encoded) */
        count = in->read(in, buffer, HUFFMAN_BUFFER_SIZE);
        if(count == 0) { break; }

        /* iterates over the range of bytes that have just been read
        to encode them using the loaded nodes array */
        for(index = 0; index < count; index++) {
            /* retrieves the current byte in iteration from the buffer
            and resolves it into the proper node from the nodes array */
            byte = buffer[index];
            node = huffman->nodes[byte];

            /* increments the bit counter by the amount of bits or the
            node and then writes into the bit stream the code representing
            the current byte using the current huffman tree */
            huffman->bit_count += node->bit_count;
            write_word_bit_stream(
                bit_stream,
                node->code,
                node->bit_count
            );
        }
    }

    /* closes the current bit stream, flushing any pending data to
    the underlying stream and then closes the input stream */
    close_bit_stream(bit_stream);
    in->close(in);

    /* deletes the (temporary) bit stream in order to avoid any kind
    of memory leaks (could create corrupted data) */
    delete_bit_stream(bit_stream);
}

void decode_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    /* verifies if the conditions are met to use the (prefix)
    table decoder in case their not use the tree based decoder
    note that there's a significant performance issue with the
    tree based decoder as it need to percolate the tree */
    unsigned char is_table = huffman->prefix_code && huffman->prefix_extra;
    if(is_table) { decode_table_huffman(huffman, in, out); }
    else { decode_tree_huffman(huffman, in, out); }
}

void decode_table_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    unsigned char code;
    unsigned char extra;
    unsigned short word;
    struct bit_stream_t *bit_stream;
    unsigned char out_buffer[HUFFMAN_BUFFER_SIZE];
    unsigned char prefix_size = huffman->prefix_size;
    long long total_count = 0;

    size_t out_count = 0;

    create_bit_stream(&bit_stream, in);

    open_bit_stream(bit_stream);
    out->open(out);

    while(TRUE) {
        if(total_count == huffman->bit_count) { break; }

        read_word_bit_stream(bit_stream, &word, prefix_size);

        code = huffman->prefix_code[word];
        extra = huffman->prefix_extra[word];

        if(extra > 0) { seek_bit_stream(bit_stream, extra); }

        out_buffer[out_count] = code;
        out_count++;
        if(out_count == HUFFMAN_BUFFER_SIZE) {
            out->write(out, out_buffer, out_count);
            out_count = 0;
        }

        total_count += prefix_size - extra;
    }

    if(out_count > 0) { out->write(out, out_buffer, out_count); }

    in->close(out);
    close_bit_stream(bit_stream);

    delete_bit_stream(bit_stream);
}

void decode_tree_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    size_t index;
    size_t count;
    unsigned char bit;
    unsigned char is_leaf;
    unsigned char bit_count;
    register unsigned char code;
    unsigned char in_buffer[HUFFMAN_BUFFER_SIZE];
    unsigned char out_buffer[HUFFMAN_BUFFER_SIZE];
    long long total_count = 0;

    size_t out_count = 0;
    size_t major_count = 0;

    struct huffman_node_t *node = huffman->root;

    in->open(in);
    out->open(out);

    while(TRUE) {
        count = in->read(in, in_buffer, HUFFMAN_BUFFER_SIZE);
        if(count == 0) { break; }

        for(index = 0; index < count; index++) {
            code = in_buffer[index];
            bit_count = 0;

            while(TRUE) {
                if(bit_count == HUFFMAN_BYTE_SIZE) { break; }
                if(total_count == huffman->bit_count) { break; }

                bit_count++;
                total_count++;

                bit = code >> (HUFFMAN_BYTE_SIZE - bit_count);
                if(bit & 1) { node = node->right; }
                else { node = node->left; }

                is_leaf = node->left == NULL && node->right == NULL;
                if(is_leaf == FALSE) { continue; }

                out_buffer[out_count] = node->symbol;
                out_count++;
                if(out_count == HUFFMAN_BUFFER_SIZE) {
                    out->write(out, out_buffer, out_count);
                    out_count = 0;
                }

                node = huffman->root;
                major_count++;
            }
        }
    }

    /* in case there are pending bytes to be writen to the output
    stream flushes them by writing them to the output stream */
    if(out_count > 0) { out->write(out, out_buffer, out_count); }

    out->close(out);
    in->close(in);
}

void generate_prefix_huffman(struct huffman_t *huffman) {
    size_t index;
    register unsigned char symbol;
    register unsigned short code;
    unsigned char extra;
    struct huffman_node_t *node;

    size_t prefix_range = 1 << huffman->prefix_size;
    size_t prefix_size = prefix_range * sizeof(unsigned char);

    huffman->prefix_code = (unsigned char *) MALLOC(prefix_size);
    huffman->prefix_extra = (unsigned char *) MALLOC(prefix_size);

    for(index = 0; index < HUFFMAN_SYMBOL_SIZE; index++) {
        node = huffman->nodes[index];
        if(node == NULL) { continue; }

        symbol = node->symbol;
        code = node->code;
        extra = huffman->prefix_size - node->bit_count;

        _fill_prefix_huffman(huffman, symbol, code, extra, extra);
    }
}

void generate_table_huffman(struct huffman_t *huffman, struct stream_t *stream) {
    size_t count;
    size_t index;
    struct huffman_node_t *node;
    struct huffman_node_t *left;
    struct huffman_node_t *right;
    struct priority_queue_t *queue;

    create_priority_queue(&queue, _compare_huffman);

    stream->open(stream);
    calc_freqs_huffman(huffman, stream);
    stream->close(stream);

    for(index = 0; index < HUFFMAN_SYMBOL_SIZE; index++) {
        count = huffman->freqs[index];
        if(count == 0) { continue; }

        create_huffman_node(&node);
        node->value = count;
        node->symbol = (unsigned char) index;

        huffman->symbol_count++;
        huffman->nodes[index] = node;
        push_priority_queue(queue, (void *) node);
    }

    while(TRUE) {
        if(queue->size <= 1) { break; }

        pop_priority_queue(queue, (void **) &left);
        pop_priority_queue(queue, (void **) &right);

        create_huffman_node(&node);
        node->left = left;
        node->right = right;
        node->value = left->value + right->value;

        push_priority_queue(queue, (void *) node);
    }

    pop_priority_queue(queue, (void **) &huffman->root);
    delete_priority_queue(queue);

    allocate_tree_huffman(huffman, huffman->root, 0, 0);
}

void calc_freqs_huffman(struct huffman_t *huffman, struct stream_t *stream) {
    /* reserves space for the index the counter for the read
    operation and for the temporary byte variable to be used
    in the buffer read iteration loop */
    size_t index;
    size_t count;
    unsigned char byte;

    /* allocates space for the buffer that is going to be used
    in the read operation of the file and then retrieves the
    reference to the frequecies sequence from the huffman */
    unsigned char buffer[HUFFMAN_BUFFER_SIZE];
    size_t *freqs = huffman->freqs;

    /* resets the current frequency table to the default zero
    value and then start iterating over the stream values to count
    them so that frquencies are possible to be calculated */
    memset(freqs, 0, HUFFMAN_SYMBOL_SIZE * sizeof(size_t));
    while(TRUE) {
        /* reads the default amount of data from the stream into
        the buffer an incase the value is zero the end of file has
        been reached and so the iteration must be stopped */
        count = stream->read(stream, buffer, HUFFMAN_BUFFER_SIZE);
        if(count == 0) { break; }

        /* iterates over the complete set of bytes in the buffer
        and for each of its values in the array increments the value
        so that the importance of it is increased */
        for(index = 0; index < count; index++) {
            byte = buffer[index];
            freqs[byte]++;
        }
    }

    /* restores the stream back to the original position so that
    additional operations may be performed correctly */
    stream->seek(stream, 0);
}

void allocate_tree_huffman(
    struct huffman_t *huffman,
    struct huffman_node_t *node,
    unsigned short code,
    unsigned char bit_count
) {
    unsigned char is_larger;
    unsigned char is_leaf = node->left == NULL && node->right == NULL;
    if(is_leaf) {
        node->code = code;
        node->bit_count = bit_count;
        is_larger = bit_count > huffman->prefix_size;
        if(is_larger) { huffman->prefix_size = bit_count; }
    } else {
        allocate_tree_huffman(
            huffman,
            node->left,
            (code << 1) | 0,
            bit_count + 1
        );
        allocate_tree_huffman(
            huffman,
            node->right,
            (code << 1) | 1,
            bit_count + 1
        );
    }
}

void _fill_prefix_huffman(
    struct huffman_t *huffman,
    unsigned char symbol,
    unsigned short code,
    unsigned char extra,
    unsigned char _extra
) {
    register unsigned short code_s;

    if(_extra == 0) {
        huffman->prefix_code[code] = symbol;
        huffman->prefix_extra[code] = extra;
        return;
    }

    code_s = code << 1;
    _fill_prefix_huffman(huffman, symbol, code_s | 1, extra, _extra - 1);
    _fill_prefix_huffman(huffman, symbol, code_s, extra, _extra - 1);
}

int _compare_huffman(void *first, void *second) {
    struct huffman_node_t *_first = (struct huffman_node_t *) first;
    struct huffman_node_t *_second = (struct huffman_node_t *) second;
    return _first->value - _second->value;
}
