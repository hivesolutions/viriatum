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

#include "file.h"

ERROR_CODE readFile(char *filePath, unsigned char **bufferPointer, size_t *fileSizePointer) {
    /* allocates space for the file */
    FILE *file;

    /* allocates space for the file size */
    size_t fileSize;

    /* allocates space for the file buffer */
    unsigned char *fileBuffer;

    /* allocates space for the number of bytes */
    size_t numberBytes;

    /* opens the file */
    FOPEN(&file, filePath, "rb");

    /* in case the file is not found */
    if(file == NULL) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
    }

    /* seeks the file until the end */
    fseek(file, 0, SEEK_END);

    /* retrieves the file size */
    fileSize = ftell(file);

    /* seeks the file until the beginning */
    fseek(file, 0, SEEK_SET);

    /* allocates space for the file buffer */
    fileBuffer = (unsigned char *) malloc(fileSize);

    /* reads the file contents */
    numberBytes = fread(fileBuffer, 1, fileSize, file);

	/* closes the file */
	fclose(file);

    /* sets the buffer as the buffer pointer */
    *bufferPointer = fileBuffer;

    /* sets the file size as the file size pointer */
    *fileSizePointer = fileSize;

    /* raise no error */
    RAISE_NO_ERROR;
}
