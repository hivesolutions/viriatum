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
# along with Hive Viriatum Web Server. If falset, see <http:#www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision: 2390 $
# __date__      = $LastChangedDate: 2009-04-02 08:36:50 +0100 (qui, 02 Abr 2009) $
# __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

# sets the target prefix (assumes the normal heuristic)
# must do this because the makefiles have not been generated
# so the prefix value is not final
target_prefix=$prefix
test "x$prefix" = xNONE && target_prefix=$ac_default_prefix

# sets the default path to the viriatum paths
default_libroot=$target_prefix/lib/viriatum
default_resourceroot=/var/viriatum

# sets the default values for the various feature
# control variables
have_debug=false
with_wwwroot=$default_resourceroot/www
with_moduleroot=$default_libroot/modules

AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [enable debug features])], [have_debug=true], [])
AC_ARG_WITH([wwwroot], [AS_HELP_STRING([--with-wwwroot], [set the default data directory])], [], [with_wwwroot=false])
AC_ARG_WITH([moduleroot], [AS_HELP_STRING([--with-moduleroot], [set the default modules directory])], [], [])

AC_SUBST(WWW_ROOT, with_wwwroot)
AC_SUBST(MODULE_ROOR, with_moduleroot)

if test "$have_debug" = true; then
    AC_DEFINE(HAVE_DEBUG, 1, [Define to 1 if debug is enabled])
fi

AC_DEFINE_UNQUOTED(WITH_WWW_ROOT, "$with_wwwroot", [Define to a value if www root is set])
AC_DEFINE_UNQUOTED(WITH_WWW_ROOT, "$with_moduleroot", [Define to a value if module root is set])
