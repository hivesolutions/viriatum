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

#include "memory_pool.h"

void create_memory_pool(struct memory_pool_t **memory_pool_pointer) {
    /* retrieves the memory pool size */
    size_t memory_pool_size = sizeof(struct memory_pool_t);

    /* allocates space for the memory pool */
    struct memory_pool_t *memory_pool = (struct memory_pool_t *) MALLOC(memory_pool_size);

    /* sets the memory pool size */
    memory_pool->size = MEMORY_POOL_SIZE;

    /* sets the memory pool current */
    memory_pool->current = memory_pool;

    /* sets the memory pool large block */
    memory_pool->large_block = NULL;

    /* sets the memory pool in the memory pool pointer */
    *memory_pool_pointer = memory_pool;
}

void delete_memory_pool(struct memory_pool_t *memory_pool) {
    /* releases the memory pool */
    FREE(memory_pool);
}

void *alloc_memory_pool(struct memory_pool_t *memory_pool, size_t size) {
    /* retrieves the current memory pool */
    struct memory_pool_t *current_memory_pool = memory_pool->current;

    /* in case the size is larger than the
    large block size */
    if(size > LARGE_BLOCK_SIZE) {

    }
    /* otherwise it's a small (normal) block size */
    else {
        /* unsets hte large block value */
        current_memory_pool->large_block = NULL;
    }

    /* returns invalid */
    return NULL;
}
