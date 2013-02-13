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

    modules = os.listdir(modules_f)

    os.chdir(repo_f)
    atm.autogen(clean = True)

    # iterates over all the modules to prepare their source code
    # for compilation distribution
    for module in modules:
        # removes the extra (non source files) from the source
        # distribution directory (for the module)
        module_f = os.path.join(modules_f, module)
        os.chdir(module_f)
        atm.autogen(clean = True)

    os.chdir(repo_f)
    atm.copy(repo_f, os.path.join(tmp_f, name_src))
    atm.configure(
        args = (
            "--prefix=" + result_f,
            "--with-configroot=/etc/viriatum",
            "--with-moduleroot=/usr/lib/viriatum/modules",
            "--with-wwwroot=/var/viriatum/www",
            "--enable-defaults"
        )
    )
    atm.make()

    atm.copy(result_f + "/bin/viriatum", deb_f + "/usr/sbin")
    atm.copy(result_f + "/etc/viriatum/viriatum.ini", deb_f + "/etc/viriatum")
    atm.copy(result_f + "/etc/init.d/viriatum", deb_f + "/etc/init.d")
    atm.copy(result_f + "/var/viriatum/www", deb_f + "/var/viriatum")
    atm.copy(script_f + "/meta/all", deb_f + "/DEBIAN")
    atm.copy(script_f + "/meta/" + arch, deb_f + "/DEBIAN")
    atm.copy(result_f, tmp_f + "/" + name_arc)

    os.chdir(deb_f)
    atm.deb()
    atm.move(os.path.join(deb_base_f, name_arc + ".deb"), dist_f)

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
