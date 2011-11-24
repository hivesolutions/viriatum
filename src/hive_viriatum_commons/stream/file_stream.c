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

#include "stdafx.h"

#include "file_stream.h"

VIRIATUM_EXPORT_PREFIX void createFileStream(struct FileStream_t **fileStreamPointer, unsigned char *filePath, unsigned char *mode) {
    /* retrieves the file stream size */
    size_t fileStreamSize = sizeof(struct FileStream_t);

    /* allocates space for the file stream */
    struct FileStream_t *fileStream = (struct FileStream_t *) MALLOC(fileStreamSize);

    /* creates the stream (structure) and sets the
    current file stream as the lower substrate */
    createStream(&fileStream->stream);
    fileStream->stream->lower = (void *) fileStream;

    /* sets the file parameters in the file stream */
    fileStream->filePath = filePath;
    fileStream->mode = mode;

    /* sets the various functions of the stream */
    fileStream->stream->open = openFileStream;
    fileStream->stream->close = closeFileStream;
    fileStream->stream->read = readFileStream;
    fileStream->stream->write = writeFileStream;
    fileStream->stream->flush = flushFileStream;

    /* sets the file stream in the file stream pointer */
    *fileStreamPointer = fileStream;
}

VIRIATUM_EXPORT_PREFIX void deleteFileStream(struct FileStream_t *fileStream) {
    /* closes the file reference */
    fclose(fileStream->file);

    /* deltes the stream (structure) */
    deleteStream(fileStream->stream);

    /* releases the file stream */
    FREE(fileStream);
}

VIRIATUM_EXPORT_PREFIX struct Stream_t *getStreamFileStream(struct FileStream_t *fileStream) {
    return fileStream->stream;
}

VIRIATUM_EXPORT_PREFIX void openFileStream(struct Stream_t *stream) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct FileStream_t *fileStream = (struct FileStream_t *) stream->lower;

    /* opens the file reference using the file path and the mode */
    FOPEN(&fileStream->file, fileStream->filePath, fileStream->mode);
}

VIRIATUM_EXPORT_PREFIX void closeFileStream(struct Stream_t *stream) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct FileStream_t *fileStream = (struct FileStream_t *) stream->lower;

    /* closes the file reference */
    fclose(fileStream->file);
}

VIRIATUM_EXPORT_PREFIX size_t readFileStream(struct Stream_t *stream, unsigned char *buffer, size_t size) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct FileStream_t *fileStream = (struct FileStream_t *) stream->lower;

    /* reads the given size of bytes into the buffer */
    return fread(buffer, sizeof(unsigned char), size, fileStream->file);
}

VIRIATUM_EXPORT_PREFIX size_t writeFileStream(struct Stream_t *stream, unsigned char *buffer, size_t size) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct FileStream_t *fileStream = (struct FileStream_t *) stream->lower;

    /* writes the given buffer into the file reference */
    return fwrite(buffer, sizeof(unsigned char), size, fileStream->file);
}

VIRIATUM_EXPORT_PREFIX void flushFileStream(struct Stream_t *stream) {
    /* retrieves the file stream from the stream (as the lowe substrate) */
    struct FileStream_t *fileStream = (struct FileStream_t *) stream->lower;

    /* flushes the file reference */
    fflush(fileStream->file);
}
