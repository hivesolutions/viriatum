#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

import atm

def run():
    # parses the various arguments provided by the
    # command line and retrieves it defaulting to
    # pre-defined values in case they do not exist
    arguments = atm.parse_args()
    _file = arguments.get("file", None)
    _version = arguments.get("previous", None)

    try:
        # starts the build process with the configuration file
        # that was provided to the configuration script and the
        # runs the validation process for the git repository, this
        # operation should also generate the log and version files
        atm.build(_file)
        version = atm.git_v(_version)
    except:
        sys.exit(0)

    # prints a log message about the two versions that are going
    # to be compared to verify if there's a change
    print("Comparing '%s' against '%s' ..." % (version, _version))

    # verifies if the version is the same as the one defined
    # as the base for comparison in case it's returns in error
    # otherwise returns the resulting (and value) of the various
    # calls (should be success by default)
    if version == _version: sys.exit(1)
    else: sys.exit(0)

if __name__ == "__main__":
    run()
