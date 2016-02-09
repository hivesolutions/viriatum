/*
 Hive Viriatum Commons
 Copyright (c) 2008-2016 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

struct stream_t;

typedef void (*stream_update) (struct stream_t *stream);
typedef void (*stream_seek) (struct stream_t *stream, size_t target);
typedef size_t (*stream_size) (struct stream_t *stream);
typedef size_t (*stream_read) (struct stream_t *stream, unsigned char *buffer, size_t size);
typedef size_t (*stream_write) (struct stream_t *stream, unsigned char *buffer, size_t size);

typedef struct stream_t {
    stream_update open;
    stream_update close;
    stream_read read;
    stream_write write;
    stream_update flush;
    stream_seek seek;
    stream_size size;
    stream_size tell;

    /**
     * Reference to the lower level
     * handler substrate (child).
     */
    void *lower;
} stream;

VIRIATUM_EXPORT_PREFIX void create_stream(struct stream_t **stream_pointer);
VIRIATUM_EXPORT_PREFIX void delete_stream(struct stream_t *stream);
