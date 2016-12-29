# Hive Viriatum Modules
# Copyright (c) 2008-2017 Hive Solutions Lda.
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
# __copyright__ = Copyright (c) 2008-2017 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

# defines the version values
m4_define([mod_wsgi_major_version], [1])
m4_define([mod_wsgi_minor_version], [0])
m4_define([mod_wsgi_micro_version], [0])
m4_define([mod_wsgi_version], m4_format(%s.%s.%s, mod_wsgi_major_version, mod_wsgi_minor_version, mod_wsgi_micro_version))
m4_define([mod_wsgi_makefiles], [Makefile m4/Makefile src/Makefile src/viriatum_mod_wsgi/Makefile])
