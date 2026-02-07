#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

__author__ = "João Magalhães <joamag@hive.pt>"
""" The author(s) of the module """

__copyright__ = "Copyright (c) 2008-2026 Hive Solutions Lda."
""" The copyright for the module """

__license__ = "Apache License, Version 2.0"
""" The license for the module """

import os.path
import re

SOURCE_FILE_EXTENSIONS = ("cpp", "c", "mm")
SOURCE_DIRS = (
    "modules/mod_diag/src/viriatum_mod_diag",
    "modules/mod_gif/src/viriatum_mod_gif",
    "modules/mod_lua/src/viriatum_mod_lua",
    "modules/mod_php/src/viriatum_mod_php",
    "modules/mod_wsgi/src/viriatum_mod_wsgi",
    "src/viriatum_commons",
    "src/viriatum"
)
base_path = "none"

path_names = []

def compare(str1, str2):
    str1AtRoot = str1.find("/") == -1
    str2AtRoot = str2.find("/") == -1

    if str1AtRoot and not str2AtRoot:
        return -1
    if not str1AtRoot and str2AtRoot:
        return 1
    else:
        return cmp(str1, str2)

def visit(arg, dirname, names):
    valid_names = [valid_name for valid_name in names if valid_name.split(".")[-1] in SOURCE_FILE_EXTENSIONS]
    for valid_name in valid_names:
        total = dirname + "/" + valid_name
        total = total.replace(base_path + "/", "")
        total = total.replace("\\", "/")

        # iterates over all the source directories
        # to remove them from the path string
        for value in SOURCE_DIRS:
            # removes the source directory from
            # the source path
            total = total.replace(value, "")

        # in case the path starts with a slash
        if total[0] == "/":
            # removes the slash
            total = total[1:]

        path_names.append(total + " \\")

# iterates over all the source directories
for source_dir in SOURCE_DIRS:
    # resets the path names
    path_names = []

    # sets the global variable
    base_path = source_dir

    # walks the source dir
    os.path.walk(source_dir, visit, None)

    # sorts the path names
    path_names.sort(compare)

    # creates the file name for the output
    filename = source_dir.replace("/", ".")
    filename += ".txt"

    # opens the file
    file = open(filename, "w+")

    try:
        # determines the length of path names
        path_names_size = len(path_names)

        # prints out the path names
        for path_name_index in range(path_names_size):
            # retrieves the current path name
            path_name = path_names[path_name_index]

            # in case it's the last path
            if path_name_index == path_names_size - 1:
                # removes the appending slash
                file.write(path_name.replace("\\", ""))
            # otherwise
            else:
                file.write(path_name)

            file.write("\n")

        # adds new line at end of file
        file.write("\n")
    finally:
        # closes the file
        file.close()
