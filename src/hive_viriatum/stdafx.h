/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
#include "_config_win32.h"
#endif

#ifdef WITH_MODULE_ROOT
#ifndef HAVE_DEFAULTS
#define VIRIATUM_MODULES_PATH WITH_MODULE_ROOT
#endif
#endif

#ifdef WITH_WWW_ROOT
#ifndef HAVE_DEFAULTS
#define VIRIATUM_CONTENTS_PATH WITH_WWW_ROOT
#endif
#endif

#ifdef HAVE_PCRE
#define VIRIATUM_PCRE
#endif

#ifdef _WIN32
#include "global/targetver.h"
#include "global/resource.h"
#endif

#include "../hive_viriatum_commons/viriatum_commons.h"

#include "global/definitions.h"

#ifdef VIRIATUM_PCRE
#define PCRE_STATIC
#include <pcre.h>
#ifdef VIRIATUM_PLATFORM_MSC
#ifdef VIRIATUM_DEBUG
#pragma comment(lib, "pcred.lib")
#else
#pragma comment(lib, "pcre.lib")
#endif
#endif
#endif

unsigned char *nameViriatum();
unsigned char *versionViriatum();
unsigned char *descriptionViriatum();
