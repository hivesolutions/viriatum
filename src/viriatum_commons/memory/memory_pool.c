/*
 Hive Viriatum Commons
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "memory_pool.h"

#ifdef VIRIATUM_MPOOL
static struct memory_pool_t *pools[256];
static size_t pool_counter = 0;

void cleanup_palloc() {
    size_t index;
    struct memory_pool_t *pool;

    for(index = 0; index < pool_counter; index++) {
        pool = pools[index];
        release_memory_pool(pool);
    }
}

void add_palloc(struct memory_pool_t *pool) {
    pools[pool_counter] = pool;
    pool_counter++;
}
#endif
