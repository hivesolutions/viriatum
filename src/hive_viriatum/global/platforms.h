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

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#if defined(__sparc__) || defined(__hppa__) || defined(__ppc__) || defined(__mips__) || (defined(__MWERKS__) && !defined(__INTEL__))
#define VIRIATUM_BIG_ENDIAN true
#else
#define VIRIATUM_LITTLE_ENDIAN true
#endif

#ifdef _WIN32
#define VIRIATUM_PLATFORM_WIN32 true
#endif

#ifdef __linux__
#define VIRIATUM_PLATFORM_LINUX true
#endif

#ifdef __MACH__
#if TARGET_OS_IPHONE
#define VIRIATUM_PLATFORM_IPHONE true
#else
#define VIRIATUM_PLATFORM_MACOSX true
#endif
#endif

#if defined(unix) || defined(__linux__)
#define VIRIATUM_PLATFORM_UNIX true
#endif

#if defined(VIRIATUM_PLATFORM_WIN32)
#define VIRIATUM_PLATFORM_DIRECT3D true
#endif

#if defined(VIRIATUM_PLATFORM_WIN32) || defined(VIRIATUM_PLATFORM_LINUX) || defined(VIRIATUM_PLATFORM_MACOSX)
#define VIRIATUM_PLATFORM_OPENGL true
#endif

#if defined(VIRIATUM_PLATFORM_IPHONE)
#define VIRIATUM_PLATFORM_OPENGLES true
#endif

#if defined(VIRIATUM_PLATFORM_IPHONE)
#define VIRIATUM_SYNC_PARALLEL_PROCESSING true
#else
#define VIRIATUM_SYNC_PARALLEL_PROCESSING true
#endif
