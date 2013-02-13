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
import json
import stat
import errno
import shutil

config = {}
""" The top level global configuration to
be used across all the definitions and
operations """

def build(config_path):
    # in case the provided config path is not valid raises an
    # error indicating the problem
    if not config_path:
        raise RuntimeError("Invalid configuration path '%s'" % config_path)

    # opens the configuration file for reading and
    # reads the complete set of contents creating the
    # configuration definition map
    file = open(config_path, "rb")
    try: _config = json.load(file)
    except: file.close()

    # @TODO: this is completely hardcoded
    name = _config.get("name", "default")
    version = _config.get("version", "0.0.0")
    architecture = "win32"  #@TODO: this is hardcoded
    name_arc = name + "-" + version + "-" + architecture
    name_raw = name_arc + "-raw"
    name_src = name_arc + "-src"

    cwd = os.getcwd()
    build_f = os.path.join(cwd, "build")
    repo_f = os.path.join(build_f, "repo")
    target_f = os.path.join(build_f, "target")
    tmp_f = os.path.join(target_f, "tmp")
    result_f = os.path.join(target_f, "result")
    dist_f = os.path.join(target_f, "dist")

    paths = {
        "build" : build_f,
        "repo" : repo_f,
        "target" : target_f,
        "tmp" : tmp_f,
        "result" : result_f,
        "dist" : dist_f
    }

    # sets the various "new" elements of the configuration
    # to be sets for the current environment
    _config["paths"] = paths
    _config["architecture"] = architecture
    _config["name_arc"] = name_arc
    _config["name_raw"] = name_raw
    _config["name_src"] = name_src

    # loads the current configuration with the provided
    # configuration map so that they are stored in the
    # corresponding global structure
    load(_config)

    # creates the various paths associated with the
    # currently loaded configuration (flush operation)
    create_paths()

def cleanup():
    tmp_f = path("tmp")
    exists = tmp_f and os.path.exists(tmp_f) or False
    if not exists: return
    remove(tmp_f)

def load(_config):
    global config
    for key, value in _config.items():
        config[key] = value

def create_paths():
    paths = config.get("paths", {})
    for _name, path in paths.items():
        if os.path.isdir(path): continue
        os.makedirs(path)

def move(src, dst):
    shutil.move(src, dst)

def copy(src, dst, create = True):
    is_dir = os.path.isdir(src)
    if not os.path.exists(dst) and create and not is_dir: os.makedirs(dst)
    if is_dir: shutil.copytree(src, dst)
    else: shutil.copy(src, dst)

def remove(path):
    exists = os.path.exists(path)
    is_dir = os.path.isdir(path)
    if not exists: return
    if is_dir: shutil.rmtree(path, ignore_errors = False, onerror = _remove_error)
    else: os.remove(path)

def conf(name, default = None):
    return config.get(name, default)

def conf_s(name, value):
    config[name] = value

def path(name, default = None):
    paths = config.get("paths", {})
    return paths.get(name, default)

def _remove_error(func, path, exc):
    excvalue = exc[1]
    if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
        func(path)
    else:
        raise
