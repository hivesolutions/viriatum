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

#include "stdafx.h"

#include "string.h"

void create_string(struct string_t **string_pointer) {
    /* retrieves the string size */
    size_t string_size = sizeof(struct string_t);

    /* allocates space for the string */
    struct string_t *string = (struct string_t *) MALLOC(string_size);

    /* initializes the string */
    string->buffer = NULL;
    string->length = 0;

    /* sets the string in the string pointer */
    *string_pointer = string;
}

void delete_string(struct string_t *string) {
    /* releases the string */
    FREE(string);
}

void string_populate(struct string_t *string, unsigned char *buffer, size_t length, char calculate) {
    string->buffer = buffer;
    string->length = calculate ? strlen((char *) buffer) : length;
}
