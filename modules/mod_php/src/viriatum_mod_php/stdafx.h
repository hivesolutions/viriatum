/*
 Hive Viriatum Modules
 Copyright (C) 2008-2012 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef HAVE_CONFIG_H
#undef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4005)
#endif
#include "undef.h"
#include <sapi/embed/php_embed.h>
#include <ext/standard/php_standard.h>
#include "../../../../src/viriatum/viriatum.h"
#include "undef.h"
#ifdef _MSC_VER
#pragma warning(default:4005)
#endif

unsigned char *name_viriatum_mod_php();
unsigned char *name_s_viriatum_mod_php();
unsigned char *version_viriatum_mod_php();
unsigned char *description_viriatum_mod_php();

#ifdef VIRIATUM_PLATFORM_MSC
#ifdef VIRIATUM_DEBUG
#pragma comment(lib, "php5embed_d.lib")
#else
#pragma comment(lib, "php5embed.lib")
#endif
#endif