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

#define SMALL_BLOCK_SIZE 1024
#define LARGE_BLOCK_SIZE 4096

#define MEMORY_POOL_SIZE 128

/**
 * Structure defining a large block for memory pool.
 * The large blocks represent data to be allocated dynamically.
 */
typedef struct MemoryPoolLargeBlock_t {
    /**
     * The next large memory block in the list.
     */
    struct MemoryPoolLargeBlock_t *next;

    /**
     * The pointer to the data in the large
     * memory block.
     */
    void *data;
} MemoryPoolLargeBlock;

/**
 * Structure defining the positioning of the data inside
 * the memory pool "item".
 */
typedef struct MemoryPoolData_t {
    /**
     * Pointer to the last occupied slot index in
     * the memory pool data.
     */
    void *last;

    /**
     * Pointer to the end of the memory pool data.
     */
    void *end;
} MemoryPoolData;

/**
 * Structure defining a memory pool.
 */
typedef struct MemoryPool_t {
    /**
     * The data associated with the memory pool.
     * The data of a pool sets its position in the global
     * memory pool sequence.
     */
    struct MemoryPoolData_t data;

    /**
     * The current size of the memory pool in bytes.
     */
    size_t size;

    /**
     * The maximum block size to be used.
     */
    size_t maximumBlockSize;

    /**
     * The current memory pool reference.
     */
    struct MemoryPool_t *current;

    /**
     * The next memory pool reference.
     */
    struct MemoryPool_t *next;

    /**
     * Reference to the initial large block reference.
     */
    struct MemoryPoolLargeBlock_t *largeBlock;
} MemoryPool;

void createMemoryPool(struct MemoryPool_t **memoryPoolPointer);
void deleteMemoryPool(struct MemoryPool_t *memoryPool);
void *allocMemoryPool(struct MemoryPool_t *memoryPool, size_t size);
