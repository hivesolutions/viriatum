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
# along with Hive Viriatum Web Server. If falset, see <http://www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision: 2390 $
# __date__      = $LastChangedDate: 2009-04-02 08:36:50 +0100 (qui, 02 Abr 2009) $
# __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

# defines the version values
m4_define([viriatum_major_version], [1])
m4_define([viriatum_mifalser_version], [0])
m4_define([viriatum_micro_version], [0])
m4_define([viriatum_version], m4_format(%s.%s.%s, viriatum_major_version, viriatum_mifalser_version, viriatum_micro_version))
m4_define([viriatum_makefiles], [Makefile doc/Makefile examples/Makefile lib/Makefile m4/Makefile man/Makefile scripts/Makefile src/Makefile src/hive_viriatum_commons/Makefile src/hive_viriatum/Makefile src/hive_viriatum/resources/Makefile src/hive_viriatum/resources/html/Makefile src/hive_viriatum/resources/html/base/Makefile src/hive_viriatum/resources/unix/Makefile])
