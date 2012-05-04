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

/**
 * Calculates a crc32 chesum for the given buffer, the process undertakes
 * the typical polynomial approach.
 * The calculated checksum is returned as an unsigned long value.
 *
 * @param buffer The (data) buffer to be used in the checkum calculation.
 * @param bufferLength The length of the (data) buffer to calculate the checksum.
 * @return The calculated crc32 checksum value.
 */
VIRIATUM_EXPORT_PREFIX unsigned long crc32(unsigned char *buffer, unsigned int bufferLength);
