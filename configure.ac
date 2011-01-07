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

m4_include([m4/libraries.m4])
m4_include([m4/headers.m4])
m4_include([m4/variables.m4])
m4_include([m4/poll.m4])
m4_include([m4/epoll.m4])

# checks for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset malloc])

# sets the entry point make files to be used by automake
AC_OUTPUT([Makefile src/Makefile src/hive_viriatum_commons/Makefile src/hive_viriatum/Makefile lib/Makefile m4/Makefile doc/Makefile man/Makefile examples/Makefile scripts/Makefile])


if test "$have_epoll" = yes; then
    AC_DEFINE(HAVE_EPOLL, 1, [Define to 1 if you have the epoll])
fi

AM_CONDITIONAL(COMPILE_EPOLL, test x"$have_epoll" = "xyes")


m4_include([m4/poll_methods.m4])
m4_include([m4/message.m4])
