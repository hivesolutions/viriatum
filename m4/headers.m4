# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
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
# __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# checks for header files
AC_CHECK_HEADERS([stdlib.h assert.h])

# checks for specific header files
AC_CHECK_HEADERS([pcre.h], [have_pcre=$have_pcre], [have_pcre=false])
AC_CHECK_HEADERS([openssl/ssl.h], [have_ssl=$have_ssl], [have_ssl=false])
AC_CHECK_HEADERS([jni.h], [have_jni=true], [have_jni=false])
