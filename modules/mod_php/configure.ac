# Hive Viriatum Modules
# Copyright (c) 2008-2020 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Modules.
#
# Hive Viriatum Modules is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Modules is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

# __author__    = Jo�o Magalh�es <joamag@hive.pt>
# __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# inclides the settings
m4_include([m4/settings.m4])

# requires autoconf
AC_PREREQ(2.61)

# defines the general values
AC_INIT(viriatum_mod_php, mod_php_version, development@hive.pt)
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

# requires the libtool
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

m4_include([m4/exports.m4])
m4_include([m4/programs.m4])
m4_include([m4/features.m4])
m4_include([m4/characteristics.m4])
m4_include([m4/libraries.m4])
m4_include([m4/headers.m4])
m4_include([m4/output.m4])
m4_include([m4/message.m4])
