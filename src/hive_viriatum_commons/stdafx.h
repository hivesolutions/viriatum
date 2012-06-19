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

#ifdef HAVE_CONFIG_H
#undef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
#include "_config_win32.h"
#endif

#ifdef HAVE_DEBUG
#define VIRIATUM_DEBUG
#endif

#ifdef HAVE_IP6
#define VIRIATUM_IP6
#endif

#ifndef HAVE_LIBPTHREAD
#define VIRIATUM_NO_THREADS
#endif

#ifdef HAVE_JNI_H
#define VIRIATUM_JNI
#endif

#ifdef __MACH__
#define unix true
#include <TargetConditionals.h>
#endif

#ifdef _WIN32
#include "global/targetver.h"
#include "global/resource.h"

/* in case the target windows version is lower than
the windows xp operative system, must disable ipv6 */
#if (WINVER < 0x0501)
#undef VIRIATUM_IP6
#endif

/* excludes rarely-used stuff from windows headers */
#define WIN32_LEAN_AND_MEAN

/* includes the extra math definitions */
#define _USE_MATH_DEFINES

/* defines the export prefix */
#define VIRIATUM_EXPORT_PREFIX __declspec(dllexport)

/* defines the no export prefix */
#define VIRIATUM_NO_EXPORT_PREFIX

/* defines the external prefix (careful usage) */
#define VIRIATUM_EXTERNAL_PREFIX __declspec(dllexport)
#else
/* defines the export prefix */
#define VIRIATUM_EXPORT_PREFIX __attribute__((visibility("default")))

/* defines the no export prefix */
#define VIRIATUM_NO_EXPORT_PREFIX __attribute__((visibility("hidden")))

/* defines the external prefix (careful usage) */
#define VIRIATUM_EXTERNAL_PREFIX extern
#endif

#include "global/definitions.h"

#ifdef VIRIATUM_PLATFORM_WIN32
#define FD_SETSIZE 10000
#ifndef SIGHUP
#define SIGHUP 1
#endif
#ifndef SIGINT
#define SIGINT 2
#endif
#ifndef SIGQUIT
#define SIGQUIT 3
#endif
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
#ifndef SIGTERM
#define SIGTERM 15
#endif
#endif

#ifdef VIRIATUM_PLATFORM_WIN32
#include <windows.h>
#include <psapi.h>
#include <io.h>
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#endif

#ifdef VIRIATUM_PLATFORM_LINUX
#include <sys/prctl.h>
#endif

#ifdef VIRIATUM_JNI
#include <jni.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#include "global/memory.h"

unsigned char *name_viriatum_commons();
unsigned char *version_viriatum_commons();
unsigned char *description_viriatum_commons();
