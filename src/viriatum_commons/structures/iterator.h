/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

typedef struct iterator_t {
    void *structure;
    void *parameters;
    void (*get_next_function) (struct iterator_t *iterator, void **next);
} iterator;

VIRIATUM_EXPORT_PREFIX void create_iterator(struct iterator_t **iterator_pointer);
VIRIATUM_EXPORT_PREFIX void delete_iterator(struct iterator_t *iterator);
VIRIATUM_EXPORT_PREFIX void get_next_iterator(struct iterator_t *iterator, void **next_pointer);
