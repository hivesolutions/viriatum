#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import shutil

def run():
    os.system("git clone git://github.com/hivesolutions/viriatum.git build")
    cwd = os.getcwd()
    os.chdir("build")
    try:
        os.system("git rev-parse HEAD > VERSION")
        version_file = open("VERSION")
        try: version = version_file.read().strip()
        finally: version_file.close()
    finally:
        os.chdir(cwd)

    if version == sys.argv[1]: sys.exit(1)
    else: sys.exit(0)  

if __name__ == "__main__":
    run()
