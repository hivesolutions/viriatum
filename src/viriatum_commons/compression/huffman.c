/*
 Hive Viriatum Commons
 Copyright (c) 2008-2017 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2017 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
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
    /* allocates space for the various variable that are required
    for the decoding of an huffman based stream using the prefix
    table based approach, note that the output buffer is included */
    unsigned char code;
    unsigned char extra;
    unsigned short word;
    struct bit_stream_t *bit_stream;
    unsigned char out_buffer[HUFFMAN_BUFFER_SIZE];
    unsigned char prefix_size = huffman->prefix_size;
    long long total_count = 0;
    size_t out_count = 0;

    /* creates the bit stream that is going to be used to read sets
    of bits from the input stream this is required for the prefix
    based decoding as the tables are read from bit ranges */
    create_bit_stream(&bit_stream, in);

    /* opens the input bit stream for reading and the (normal) output
    stream in writing mode, these are the stream that are going to be
    used for the decoding operation */
    open_bit_stream(bit_stream);
    out->open(out);

    /* iterates continuously reading bits from the input stream until
    the ammount of bits reach the value defined in the bit counter */
    while(TRUE) {
        /* in case the total amount of bits read from the bit stream
        has reached the pre-defined value (the end has been reached)
        and so the current loop must break */
        if(total_count == huffman->bit_count) { break; }

        /* reads the maximum prefix size of bits from the bit stream
        to the word value, this is the value that is going to be used
        for prefix value matching in the tables */
        read_word_bit_stream(bit_stream, &word, prefix_size);

        /* retrieves the code and the extra bits from the word that was
        just read from the bit stream */
        code = huffman->prefix_code[word];
        extra = huffman->prefix_extra[word];

        /* in case the're are extra bits for the current work seek back
        the current bit stream (required) */
        if(extra > 0) { seek_bit_stream(bit_stream, extra); }

        /* "writes" the resolved code to the output buffer and increments
        the associated counter, in case the value has reached the buffer
        limit flushed the data to the underlying stream by writing it */
        out_buffer[out_count] = code;
        out_count++;
        if(out_count == HUFFMAN_BUFFER_SIZE) {
            out->write(out, out_buffer, out_count);
            out_count = 0;
        }

        /* increments the total (bit) counter byt the "real" amount of bits
        that were read from the input stream */
        total_count += prefix_size - extra;
    }

    /* in case the're pending bytes to be written to the output
    stream writes them (flush operation) */
    if(out_count > 0) { out->write(out, out_buffer, out_count); }

    /* closes the output stream flusing all of it's data and then
    closes the input bit based stream also*/
    in->close(out);
    close_bit_stream(bit_stream);

    /* deletes the bit stream structure, avoiding the leak of any
    memory resulting from that structure */
    delete_bit_stream(bit_stream);
}

void decode_tree_huffman(struct huffman_t *huffman, struct stream_t *in, struct stream_t *out) {
    /* allocates space for the various local and temporary values
    that are going to be used in the decoding operation for the
    huffman infra-structure */
    size_t index;
    size_t count;
    unsigned char bit;
    unsigned char is_leaf;
    register unsigned char bit_count;
    register unsigned char code;
    struct huffman_node_t *node;
    unsigned char in_buffer[HUFFMAN_BUFFER_SIZE];
    unsigned char out_buffer[HUFFMAN_BUFFER_SIZE];
    long long total_count = 0;
    size_t out_count = 0;

    /* opens both the input and the ouput streams so that they
    become ready to received and provide information */
    in->open(in);
    out->open(out);

    /* sets the initial node for the iterative approach as
    the root node, as this is the start of the iterative process */
    node = huffman->root;

    /* iterates continuously, reading data from the input buffer
    in a one byte size approach and the using all of it's bit to
    percolate over the huffman tree */
    while(TRUE) {
        /* reads a set of data from the input buffer in case the
        returned amount of bytes read is zero the end of stream
        has been reached and so the loop must break */
        count = in->read(in, in_buffer, HUFFMAN_BUFFER_SIZE);
        if(count == 0) { break; }

        /* iterates over the range of bytes that have just been
        read from the input stream, to use all of its bits in the
        percolation of the huffman tree */
        for(index = 0; index < count; index++) {
            /* retrieves the current byte (code) and resets the bit
            counter to the initial zero value */
            code = in_buffer[index];
            bit_count = 0;

            /* iterates continuously until either the complete set of
            bits in the code have been processed or the complete set
            of huffman bits "inside" the file have been read */
            while(TRUE) {
                /* in case the the complete set of bits in the code
                have been processed or the end of huffman stream has
                been reached the current loop is break */
                if(bit_count == HUFFMAN_BYTE_SIZE) { break; }
                if(total_count == huffman->bit_count) { break; }

                /* increments both the internal byte counter of bits
                and the global bit counter for the file */
                bit_count++;
                total_count++;

                /* shifts the code by the rquired amount of bits for
                the current iteration step and then verifies if the
                bit is odd or even and selects the apropriate next
                node value (tree percolation step) */
                bit = code >> (HUFFMAN_BYTE_SIZE - bit_count);
                if(bit & 1) { node = node->right; }
                else { node = node->left; }

                /* verifies if the current selected node is a leaf
                node (no child) and in case it's not continus the loop
                immediately as there's nothing to be done */
                is_leaf = node->left == NULL && node->right == NULL;
                if(is_leaf == FALSE) { continue; }

                /* "writes" the current node's symbol in the output
                buffer and increments the counter for the output buffer
                in case this value has reached the buffer limit flushes
                the data to the stream (writing it) */
                out_buffer[out_count] = node->symbol;
                out_count++;
                if(out_count == HUFFMAN_BUFFER_SIZE) {
                    out->write(out, out_buffer, out_count);
                    out_count = 0;
                }

                /* updates the current node in teiration with the huffman's
                root node as a new tree traversal will begin */
                node = huffman->root;
            }
        }
    }

    /* in case there are pending bytes to be writen to the output
    stream flushes them by writing them to the output stream */
    if(out_count > 0) { out->write(out, out_buffer, out_count); }

    /* closes both the output stream and the input one so that
    no more that is going to be writen or read from them */
    out->close(out);
    in->close(in);
}

void generate_prefix_huffman(struct huffman_t *huffman) {
    /* allocates space for the vairous local variables
    to be used in the generation of the prefix tables */
    size_t index;
    register unsigned char symbol;
    register unsigned short code;
    unsigned char extra;
    struct huffman_node_t *node;

    /* calculates the range of prefix using the prefix size
    then calculates the size of each of the prefix tables */
    size_t prefix_range = 1 << huffman->prefix_size;
    size_t prefix_size = prefix_range * sizeof(unsigned char);

    /* allocates space for each of the prefix tables, one for
    the prefix to code association and other for the prefix to
    extra bits association, the allocation is dynamic using the
    prefix size as the reference */
    huffman->prefix_code = (unsigned char *) MALLOC(prefix_size);
    huffman->prefix_extra = (unsigned char *) MALLOC(prefix_size);

    /* iterates over the complete set of symbols for huffman
    (ascii symbols) in order to populate both prefix tables */
    for(index = 0; index < HUFFMAN_SYMBOL_SIZE; index++) {
        /* retrieves the node associated with the current symbol
        in case the value is null (not valid value) skyps the
        current iterateion (not required) */
        node = huffman->nodes[index];
        if(node == NULL) { continue; }

        /* unpacks the symbol and the code from the current symbol's
        node and then calculates the extra number of bits for the
        current symbol's huffman code as the (maximum) prefix size
        minus the current's huffman code bit count */
        symbol = node->symbol;
        code = node->code;
        extra = huffman->prefix_size - node->bit_count;

        /* runs the recursive based filling operation of the prefix
        tables that fills the tables with all the prefix size based
        combinations that represent this huffman code */
        _fill_prefix_huffman(huffman, symbol, code, extra, extra);
    }
}

void generate_table_huffman(struct huffman_t *huffman, struct stream_t *stream) {
    /* allocates the various temporary variables that are
    required for the generation of the huffman table/tree */
    size_t count;
    size_t index;
    struct huffman_node_t *node;
    struct huffman_node_t *left;
    struct huffman_node_t *right;
    struct priority_queue_t *queue;

    /* creates a new priority queue and sets the default
    comparision algorithm to the compare huffman one */
    create_priority_queue(&queue, _compare_huffman);

    /* runs the calculus of the frequencies for the provided
    stream, at the end of the operation a table of frequencies
    for each of the ascii symbols is created, please no that
    this may be an expensive operation */
    calc_freqs_huffman(huffman, stream);

    /* iterates over the range of symbols of the default huffman
    tree implementation to create and insert a new node for each
    of the symbols that are considered value (more that one occurence) */
    for(index = 0; index < HUFFMAN_SYMBOL_SIZE; index++) {
        /* retrieves the count for the current symbol and
        in case there are no counts for it ignores it and
        continues the loop skipping one step */
        count = huffman->freqs[index];
        if(count == 0) { continue; }

        /* create a new huffman node and updates it with the
        count for the current symbol and with the symbol itself */
        create_huffman_node(&node);
        node->value = count;
        node->symbol = (unsigned char) index;

        /* increments the symbol counter (valid symbols counter)
        and sets the node in the symbol to node array map then
        "pushes" the new node into the priority queue */
        huffman->symbol_count++;
        huffman->nodes[index] = node;
        push_priority_queue(queue, (void *) node);
    }

    /* iterates continuously creating the huffman tree using
    the pre-defined algorithm of frequecies */
    while(TRUE) {
        /* in case the size has reached the value of one
        only one (master root node) is present the algorithm
        is considered complete and the loop must break */
        if(queue->size <= 1) { break; }

        /* retrieves two nodes from the priority queue that
        are going to be set as the left and right childs of
        the new master node to be created */
        pop_priority_queue(queue, (void **) &left);
        pop_priority_queue(queue, (void **) &right);

        /* creates the new "master" node and sets the references
        to the just retrieved left and right nodes, the new (frequency)
        value for the master node is going to be the sum of both
        values as the node represents both of them */
        create_huffman_node(&node);
        node->left = left;
        node->right = right;
        node->value = left->value + right->value;

        /* pushes the new node into the priority queue so that
        it may be also grouped again if required */
        push_priority_queue(queue, (void *) node);
    }

    /* pops the current and only element of the queue as the
    root node of the huffman tree and then deletes the priority
    queue structure as it's not longer required */
    pop_priority_queue(queue, (void **) &huffman->root);
    delete_priority_queue(queue);

    /* runs the allocation algorithm in the huffman tree so that
    each of the nodes is represented by a binary number that is
    inversely big as its size (huffman strategy) */
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

    /* opens the stream that is going to be used for the calculus
    of the frequencies for each of the symbols (read mode) */
    stream->open(stream);

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

    /* runs the close operation on the stream so that no more operations
    occur in the current stream without a new open call */
    stream->close(stream);
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
