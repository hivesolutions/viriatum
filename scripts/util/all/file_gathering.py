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

"""
File gathering utility that walks each of the source directories of
the tree and writes the sources it finds under every one of them as a
list of continuation lines, the shape the lists of the packaging are
written in, so that one of them may be rebuilt rather than kept by
hand.

Writes one file per source directory, named after the directory with
its separators turned into dots, holding every source of it relative
to that directory and closed by a line that carries no continuation.

Features:
    - The sources of the roots of a directory listed ahead of the
      ones that sit under it, which is the order the lists of the
      packaging are written in.
    - The separators of the platform normalised to the forward one,
      so that the list is the same whichever machine gathered it.
    - Only the extensions a source may carry are gathered, everything
      else the directory holds being left out of the list.

Run from the project root with:
    python scripts/util/all/file_gathering.py
"""

from os import walk
from os.path import join

__author__ = "João Magalhães <joamag@hive.pt>"
""" The author(s) of the module """

__copyright__ = "Copyright (c) 2008-2026 Hive Solutions Lda."
""" The copyright for the module """

__license__ = "Apache License, Version 2.0"
""" The license for the module """

SOURCE_FILE_EXTENSIONS = ("cpp", "c", "mm")
""" The extensions a source of the tree may carry, everything else a
directory holds is never part of a list """

SOURCE_DIRS = (
    "modules/mod_diag/src/viriatum_mod_diag",
    "modules/mod_gif/src/viriatum_mod_gif",
    "modules/mod_lua/src/viriatum_mod_lua",
    "modules/mod_php/src/viriatum_mod_php",
    "modules/mod_python/src/viriatum_mod_python",
    "src/viriatum_commons",
    "src/viriatum",
)
""" The directories a list is gathered for, one file being written
for each of them """


def order(name: str) -> tuple[int, str]:
    # the key a name is sorted by, the ones that sit at the root of a
    # directory coming ahead of the ones that sit under it and the
    # rest of the ordering being the plain one of the names
    return (0 if "/" not in name else 1, name)


def gather(source_dir: str) -> list[str]:
    # walks the directory and gathers every source under it, the name
    # of each one taken relative to the directory itself and with the
    # separators of the platform normalised to the forward one
    names = []
    for path, _folders, files in walk(source_dir):
        for file in files:
            if file.split(".")[-1] not in SOURCE_FILE_EXTENSIONS:
                continue
            total = join(path, file).replace("\\", "/")
            total = total.replace(source_dir + "/", "")
            names.append(total)
    names.sort(key=order)
    return names


def write(source_dir: str, names: list[str]) -> str:
    # writes the gathered names as the lists of the packaging are
    # written, one name per line and a continuation closing every one
    # of them but the last, which closes the list itself
    path = source_dir.replace("/", ".") + ".txt"
    with open(path, "w") as file:
        for index, name in enumerate(names):
            last = index == len(names) - 1
            file.write(name if last else name + " \\")
            file.write("\n")
        file.write("\n")
    return path


def main() -> None:
    for source_dir in SOURCE_DIRS:
        names = gather(source_dir)
        path = write(source_dir, names)
        print("Gathered %d sources of '%s' into '%s'" % (len(names), source_dir, path))


if __name__ == "__main__":
    main()
