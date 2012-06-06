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

#include "huffman.h"

void create_huffman(struct huffman_t **huffman_pointer) {
    /* retrieves the huffman size */
    size_t huffman_size = sizeof(struct huffman_t);

    /* allocates space for the huffman */
    struct huffman_t *huffman = (struct huffman_t *) MALLOC(huffman_size);

    /* sets the huffman in the huffman pointer */
    *huffman_pointer = huffman;
}

void delete_huffman(struct huffman_t *huffman) {
    /* releases the huffman */
    FREE(huffman);
}

void generate_table_huffman(struct huffman_t *huffman, struct stream_t *stream) {
    // seeks to the end of the file
    /*this->file_stream->seekg(0, std::fstream::end);

    // get length of file:
    this->original_file_size = this->file_stream->tellg();

    // seeks to the beginning of the file
    this->file_stream->seekg(0, std::fstream::beg);*/

    /* allocates the file buffer */
    /*char file_buffer[HUFFMAN_BUFFER_SIZE];*/

    /* allocates space for the read size */
    /*unsigned int read_size;*/

    // allocates space for the lowest huffman node
    /*HuffmanNode_t *lowest_huffman_node;

    // allocates space for the second lowest huffman node
    HuffmanNode_t *second_lowest_huffman_node;*/

    /* sets the (inner) stream */
    huffman->stream = stream;
/*
    // iterates continuously
    while(1) {
        // reads the buffer
        this->file_stream->read(file_buffer, HUFFMAN_FILE_BUFFER_SIZE);

        // retrieves the read size
        read_size = this->file_stream->gcount();

        // updates the occurence values for the file buffer
        this->update_occurrence_values(file_buffer, read_size);

        // in case the end of file was reached
        if(this->file_stream->eof()) {
            // breaks the cycle
            break;
        }
    }

    // clears the error bits
    this->file_stream->clear();

    // seeks to the beginning of the file
    this->file_stream->seekg(0, std::fstream::beg);

    // creates the code table priority queue
    std::priority_queue<HuffmanNode_t *, std::vector<HuffmanNode_t *>, HuffmanNodeCompare> code_table;

    // allocates space for the huffman nodes buffer
    HuffmanNode_t huffman_nodes_buffer[HUFFMAN_SYMBOL_TABLE_SIZE];

    // iterates over all the huffman symbols
    for(unsigned int index = 0; index < HUFFMAN_SYMBOL_TABLE_SIZE; index++) {
        // in case the digit did not occurred any time
        if(!this->occurrence_count_list[index]) {
            // continues the loop
            continue;
        }

        // retrieves the current huffman node
        HuffmanNode_t &current_huffman_node = huffman_nodes_buffer[index];

        // sets the current huffman node values
        current_huffman_node.value = this->occurrence_count_list[index];
        current_huffman_node.symbol = index;
        current_huffman_node.code = std::string();
        current_huffman_node.left = NULL;
        current_huffman_node.right = NULL;
        current_huffman_node.parent = NULL;

        // adds the current huffman node to the code table
        code_table.push(&current_huffman_node);
    }

    // iterates continuously
    while(1) {
        // the lowest value node
        lowest_huffman_node = code_table.top();

        // pops the code table
        code_table.pop();

        // in case
        if(code_table.empty()) {
            // breaks the cycle
            break;
        } else {
            // the second lowest value node
            second_lwowest_huffman_node = code_table.top();

            // pops the code table
            code_table.pop();

            // creates the merged huffman node
            HuffmanNode_t *merged_huffman_node = new huffman_node_t();

            // sets the merged huffman node values
            merged_huffman_node->value = lowest_huffman_node->value + second_lowest_huffman_node->value;
            merged_huffman_node->symbol = HUFFMAN_SYMBOL_TABLE_EXTRA_VALUE;
            merged_huffman_node->code = std::string();
            merged_huffman_node->left = second_lowest_huffman_node;
            merged_huffman_node->right = lowest_huffman_node;
            merged_huffman_node->parent = NULL;

            // sets the parent in the huffman nodes
            lowest_huffman_node->parent = merged_huffman_node;
            second_lowest_huffman_node->parent = merged_huffman_node;

            // adds the merged node to the priority_queue
            code_table.push(merged_huffman_node);
        }
    }

    // generates the huffman table
    this->_generate_table(lowest_huffman_node);

    // computes the table
    this->compute_table();

    // cleans the structures
    this->clean_structures(lowest_huffman_node);*/
}
