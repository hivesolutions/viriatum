#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Build verification that runs the build described by a configuration
file and then decides whether the version of the repository has moved
since the one it is handed, so that a build which produced nothing new
is told apart from one that did.

Answers through the status of the process rather than through what it
writes, a status of one saying the version is the one it already was
and a status of zero saying it moved or that the build never ran.

Run from the project root with:
    python scripts/build/all/verify.py --file=<configuration> --previous=<version>

Arguments:
    file        The configuration the build is described by
    previous    The version the one that comes out is compared against
"""

from sys import exit

import atm


def run() -> None:
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
        exit(0)

    # prints a log message about the two versions that are going
    # to be compared to verify if there's a change
    print("Comparing '%s' against '%s' ..." % (version, _version))

    # verifies if the version is the same as the one defined
    # as the base for comparison in case it's returns in error
    # otherwise returns the resulting (and value) of the various
    # calls (should be success by default)
    if version == _version:
        exit(1)
    else:
        exit(0)


if __name__ == "__main__":
    run()
