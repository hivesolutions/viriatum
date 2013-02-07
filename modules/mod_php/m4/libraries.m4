# Hive Viriatum Modules
# Copyright (C) 2008-2012 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Modules.
#
# Hive Viriatum Modules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hive Viriatum Modules is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

# sets the default values for the various library
# control variables
have_m=true
have_resolv=true
have_php=true

# checks for libraries
AC_CHECK_LIB([viriatum], [main])
AC_CHECK_LIB([gmp], [main])
AC_CHECK_LIB([xml], [main])
AC_CHECK_LIB([m], [main], [], [AC_MSG_ERROR([m library is required])])
AC_CHECK_LIB([resolv], [main], [], [AC_MSG_ERROR([resolv library is required])])
AC_CHECK_LIB([php5], [main], [], [AC_MSG_ERROR([php 5 library is required])])

# library variables activation
AM_CONDITIONAL(LINK_M, [test "$have_m" != "false"])
AM_CONDITIONAL(LINK_RESOLV, [test "$have_resolv" != "false"])
AM_CONDITIONAL(LINK_PHP, [test "$have_php" != "false"])
