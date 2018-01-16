/*
 Hive Viriatum Commons
 Copyright (c) 2008-2018 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "cpu.h"
#include "types.h"
#include "platforms.h"
#include "compiler.h"
#include "compilation.h"

/**
 * The name of the environment variable used to build
 * the viriatum path.
 */
#define VIRIATUM_ENVIRONMENT_PATH "VIRIATUM_PATH"

/**
 * The root path being used.
 */
#define VIRIATUM_ROOT_PATH "."

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
#ifdef VIRIATUM_PLATFORM_MACOSX
#define VIRIATUM_SHARED_OBJECT_EXTENSION ".dylib"
#else
#define VIRIATUM_SHARED_OBJECT_EXTENSION ".so"
#endif
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
#define VIRIATUM_HTTP_SIZE 4096
#define VIRIATUM_HTTP_MAX_SIZE VIRIATUM_HTTP_SIZE + 16 * VIRIATUM_MAX_HEADER_C_SIZE + VIRIATUM_MAX_HEADER_P_SIZE
#define VIRIATUM_MAX_WORKERS 64
#define VIRIATUM_MAX_URL_SIZE 4096
#define VIRIATUM_MAX_PATH_SIZE 1024
#define VIRIATUM_MAX_EXTENSION_SIZE 256
#define VIRIATUM_MAX_SSL_SIZE 128
#define VIRIATUM_MAX_HEADER_COUNT 64
#define VIRIATUM_MAX_HEADER_SIZE 1024
#define VIRIATUM_MAX_HEADER_V_SIZE 4096
#define VIRIATUM_MAX_HEADER_P_SIZE 2
#define VIRIATUM_MAX_HEADER_C_SIZE VIRIATUM_MAX_HEADER_SIZE + VIRIATUM_MAX_HEADER_V_SIZE + VIRIATUM_MAX_HEADER_P_SIZE
#endif
#ifdef VIRIATUM_PLATFORM_UNIX
#define VIRIATUM_HTTP_SIZE 4096
#define VIRIATUM_HTTP_MAX_SIZE VIRIATUM_HTTP_SIZE + 16 * VIRIATUM_MAX_HEADER_C_SIZE + VIRIATUM_MAX_HEADER_P_SIZE
#define VIRIATUM_MAX_WORKERS 64
#define VIRIATUM_MAX_URL_SIZE 4096
#define VIRIATUM_MAX_PATH_SIZE 1024
#define VIRIATUM_MAX_EXTENSION_SIZE 256
#define VIRIATUM_MAX_SSL_SIZE 128
#define VIRIATUM_MAX_HEADER_COUNT 64
#define VIRIATUM_MAX_HEADER_SIZE 1024
#define VIRIATUM_MAX_HEADER_V_SIZE 4096
#define VIRIATUM_MAX_HEADER_P_SIZE 2
#define VIRIATUM_MAX_HEADER_C_SIZE VIRIATUM_MAX_HEADER_SIZE + VIRIATUM_MAX_HEADER_V_SIZE + VIRIATUM_MAX_HEADER_P_SIZE
#endif

#ifdef VIRIATUM_THREAD_SAFE
#undef VIRIATUM_MPOOL
#endif
