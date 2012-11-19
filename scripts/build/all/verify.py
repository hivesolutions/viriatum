#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import shutil

def run():
    # in case a version value was sent to the program
    # must retrieve it to be validated, otherwise the
    # version is set as invalid (unset)
    if len(sys.argv) > 1: _version = sys.argv[1]
    else: _version = None

    os.system("git clone git://github.com/hivesolutions/viriatum.git verify")
    cwd = os.getcwd()
    os.chdir("verify")
    try:
        os.system("git rev-parse HEAD > VERSION")
        version_file = open("VERSION")
        try: version = version_file.read().strip()
        finally: version_file.close()
        if _version: os.system("git log " + _version + "..HEAD --pretty=format:\"%h%x09%an%x09%ad%x09%s\" > LOG")
        else: os.system("git log --pretty=format:\"%h%x09%an%x09%ad%x09%s\" > LOG")
    finally:
        os.chdir(cwd)

    if version == _version: sys.exit(1)
    else: sys.exit(0)  

if __name__ == "__main__":
    run()
