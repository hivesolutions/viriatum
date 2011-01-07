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

# requires autoconf
AC_PREREQ(2.61)

# defines the version values
m4_define([viriatum_major_version], [1])
m4_define([viriatum_minor_version], [0])
m4_define([viriatum_micro_version], [0])
m4_define([viriatum_version], m4_format('%s.%s.%s', viriatum_major_version, viriatum_minor_version, viriatum_micro_version))

# defines the general values
AC_INIT(hive_viriatum, viriatum_version, development@hive.pt)
AM_INIT_AUTOMAKE(hive_viriatum, viriatum_version)
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# exports the version definitions to automake
VIRIATUM_MAJOR_VERSION=1
VIRIATUM_MINOR_VERSION=0
VIRIATUM_MICRO_VERSION=0
AC_SUBST(VIRIATUM_MAJOR_VERSION)
AC_SUBST(VIRIATUM_MINOR_VERSION)
AC_SUBST(VIRIATUM_MICRO_VERSION)

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



# checks for epoll
AC_CHECK_FUNC(poll, have_poll=yes)
AC_CHECK_FUNC(kqueue, have_kqueue=yes)
AC_CHECK_FUNC(select, have_select=yes)



# checks for the epoll
AC_CHECK_HEADER(sys/epoll.h, have_epoll_include=yes, have_epoll_include=no)

# checks if the user wants epoll
AC_ARG_ENABLE(epoll, AC_HELP_STRING([--disable-epoll], [Disable epoll() support]), wants_epoll="$enableval", wants_epoll="yes")

# in case the user wants epoll and the system has epoll
if test "x$have_epoll_include" = "xyes" && test "x$wants_epoll" = "xyes"; then
	AC_MSG_CHECKING(for epoll system call)
	AC_RUN_IFELSE([
                   #include <stdint.h>
                   #include <sys/param.h>
                   #include <sys/types.h>
                   #include <sys/syscall.h>
                   #include <sys/epoll.h>
                   #include <unistd.h>

                   int epoll_create(int size) {
                      return (syscall(__NR_epoll_create, size));
                   }

                   int main (int argc, char **argv) {
                       int epfd;
                       epfd = epoll_create(256);
                       exit(epfd == -1 ? 1 : 0);
                   }
                  ], have_epoll=yes, have_epoll=no, have_epoll=yes)
    AC_MSG_RESULT($have_epoll)
fi

# checks for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset])

# sets the entry point make files to be used by automake
AC_OUTPUT([Makefile src/Makefile src/hive_viriatum_commons/Makefile src/hive_viriatum/Makefile lib/Makefile doc/Makefile man/Makefile examples/Makefile scripts/Makefile])




methods=""
if test "$have_epoll" = yes; then methods="${methods}epoll "; fi
if test "$have_kqueue" = yes; then methods="${methods}kqueue "; fi
if test "$have_poll" = yes; then methods="${methods}poll "; fi
if test "$have_port" = yes; then methods="${methods}port "; fi
if test "$have_win32_select" = yes; then methods="${methods}win32 "; fi
if test "$have_select" = yes; then methods="${methods}select"; fi


if test "$have_epoll" = yes; then
    AC_DEFINE(HAVE_EPOLL, 1, [Define to 1 if you have the epoll])
fi

AM_CONDITIONAL(COMPILE_EPOLL, test x"$have_epoll" = "xyes")





echo
echo ============================
echo "Host                  $host_os"
echo "Install prefix        $prefix"
echo "CFLAGS                $CFLAGS"
echo "Python                $have_python"
index=1
for method in $methods; do
    echo "Polling method $index      $method"
    index=$((index+1))
done
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
