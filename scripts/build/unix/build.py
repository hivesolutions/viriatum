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

INCLUDES_CROSS = (
    "/opt/%s/include/php",
    "/opt/%s/include/php/main",
    "/opt/%s/include/php/TSRM",
    "/opt/%s/include/php/Zend",
    "/opt/%s/include/python2.7"
)
""" The list of extra include directories
for the build process, these values are just
templates that should be completed with the
cross compilation host value """

def build(file = None, build_m = True, cflags = None, cross = None):
    # runs the initial assertion for the various commands
    # that are mandatory for execution, this should avoid
    # errors in the middle of the build
    atm.assert_c(("git", "make", "dpkg-deb"))

    # starts the build process with the configuration file
    # that was provided to the configuration script
    atm.build(file, cross = cross)

    # retrieves the various values from the global configuration
    # that are going to be used around the configuration
    name_arc = atm.conf("name_arc")
    name_raw = atm.conf("name_raw")
    name_src = atm.conf("name_src")
    name_deb = name_arc.replace("-", "_")

    # creates the various paths to the folders to be used
    # for the build operation, from the ones already loaded
    repo_f = atm.path("repo")
    result_f = atm.path("result")
    tmp_f = atm.path("tmp")
    dist_f = atm.path("dist")
    target_f = atm.path("target")
    modules_f = os.path.join(repo_f, "modules")
    deb_base_f = os.path.join(target_f, "deb")
    deb_f = os.path.join(deb_base_f, name_deb)
    script_f = os.path.dirname(__file__)
    script_f = os.path.abspath(script_f)

    # clones the current repository using the git command this
    # should retrieve all the source data from the server
    atm.git(clean = True)

    # lists the modules directory so that all the modules are
    # discovered (module folder names) this will be used to
    # build the various modules (iteration trigger)
    modules = build_m and os.listdir(modules_f) or []

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
        ),
        cflags = cflags,
        cross = cross
    )
    atm.make()

    # iterates over each of the modules to run the build process
    # operations for each of them
    for module in modules:
        module_f = os.path.join(modules_f, module)
        os.chdir(module_f)
        atm.configure(
            args = (
                "--prefix=" + result_f,
            ),
            includes = cross and INCLUDES_CROSS or INCLUDES,
            libraries = (result_f + "/lib",),
            cflags = cflags,
            cross = cross
        )
        atm.make()

    # copies the various build resulting files into the appropriate
    # deb associated directories and the resulting binaries into the
    # temporary folder associated with the project
    atm.copy(result_f + "/lib", deb_f + "/usr/lib")
    atm.copy(result_f + "/include", deb_f + "/usr/include")
    atm.copy(result_f + "/bin/viriatum", deb_f + "/usr/sbin")
    atm.copy(result_f + "/etc/viriatum", deb_f + "/etc/viriatum")
    atm.copy(result_f + "/etc/init.d/viriatum", deb_f + "/etc/init.d")
    atm.copy(result_f + "/var/viriatum/www", deb_f + "/var/viriatum/www")
    atm.copy(script_f + "/meta", deb_f + "/DEBIAN")
    atm.copy(result_f, tmp_f + "/" + name_arc)

    # changes the current directory to the deb folder and starts
    # the process of packing the deb file from the files in the
    # folder and then moves the resulting deb file to the distribution
    # based directory
    os.chdir(deb_f)
    atm.deb(
        section = "httpd",
        depends = "libc6",
        size = "1024"
    )
    atm.move(os.path.join(deb_base_f, name_deb + ".deb"), dist_f)

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

def run():
    # parses the various arguments provided by the
    # command line and retrieves it defaulting to
    # pre-defined values in case they do not exist
    arguments = atm.parse_args(names = ("no-modules", "cflags=", "cross="))
    file = arguments.get("file", None)
    build_m = not arguments.get("no-modules", False)
    cflags = arguments.get("cflags", None)
    cross = arguments.get("cross", None)

    # starts the build process with the parameters
    # retrieved from the current environment
    build(
        file = file,
        build_m = build_m,
        cflags = cflags,
        cross = cross
    )

def cleanup():
    atm.cleanup()

if __name__ == "__main__":
    try: run()
    finally: cleanup()
