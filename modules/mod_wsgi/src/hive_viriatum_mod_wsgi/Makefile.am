# Hive Viriatum Commons
# Copyright (C) 2008 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Commons.
#
# Hive Viriatum Commons is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hive Viriatum Commons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision: 2390 $
# __date__      = $LastChangedDate: 2009-04-02 08:36:50 +0100 (qui, 02 Abr 2009) $
# __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

include $(top_srcdir)/Common.am

lib_LIBRARIES = libviriatum_mod_php.a

lib_LTLIBRARIES = libviriatum_mod_php.la

libviriatum_mod_php_a_CFLAGS = $(AM_CFLAGS)
libviriatum_mod_php_a_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_mod_php_la_CFLAGS = $(AM_CFLAGS)
libviriatum_mod_php_la_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_mod_php_la_LDFLAGS = -version-info $(MOD_PHP_MAJOR_VERSION):$(MOD_PHP_MINOR_VERSION):$(MOD_PHP_MICRO_VERSION)

libviriatum_mod_php_a_SOURCES = stdafx.c \
module/entry.c \
module/extension.c \
module/handler.c

libviriatum_mod_php_la_SOURCES = stdafx.c \
module/entry.c \
module/extension.c \
module/handler.c
