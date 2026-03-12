# Hive Viriatum Modules
# Copyright (c) 2008-2026 Hive Solutions Lda.
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

# __author__    = João Magalhães <joamag@hive.pt>
# __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# checks for header files
AC_CHECK_HEADERS([stdlib.h assert.h])

# checks for lua header using a compile test that mirrors
# the actual build flags (headers are at lua5.1/lua.h)
AC_MSG_CHECKING([for lua5.1/lua.h])
AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([[#include <lua5.1/lua.h>]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([lua headers are required (install lua-dev or lua5.1-dev)])])
