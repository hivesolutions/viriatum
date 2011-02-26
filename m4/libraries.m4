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

# checks for libraries
AC_CHECK_LIB([dl], [main], [], [AC_MSG_ERROR(can't build without dynamic linking (libdl) libraries)])
AC_CHECK_LIB([pthread], [main], [], [AC_MSG_ERROR(can't build without posix threads (libpthread) libraries)])
AC_CHECK_LIB([ws2_32], [main], [], [])
AC_CHECK_LIB([lua5.1], [main], [have_lua=yes], [])
AC_CHECK_LIB([viriatum], [main], [have_viriatum=yes], [])

# library variables activation
AM_CONDITIONAL(LINK_LUA, [test "$have_lua" = "yes"])

if test "$have_lua" = no; then
    AC_MSG_WARN(building without Lua (liblua) libraries support)
fi
