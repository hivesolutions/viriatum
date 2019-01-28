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

#include "io_stream.h"

void create_stream(struct stream_t **stream_pointer) {
    /* retrieves the stream size and uses the value in the
    allocation of the stream structure and then returns the
    value to the caller function through pointer */
    size_t stream_size = sizeof(struct stream_t);
    struct stream_t *stream = (struct stream_t *) MALLOC(stream_size);
    *stream_pointer = stream;
}

void delete_stream(struct stream_t *stream) {
    /* releases the stream */
    FREE(stream);
}
