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

#ifdef VIRIATUM_DEBUG
extern size_t allocations;
#define ALLOCATIONS allocations
#define START_MEMORY size_t allocations = 0
#define MALLOC(size) mallocDebug(size, __FILE__, __LINE__)
#define CALLOC(count, size) calloc(count, size)
#define REALLOC(pointer, size) realloc(pointer, size)
#define FREE(pointer) freeDebug(pointer, __FILE__, __LINE__)
static __inline void *mallocDebug(size_t size, char *file, int line) { printf("malloc() %s: %d\n",  file, line);  allocations++; return malloc(size); }
static __inline void freeDebug(void *pointer, char *file, int line) { printf("free() %s: %d\n", file, line); allocations--; free(pointer); }
#endif

#ifndef VIRIATUM_DEBUG
extern size_t allocations;
#define ALLOCATIONS allocations
#define START_MEMORY size_t allocations = -1
#define MALLOC(size) malloc(size)
#define CALLOC(count, size) calloc(count, size)
#define REALLOC(pointer, size) realloc(pointer, size)
#define FREE(pointer) free(pointer)
#endif
