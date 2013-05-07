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

#include "../global/memory.h"
#include "../system/system_util.h"

/**
 * The size of each chunk (in number of items), should
 * be defined in accordance with a correct memory usage.
 */
#define CHUNK_SIZE 10240

/**
 * The factor to be used in the calculus of the size of
 * the map associating the buffer of the item with the item
 * instance, the map buffer must be bigger that the required
 * size to avoid a high level of collisions in the open
 * addressing algorithm (would slow down the process).
 */
#define CHUNK_FACTOR 1.4

#define CHUNK_MARGIN(value) (size_t) ((float) value * CHUNK_FACTOR)

struct memory_chunk_t;

typedef struct memory_item_t {
    /**
     * The zero based index of the item position in
     * the sequence of items defined in the "parent"
     * memory chunk.
     */
    size_t index;

    /**
     * The pointer to the chunk structure that
     * "owns" this item, this is considered a
     * reverse pointer in the inherachy.
     */
    struct memory_chunk_t *chunk;

    /**
     * Pointer to the memory buffer allocated for
     * the item, note that the size of the buffer
     * must be consistent with the item size value
     * defined in the "parent" chunk structure.
     */
    void *buffer;
} memory_item;

typedef struct memory_chunk_t {
    /**
     * The zero based index of the chunk "inside" the
     * parent memory pool structure. This value may be
     * used to update the free index in the pool.
     */
    size_t index;

    /**
     * The sequence (array) of items that compose
     * the current memory chunk, they should contain
     * both their index in the chunk and the buffer
     * pointer for their contents.
     */
    struct memory_item_t items[CHUNK_SIZE];

    /**
     * The bitmap array that indicates if the associated
     * item is currently in a free or used state.
     * Should be used together with the free index for
     * a fast access to first free element.
     */
    char bitmap[CHUNK_SIZE];

    /**
     * Boolean flag that indicates if the chunk is full,
     * so that all of its items are set to an used state.
     * This flag is used to accelerate operations that
     * require verification of the state.
     */
    char is_full;

    /**
     * The (bitmap) array index indicating the
     * first element in the array that is considered
     * to be in a free state. This value may be used
     * to gather information for the bes possible item
     * allocation for the chunk.
     */
    size_t free;

    /**
     * The pointer to the begining of the memory buffer
     * that is used for the items in the chunk.
     * The buffer should be allocated at runtime with
     * a multiple of the size of the first element to be
     * allocated for the pool.
     */
    void *buffer;

    /**
     * The number of items that are considered "used" in
     * terms of being allocated to a certain context. When
     * this value is equals to the static chunk size the
     * chunk is considered to be full, when this values is
     * equals to zero the cunk is considered empty.
     */
    size_t item_alloc;

    /**
     * The size in bytes for each of the item's value.
     * This may be used to compute offsets in releasing
     * the memory.
     */
    size_t item_size;

    /**
     * Complete size of the allocated buffer in bytes for
     * the current chunk. This value must be the equivalent
     * to the size of each item multiplied by the number
     * of items in the chunk.
     */
    size_t chunk_size;
} memory_chunk;

/**
 * Structure describing the various structures that
 * aim at creating a memory pool that is capable of
 * pre-allocating memory for a constant size problem.
 * This structure mus provide a faster solution compared
 * to the normal heap memory allocation.
 */
typedef struct memory_pool_t {
    /**
     * The sequence of (dynamically) allocated chunks
     * that compose (are part of) the memory pool.
     */
    struct memory_chunk_t **chunks;

    /**
     * The open addressing base map structure that associates
     * a pointer to the data buffer of an item with the item
     * structure that "owns" that buffer.
     * This map is used in the releasing process for the buffer.
     */
    struct memory_item_t **buffer_item_map;

    /**
     * The index of the first chunk that is considered non full
     * and that may be used for allocations.
     */
    size_t free;

    /**
     * Counter that measures the ammount of chunks that currently
     * have at least one item considered allocated. This value is
     * used for the shrink operation of the pool.
     */
    size_t chunk_alloc;

    /**
     * The number of chunks that are currently defined
     * under the memory pool, this value must be zero at
     * the begining of a memory pool.
     */
    size_t chunk_count;

    /**
     * The maximum number of chunks that may be created for
     * the memory pool, this value must be previously defined
     * so that a proper buffer to item map may be created.
     */
    size_t chunk_max_size;

    /**
     * The maximum number of items that may be allocacated for
     * the complete set of chunks in the mory pool, this value
     * is the result of the simple calculus of chunk max size
     * times the size of the chunk (number of items in chunk).
     */
    size_t items_max_size;
} memory_pool;

VIRIATUM_EXPORT_PREFIX void cleanup_palloc();
VIRIATUM_EXPORT_PREFIX void add_palloc(struct memory_pool_t *pool);

static __inline void create_chunk(struct memory_chunk_t **chunk_pointer, size_t size, size_t index_pool) {
    /* reserves space for the index counter for the pointer to
    a temporary item and then allocates the chunk structure */
    size_t index;
    struct memory_item_t *item;
    struct memory_chunk_t *chunk = MALLOC(sizeof(struct memory_chunk_t));

    /* resets the chunk bitmap so that all of its values are set
    to the zero value and then resets the complete set of contents
    of the chunk structure, noting that all the items are also set
    to their default initial value and with the apropriate part of
    the chunk buffer associated */
    memset(chunk->bitmap, 0, sizeof(char) * CHUNK_SIZE);
    chunk->index = index_pool;
    chunk->is_full = FALSE;
    chunk->free = 0;
    chunk->item_alloc = 0;
    chunk->item_size = size;
    chunk->chunk_size = size * CHUNK_SIZE;
    chunk->buffer = MALLOC(chunk->chunk_size);
    for(index = 0; index < CHUNK_SIZE; index++) {
        item = &chunk->items[index];
        item->index = index;
        item->chunk = chunk;
        item->buffer = (char *) chunk->buffer + (chunk->item_size * index);
    }

    /* returns the allocated chunk using the provided pointer to it
    as the returning value (indirect returning) */
    *chunk_pointer = chunk;
}

static __inline void delete_chunk(struct memory_chunk_t *chunk) {
    /* resets all the chunk data to the original values so that
    no side effect is created with mutiple re-usage of the chunk */
    chunk->index = 0;
    chunk->is_full = FALSE;
    chunk->free = 0;
    chunk->item_alloc = 0;

    /* releases both the chunk buffer (used for the item contents)
    and the chunk structure itself (avoiding any memory leak) */
    FREE(chunk->buffer);
    FREE(chunk);
}

static __inline void alloc_memory_pool(struct memory_pool_t *pool, size_t chunk_max_size) {
    /* in case the chunk (initial) maximum size is not defined
    sets it with the default value */
    chunk_max_size = chunk_max_size > 0 ? chunk_max_size : 4;

    /* populates the pool structure with the initial values and
    calculates the various calculated attributes in it */
    pool->free = 0;
    pool->chunk_alloc = 0;
    pool->chunk_count = 0;
    pool->chunk_max_size = chunk_max_size;
    pool->items_max_size = pool->chunk_max_size * CHUNK_MARGIN(CHUNK_SIZE);

    /* allocates the buffer of chunks for the memory pool and then
    allocates the map that associates a hashed buffer pointer with
    the item that it contains */
    pool->chunks = MALLOC(
        sizeof(struct memory_chunk_t *) * pool->chunk_max_size
    );
    pool->buffer_item_map = MALLOC(
        sizeof(struct memory_item_t *) * pool->items_max_size
    );

    /* resets the buffer item map to the initial zerified value so
    that no problems happen for initial searching */
    memset(
        pool->buffer_item_map, 0,
        sizeof(struct memory_item_t *) * pool->items_max_size
    );

    /* adds the newly allocated pool to the global pool structures
    so that it may be relased with the global pools release operation */
    add_palloc(pool);
}

static __inline void release_memory_pool(struct memory_pool_t *pool) {
    /* allocates space for the temporary variables that
    will hold both the index counter for iteration and
    the pointer to the chunk holder */
    size_t index;
    struct memory_chunk_t *chunk;

    /* iterates over the complete set of chunks present
    in the pool to release their memory (avoid leaks) */
    for(index = 0; index < pool->chunk_count; index++) {
        chunk = pool->chunks[index];
        delete_chunk(chunk);
    }

    /* releases the buffer containing the chunk structure
    references and the buffer for the map associating the
    buffer pointers with the item structures */
    FREE(pool->chunks);
    FREE(pool->buffer_item_map);

    /* resets all of the pool values to their original
    values in order to avoid any duplicated initialization
    problems that might occur */
    pool->free = 0;
    pool->chunk_count = 0;
    pool->chunk_max_size = 0;
    pool->items_max_size = 0;
}

static __inline void resize_memory_pool(struct memory_pool_t *pool, size_t chunk_max_size) {
    /* allocates space for the index values and for the various
    storage attributes that will hold the old values of the pool
    (before the resize operation) */
    char is_grow;
    char is_set;
    size_t index;
    size_t index_c;
    size_t index_m;
    size_t old_chunk_count = pool->chunk_count;
    size_t old_items_max_size = pool->items_max_size;
    struct memory_chunk_t **old_chunks = pool->chunks;
    struct memory_item_t **old_buffer_item_map = pool->buffer_item_map;

    /* allocates space for the temporary reference to the chunk
    structure to be used for local operations */
    struct memory_chunk_t *chunk;

    /* allocates space for the two temporary item iteration values
    to be used in each of the iteration steps */
    struct memory_item_t *item;
    struct memory_item_t *_item;

    /* in case there's no (real) resize operation because the target
    pool size is the same as the one already defined, must return the
    control to the caller function immediately (nothing to be done) */
    if(chunk_max_size == pool->chunk_max_size) { return; }

    /* verifies if the resize operation in the memory pool is a grow
    operation or a shrink operation (based on the curren chunk max size) */
    is_grow = chunk_max_size > pool->chunk_max_size;

    /* updates both the chunk maximum size value and the "calculated"
    items maximu, size values (based on chunk size) */
    pool->chunk_max_size = chunk_max_size;
    pool->items_max_size = pool->chunk_max_size * CHUNK_MARGIN(CHUNK_SIZE);

    /* set the possible "new" chunk count value taking into account
    that if the chunk max size value is lower than the current chunk
    count the chunk count must be updated to that value */
    pool->chunk_count = pool->chunk_count < chunk_max_size ?\
        pool->chunk_count : chunk_max_size;

    /* allocates the new memory buffers for the chunks and for the map
    that associated the buffer pointer with the associated memory item */
    pool->chunks = MALLOC(
        sizeof(struct memory_chunk_t *) * pool->chunk_max_size
    );
    pool->buffer_item_map = MALLOC(
        sizeof(struct memory_item_t *) * pool->items_max_size
    );

    /* "resets" the memory buffer for the map with zero values so that
    no problems occur when the map is accessed */
    memset(
        pool->buffer_item_map, 0,
        sizeof(struct memory_item_t *) * pool->items_max_size
    );

    /* verifies if the current resize operation is of type grow or if
    it is a shrink operation, if the operation is grow all the chunks
    are copied to the "new" pool otherwise only the valid ones are copied
    and then the empty ones are copied to fill the "new" smaller pool */
    if(is_grow) {
        /* iterates over the current set of chunks to copy the existing
        chunks to the new chunks buffer (copy operation) */
        for(index = 0; index < old_chunk_count; index++) {
            pool->chunks[index] = old_chunks[index];
        }
    } else {
        /* starts the index value to be used in the chunk buffer filling
        operation and also start the is set flag that control the setting
        of the base free index in the pool */
        index_c = 0;
        is_set = FALSE;

        /* iterates over the various chunks present in the pool to filter
        the onnes that are valid (have items allocated) and then copies
        tose chunks into the "new" pool chunks buffer */
        for(index = 0; index < old_chunk_count; index++) {
            /* retrieves the current chunk an in case it has no items
            allocated continues the loop (ignores the chunk) */
            chunk = old_chunks[index];
            if(chunk->item_alloc == 0) { continue; }
            if(is_set == FALSE && chunk->is_full == FALSE) {
                is_set = TRUE;
                pool->free = index_c;
            }

            /* updates the chunk index position relative to the pool
            buffer so that it matched its new position */
            chunk->index = index_c;

            /* copies the chunk reference into the new chunks buffer and
            increments the index counter for the chunks buffer */
            pool->chunks[index_c] = chunk;
            index_c++;
        }

        /* updates the lowest free chunk index in the pool to the current
        index, (the index of the last allocated chunk) */
        if(is_set == FALSE) { pool->free = index_c; }

        /* iterates over the various chnks present in the pool to be able
        to fill the new chunks buffer with unallocated chunks (to avoid)
        extra memory allocations (expensive operations) for the extra chunks
        there should be a delete operation */
        for(index = 0; index < old_chunk_count; index++) {
            /* retrieves the current chunk an in case it's a valid chunk
            ignores it (continues the loop) otherwise in case the filling
            operation has finished deletes the chunk */
            chunk = old_chunks[index];
            if(chunk->item_alloc != 0) { continue; }
            if(index_c == chunk_max_size) {
                delete_chunk(chunk);
                continue;
            }

            /* updates the chunk index position relative to the pool
            buffer so that it matched its new position */
            chunk->index = index_c;

            /* sets the invalid (no items allocated) chunk in the current
            chunks buffer and then increments the chunk index counter */
            pool->chunks[index_c] = chunk;
            index_c++;
        }
    }

    /* runs the iteration around the old map to move the old items into the
    new map (new hash must be made) this an extremly slow operation */
    for(index = 0; index < old_items_max_size; index++) {
        /* retrieves the current item in iteration in case it's null
        or the buffer is not defined consideres the item as invalid
        and skips the current loop iteration */
        item = old_buffer_item_map[index];
        if(item == NULL || item->buffer == NULL) { continue; }

        /* retrieves the first attempt map index for the buffer based
        on the modulus operation on it and then starts the iteration
        to try to find the best fit for it base on open addressing */
        index_m = (size_t) item->buffer % pool->items_max_size;
        while(TRUE) {
            /* retrieves the item for the current index in the map
            and in case the item is undefined sets it */
            _item = pool->buffer_item_map[index_m];
            if(_item == NULL) {
                pool->buffer_item_map[index_m] = item;
                break;
            }

            /* in case the current index has reached the limit
            resets it to the zero value, otherwise increments it */
            if(index_m + 1 == pool->items_max_size) { index_m = 0; }
            else { index_m++; }
        }
    }

    /* releases the memory from the old chunks buffer and from the
    old map that associated the buffer to the item (avoids memory leak */
    FREE(old_chunks);
    FREE(old_buffer_item_map);
}

static __inline size_t pavailable(struct memory_pool_t *pool) {
    /* allocates the space for the temporary index counter,
    for the available counter and for the temporary item */
    size_t index;
    size_t available = 0;
    struct memory_item_t *item = NULL;

    /* iterates over the pool set of items to count the number
    of avialble "spots" in the memory pool (increments available) */
    for(index = 0; index < pool->items_max_size; index++) {
        item = pool->buffer_item_map[index];
        if(item == NULL) { available++; }
    }

    /* returns the "counted" number of available "spots" in the
    provided memory pool object */
    return available;
}

static __inline void pdiag(struct memory_pool_t *pool, char *string, size_t size) {
    SPRINTF(
        string,
        size,
        "Available: %ld, "
        "Max Items: %ld, "
        "Max Chunks: %ld, "
        "Used Chunks: %ld, "
        "Free Chunks: %ld\n",
        (long int) pavailable(pool),
        (long int) pool->items_max_size,
        (long int) pool->chunk_max_size,
        (long int) pool->chunk_count,
        (long int) pool->chunk_count - pool->free
    );
}

static __inline void *palloc(struct memory_pool_t *pool, size_t size) {
#ifdef VIRIATUM_MEMORY_POOL
    /* allocates space for the various index and counter pointes
    and then allocates the pointers to the buffer and to the various
    structures that are going to be created and/or used */
    size_t free;
    size_t index;
    size_t index_m;
    size_t increment;
    size_t chunk_count;
    void *buffer;
    struct memory_chunk_t *chunk;
    struct memory_chunk_t *_chunk;
    struct memory_item_t *item;
    struct memory_item_t *_item;

    /* in case the number of chunks in the memory pool is
    zero the pool is condered not initialized, must create it */
    if(pool->chunk_count == 0) { alloc_memory_pool(pool, 0); }

    /* in case the index of the first chunk to be considered
    free is the same as the number of chunks the pool is considered
    to be full and so must be extended with new chunk(s) */
    if(pool->free == pool->chunk_count) {
        /* calculates the new chunk count increment based on the
        current size in case it's the initial allocation only one
        chunk is allocated otherwise the increments is the current
        chunk count (duplicates the size) */
        increment = pool->chunk_count > 0 ? pool->chunk_count : 1;

        /* calculates the new chunk count for the pool and in case that
        value is larger than the current maximum chunk storage available
        in the memory pool must resize the memory pool */
        chunk_count = pool->chunk_count + increment;
        if(chunk_count > pool->chunk_max_size) {
            resize_memory_pool(pool, chunk_count);
        }

        /* iterates over the range of the increment to create the
        associated chunks and set them in the chunk buffer */
        for(index = 0; index < increment; index ++) {
            /* creates the chunk according to the size requested for
            the allocation (size must be constant across allocations) */
            create_chunk(&chunk, size, pool->chunk_count);

            /* sets the created chunck structure in the appropriate
            position inside the buffer and then increments the counter
            that controls the currently created chunks */
            pool->chunks[pool->chunk_count] = chunk;
            pool->chunk_count++;
        }
    }

    /* retrieves the reference to the first available chunk
    in the memory pool (the one to be used) */
    chunk = pool->chunks[pool->free];

    /* in case this is the first item allocation for the chunk
    the chunk is sets as allocated in the pool */
    if(chunk->item_alloc == 0) { pool->chunk_alloc++; }

    /* retrieves the index of the lowest free (available) item
    and then uses it to both retrieve the item structure and set
    it's index in the bitmap as not available (used), then
    increments the current item alloc in the chunk */
    free = chunk->free;
    item = &chunk->items[free];
    chunk->bitmap[free] = 1;
    chunk->item_alloc++;

    /* iterates over all the items from the currently
    allocated one to try to find a new one free in order
    to set it as the lowest free item in the chunk, note
    that the iteration starts at the current free index
    plus one (default behaviour) */
    for(index = chunk->free + 1; index < CHUNK_SIZE; index++) {
        if(chunk->bitmap[index] == 1) { continue; }
        chunk->free = index;
        break;
    }

    /* in case the lowest free item index in the chunk
    remains the same as before then the chunk is considered
    as full (all items are set as used) then the flag in the
    chunk is set and the pool free index is updated with the
    first free chunk found in the chunk sequence */
    if(chunk->free == free) {
        /* sets the full flag in the chunk so that it is
        correctly identified for any code using the pool */
        chunk->is_full = TRUE;

        /* because the current chunk is full a new one must
        be found in the chunk sequence, the available (if any)
        chunks should be found after the current chunk */
        for(index = pool->free + 1; index < pool->chunk_count; index++) {
            _chunk = pool->chunks[index];
            if(_chunk->is_full) { continue; }
            pool->free = index;
            break;
        }

        /* in case the index (after iteration) is the same as the
        chunk count index then the next free chunk is not available,
        pool chunks are going to be allocated in next alloc operation */
        if(index == pool->chunk_count) { pool->free = pool->chunk_count; }
    }

    /* retrieves the reference to the buffer that is allocated
    for the retrieved item and retrieves the initial value for
    the index in the map associating the buffer with the item it's
    going to be using open addressing with linear probing */
    buffer = item->buffer;
    index_m = (size_t) buffer % pool->items_max_size;
    while(TRUE) {
        /* retrieves the item for the current index in the map
        and in case the item is undefined sets it */
        _item = pool->buffer_item_map[index_m];
        if(_item == NULL) {
            pool->buffer_item_map[index_m] = item;
            break;
        }

        /* in case the current index has reached the limit
        resets it to the zero value, otherwise increments it */
        if(index_m + 1 == pool->items_max_size) { index_m = 0; }
        else { index_m++; }
    }

    /* returns the buffer that was just allocated to the
    caller method, "emulating" the normal malloc behaviour */
    return buffer;
#else
    return MALLOC(size);
#endif
}

static __inline void pfree(struct memory_pool_t *pool, void *buffer) {
#ifdef VIRIATUM_MEMORY_POOL
    /* allocates space for both the current index counter and
    the index for the chunk, the allocates memory for the item
    and chunk pointers */
    size_t index;
    size_t index_c;
    struct memory_item_t *item;
    struct memory_chunk_t *chunk;

    /* calculates the "initial" guess for the the map index
    assoicated with the buffer to be released, the releasing
    of the set wull be using an open addressing approach */
    index = (size_t) buffer % pool->items_max_size;
    while(TRUE) {
        /* retrieves the current item and in case there's
        a valid value set and the buffer is equivalent breaks
        the loop indicating a match */
        item = pool->buffer_item_map[index];
        if(item != NULL && item->buffer == buffer) { break; }

        /* in case the final index value has been reached resets
        the index value otherwise keeps incrementing the value */
        if(index + 1 == pool->items_max_size) { index = 0; }
        else { index++; }
    }

    /* removes the buffer item association in the map by
    setting the value in it as unset */
    pool->buffer_item_map[index] = NULL;

    /* retrieves the chunk associated with the item and uses
    it (with it's base buffer) to calculate the relative index
    of the current buffer to be released in the chunk's buffer */
    chunk = item->chunk;
    index_c = ((size_t) buffer - (size_t) chunk->buffer) / chunk->item_size;
    chunk->bitmap[index_c] = 0;

    /* ensures that the is full flag of the chunk is set
    to false as there's at least one item avaialble for
    alloication, then decrement the current item alloc
    for the chunk (one less element) */
    chunk->is_full = FALSE;
    chunk->item_alloc--;

    /* in case the index of the current chunk (that is
    not full now) is lower than the current lowest free one,
    then updates the value */
    if(chunk->index < pool->free) { pool->free = chunk->index; }

    /* verifies if the index of the "just" released item
    is lower than the current (lowest) free index in case
    it's updates the free index to the released one */
    if(index_c < chunk->free) { chunk->free = index_c; }

    /* in case the number of allocated items in the chunk
    has reached the zero value then the chunk is considered
    empty and is set as free to be released */
    if(chunk->item_alloc == 0) {
        /* decrements the number of allocated chunks in the pool
        and then verifies if the number of chunks in it has reached
        a level for which it must be shrinked */
        pool->chunk_alloc--;
        if(pool->chunk_alloc == pool->chunk_max_size / 3) {
            resize_memory_pool(pool, pool->chunk_max_size / 2);
        }
    }
#else
    FREE(buffer);
#endif
}
