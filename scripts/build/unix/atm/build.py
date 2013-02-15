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

import copy as _copy

import atm

DEV_HOME = "\\dev"
""" The default directory to the development directory
to be used in the building stages """

def autogen(path = None, clean = False):
    path = path or "./autogen.sh"
    result = subprocess.call([
        path
    ])
    if not result == 0: raise RuntimeError("Problem executing autogen not successful")

    if not clean: return

    base_path = os.path.dirname(path)
    cache = os.path.join(base_path, "autom4te.cache")
    autogen = os.path.join(base_path, "autogen.sh")
    makefile = os.path.join(base_path, "Makefile-autoconfig")
    atm.remove(cache)
    atm.remove(autogen)
    atm.remove(makefile)

def configure(path = None, args = (), includes = ()):
    # creates the pre-defined path for the configuration
    # file to be used in case it was not provided
    path = path or "./configure"

    # copies the current set of environment variables and
    # creates the includes string from the various provided
    # include paths and sets the cflags variable with it
    env = _copy.copy(os.environ)
    includes_s = ""
    for include in includes: includes_s += "-I" + include + " "
    env["CFLAGS"] = includes_s

    # runs the configuration process with the newly set environment
    # variables and in case the execution fails raises an exception
    result = subprocess.call([
        path
    ] + list(args), env = env)
    if not result == 0: raise RuntimeError("Problem executing configure not successful")

def make(install = True):
    result = subprocess.call([
        "make"
    ])
    if not result == 0: raise RuntimeError("Problem executing make not successful")

    if not install: return

    result = subprocess.call([
        "make",
        "install"
    ])
    if not result == 0: raise RuntimeError("Problem executing make install not successful")

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
