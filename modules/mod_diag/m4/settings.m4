# Hive Viriatum Modules
# Copyright (C) 2008-2012 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Modules.
#
# Hive Viriatum Modules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hive Viriatum Modules is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
# __license__   = GNU General Public License (GPL), Version 3

# defines the version values
m4_define([mod_diag_major_version], [1])
m4_define([mod_diag_minor_version], [0])
m4_define([mod_diag_micro_version], [0])
m4_define([mod_diag_version], m4_format(%s.%s.%s, mod_diag_major_version, mod_diag_minor_version, mod_diag_micro_version))
m4_define([mod_diag_makefiles], [Makefile m4/Makefile src/Makefile src/hive_viriatum_mod_diag/Makefile])
