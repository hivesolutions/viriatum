/*
 Hive Viriatum Commons
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

static __inline void random_buffer(unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the index counter, the seconds
    since epoch (timestamp), for the random generator and
    for the currently generated byte value */
    size_t index;
    time_t seconds;
    int random;
    unsigned char byte;

    /* starts the random seed value with the current
    number of seconds (random initialization) */
    time(&seconds);
    srand((unsigned int) seconds);

    /* iterates over the requested buffer size to
    generate "human readable" random characters for
    the complete set of buffer positions */
    for(index = 0; index < buffer_size; index++) {
        random = rand();
        byte = (unsigned char) (random % 94);
        buffer[index] = byte + 34;
    }
}
