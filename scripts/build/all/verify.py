#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

import atm

def run():
    # in case a file path value was sent to the program
    # must retrieve it to be validated, otherwise the
    # file is set as invalid (unset)
    if len(sys.argv) > 1: _file = sys.argv[1]
    else: _file = None

    # in case a version value was sent to the program
    # must retrieve it to be validated, otherwise the
    # version is set as invalid (unset)
    if len(sys.argv) > 2: _version = sys.argv[2]
    else: _version = None

    try:
        # starts the build process with the configuration file
        # that was provided to the configuration script and the
        # runs the validation process for the git repository, this
        # operation should also generate the log and version files
        atm.build(_file)
        version = atm.git_v(_version)
    except:
        sys.exit(0)

    # verifies if the version is the same as the one defined
    # as the base for comparison in case it's returns in error
    # otherwise returns the resulting (and value) of the various
    # calls (should be success by default)
    if version == _version: sys.exit(1)
    else: sys.exit(0)

if __name__ == "__main__":
    run()
