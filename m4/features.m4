# Hive Viriatum Web Server
# Copyright (c) 2008-2020 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# sets the target prefix (assumes the normal heuristic)
# must do this because the makefiles have not been generated
# so the prefix value is not final
target_prefix=$prefix
test "x$prefix" = xNONE && target_prefix=$ac_default_prefix

# runs the evaluation process in the system configuration
# directory so that it's possible to properly retrieve it
eval "prefix=$target_prefix && eval target_sysconfdir=$sysconfdir"

# sets the default path to the viriatum paths
with_resourceroot=/var/viriatum
with_libroot=$target_prefix/lib/viriatum
with_configroot=$target_sysconfdir/viriatum

# sets the default values for the various feature
# control variables
have_debug=false
have_defaults=false
have_mpool=false
have_prefork=false

AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [enable debug features])], [have_debug=true], [])
AC_ARG_ENABLE([defaults], [AS_HELP_STRING([--enable-defaults], [enable default path features])], [have_defaults=true], [])
AC_ARG_ENABLE([mpool], [AS_HELP_STRING([--enable-mpool], [enable memory pool])], [have_mpool=true], [])
AC_ARG_ENABLE([prefork], [AS_HELP_STRING([--enable-prefork], [enable prefork support])], [have_prefork=true], [])
AC_ARG_WITH([moduleroot], [AS_HELP_STRING([--with-moduleroot], [set the default modules directory])], [], [with_moduleroot=$with_libroot/modules])
AC_ARG_WITH([wwwroot], [AS_HELP_STRING([--with-wwwroot], [set the default data directory])], [], [with_wwwroot=$with_resourceroot/www])

AC_SUBST(with_configroot, $with_configroot)
AC_SUBST(modulerootdir, $with_moduleroot)
AC_SUBST(wwwrootdir, $with_wwwroot)

if test "$have_debug" = true; then
    AC_DEFINE(HAVE_DEBUG, 1, [Define to 1 if debug is enabled])
fi

if test "$have_defaults" = true; then
    AC_DEFINE(HAVE_DEFAULTS, 1, [Define to 1 if defaults is enabled])
fi

if test "$have_mpool" = true; then
    AC_DEFINE(HAVE_MPOOL, 1, [Define to 1 if memory pool is enabled])
fi

if test "$have_prefork" = true; then
    AC_DEFINE(HAVE_PREFORK, 1, [Define to 1 if prefork is enabled])
fi

AC_DEFINE_UNQUOTED(WITH_PREFIX, "$target_prefix", [Define to a value if prefix is set])
AC_DEFINE_UNQUOTED(WITH_CONFIG_ROOT, "$with_configroot", [Define to a value if config root is set])
AC_DEFINE_UNQUOTED(WITH_MODULE_ROOT, "$with_moduleroot", [Define to a value if module root is set])
AC_DEFINE_UNQUOTED(WITH_WWW_ROOT, "$with_wwwroot", [Define to a value if www root is set])

AC_DEFINE_UNQUOTED(HOST_OS, "$host_os", [Define to the value that describes the host os])
AC_DEFINE_UNQUOTED(CFLAGS, "$CFLAGS", [Define to the value of the compilation flags])
