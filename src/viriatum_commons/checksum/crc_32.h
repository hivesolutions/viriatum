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

#pragma once

/**
 * Calculates a crc32 chesum for the given buffer, the process undertakes
 * the typical polynomial approach.
 * The calculated checksum is returned as an unsigned long value.
 *
 * @param buffer The (data) buffer to be used in the checkum calculation.
 * @param buffer_length The length of the (data) buffer to calculate the checksum.
 * @return The calculated crc32 checksum value.
 */
VIRIATUM_EXPORT_PREFIX unsigned long crc_32(unsigned char *buffer, unsigned int buffer_length);
