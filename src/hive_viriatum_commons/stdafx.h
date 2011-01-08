/*
 Hive Viriatum Commons
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef __MACH__
#define unix true
#include <TargetConditionals.h>
#endif

#ifdef _WIN32
#include "global/targetver.h"
#include "global/resource.h"

/* excludes rarely-used stuff from windows headers */
#define WIN32_LEAN_AND_MEAN

/* includes the extra math definitions */
#define _USE_MATH_DEFINES

/* defines the export prefix */
#define VIRIATUM_EXPORT_PREFIX __declspec(dllexport)

/* defines the no export prefix */
#define VIRIATUM_NO_EXPORT_PREFIX
#else
/* defines the export prefix */
#define VIRIATUM_EXPORT_PREFIX __attribute__((visibility("default")))

/* defines the no export prefix */
#define VIRIATUM_NO_EXPORT_PREFIX __attribute__((visibility("hidden")))
#endif

#include "global/definitions.h"

#ifdef VIRIATUM_PLATFORM_WIN32
#include <Windows.h>
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#include <unistd.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

unsigned char *nameViriatumCommons();
unsigned char *versionViriatumCommons();

#define VIRIATUM_DEBUG
