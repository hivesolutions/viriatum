#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Automium System
# Copyright (C) 2008-2012 Hive Solutions Lda.
#
# This file is part of Hive Automium System.
#
# Hive Automium System is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hive Automium System is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hive Automium System. If not, see <http://www.gnu.org/licenses/>.

__author__ = "João Magalhães <joamag@hive.pt>"
""" The author(s) of the module """

__version__ = "1.0.0"
""" The version of the module """

__revision__ = "$LastChangedRevision$"
""" The revision number of the module """

__date__ = "$LastChangedDate$"
""" The last change date of the module """

__copyright__ = "Copyright (c) 2008-2012 Hive Solutions Lda."
""" The copyright for the module """

__license__ = "GNU General Public License (GPL), Version 3"
""" The license for the module """

import os
import subprocess

import atm

DEV_HOME = "\\dev"
""" The default directory to the development directory
to be used in the building stages """

def autogen(path = None, clean = False):
    path = path or "autogen.sh"
    result = subprocess.call([
        path
    ])
    if not result == 0: raise RuntimeError("Problem executing autogen not successful")

    if not clean: return

    base_path = os.path.dirname(path)
    cache = os.path.join(base_path, "autom4te.cache")
    makefile = os.path.join(base_path, "Makefile-autoconfig")
    atm.remove(cache)
    atm.remove(makefile)

def msbuild(path, dev = True):
    # ensures that the development settings are correctly set
    # in the environment in case the development mode is set
    # then calls the msbuild command to start the process
    dev and ensure_dev()
    result = subprocess.call([
        "msbuild",
        path,
        "/p:Configuration=Release",
        "/p:VCBuildAdditionalOptions=/useenv"
    ])
    if not result == 0: raise RuntimeError("Problem executing msbuild not successful")

def ensure_dev():
    dev_home = atm.environ("DEV_HOME", DEV_HOME)
    atm.environ_s("INCLUDE", dev_home + "\\include")
    atm.environ_s("LIB", dev_home + "\\lib")
    atm.environ_s("PATH", dev_home + "\\bin")
    atm.environ_s("PATH", dev_home + "\\util")
