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

# sets the default values for the various library
# control variables
have_lua=true

# checks for libraries
AC_CHECK_LIB([viriatum], [main], [], [AC_MSG_ERROR([viriatum library is required])])

# detects the Lua library, trying common naming conventions across distros
# (lua5.1, lua-5.1, lua5.4, lua on Alpine, luajit)
AC_MSG_CHECKING([for lua library])
LUA_LIB=""
for lualib in lua5.1 lua-5.1 lua5.4 lua-5.4 lua luajit-5.1; do
    AC_CHECK_LIB([$lualib], [luaL_newstate], [LUA_LIB="$lualib"; break], [], [])
done
if test -n "$LUA_LIB"; then
    AC_MSG_RESULT([$LUA_LIB])
    LIBS="-l$LUA_LIB $LIBS"
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([lua library is required (install lua-dev or lua5.1-dev)])
fi

# library variables activation
AM_CONDITIONAL(LINK_LUA, [test "$have_lua" != "false"])
