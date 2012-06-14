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

#include "cpu.h"
#include "platforms.h"
#include "compiler.h"
#include "compilation.h"

/**
 * The name of the environment variable used to build
 * the viriatum path.
 */
#define HIVE_VIRIATUM_ENVIRONMENT_PATH "VIRIATUM_PATH"

/**
 * The root path being used.
 */
#define HIVE_VIRIATUM_ROOT_PATH "."

#ifdef VIRIATUM_PLATFORM_WIN32
#define VIRIATUM_ENVIRONMENT_SEPARATOR ";"
#endif
#ifdef VIRIATUM_PLATFORM_UNIX
#define VIRIATUM_ENVIRONMENT_SEPARATOR ":"
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
#define VIRIATUM_SHARED_OBJECT_EXTENSION ".dll"
#endif
#ifdef VIRIATUM_PLATFORM_UNIX
#define VIRIATUM_SHARED_OBJECT_EXTENSION ".so"
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
#ifndef VIRIATUM_SYSTEM_BASE_PATH
#define VIRIATUM_SYSTEM_BASE_PATH "/"
#endif
#endif
#ifdef VIRIATUM_PLATFORM_IPHONE
#ifndef VIRIATUM_SYSTEM_BASE_PATH
#define VIRIATUM_SYSTEM_BASE_PATH getBundleDirectory()
#endif
#endif
#ifdef VIRIATUM_PLATFORM_UNIX
#ifndef VIRIATUM_SYSTEM_BASE_PATH
#define VIRIATUM_SYSTEM_BASE_PATH "/"
#endif
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
#define VIRIATUM_MAX_URL_SIZE 4096
#define VIRIATUM_MAX_PATH_SIZE 1024
#define VIRIATUM_MAX_HEADER_COUNT 64
#define VIRIATUM_MAX_HEADER_SIZE 1024
#define VIRIATUM_MAX_HEADER_V_SIZE 4096
#endif
#ifdef VIRIATUM_PLATFORM_UNIX
#define VIRIATUM_MAX_URL_SIZE 4096
#define VIRIATUM_MAX_PATH_SIZE 1024
#define VIRIATUM_MAX_HEADER_COUNT 64
#define VIRIATUM_MAX_HEADER_SIZE 1024
#define VIRIATUM_MAX_HEADER_V_SIZE 4096
#endif
