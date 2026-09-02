# Hive Viriatum Modules
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Modules.
#
# Hive Viriatum Modules is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Modules is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# sets the default values for the various library
# control variables
have_python=true

# checks for libraries
AC_CHECK_LIB([viriatum], [main], [], [AC_MSG_ERROR([viriatum library is required])])

# detects the Python 3 library using python3-config, falls back to
# common versioned names (python3.x) and finally python2.7 for legacy
AC_MSG_CHECKING([for python library])
PYTHON_LIB=""
if test -z "$PYTHON_LIB"; then
    # try python3-config --ldflags first (most reliable)
    PYTHON3_CONFIG=$(which python3-config 2>/dev/null)
    if test -n "$PYTHON3_CONFIG"; then
        PYTHON_VERSION=$($PYTHON3_CONFIG --embed --ldflags 2>/dev/null | grep -o '\-lpython[[0-9.]]*' | head -1 | sed 's/-l//')
        if test -n "$PYTHON_VERSION"; then
            PYTHON_LIB="$PYTHON_VERSION"
        fi
    fi
fi
if test -z "$PYTHON_LIB"; then
    # try common Python 3 versioned library names
    for pylib in python3.14 python3.13 python3.12 python3.11 python3.10 python3; do
        AC_CHECK_LIB([$pylib], [Py_Initialize], [PYTHON_LIB="$pylib"; break], [], [])
    done
fi
if test -n "$PYTHON_LIB"; then
    AC_MSG_RESULT([$PYTHON_LIB])
    LIBS="-l$PYTHON_LIB $LIBS"
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([python 3 library is required (install python3-dev)])
fi

# library variables activation
AM_CONDITIONAL(LINK_PYTHON, [test "$have_python" != "false"])
