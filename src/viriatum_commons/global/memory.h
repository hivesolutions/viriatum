/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../util/string_util_s.h"

#ifdef VIRIATUM_DEBUG
VIRIATUM_EXTERNAL_PREFIX size_t allocations;
#define ALLOCATIONS allocations
#define START_MEMORY size_t allocations = 0
#define MALLOC(size) malloc_debug(size, (char *) base_string_value_s((unsigned char *) __FILE__), __LINE__)
#define CALLOC(count, size) calloc_debug(count, size, (char *) base_string_value_s((unsigned char *) __FILE__), __LINE__)
#define REALLOC(pointer, size) realloc(pointer, size)
#define FREE(pointer) free_debug(pointer, (char *) base_string_value_s((unsigned char *) __FILE__), __LINE__)
#ifdef VIRIATUM_MEMORY_DEBUG
static __inline void *malloc_debug(size_t size, char *file, int line) { allocations++; printf("[MEMORY] [%s: %d] malloc() called [%d bytes] [%d objects]\n", file, line, size, allocations); return malloc(size); }
static __inline void *calloc_debug(size_t count, size_t size, char *file, int line) { allocations++; printf("[MEMORY] [%s: %d] calloc() called [%d count %d bytes] [%d objects]\n", file, line, count, size, allocations); return calloc(count, size); }
static __inline void free_debug(void *pointer, char *file, int line) { allocations--; printf("[MEMORY] [%s: %d] free() called [%d objects]\n", file, line, allocations); free(pointer); }
#endif
#ifndef VIRIATUM_MEMORY_DEBUG
static __inline void *malloc_debug(size_t size, char *file, int line) { allocations++; return malloc(size); }
static __inline void *calloc_debug(size_t count, size_t size, char *file, int line) { allocations++; return calloc(count, size); }
static __inline void free_debug(void *pointer, char *file, int line) { allocations--; free(pointer); }
#endif
#endif

#ifndef VIRIATUM_DEBUG
VIRIATUM_EXTERNAL_PREFIX size_t allocations;
#define ALLOCATIONS allocations
#define START_MEMORY size_t allocations = -1
#define MALLOC(size) malloc(size)
#define CALLOC(count, size) calloc(count, size)
#define REALLOC(pointer, size) realloc(pointer, size)
#define FREE(pointer) free(pointer)
#endif
