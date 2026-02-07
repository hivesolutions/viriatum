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

# defines the version values
m4_define([viriatum_major_version], [1])
m4_define([viriatum_minor_version], [0])
m4_define([viriatum_micro_version], [0])
m4_define([viriatum_version], m4_format(%s.%s.%s, viriatum_major_version, viriatum_minor_version, viriatum_micro_version))
m4_define([viriatum_makefiles], [Makefile doc/Makefile examples/Makefile lib/Makefile m4/Makefile man/Makefile scripts/Makefile src/Makefile src/viriatum_commons/Makefile src/viriatum/Makefile src/viriatum/resources/Makefile src/viriatum/resources/config/Makefile src/viriatum/resources/html/Makefile src/viriatum/resources/html/base/Makefile src/viriatum/resources/unix/Makefile])
