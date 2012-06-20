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

#include "../structures/structures.h"

/**
 * Strucrure defining the various
 * possible types of files.
 */
typedef enum file_type_e {
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
} file_type;

/**
 * Structure defining a series
 * of meta information about a file
 * from the file system.
 */
typedef struct file_t {
    /**
     * The type of file (eg: normal, directory
     * symbolic link).
     */
    enum file_type_e type;

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

    /**
     * The date time of the last write
     * access to the file.
     */
    struct date_time_t time;
} file;

/**
 * Reads the file at the given file path to a generated
 * buffer and retrieves the size of the file.
 *
 * @param file_path The path to the file to be read.
 * @param buffer_pointer The pointer to the generated buffer
 * containing the file contents.
 * @param file_size_pointer The pointer to the file size.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE read_file(char *file_path, unsigned char **buffer_pointer, size_t *file_size_pointer);

/**
 * Writes the provided buffer into the file in the given
 * path, in case an error occurs it's raised.
 *
 * @param file_path The path to the file to be written.
 * @param buffer The buffer to be used in the writting
 * operation to the file.
 * @param buffer_size The size of the buffer to be used
 * for the writing operation.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE write_file(char *file_path, unsigned char *buffer, size_t buffer_size);

/**
 * Counts the number of bytes in the file in the
 * given path and returns the resulting count.
 *
 * @param file_path The path to the file to be "counted".
 * @param file_size_pointer The pointer to the file size.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE count_file(char *file_path, size_t *file_size_pointer);

/**
 * Retrieves the last write time (as a date time structure)
 * for the file in the given file path.
 *
 * @param file_path The path to the file to retrieve the write time.
 * @param date_time The time of the last write time in the file.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE get_write_time_file(char *file_path, struct date_time_t *date_time);

/**
 * Checks if the given file path referes a directory path.
 * The result is set to the given integer pointer.
 *
 * @param file_path The path to be verified to be a directory.
 * @param is_directory If the given path refers a directory.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE is_directory_file(char *file_path, unsigned int *is_directory);

/**
 * Lists the various (copies them) directory entries for the
 * given file (dirctory) path.
 * The "new" entries are copied to the given linked list of entries.
 *
 * @param file_path The path to the directory to be listed.
 * @param entries The linked list of entries to be used to
 * hold the various directory file entries.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE list_directory_file(char *file_path, struct linked_list_t *entries);

/**
 * Deletes, releases memory from all the directory entries described
 * in the given entries list.
 *
 * @param entries The list of entries to be deleted.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_directory_entries_file(struct linked_list_t *entries);

/**
 * Deletes, releases memory from all the directory entries (map) described
 * in the given entries list (map).
 *
 * @param map The list of entries (map) to be deleted.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE delete_directory_entries_map_file(struct linked_list_t *map);

/**
 * Converts a linked list of entries into a list of maps describing
 * the same entries (for dynamic usage).
 *
 * @param entries The list of file entries to be converted into
 * a list of (descriptive) maps.
 * @param map_pointer The pointer to the list to hold the various
 * maps describing the entries.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE entries_to_map_file(struct linked_list_t *entries, struct linked_list_t **map_pointer);

/**
 * Joins a file path with a given base path.
 * The joining of the path uses the default system separator.
 *
 * @param base_path The base path for the joined path.
 * @param name The name to be used as sufix in the path joining.
 * @param joined_path The joined path buffer to receive the final result.
 */
VIRIATUM_EXPORT_PREFIX ERROR_CODE join_path_file(char *base_path, char *name, char *joined_path);

/**
 * Comparator function to be used to compare two file
 * entries value, in order to sort them .
 *
 * @param first The first value to be compared.
 * @param second The second value to be compared.
 * @return The result of the comparison.
 */
VIRIATUM_EXPORT_PREFIX int _entry_compare_file(void *first, void *second);

static __inline char *validate_file(char *base_path, char *buffer, size_t count, size_t size) {
	/* allocates space for the valid flag and for the
	internal index counter */
	char valid;
	size_t index;

	/* allocates space for the temporary file path to
	be used for access verification */
	char file_path[VIRIATUM_MAX_PATH_SIZE];

	/* iterate over the range of element in the buffer
	to be verified */
	for(index = 0; index < count; index++) {
		/* creates the complete file path using the
		current buffer pointer value (current file
		path for validation) */
		SPRINTF(
			file_path,
			VIRIATUM_MAX_PATH_SIZE,
			"%s/%s",
			base_path,
			buffer
		);

		/* checks if the created file path really exists
		and in case it does returns immediately with the
        the buffer pointer value (value in verification) */
		valid = ACCESS(file_path, F_OK) == 0;
		if(valid) { return buffer; }

		/* increments the buffer pointer with the size of
		one element in the buffer (iteration increment) */
		buffer += size;
	}

	/* returns an invalid pointer because it was not possible
	to find any valid file path in the provided buffer */
    return NULL;
}