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

#define DEFAULT_ARRAY_LIST_SIZE 256

typedef struct ArrayList_t {
    size_t size;
    size_t elementSize;
    size_t elementsBufferSize;
    void **elementsBuffer;
} ArrayList;

VIRIATUM_EXPORT_PREFIX void createArrayList(struct ArrayList_t **arrayListPointer, size_t valueSize, size_t initialSize);
VIRIATUM_EXPORT_PREFIX void deleteArrayList(struct ArrayList_t *arrayList);
VIRIATUM_EXPORT_PREFIX void setArrayList(struct ArrayList_t *arrayList, size_t index, void *element);
VIRIATUM_EXPORT_PREFIX void getArrayList(struct ArrayList_t *arrayList, size_t index, void **elementPointer);
