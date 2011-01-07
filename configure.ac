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

# inclides the settings
m4_include([m4/settings.m4])

# requires autoconf
AC_PREREQ(2.61)

# defines the general values
AC_INIT(hive_viriatum, viriatum_version, development@hive.pt)
AM_INIT_AUTOMAKE(hive_viriatum, viriatum_version)
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

# requires the libtool
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

m4_include([m4/initialization.m4])
m4_include([m4/exports.m4])
m4_include([m4/programs.m4])
m4_include([m4/characteristics.m4])
m4_include([m4/libraries.m4])
m4_include([m4/headers.m4])
m4_include([m4/functions.m4])
m4_include([m4/variables.m4])
m4_include([m4/poll.m4])
m4_include([m4/epoll.m4])
m4_include([m4/poll_methods.m4])
m4_include([m4/have_validation.m4])
m4_include([m4/output.m4])
m4_include([m4/message.m4])
