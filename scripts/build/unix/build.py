#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

import atm

def run():
    # tries to retrieve the configuration file path
    # from the provided arguments
    if len(sys.argv) > 1: _file = sys.argv[1]
    else: _file = None

    # starts the build process with the configuration file
    # that was provided to the configuration script
    atm.build(_file)

    # creates the various paths to the folders to be used
    # for the build operation, from the ones already loaded
    repo_f = atm.path("repo")
    result_f = atm.path("result")
    tmp_f = atm.path("tmp")
    dist_f = atm.path("dist")
    build_f = atm.path("build")
    modules_f = os.path.join(repo_f, "modules")

    # retrieves the various values from the global configuration
    # that are going to be used around the configuration
    name = atm.conf("name")
    description = atm.conf("description")
    name_arc = atm.conf("name_arc")
    name_raw = atm.conf("name_raw")
    name_src = atm.conf("name_src")

    # clones the current repository using the git command and then
    # copies the resulting directory to the temporary directory
    atm.git(clean = True)
    atm.copy(repo_f, os.path.join(tmp_f, name_src))
    
    modules = os.listdir(modules_f)
    
    os.chdir(repo_f)
    atm.autogen(clean = True)
    
    # iterates over all the modules to prepare their source code
    # for compilation distribution
    for module in modules:
        # removes the extra (non source files) from the source
        # distribution directory (for the module)
        module_f = os.path.join(modules_f, module)
        os.chdir(repo_f)
        atm.autogen(clean = True)
        
    # creates the various compressed files for both the archive and
    # source directories (distribution files)
    #os.chdir(tmp_f)
    #atm.compress(name_arc, target = dist_f)
    #atm.compress(name_src, target = dist_f)
    
    # creates the various hash files for the complete set of files in
    # the distribution directory
    #os.chdir(dist_f)
    #atm.hash_d()

def cleanup():
    atm.cleanup()

if __name__ == "__main__":
    try: run()
    except: cleanup()
