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
 * Strucrure defining the various
 * possible types of files.
 */
typedef enum FileType_e {
    /**
     * The normal and regular file,
     * according to the file system.
     */
    FILE_TYPE_REGULAR = 1,

    /**
     * The directory, folder reference.
     */
    FILE_TYPE_DIRECTORY,

    /**
     * The symbolic or absolute type
     * file link.
     */
    FILE_TYPE_LINK
} FileType;

/**
 * Structure defining a series
 * of meta information about a file
 * from the file system.
 */
typedef struct File_t {
    /**
     * The type of file (eg: normal, directory
     * symbolic link).
     */
    enum FileType_e type;

    /**
     * The name of the file as a
     * string value.
     */
    unsigned char *name;

    /**
     * The size of the file represented
     * in the longest manner.
     */
    size_t size;
} File;

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

/**
 * Checks if the given file path referes a directory path.
 * The result is set to the given integer pointer.
 *
 * @param filePath The path to be verified to be a directory.
 * @param isDirectory If the given path refers a directory.
 */
ERROR_CODE isDirectoryFile(char *filePath, unsigned int *isDirectory);

/**
 * Lists the various (copies them) directory entries for the
 * given file (dirctory) path.
 * The "new" entries are copied to the given linked list of entries.
 *
 * @param filePath The path to the directory to be listed.
 * @param entries The linked list of entries to be used to
 * hold the various directory file entries.
 */
ERROR_CODE listDirectoryFile(char *filePath, struct LinkedList_t *entries);

/**
 * Deletes, releases memory from all the directory entries described
 * in the given entries list.
 *
 * @param entries The list of entries to be deleted.
 */
ERROR_CODE deleteDirectoryEntriesFile(struct LinkedList_t *entries);

/**
 * Deletes, releases memory from all the directory entries (map) described
 * in the given entries list (map).
 *
 * @param map The list of entries (map) to be deleted.
 */
ERROR_CODE deleteDirectoryEntriesMapFile(struct LinkedList_t *map);

/**
 * Converts a linked list of entries into a list of maps describing
 * the same entries (for dynamic usage).
 *
 * @param entries The list of file entries to be converted into
 * a list of (descriptive) maps.
 * @param mapPointer The pointer to the list to hold the various
 * maps describing the entries.
 */
ERROR_CODE entriesToMapFile(struct LinkedList_t *entries, struct LinkedList_t **mapPointer);

/**
 * Joins a file path with a given base path.
 * The joining of the path uses the default system separator.
 *
 * @param basePath The base path for the joined path.
 * @param name The name to be used as sufix in the path joining.
 * @param joinedPath The joined path buffer to receive the final result.
 */
ERROR_CODE joinPathFile(char *basePath, char *name, char *joinedPath);

/**
 * Comparator function to be used to compare two file
 * entries value, in order to sort them .
 *
 * @param first The first value to be compared.
 * @param second The second value to be compared.
 * @return The result of the comparison.
 */
int _entryCompareFile(void *first, void *second);
