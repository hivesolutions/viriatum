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

#define VIRIATUM_RESOLVE_PATH(file_path, default_path, result_path)\
    resolve_config_path(file_path, default_path, result_path)

static __inline char *resolve_config_path(char *file_path, char *default_path, char *result_path) {
    /* allocates space for the temporary valid flag to be used
    to store the validation result and for the pointer to the
    config path to be retrieved */
    int valid;
    char *config_path;

    /* checks if the file is ready to be accessed (present in the
    file system) and returns its path in such case */
    valid = file_path == NULL ? -1 : ACCESS(file_path, F_OK);
    if(valid == 0) { return file_path; }

    /* retrieves the configuration path because "now" this path
    is going to be required for complete path construction */
    config_path = VIRIATUM_CONFIG_PATH;

    /* in case the file path is not defined (invalid) there's no
    need to construct the complete configuration path */
    if(file_path != NULL) {
        SPRINTF(
            result_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s/%s",
            config_path,
            file_path
        );
    }

    /* checks if the file is ready to be accessed (present in the
    file system) and returns its path in such case */
    valid = file_path == NULL ? -1 : ACCESS(result_path, F_OK);
    if(valid == 0) { return result_path; }

    /* checks if the file is ready to be accessed (present in the
    file system) and returns its path in such case */
    valid = default_path == NULL ? -1 : ACCESS(default_path, F_OK);
    if(valid == 0) { return default_path; }

    /* in case the default path is not defined (invalid) there's no
    need to construct the complete configuration path */
    if(default_path != NULL) {
        SPRINTF(
            result_path,
            VIRIATUM_MAX_PATH_SIZE,
            "%s/%s",
            config_path,
            default_path
        );
    }

    /* checks if the file is ready to be accessed (present in the
    file system) and returns its path in such case */
    valid = default_path == NULL ? -1 : ACCESS(result_path, F_OK);
    if(valid == 0) { return result_path; }

    /* returns the "default" invalid result for the because no file
    path was found using the "regular" strategy */
    return NULL;
}
