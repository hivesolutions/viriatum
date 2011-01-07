# Hive Viriatum Web Server
# Copyright (C) 2008 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hive Viriatum Web Server. If not, see <http:#www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision: 2390 $
# __date__      = $LastChangedDate: 2009-04-02 08:36:50 +0100 (qui, 02 Abr 2009) $
# __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

AC_PREREQ(2.61)
AC_INIT(hive_viriatum, 1.0, development@hive.pt)
AM_INIT_AUTOMAKE(hive_viriatum, 1.0)
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# checks for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_C99
AC_PROG_MAKE_SET

have_python="yes"

# checks for programs
AC_PATH_PROGS(PYTHON, python python2.7 python2.6 python2.5 python2.4)

# checks for libraries
AC_CHECK_LIB([pthread], [main], [], [AC_MSG_ERROR(can't build without POSIX threads (libpthread) libraries)])
AC_CHECK_LIB([viriatum], [main])

# checks for header files
AC_CHECK_HEADERS([stdlib.h])

# checks for the epoll
AC_CHECK_HEADER(sys/epoll.h, have_epoll_include=yes, have_epoll_include=no)

# checks if the user wants epoll
AC_ARG_ENABLE(epoll, AC_HELP_STRING([--disable-epoll], [Disable epoll() support]), wants_epoll="$enableval", wants_epoll="yes")

# in case the user wants epoll and the system has epoll
if test "x$have_epoll_include" = "xyes" && test "x$wants_epoll" = "xyes"; then
	AC_MSG_CHECKING(for epoll system call)
	AC_RUN_IFELSE([], have_epoll=yes, have_epoll=no, have_epoll=yes)
    AC_MSG_RESULT($have_epoll)
fi

# checks for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset])

# sets the config files to be used in automake
AC_CONFIG_FILES([src/hive_viriatum_commons/Makefile src/hive_viriatum/Makefile])

# sets the entry point make files to be used by automake
AC_OUTPUT([Makefile src/Makefile lib/Makefile doc/Makefile man/Makefile examples/Makefile scripts/Makefile])

echo
echo ============================
echo "Host                  $host_os"
echo "Install prefix        $prefix"
echo "CFLAGS                $CFLAGS"
echo "Python                $have_python"

echo
cat <<EOF
+------------------------------------------------------------------+
| License:                                                         |
| This software is subject to the GPL License, available in this   |
| distribution in the file COPYING. Please, remember that any copy |
| distribution or modification of the software is subject to it.   |
+------------------------------------------------------------------+

Thank you for using Viriatum.
EOF
