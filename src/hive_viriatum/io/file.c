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
    fileBuffer = (unsigned char *) MALLOC(fileSize);

    /* reads the file contents */
    numberBytes = fread(fileBuffer, 1, fileSize, file);

    /* in case the number of read bytes is not the
    same as the total bytes in file (error) */
    if(numberBytes != fileSize) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading from file");
    }

    /* closes the file */
    fclose(file);

    /* sets the buffer as the buffer pointer */
    *bufferPointer = fileBuffer;

    /* sets the file size as the file size pointer */
    *fileSizePointer = fileSize;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE countFile(char *filePath, size_t *fileSizePointer) {
    /* allocates space for the file */
    FILE *file;

    /* allocates space for the file size */
    size_t fileSize;

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

    /* closes the file */
    fclose(file);

    /* sets the file size as the file size pointer */
    *fileSizePointer = fileSize;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteDirectoryEntriesFile(struct LinkedList_t *entries) {
    /* allocates space for an entry */
    struct File_t *entry;

    /* iterates continuously for entries list
    cleanup (removal of all nodes) */
    while(1) {
        /* pops an entry from the entries list */
        popValueLinkedList(entries, (void **) &entry, 1);

        /* in case the value is invalid (empty list) */
        if(entry == NULL) {
            /* breaks the cycle */
            break;
        }

        /* releases the entry name memory and
        the entry memory */
        FREE(entry->name);
        FREE(entry);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE deleteDirectoryEntriesMapFile(struct LinkedList_t *map) {
    /* allocates space for an entry (map) */
    struct HashMap_t *entryMap;

    /* iterates continuously for entries list
    cleanup (removal of all nodes) */
    while(1) {
        /* pops an entry from the entries (map) list */
        popValueLinkedList(map, (void **) &entryMap, 1);

        /* in case the value is invalid (empty list) */
        if(entryMap == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the hash map */
        deleteHashMap(entryMap);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE entriesToMapFile(struct LinkedList_t *entries, struct LinkedList_t **mapPointer) {
    /* allocats space for the entry, for the iterator to be used
	to percolate the various entries and for the entry map and
	the map object used in the entry map percolation */
    struct File_t *entry;
	struct Iterator_t *entriesIterator;
    struct HashMap_t *entryMap;
    struct LinkedList_t *map;

	/* creates a new linke list in the for the entries maps */
    createLinkedList(&map);

	/* creates a new linked list iterator for the entries */
    createIteratorLinkedList(entries, &entriesIterator);

    /* iterates continuously */
    while(1) {
		/* retrieves the next entry from the entries iterator */
        getNextIterator(entriesIterator, (void **) &entry);

		/* in case the entry is not valid (no more
		items available) */
        if(entry == NULL) {
            /* breaks the switch */
            break;
        }

        /* creates the hash map */
        createHashMap(&entryMap, 0);

        /* sets the various entry values in the hash map */
        setValueStringHashMap(entryMap, (unsigned char *) "type", (void *) entry->type);
        setValueStringHashMap(entryMap, (unsigned char *) "name", (void *) entry->name);
        setValueStringHashMap(entryMap, (unsigned char *) "size", (void *) entry->size);

        /* adds the entry map to the (list) of maps */
        appendValueLinkedList(map, (void *) entryMap);
    }

	/* deletes the entries iterator */
    deleteIteratorLinkedList(entries, entriesIterator);

	/* sets the entries map in the reference pointed
	by the map pointer value */
    *mapPointer = map;

    /* raise no error */
    RAISE_NO_ERROR;
}

int _entryCompareFile(void *first, void *second) {
    /* returns the result of the string comparision
    of both values (second and first) */
    return strcmp((char *) first, (char *) second);
}

#ifdef VIRIATUM_PLATFORM_WIN32

ERROR_CODE isDirectoryFile(char *filePath, unsigned int *isDirectory) {
    /* in case the attributes value contains the file attribute
    directory reference (the file is a directory)*/
    if(GetFileAttributes(filePath) & FILE_ATTRIBUTE_DIRECTORY) {
        /* sets the is directory flag */
        *isDirectory = 1;
    }
    /* otherwise the file path does not refer a directory
    it must be a file or it may not exist */
    else {
        /* unsets the is directory flag */
        *isDirectory = 0;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE listDirectoryFile(char *filePath, struct LinkedList_t *entries) {
    /* allocates the string to hold the composite wildcard
    listing directory path */
    char *listPath;

    /* allocates space for both the entry and the
    length of the entry name */
    struct File_t *entry;
    size_t entryNameLength;

    /* allocates the various windows internal structures
    for the findind of the directory entries */
    WIN32_FIND_DATA findData;
    HANDLE handlerFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    /* retrieves the length of the file path */
    size_t pathLength = strlen(filePath);

    /* allocates and populates the list (directory) path
    with the appropriate wildcard value */
    listPath = (char *) MALLOC(pathLength + 3);
    memcpy(listPath, filePath, pathLength + 1);
    memcpy(listPath + pathLength, "\\*", 3);

    /* tries to find the first file with the wildcard
    value (checks for error) */
    handlerFind = FindFirstFile(listPath, &findData);

    /* in case the retieved handler is not valid
    (error) */
    if(handlerFind == INVALID_HANDLE_VALUE) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem listing directory");
    }

    do {
        /* allocates a new entry value */
        entry = MALLOC(sizeof(struct File_t));

        /* in case the file is of type directory */
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            /* sets the entry type as directory */
            entry->type = FILE_TYPE_DIRECTORY;
        }
        /* otherwise it must be a regular file */
        else {
            /* sets the entry type as regular */
            entry->type = FILE_TYPE_REGULAR;
        }

        /* sets the entry size from the find data information */
        entry->size = findData.nFileSizeLow;

        /* calculates the length of the entry name and uses
        it to create the memory space for the entry name and then
        copies the contents into it */
        entryNameLength = strlen(findData.cFileName);
        entry->name = (char *) MALLOC(entryNameLength + 1);
        memcpy(entry->name, findData.cFileName, entryNameLength + 1);

        /* adds the entry to the list of entries for
        the current directory (path) */
        appendValueLinkedList(entries, entry);
    } while(FindNextFile(handlerFind, &findData) != 0);

    /* closes the handler to find */
    FindClose(handlerFind);

    /* releases the list path reference */
    FREE(listPath);

    /* raise no error */
    RAISE_NO_ERROR;
}

#endif

#ifdef VIRIATUM_PLATFORM_UNIX

ERROR_CODE isDirectoryFile(char *filePath, unsigned int *isDirectory) {
    /* allocates space for the directory reference */
    DIR *directory;

    /* tries to open the directory for the file path */
    directory = opendir(filePath);

    /* in case the directory reference is not valid (null) */
    if(directory == NULL) {
        /* unsets the is directory flag */
        *isDirectory = 0;
    }
    /* otherwise the directory reference is valid */
    else {
        /* sets the is directory flag */
        *isDirectory = 1;
    }

    /* closes the directory reference */
    closedir(directory);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE listDirectoryFile(char *filePath, struct LinkedList_t *entries) {
    /* allocates space for the directory reference and
    for the entity reference */
    DIR *directory;
    struct dirent *entity;

    /* allocates space for both the entry and the
    length of the entry name */
    struct File_t *entry;
    size_t entryNameLength;

    /* opens the directory for the file path */
    directory = opendir(filePath);

    /* in case the directory reference is not valid */
    if(directory == NULL) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem listing directory");
    }

    /* print all the files and directories within directory */
    while(1) {
        /* retrieves the entity by reading it from the directory */
        entity = readdir(directory);

        /* in case the entity is not defined
        (the directory list is finished) */
        if(entity == NULL) {
            /* breaks the switch */
            break;
        }

        /* allocates a new entry value */
        entry = MALLOC(sizeof(struct File_t));

		entry->type = 1;
		entry->size = 199;

        /* calculates the length of the entry name and uses
        it to create the memory space for the entry name and then
        copies the contents into it */
        entryNameLength = strlen(entity->d_name);
        entry->name = (char *) MALLOC(entryNameLength + 1);
        memcpy(entry->name, entity->d_name, entryNameLength + 1);

        /* adds the entrys name to the list of entries for
        the current directory (path) */
        appendValueLinkedList(entries, entryName);
    }

    /* sorts the entries list according to the entry
    compare file (comparator) function */
    sortLinkedList(entries, _entryCompareFile);

    /* closes the directory reference */
    closedir(directory);

    /* raise no error */
    RAISE_NO_ERROR;
}

#endif
