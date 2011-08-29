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

#pragma once

/**
 * Reads the file at the given file path to a generated
 * buffer and retrieves the size of the file.
 *
 * @param filePath The path to the file to be read.
 * @param bufferPointer The pointer to the generated buffer
 * containing the file contents.
 * @param fileSizePointer The pointer to the file size.
 */
ERROR_CODE readFile(char *filePath, unsigned char **bufferPointer, size_t *fileSizePointer);

/**
 * Counts the number of bytes in the file in the
 * given path and returns the resulting count.
 *
 * @param filePath The path to the file to be "counted".
 * @param fileSizePointer The pointer to the file size.
 */
ERROR_CODE countFile(char *filePath, size_t *fileSizePointer);

