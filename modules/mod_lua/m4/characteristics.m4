# Hive Viriatum Modules
# Copyright (c) 2008-2016 Hive Solutions Lda.
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
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# checks for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# sets the silent rules for compilation
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# verifies if the current compiler to be used is clang
# if that's the case exposed it as an automake variable
AM_CONDITIONAL(CLANG, [ test ${CC:0:5} = "clang" ])

# changes the library names spec to a non versioned
# setting (avoid possible import problems)
library_names_spec='$libname${shared_ext}'
soname_spec='${libname}${shared_ext}'
