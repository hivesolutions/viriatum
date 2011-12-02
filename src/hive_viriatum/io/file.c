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
    /* allocates space for an entry (type) and
    for the entry value type */
    struct Type_t *entryType;
    struct Type_t *entryValueType;

    /* iterates continuously for entries list
    cleanup (removal of all nodes) */
    while(1) {
        /* pops an entry from the entries (type) list */
        popValueLinkedList(map, (void **) &entryType, 1);

        /* in case the value is invalid (empty list) */
        if(entryType == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the various values (types) from the hash map
        (first retrieves them and the excludes them)*/
        getValueStringHashMap(entryType->value.valueMap, (unsigned char *) "type", (void **) &entryValueType);
        deleteType(entryValueType);
        getValueStringHashMap(entryType->value.valueMap, (unsigned char *) "name", (void **) &entryValueType);
        deleteType(entryValueType);
        getValueStringHashMap(entryType->value.valueMap, (unsigned char *) "size", (void **) &entryValueType);
        deleteType(entryValueType);
        getValueStringHashMap(entryType->value.valueMap, (unsigned char *) "time", (void **) &entryValueType);
        FREE(entryValueType->value.valueString);
        deleteType(entryValueType);

        /* deletes the hash map and the entry type */
        deleteHashMap(entryType->value.valueMap);
        deleteType(entryType);
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

    /* allocates space for an entry (type) and
    for the entry value type */
    struct Type_t *entryType;
    struct Type_t *entryValueType;

    /* allocates space for the date time string
    to be created for the file entry */
    char *dateTimeString;

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

        /* allocates spac for the date time string */
        dateTimeString = MALLOC(17);

        /* creates the date time string for the file entry */
        SPRINTF(dateTimeString, 17, "%04d-%02d-%02d %02d:%02d", entry->time.year, entry->time.month, entry->time.day, entry->time.hour, entry->time.minute);

        /* creates the various types for the various entry values
        and sets them in the entry map for reference */
        createType(&entryValueType, INTEGER_TYPE);
        entryValueType->value.valueInt = entry->type;
        setValueStringHashMap(entryMap, (unsigned char *) "type", (void *) entryValueType);
        createType(&entryValueType, STRING_TYPE);
        entryValueType->value.valueString = (char *) entry->name;
        setValueStringHashMap(entryMap, (unsigned char *) "name", (void *) entryValueType);
        createType(&entryValueType, INTEGER_TYPE);
        entryValueType->value.valueInt = entry->size;
        setValueStringHashMap(entryMap, (unsigned char *) "size", (void *) entryValueType);
        createType(&entryValueType, STRING_TYPE);
        entryValueType->value.valueString = dateTimeString;
        setValueStringHashMap(entryMap, (unsigned char *) "time", (void *) entryValueType);

        /* creates the entry type (for the map) and sets the
        entry map on it */
        createType(&entryType, MAP_TYPE);
        entryType->value.valueMap = entryMap;

        /* adds the entry type to the (list) of maps */
        appendValueLinkedList(map, (void *) entryType);
    }

    /* deletes the entries iterator */
    deleteIteratorLinkedList(entries, entriesIterator);

    /* sets the entries map in the reference pointed
    by the map pointer value */
    *mapPointer = map;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE joinPathFile(char *basePath, char *name, char *joinedPath) {
    /* retrieves the length for both the base path and
    the name values */
    size_t basePathLength = strlen(basePath);
    size_t nameLength = strlen(name);

    /* adds the "initial" base path to the joined
    path buffer (initial step) */
    memcpy(joinedPath, basePath, basePathLength);

    /* in case the base path ends with the separator
    (no need to add the extra separator)*/
    if(basePath[basePathLength - 1] == '/') {
        /* adds the name part to the joined path */
        memcpy(joinedPath + basePathLength, name, nameLength + 1);
    }
    /* otherwise the separator must be added to the joined path */
    else {
        /* adds the separator to the joined path and then adds
        the name to the joined path also */
        memcpy(joinedPath + basePathLength, "/", 1);
        memcpy(joinedPath + basePathLength + 1, name, nameLength + 1);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

int _entryCompareFile(void *first, void *second) {
    /* casts the first and second item as files */
    struct File_t *firstFile = (struct File_t *) first;
    struct File_t *secondFile = (struct File_t *) second;

    /* in case the file type is the same in both elements */
    if(firstFile->type == secondFile->type) {
        /* returns the result of the string comparision
        of both values (second and first) */
        return strcmp((char *) firstFile->name, (char *) secondFile->name);
    }
    /* otherwise the type of the file shall be used
    as the main comparision method */
    else {
        /* returns the result of the difference between
        the second file type and the first file type */
        return secondFile->type - firstFile->type;
    }
}

#ifdef VIRIATUM_PLATFORM_WIN32

ERROR_CODE getWriteTimeFile(char *filePath, struct DateTime_t *dateTime) {
    /* allocates space for the various file times */
    FILETIME timeCreate;
    FILETIME timeAccess;
    FILETIME timeWrite;

    /* allocates space for the structure holding
    the time write local */
    SYSTEMTIME timeWriteLocal;

    /* allocates space for the handle to the file */
    HANDLE fileHandle;

    /* retrieves the file for reading in the requested file path */
    fileHandle = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, NULL);

    /* in case the created file handle is not valid */
    if(fileHandle == INVALID_HANDLE_VALUE) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, "Could not create file");
    }

    /* retrieve the file times for the file */
    if(!GetFileTime(fileHandle, &timeCreate, &timeAccess, &timeWrite)) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, "Problem retrieving file time");
    }

    /* convert the last write time to "local" time */
    FileTimeToSystemTime(&timeWrite, &timeWriteLocal);

    /* populates the date time structure with the information
    on the file various parts */
    dateTime->year = timeWriteLocal.wYear;
    dateTime->month = timeWriteLocal.wMonth;
    dateTime->day = timeWriteLocal.wDay;
    dateTime->hour = timeWriteLocal.wHour;
    dateTime->minute = timeWriteLocal.wMinute;
    dateTime->second = timeWriteLocal.wSecond;

    /* closes the file handle */
    CloseHandle(fileHandle);

    /* raise no error */
    RAISE_NO_ERROR;
}

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

    /* allocates space for the entry, entry full name
    and the length of the entry name */
    struct File_t *entry;
    char entryFullName[4096];
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
        /* allocates a new entry value and resets the
        necessary structures (time) */
        entry = MALLOC(sizeof(struct File_t));
        memset(&entry->time, 0, sizeof(struct DateTime_t));

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

        /* joins the base name with the directory path to
        retrieve the full entry name then uses it to retrieve
        the write time for the file */
        joinPathFile(filePath, findData.cFileName, entryFullName);
        getWriteTimeFile(entryFullName, &entry->time);

        /* calculates the length of the entry name and uses
        it to create the memory space for the entry name and then
        copies the contents into it */
        entryNameLength = strlen(findData.cFileName);
        entry->name = (unsigned char *) MALLOC(entryNameLength + 1);
        memcpy(entry->name, findData.cFileName, entryNameLength + 1);

        /* adds the entry to the list of entries for
        the current directory (path) */
        appendValueLinkedList(entries, entry);
    } while(FindNextFile(handlerFind, &findData) != 0);

    /* sorts the entries list according to the entry
    compare file (comparator) function */
    sortLinkedList(entries, _entryCompareFile);

    /* closes the handler to find */
    FindClose(handlerFind);

    /* releases the list path reference */
    FREE(listPath);

    /* raise no error */
    RAISE_NO_ERROR;
}

#endif

#ifdef VIRIATUM_PLATFORM_UNIX

ERROR_CODE getWriteTimeFile(char *filePath, struct DateTime_t *dateTime) {
	struct stat fileStat;

    stat(filePath, &fileStat);

    /* populates the date time structure with the information
    on the file various parts */
    dateTime->year = fileStat.st_mtime.tm_year + 1900;
    dateTime->month = fileStat.st_mtime.tm_mon + 1;
    dateTime->day = fileStat.st_mtime.tm_mday;
    dateTime->hour = fileStat.st_mtime.tm_hour + 1;
    dateTime->minute = fileStat.st_mtime.tm_min + 1;
    dateTime->second = fileStat.st_mtime.tm_sec + 1;

    /* raise no error */
    RAISE_NO_ERROR;
}

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

    /* allocates space for the entry (structure), the
    the entry full name, the length of the entry name and
    the entry stat structure (for file attributes)*/
    struct File_t *entry;
    char entryFullName[4096];
    size_t entryNameLength;
    struct stat entryStat;

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

        /* in case the file is of type regular */
        if(entity->d_type == DT_REG) {
            /* sets the entry type as regular */
            entry->type = FILE_TYPE_REGULAR;
        }
        /* otherwise in case the file is of
        type directory */
        else if(entity->d_type == DT_DIR) {
            /* sets the entry type as directory */
            entry->type = FILE_TYPE_DIRECTORY;
        }

        /* joins the base name with the directory path to
        retrieve the full entry name then uses it to retrieve
        the entry stat structure and then uses it to retrieve its size */
        joinPathFile(filePath, entity->d_name, entryFullName);
        stat(entryFullName, &entryStat);
        entry->size = entryStat.st_size;

        /* calculates the length of the entry name and uses
        it to create the memory space for the entry name and then
        copies the contents into it */
        entryNameLength = strlen(entity->d_name);
        entry->name = (unsigned char *) MALLOC(entryNameLength + 1);
        memcpy(entry->name, entity->d_name, entryNameLength + 1);

        /* adds the entry to the list of entries for
        the current directory (path) */
        appendValueLinkedList(entries, entry);
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
