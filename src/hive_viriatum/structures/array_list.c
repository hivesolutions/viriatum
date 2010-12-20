/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "array_list.h"

void createArrayList(struct ArrayList_t **arrayListPointer, size_t valueSize, size_t initialSize) {
    /* retrieves the array list size */
    size_t arrayListSize = sizeof(struct ArrayList_t);

    /* allocates space for the array list */
    struct ArrayList_t *arrayList = (struct ArrayList_t *) malloc(arrayListSize);

    /* in case the initial size is not set (zero) */
    if((void *) initialSize == 0) {
        /* sets the default initial size value */
        initialSize = DEFAULT_ARRAY_LIST_SIZE;
    }

    /* initializes the linked list size */
    arrayList->size = 0;

    /* allocates space for the elements buffer */
    arrayList->elementsBuffer = (void *) malloc(valueSize * initialSize);

    /* sets the array list in the array list pointer */
    *arrayListPointer = arrayList;
}
