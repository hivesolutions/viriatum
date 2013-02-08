#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

def run():
    # in case a version value was sent to the program
    # must retrieve it to be validated, otherwise the
    # version is set as invalid (unset)
    if len(sys.argv) > 1: _version = sys.argv[1]
    else: _version = None

    # in case there's no version provided the build
    # is considered invalid immediately (if not version
    # is provided there's nothing to be verified)
    if _version == None: exit(0)

    # starts the return value variable to zero so that if
    # no error occurs over all commands no error is returned
    # to the caller program
    return_v = 0

    # clones the repository to the target (verify) directory
    # to be used for the processing of the log
    return_v += os.system("git clone git://github.com/hivesolutions/viriatum.git verify")
    cwd = os.getcwd()
    os.chdir("verify")
    try:
        # puts the current revision in the head branch into
        # the version file and then opens the file to be able
        # to prettify it into the normalized version value
        return_v += os.system("git rev-parse HEAD > VERSION")
        version_file = open("VERSION")
        try: version = version_file.read().strip()
        finally: version_file.close()
        if _version: return_v += os.system("git log " + _version + "..HEAD --pretty=format:\"%h%x09%an%x09%ad%x09%s\" > LOG")
        else: return_v += os.system("git log --pretty=format:\"%h%x09%an%x09%ad%x09%s\" > LOG")
    finally:
        # changes the directory back to the original value in
        # order to restore the settings to the original value
        os.chdir(cwd)

    # verifies if the version is the same as the one defined
    # as the base for comparison in case it's returns in error
    # otherwise returns the resulting (and value) of the various
    # calls (should be success by default)
    if version == _version: sys.exit(1)
    else: sys.exit(return_v)

if __name__ == "__main__":
    run()
