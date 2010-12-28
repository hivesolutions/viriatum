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

#include "crc_32.h"

unsigned long crc32(unsigned char *buffer, unsigned int bufferLength) {
	/* allocates space for the index */
    unsigned int index;

	/* allocates space for the crc 32 value */
    unsigned long crc32Value;
  
    /* starts the crc 32 value */
    crc32Value = 0;

	/* iterates over the length of the data */
    for(index = 0; index < bufferLength; index++) {
        crc32Value = crc32_tab[(crc32Value ^ buffer[index]) & 0xff] ^ (crc32Value >> 8);
    }

    /* returns the crc 32 value */
    return crc32Value;
}
