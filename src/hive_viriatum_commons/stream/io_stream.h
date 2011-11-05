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

typedef void (*streamUpdate) (struct Stream_t *stream);
typedef size_t (*streamWrite) (struct Stream_t *stream, unsigned char *buffer, size_t size);

typedef struct Stream_t {
    streamUpdate open;
    streamUpdate close;
    streamWrite write;
    /*httpConnectionUpdate read;
    httpConnectionUpdate reset;*/

    /**
     * Reference to the lower level
     * handler substrate (child).
     */
    void *lower;
} Stream;

VIRIATUM_EXPORT_PREFIX void createStream(struct Stream_t **streamPointer);
VIRIATUM_EXPORT_PREFIX void deleteStream(struct Stream_t *stream);
