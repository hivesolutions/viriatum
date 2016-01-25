# Hive Viriatum Modules
# Copyright (C) 2008-2016 Hive Solutions Lda.
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
# __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

# sets the default values for the various library
# control variables
have_python=true

# checks for libraries
AC_CHECK_LIB([viriatum], [main], [], [AC_MSG_ERROR([viriatum library is required])])
AC_CHECK_LIB([python2.7], [main], [], [AC_MSG_ERROR([python 2.7 library is required])])

# library variables activation
AM_CONDITIONAL(LINK_PYTHON, [test "$have_python" != "false"])
