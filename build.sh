#!/bin/bash
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2016 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

DARWIN=${DARWIN-0}
RUN_GEN=${RUN_CONFIGURE-1}
RUN_CONFIGURE=${RUN_CONFIGURE-0}
RUN_MAKE=${RUN_MAKE-0}
RUN_TEST=${RUN_TEST-0}

if [ "$RUN_GEN" == "1" ]; then
    if [ ! -e "configure" ]; then
        if [ "$DARWIN" == "1" ]; then
            make -f Makefile-gen mac-darwin
        else
            make -f Makefile-gen all
        fi
    fi
fi

if [ "$RUN_CONFIGURE" == "1" ]; then
    if [ ! -e "Makefile" ]; then
        ./configure
    fi
fi

if [ "$RUN_MAKE" == "1" ]; then
    make
fi

if [ "$RUN_TEST" == "1" ] || [ "$1" == "test" ]; then
    src/viriatum/viriatum --test
fi
