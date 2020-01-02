/*
 Hive Viriatum Modules
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

static unsigned char name[] = "viriatum_mod_wsgi";
static unsigned char name_s[] = "wsgi";
static unsigned char version[] = "1.0.0";
static unsigned char description[] = "Viriatum Wsgi Module";

unsigned char *name_viriatum_mod_wsgi() { return name; }
unsigned char *name_s_viriatum_mod_wsgi() { return name_s; }
unsigned char *version_viriatum_mod_wsgi() { return version; }
unsigned char *description_viriatum_mod_wsgi() { return description; }
