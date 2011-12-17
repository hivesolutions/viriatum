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
 
 __author__    = Jo‹o Magalh‹es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "huffman.h"

void createHuffman(struct Huffman_t **huffmanPointer) {
    /* retrieves the huffman size */
    size_t huffmanSize = sizeof(struct Huffman_t);

    /* allocates space for the huffman */
    struct Huffman_t *huffman = (struct Huffman_t *) MALLOC(huffmanSize);

    /* sets the huffman in the huffman pointer */
    *huffmanPointer = huffman;
}

void deleteHuffman(struct Huffman_t *huffman) {
    /* releases the huffman */
    FREE(huffman);
}

void generateTableHuffman(struct Huffman_t *huffman, struct Stream_t *stream) {
    // seeks to the end of the file
    /*this->fileStream->seekg(0, std::fstream::end);

    // get length of file:
    this->originalFileSize = this->fileStream->tellg();

    // seeks to the beginning of the file
    this->fileStream->seekg(0, std::fstream::beg);*/

    /* allocates the file buffer */
    /*char fileBuffer[HUFFMAN_BUFFER_SIZE];*/

    /* allocates space for the read size */
    /*unsigned int readSize;*/

    // allocates space for the lowest huffman node
    /*HuffmanNode_t *lowestHuffmanNode;

    // allocates space for the second lowest huffman node
    HuffmanNode_t *secondLowestHuffmanNode;*/

    /* sets the (inner) stream */
    huffman->stream = stream;
/*
    // iterates continuously
    while(1) {
        // reads the buffer
        this->fileStream->read(fileBuffer, HUFFMAN_FILE_BUFFER_SIZE);

        // retrieves the read size
        readSize = this->fileStream->gcount();

        // updates the occurence values for the file buffer
        this->updateOccurrenceValues(fileBuffer, readSize);

        // in case the end of file was reached
        if(this->fileStream->eof()) {
            // breaks the cycle
            break;
        }
    }

    // clears the error bits
    this->fileStream->clear();

    // seeks to the beginning of the file
    this->fileStream->seekg(0, std::fstream::beg);

    // creates the code table priority queue
    std::priority_queue<HuffmanNode_t *, std::vector<HuffmanNode_t *>, HuffmanNodeCompare> codeTable;

    // allocates space for the huffman nodes buffer
    HuffmanNode_t huffmanNodesBuffer[HUFFMAN_SYMBOL_TABLE_SIZE];

    // iterates over all the huffman symbols
    for(unsigned int index = 0; index < HUFFMAN_SYMBOL_TABLE_SIZE; index++) {
        // in case the digit did not occurred any time
        if(!this->occurrenceCountList[index]) {
            // continues the loop
            continue;
        }

        // retrieves the current huffman node
        HuffmanNode_t &currentHuffmanNode = huffmanNodesBuffer[index];

        // sets the current huffman node values
        currentHuffmanNode.value = this->occurrenceCountList[index];
        currentHuffmanNode.symbol = index;
        currentHuffmanNode.code = std::string();
        currentHuffmanNode.left = NULL;
        currentHuffmanNode.right = NULL;
        currentHuffmanNode.parent = NULL;

        // adds the current huffman node to the code table
        codeTable.push(&currentHuffmanNode);
    }

    // iterates continuously
    while(1) {
        // the lowest value node
        lowestHuffmanNode = codeTable.top();

        // pops the code table
        codeTable.pop();

        // in case
        if(codeTable.empty()) {
            // breaks the cycle
            break;
        } else {
            // the second lowest value node
            secondLowestHuffmanNode = codeTable.top();

            // pops the code table
            codeTable.pop();

            // creates the merged huffman node
            HuffmanNode_t *mergedHuffmanNode = new HuffmanNode_t();

            // sets the merged huffman node values
            mergedHuffmanNode->value = lowestHuffmanNode->value + secondLowestHuffmanNode->value;
            mergedHuffmanNode->symbol = HUFFMAN_SYMBOL_TABLE_EXTRA_VALUE;
            mergedHuffmanNode->code = std::string();
            mergedHuffmanNode->left = secondLowestHuffmanNode;
            mergedHuffmanNode->right = lowestHuffmanNode;
            mergedHuffmanNode->parent = NULL;

            // sets the parent in the huffman nodes
            lowestHuffmanNode->parent = mergedHuffmanNode;
            secondLowestHuffmanNode->parent = mergedHuffmanNode;

            // adds the merged node to the priority_queue
            codeTable.push(mergedHuffmanNode);
        }
    }

    // generates the huffman table
    this->_generateTable(lowestHuffmanNode);

    // computes the table
    this->computeTable();

    // cleans the structures
    this->cleanStructures(lowestHuffmanNode);*/
}
