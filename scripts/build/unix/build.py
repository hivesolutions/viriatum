#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

import atm

INCLUDES = (
    "/usr/local/include/php",
    "/usr/local/include/php/main",
    "/usr/local/include/php/TSRM",
    "/usr/local/include/php/Zend",
    "/usr/include/python2.7"
)
""" The list of extra include directories
for the build process """

def run(build_m = True):
    # tries to retrieve the configuration file path
    # from the provided arguments
    if len(sys.argv) > 1: _file = sys.argv[1]
    else: _file = None

    # starts the build process with the configuration file
    # that was provided to the configuration script
    atm.build(_file)

    # retrieves the various values from the global configuration
    # that are going to be used around the configuration
    arch = atm.conf("arch")
    name_arc = atm.conf("name_arc")
    name_raw = atm.conf("name_raw")
    name_src = atm.conf("name_src")

    # creates the various paths to the folders to be used
    # for the build operation, from the ones already loaded
    repo_f = atm.path("repo")
    result_f = atm.path("result")
    tmp_f = atm.path("tmp")
    dist_f = atm.path("dist")
    target_f = atm.path("target")
    modules_f = os.path.join(repo_f, "modules")
    deb_base_f = os.path.join(target_f, "deb")
    deb_f = os.path.join(deb_base_f, name_arc)
    script_f = os.path.dirname(__file__)
    script_f = os.path.abspath(script_f)

    # clones the current repository using the git command this
    # should retrieve all the source data from the server
    atm.git(clean = True)

    # lists the modules directory so that all the modules are
    # discovered (module folder names) this will be used to
    # build the various modules (iteration trigger)
    modules = os.listdir(modules_f)

    # changes the current directory into the repository one and
    # runs the auto generation process for the creation of the
    # configuration files
    os.chdir(repo_f)
    atm.autogen(clean = True)

    # iterates over all the modules to prepare their source code
    # for compilation distribution
    for module in modules:
        module_f = os.path.join(modules_f, module)
        os.chdir(module_f)
        atm.autogen(clean = True)

    # changes the current directory to the repository one and
    # copies the contents of it into the temporary folder named
    # after the project name, then runs the configuration program
    # and the build process (compilation of the project)
    os.chdir(repo_f)
    atm.copy(repo_f, os.path.join(tmp_f, name_src))
    atm.configure(
        args = (
            "--prefix=" + result_f,
            "--with-wwwroot=" + result_f + "/var/viriatum/www",
            "--enable-defaults"
        )
    )
    atm.make()

    # iterates over each of the modules to run the build process
    # operations for each of them
    for module in modules:
        module_f = os.path.join(modules_f, module)
        os.chdir(module_f)
        atm.configure(
            args = (
                "--prefix=" + result_f
            ),
            includes = INCLUDES
        )
        atm.make()

    # copies the various build resulting files into the appropriate
    # deb associated directories and the resulting binaries into the
    # temporary folder associated with the project
    atm.copy(result_f + "/bin/viriatum", deb_f + "/usr/sbin")
    atm.copy(result_f + "/etc/viriatum/viriatum.ini", deb_f + "/etc/viriatum")
    atm.copy(result_f + "/etc/init.d/viriatum", deb_f + "/etc/init.d")
    atm.copy(result_f + "/var/viriatum/www", deb_f + "/var/viriatum")
    atm.copy(result_f + "/lib", deb_f + "/usr")
    atm.copy(script_f + "/meta/all", deb_f + "/DEBIAN")
    atm.copy(script_f + "/meta/" + arch, deb_f + "/DEBIAN")
    atm.copy(result_f, tmp_f + "/" + name_arc)

    # changes the current directory to the deb folder and starts
    # the process of packing the deb file from the files in the
    # folder and then moves the resulting deb file to the distribution
    # based directory
    os.chdir(deb_f)
    atm.deb()
    atm.move(os.path.join(deb_base_f, name_arc + ".deb"), dist_f)

    # changes the current directory to the resulting folder and
    # creates the tar file for it moving it then to the distribution
    # based folder (final place)
    os.chdir(result_f)
    atm.tar(name_raw + ".tar")
    atm.move(name_raw + ".tar", dist_f)

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
