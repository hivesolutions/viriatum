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

#include "file.h"

ERROR_CODE read_file(char *file_path, unsigned char **buffer_pointer, size_t *file_size_pointer) {
    /* allocates space for the file */
    FILE *file;

    /* allocates space for the file size */
    size_t file_size;

    /* allocates space for the file buffer */
    unsigned char *file_buffer;

    /* allocates space for the number of bytes */
    size_t number_bytes;

    /* opens the file */
    FOPEN(&file, file_path, "rb");

    /* in case the file is not found */
    if(file == NULL) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
    }

    /* seeks the file until the end */
    fseek(file, 0, SEEK_END);

    /* retrieves the file size */
    file_size = ftell(file);

    /* seeks the file until the beginning */
    fseek(file, 0, SEEK_SET);

    /* allocates space for the file buffer */
    file_buffer = (unsigned char *) MALLOC(file_size);

    /* reads the file contents */
    number_bytes = fread(file_buffer, 1, file_size, file);

    /* in case the number of read bytes is not the
    same as the total bytes in file (error) */
    if(number_bytes != file_size) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading from file");
    }

    /* closes the file */
    fclose(file);

    /* sets the buffer as the buffer pointer */
    *buffer_pointer = file_buffer;

    /* sets the file size as the file size pointer */
    *file_size_pointer = file_size;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE write_file(char *file_path, unsigned char *buffer, size_t buffer_size) {
    /* allocates space for the file */
    FILE *file;

    /* opens the file */
    FOPEN(&file, file_path, "wb");

    /* writes the provided buffer into the file */
    fwrite(buffer, sizeof(char), buffer_size, file);

    /* closes the file */
    fclose(file);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE count_file(char *file_path, size_t *file_size_pointer) {
    /* allocates space for the file */
    FILE *file;

    /* allocates space for the file size */
    size_t file_size;

    /* opens the file */
    FOPEN(&file, file_path, "rb");

    /* in case the file is not found */
    if(file == NULL) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem loading file");
    }

    /* seeks the file until the end */
    fseek(file, 0, SEEK_END);

    /* retrieves the file size */
    file_size = ftell(file);

    /* closes the file */
    fclose(file);

    /* sets the file size as the file size pointer */
    *file_size_pointer = file_size;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_directory_entries_file(struct linked_list_t *entries) {
    /* allocates space for an entry */
    struct file_t *entry;

    /* iterates continuously for entries list
    cleanup (removal of all nodes) */
    while(1) {
        /* pops an entry from the entries list */
        pop_value_linked_list(entries, (void **) &entry, 1);

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

ERROR_CODE delete_directory_entries_map_file(struct linked_list_t *map) {
    /* allocates space for an entry (type) and
    for the entry value type */
    struct type_t *entry_type;
    struct type_t *entry_value_type;

    /* iterates continuously for entries list
    cleanup (removal of all nodes) */
    while(1) {
        /* pops an entry from the entries (type) list */
        pop_value_linked_list(map, (void **) &entry_type, 1);

        /* in case the value is invalid (empty list) */
        if(entry_type == NULL) {
            /* breaks the cycle */
            break;
        }

        /* deletes the various values (types) from the hash map
        (first retrieves them and the excludes them)*/
        get_value_string_hash_map(entry_type->value.value_map, (unsigned char *) "type", (void **) &entry_value_type);
        delete_type(entry_value_type);
        get_value_string_hash_map(entry_type->value.value_map, (unsigned char *) "name", (void **) &entry_value_type);
        delete_type(entry_value_type);
        get_value_string_hash_map(entry_type->value.value_map, (unsigned char *) "size", (void **) &entry_value_type);
        delete_type(entry_value_type);
        get_value_string_hash_map(entry_type->value.value_map, (unsigned char *) "time", (void **) &entry_value_type);
        FREE(entry_value_type->value.value_string);
        delete_type(entry_value_type);

        /* deletes the hash map and the entry type */
        delete_hash_map(entry_type->value.value_map);
        delete_type(entry_type);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE entries_to_map_file(struct linked_list_t *entries, struct linked_list_t **map_pointer) {
    /* allocats space for the entry, for the iterator to be used
    to percolate the various entries and for the entry map and
    the map object used in the entry map percolation */
    struct file_t *entry;
    struct iterator_t *entries_iterator;
    struct hash_map_t *entry_map;
    struct linked_list_t *map;

    /* allocates space for an entry (type) and
    for the entry value type */
    struct type_t *entry_type;
    struct type_t *entry_value_type;

    /* allocates space for the date time string
    to be created for the file entry */
    char *date_time_string;

    /* creates a new linke list in the for the entries maps */
    create_linked_list(&map);

    /* creates a new linked list iterator for the entries */
    create_iterator_linked_list(entries, &entries_iterator);

    /* iterates continuously */
    while(1) {
        /* retrieves the next entry from the entries iterator */
        get_next_iterator(entries_iterator, (void **) &entry);

        /* in case the entry is not valid (no more
        items available) */
        if(entry == NULL) {
            /* breaks the switch */
            break;
        }

        /* creates the hash map */
        create_hash_map(&entry_map, 0);

        /* allocates spac for the date time string */
        date_time_string = MALLOC(17);

        /* creates the date time string for the file entry */
        SPRINTF(
            date_time_string,
            17,
            "%04d-%02d-%02d %02d:%02d",
            entry->time.year,
            entry->time.month,
            entry->time.day,
            entry->time.hour,
            entry->time.minute
        );

        /* creates the various types for the various entry values
        and sets them in the entry map for reference */
        create_type(&entry_value_type, INTEGER_TYPE);
        entry_value_type->value.value_int = entry->type;
        set_value_string_hash_map(entry_map, (unsigned char *) "type", (void *) entry_value_type);
        create_type(&entry_value_type, STRING_TYPE);
        entry_value_type->value.value_string = (char *) entry->name;
        set_value_string_hash_map(entry_map, (unsigned char *) "name", (void *) entry_value_type);
        create_type(&entry_value_type, INTEGER_TYPE);
        entry_value_type->value.value_int = (int) entry->size;
        set_value_string_hash_map(entry_map, (unsigned char *) "size", (void *) entry_value_type);
        create_type(&entry_value_type, STRING_TYPE);
        entry_value_type->value.value_string = date_time_string;
        set_value_string_hash_map(entry_map, (unsigned char *) "time", (void *) entry_value_type);

        /* creates the entry type (for the map) and sets the
        entry map on it */
        create_type(&entry_type, MAP_TYPE);
        entry_type->value.value_map = entry_map;

        /* adds the entry type to the (list) of maps */
        append_value_linked_list(map, (void *) entry_type);
    }

    /* deletes the entries iterator */
    delete_iterator_linked_list(entries, entries_iterator);

    /* sets the entries map in the reference pointed
    by the map pointer value */
    *map_pointer = map;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE join_path_file(char *base_path, char *name, char *joined_path) {
    /* retrieves the length for both the base path and
    the name values */
    size_t base_path_length = strlen(base_path);
    size_t name_length = strlen(name);

    /* adds the "initial" base path to the joined
    path buffer (initial step) */
    memcpy(joined_path, base_path, base_path_length);

    /* in case the base path ends with the separator
    (no need to add the extra separator)*/
    if(base_path[base_path_length - 1] == '/') {
        /* adds the name part to the joined path */
        memcpy(joined_path + base_path_length, name, name_length + 1);
    }
    /* otherwise the separator must be added to the joined path */
    else {
        /* adds the separator to the joined path and then adds
        the name to the joined path also */
        memcpy(joined_path + base_path_length, "/", 1);
        memcpy(joined_path + base_path_length + 1, name, name_length + 1);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

int _entry_compare_file(void *first, void *second) {
    /* casts the first and second item as files */
    struct file_t *first_file = (struct file_t *) first;
    struct file_t *second_file = (struct file_t *) second;

    /* in case the file type is the same in both elements */
    if(first_file->type == second_file->type) {
        /* returns the result of the string comparision
        of both values (second and first) */
        return strcmp((char *) first_file->name, (char *) second_file->name);
    }
    /* otherwise the type of the file shall be used
    as the main comparision method */
    else {
        /* returns the result of the difference between
        the second file type and the first file type */
        return second_file->type - first_file->type;
    }
}

#ifdef VIRIATUM_PLATFORM_WIN32

ERROR_CODE get_write_time_file(char *file_path, struct date_time_t *date_time) {
    /* allocates space for the various file times */
    FILETIME time_create;
    FILETIME time_access;
    FILETIME time_write;

    /* allocates space for the structure holding
    the time write local */
    SYSTEMTIME time_write_local;

    /* allocates space for the handle to the file */
    HANDLE file_handle;

    /* retrieves the file for reading in the requested file path */
    file_handle = CreateFile(
        file_path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    /* in case the created file handle is not valid */
    if(file_handle == INVALID_HANDLE_VALUE) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Could not create file");
    }

    /* retrieve the file times for the file */
    if(!GetFileTime(file_handle, &time_create, &time_access, &time_write)) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem retrieving file time");
    }

    /* convert the last write time to "local" time */
    FileTimeToSystemTime(&time_write, &time_write_local);

    /* populates the date time structure with the information
    on the file various parts */
    date_time->year = time_write_local.wYear;
    date_time->month = time_write_local.wMonth;
    date_time->day = time_write_local.wDay;
    date_time->hour = time_write_local.wHour;
    date_time->minute = time_write_local.wMinute;
    date_time->second = time_write_local.wSecond;

    /* closes the file handle */
    CloseHandle(file_handle);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE is_directory_file(char *file_path, unsigned int *is_directory) {
    /* retrieves the file attributes from the file in
    the current path, must be able to detect errors */
    int file_attributes = GetFileAttributes(file_path);

    /* in case there is an error retrieving the file attributes
    this is considered to be an invalid file */
    if(file_attributes == INVALID_FILE_ATTRIBUTES) {
        /* unsets the is directory flag */
        *is_directory = 0;
    }
    /* in case the attributes value contains the file attribute
    directory reference (the file is a directory)*/
    else if(file_attributes & FILE_ATTRIBUTE_DIRECTORY) {
        /* sets the is directory flag */
        *is_directory = 1;
    }
    /* otherwise the file path does not refer a directory
    it must be a file or it may not exist */
    else {
        /* unsets the is directory flag */
        *is_directory = 0;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE list_directory_file(char *file_path, struct linked_list_t *entries) {
    /* allocates the string to hold the composite wildcard
    listing directory path */
    char *list_path;

    /* allocates space for the entry, entry full name
    and the length of the entry name */
    struct file_t *entry;
    char entry_full_name[4096];
    size_t entry_name_length;

    /* allocates the various windows internal structures
    for the findind of the directory entries */
    WIN32_FIND_DATA find_data;
    HANDLE handler_find = INVALID_HANDLE_VALUE;

    /* retrieves the length of the file path */
    size_t path_length = strlen(file_path);

    /* allocates and populates the list (directory) path
    with the appropriate wildcard value */
    list_path = (char *) MALLOC(path_length + 3);
    memcpy(list_path, file_path, path_length + 1);
    memcpy(list_path + path_length, "\\*", 3);

    /* tries to find the first file with the wildcard
    value (checks for error) */
    handler_find = FindFirstFile(list_path, &find_data);

    /* in case the retieved handler is not valid
    (error) */
    if(handler_find == INVALID_HANDLE_VALUE) {
        /* raises an error */
        RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem listing directory");
    }

    do {
        /* allocates a new entry value and resets the
        necessary structures (time) */
        entry = MALLOC(sizeof(struct file_t));
        memset(&entry->time, 0, sizeof(struct date_time_t));

        /* in case the file is of type directory */
        if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            /* sets the entry type as directory */
            entry->type = FILE_TYPE_DIRECTORY;
        }
        /* otherwise it must be a regular file */
        else {
            /* sets the entry type as regular */
            entry->type = FILE_TYPE_REGULAR;
        }

        /* sets the entry size from the find data information */
        entry->size = find_data.nFileSizeLow;

        /* joins the base name with the directory path to
        retrieve the full entry name then uses it to retrieve
        the write time for the file */
        join_path_file(file_path, find_data.cFileName, entry_full_name);
        get_write_time_file(entry_full_name, &entry->time);

        /* calculates the length of the entry name and uses
        it to create the memory space for the entry name and then
        copies the contents into it */
        entry_name_length = strlen(find_data.cFileName);
        entry->name = (unsigned char *) MALLOC(entry_name_length + 1);
        memcpy(entry->name, find_data.cFileName, entry_name_length + 1);

        /* adds the entry to the list of entries for
        the current directory (path) */
        append_value_linked_list(entries, entry);
    } while(FindNextFile(handler_find, &find_data) != 0);

    /* sorts the entries list according to the entry
    compare file (comparator) function */
    sort_linked_list(entries, _entry_compare_file);

    /* closes the handler to find */
    FindClose(handler_find);

    /* releases the list path reference */
    FREE(list_path);

    /* raise no error */
    RAISE_NO_ERROR;
}

#endif

#ifdef VIRIATUM_PLATFORM_UNIX

ERROR_CODE get_write_time_file(char *file_path, struct date_time_t *date_time) {
    struct stat file_stat;
    struct tm time;

    stat(file_path, &file_stat);

    gmtime_r((const time_t *) &file_stat.st_mtime, &time);

    /* populates the date time structure with the information
    on the file various parts */
    date_time->year = time.tm_year + 1900;
    date_time->month = time.tm_mon + 1;
    date_time->day = time.tm_mday;
    date_time->hour = time.tm_hour + 1;
    date_time->minute = time.tm_min + 1;
    date_time->second = time.tm_sec + 1;

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE is_directory_file(char *file_path, unsigned int *is_directory) {
    /* allocates space for the directory reference */
    DIR *directory;

    /* tries to open the directory for the file path */
    directory = opendir(file_path);

    /* in case the directory reference is not valid (null) */
    if(directory == NULL) {
        /* unsets the is directory flag */
        *is_directory = 0;
    }
    /* otherwise the directory reference is valid */
    else {
        /* sets the is directory flag */
        *is_directory = 1;

        /* closes the directory reference */
        closedir(directory);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE list_directory_file(char *file_path, struct linked_list_t *entries) {
    /* allocates space for the directory reference and
    for the entity reference */
    DIR *directory;
    struct dirent *entity;

    /* allocates space for the entry (structure), the
    the entry full name, the length of the entry name and
    the entry stat structure (for file attributes)*/
    struct file_t *entry;
    char entry_full_name[4096];
    size_t entry_name_length;
    struct stat entry_stat;

    /* opens the directory for the file path */
    directory = opendir(file_path);

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

        /* allocates a new entry value and resets the
        necessary structures (time) */
        entry = MALLOC(sizeof(struct file_t));
        memset(&entry->time, 0, sizeof(struct date_time_t));

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
        the entry stat structure and then uses it to retrieve its size
        and it's last write time */
        join_path_file(file_path, entity->d_name, entry_full_name);
        stat(entry_full_name, &entry_stat);
        entry->size = entry_stat.st_size;
        get_write_time_file(entry_full_name, &entry->time);

        /* calculates the length of the entry name and uses
        it to create the memory space for the entry name and then
        copies the contents into it */
        entry_name_length = strlen(entity->d_name);
        entry->name = (unsigned char *) MALLOC(entry_name_length + 1);
        memcpy(entry->name, entity->d_name, entry_name_length + 1);

        /* adds the entry to the list of entries for
        the current directory (path) */
        append_value_linked_list(entries, entry);
    }

    /* sorts the entries list according to the entry
    compare file (comparator) function */
    sort_linked_list(entries, _entry_compare_file);

    /* closes the directory reference */
    closedir(directory);

    /* raise no error */
    RAISE_NO_ERROR;
}

#endif
