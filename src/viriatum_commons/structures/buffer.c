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

#include "buffer.h"

void create_buffer(struct buffer_t **buffer_pointer, size_t initial_size) {
    /* retrieves the buffer size */
    size_t buffer_size = sizeof(struct buffer_t);

    /* allocates space for the buffer */
    struct buffer_t *buffer = (struct buffer_t *) MALLOC(buffer_size);

    /* starts the buffer attributes */
    buffer->pointer = NULL;
    buffer->size = 0;

    /* sets the buffer in the buffer pointer */
    *buffer_pointer = buffer;
}

void delete_buffer(struct buffer_t *buffer) {
    /* releases the buffer */
    FREE(buffer);
}

char *to_string_buffer(struct buffer_t *buffer) {
    /* allocates space for the string */
    char *string = (char *) MALLOC(buffer->size + 1);

    /* copies the buffer value into the string and then
    finalizes it with an end of string character */
    memcpy(string, buffer->pointer, buffer->size);
    string[buffer->size] = '\0';

    /* returns the allocated string, it must
    be released by the caller */
    return string;
}
