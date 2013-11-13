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
    huffman->root = NULL;
    *huffman_pointer = huffman;
}

void delete_huffman(struct huffman_t *huffman) {
    /* releases the huffman structure avoiding any kind
    of memory leak (could create problems )*/
    if(huffman->root) { delete_tree_huffman(huffman->root); }
    FREE(huffman);
}

void create_huffman_node(struct huffman_node_t **huffman_node_pointer) {
    size_t huffman_node_size = sizeof(struct huffman_node_t);
    struct huffman_node_t *huffman_node =\
        (struct huffman_node_t *) MALLOC(huffman_node_size);
    huffman_node->left = NULL;
    huffman_node->right = NULL;
    huffman_node->value = 0;
    huffman_node->symbol = 0;
    huffman_node->code = 0;
    huffman_node->bit_count = 0;
    *huffman_node_pointer = huffman_node;
}

void delete_huffman_node(struct huffman_node_t *huffman_node) {
    FREE(huffman_node);
}

void delete_tree_huffman(struct huffman_node_t *node) {
    if(node->left != NULL) { delete_tree_huffman(node->left); }
    if(node->right != NULL) { delete_tree_huffman(node->right); }
    delete_huffman_node(node);
}

void encode_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    size_t count;
    size_t index;
    unsigned char byte;
    struct huffman_node_t *node;
    unsigned char buffer[HUFFMAN_BUFFER_SIZE];
    struct bit_stream_t *bit_stream;

    create_bit_stream(&bit_stream, out);

    in->open(in);
    open_bit_stream(bit_stream);

    huffman->bit_count = 0;

    while(TRUE) {
        count = in->read(in, buffer, HUFFMAN_BUFFER_SIZE);
        if(count == 0) { break; }

        for(index = 0; index < count; index++) {
            byte = buffer[index];
            node = huffman->nodes[byte];

            huffman->bit_count += node->bit_count;
            write_word_bit_stream(
			    bit_stream,
				node->code,
				node->bit_count
			);
        }
    }

    close_bit_stream(bit_stream);
    in->close(in);

    delete_bit_stream(bit_stream);
}

void decode_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    size_t index;
    size_t count;
    size_t out_count;
    unsigned char bit;
    unsigned char is_leaf;
    unsigned char bit_count;
	register unsigned char code;
    unsigned char in_buffer[HUFFMAN_BUFFER_SIZE];
    unsigned char out_buffer[HUFFMAN_BUFFER_SIZE];
    long long total_count = 0;

    size_t major_count = 0;

    struct huffman_node_t *node = huffman->root;

    in->open(in);
    out->open(out);

    while(TRUE) {
        count = in->read(in, in_buffer, HUFFMAN_BUFFER_SIZE);
        if(count == 0) { break; }

        out_count = 0;

        for(index = 0; index < count; index++) {
            code = in_buffer[index];
            bit_count = 0;

            while(TRUE) {
                if(bit_count == 8) { break; }
                if(total_count == huffman->bit_count) { break; }

                bit_count++;
                total_count++;

                bit = code >> (8 - bit_count);
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

        if(out_count > 0) {
            out->write(out, out_buffer, out_count);
        }
    }

    out->close(out);
    in->close(in);
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
    unsigned char is_leaf = node->left == NULL && node->right == NULL;
    if(is_leaf) {
        node->code = code;
        node->bit_count = bit_count;
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

int _compare_huffman(void *first, void *second) {
    struct huffman_node_t *_first = (struct huffman_node_t *) first;
    struct huffman_node_t *_second = (struct huffman_node_t *) second;
    return _first->value - _second->value;
}
