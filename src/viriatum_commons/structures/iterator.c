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

#include "iterator.h"

void create_iterator(struct iterator_t **iterator_pointer) {
    /* retrieves the iterator size */
    size_t iterator_size = sizeof(struct iterator_t);

    /* allocates space for the iterator */
    struct iterator_t *iterator = (struct iterator_t *) MALLOC(iterator_size);

    /* sets the iterator in the iterator pointer */
    *iterator_pointer = iterator;
}

void delete_iterator(struct iterator_t *iterator) {
    /* releases the iterator */
    FREE(iterator);
}

void get_next_iterator(struct iterator_t *iterator, void **next_pointer) {
    /* calls the iterator get next function */
    iterator->get_next_function(iterator, next_pointer);
}
