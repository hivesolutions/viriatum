#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

import atm

DEV_HOME = atm.environ("DEV_HOME", "\\dev")
""" The development directory to be used
as the root for the includes, libraries and
binary utilities """

INCLUDES = (
    DEV_HOME + "\include\php",
    DEV_HOME + "\include\php\main",
    DEV_HOME + "\include\php\TSRM",
    DEV_HOME + "\include\php\Zend"
)
""" The list of extra include directories
for the build process """

def run(build_m = True, mode = "Release"):
    # tries to retrieve the configuration file path
    # from the provided arguments
    if len(sys.argv) > 1: _file = sys.argv[1]
    else: _file = None

    # starts the build process with the configuration file
    # that was provided to the configuration script
    atm.build(_file, arch = "win32")

    # creates the various paths to the folders to be used
    # for the build operation, from the ones already loaded
    repo_f = atm.path("repo")
    result_f = atm.path("result")
    tmp_f = atm.path("tmp")
    dist_f = atm.path("dist")
    build_f = atm.path("build")
    base_f = repo_f
    bin_f = os.path.join(base_f, "bin/hive_viriatum/i386/win32/%s" % mode)
    solution_f = os.path.join(base_f, "win32/vs2008ex")
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

    # lists the modules directory so that all the modules are
    # discovered (module folder names) this will be used to
    # build the various modules (iteration trigger)
    modules = build_m and os.listdir(modules_f) or []
    
    # constructs the path to the solution file and uses it for
    # the msbuild command to build the project
    sln_path = os.path.join(solution_f, "hive_viriatum.sln")
    atm.msbuild(sln_path)

    # changes to the binary directory and copies the built files
    # to the result directory
    os.chdir(bin_f)
    atm.copy("viriatum.exe", result_f)
    atm.copy("config", os.path.join(result_f, "config"))
    atm.copy("htdocs", os.path.join(result_f, "htdocs"))

    # constructs the path to the solution file and uses it for
    # the msbuild command to build the project
    mod_sln_path = os.path.join(solution_f, "hive_viriatum_mod.sln")
    build_m and atm.msbuild(mod_sln_path, includes = INCLUDES)

    # iterates over all the modules to copy their resulting files
    # into the apropriate modules directory
    for module in modules:
        module_bin_f = os.path.join(
            base_f,
            "bin/hive_viriatum_%s/i386/win32/%s" % (module, mode)
        )
        os.chdir(module_bin_f)
        atm.copy("viriatum_%s.dll" % module, os.path.join(result_f, "modules"))
    
    # copies the resulting files to the temporary directory with
    # the name of the build for later compression
    atm.copy(result_f, os.path.join(tmp_f, name_arc))
    
    # changes the current directory to the result directory and
    # creates a tar based file with the binary contents
    os.chdir(result_f)
    atm.tar(name_raw + ".tar", ("viriatum.exe", "config", "htdocs", "modules"))
    atm.move(name_raw + ".tar", dist_f)

    # changes to build directory and creates the capsule file for the
    # current configuration, the metada values will be used from the
    # context that is currently defined
    os.chdir(build_f)
    atm.capsule(
        os.path.join(dist_f, name_arc + ".exe"),
        os.path.join(dist_f, name_raw + ".tar")
    )

    # creates the various compressed files for both the archive and
    # source directories (distribution files)
    os.chdir(tmp_f)
    atm.compress(name_arc, target = dist_f)
    atm.compress(name_src, target = dist_f)

    # creates the various hash files for the complete set of files in
    # the distribution directory
    os.chdir(dist_f)
    atm.hash_d()

def cleanup():
    atm.cleanup()

if __name__ == "__main__":
    try: run()
    finally: cleanup()
