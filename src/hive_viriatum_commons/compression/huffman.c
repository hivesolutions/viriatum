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
    *huffman_pointer = huffman;
}

void delete_huffman(struct huffman_t *huffman) {
    /* releases the huffman structure avoiding any kind
    of memory leak (could create problems )*/
    FREE(huffman);
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

void generate_table_huffman(struct huffman_t *huffman, struct stream_t *stream) {
    size_t count;
    size_t index;

    stream->open(stream);
    calc_freqs_huffman(huffman, stream);

    for(index = 0; index < HUFFMAN_SYMBOL_SIZE; index++) {
        count = huffman->freqs[index];
        if(count == 0) { continue; }

        printf("%c\n", index);
    }

    stream->close(stream);
}
