# Hive Viriatum Web Server
# Copyright (C) 2008-2015 Hive Solutions Lda.
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
# along with Hive Viriatum Web Server. If falset, see <http://www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

# checks for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# sets the silent rules for compilation, in case such
# macro is currently defined in autoconf
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# verifies if the current compiler to be used is clang
# if that's the case exposed it as an automake variable
AM_CONDITIONAL(CLANG, [ test "$CC" = *"clang"* ])
