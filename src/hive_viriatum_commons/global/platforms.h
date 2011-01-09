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

#if defined(__arm__) || defined(__sparc__) || defined(__hppa__) || defined(__ppc__) || defined(__mips__)
#define VIRIATUM_BIG_ENDIAN true
#else
#define VIRIATUM_LITTLE_ENDIAN true
#endif

#ifdef _WIN32 
#define VIRIATUM_PLATFORM_WIN32 true
#define VIRIATUM_PLATFORM "win32"
#endif

#ifdef linux
#define VIRIATUM_PLATFORM_LINUX true
#define VIRIATUM_PLATFORM "linux"
#endif

#ifdef __MACH__
#if TARGET_OS_IPHONE
#define VIRIATUM_PLATFORM_IPHONE true
#else
#define VIRIATUM_PLATFORM_MACOSX true
#endif
#define VIRIATUM_PLATFORM "mach"
#endif

#ifdef unix
#define VIRIATUM_PLATFORM_UNIX true
#endif

#ifdef __MINGW32__
#define VIRIATUM_PLATFORM_MINGW true
#define VIRIATUM_PLATFORM "mingw32"
#endif

// CPU

// GCC DETECTION

#ifdef __arm__
#define VIRIATUM_PLATFORM_CPU "arm"
#endif

#ifdef __sparc__
#define VIRIATUM_PLATFORM_CPU "sparc"
#endif

#ifdef __hppa__
#define VIRIATUM_PLATFORM_CPU "hp/pa risc"
#endif

#ifdef __ppc__
#define VIRIATUM_PLATFORM_CPU "powerpc"
#endif

#ifdef __mips__
#define VIRIATUM_PLATFORM_CPU "mips"
#endif

#if defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_X86_)
#define VIRIATUM_PLATFORM_CPU "intel x86"
#endif

#if defined(__ia64__) || defined(_IA64) || defined(__IA64__)
#define VIRIATUM_PLATFORM_CPU "intel x64"
#endif

// MVC detection

#ifdef _M_IX86
#define VIRIATUM_PLATFORM_CPU "intel x86"
#endif

#ifdef _M_X64
#define VIRIATUM_PLATFORM_CPU "intel x64"
#endif

#ifdef _M_IA64
#define VIRIATUM_PLATFORM_CPU "intel ia64"
#endif

#ifdef _M_MPPC
#define VIRIATUM_PLATFORM_CPU "powerpc"
#endif

#ifdef _M_PPC
#define VIRIATUM_PLATFORM_CPU "powerpc"
#endif

#ifdef _M_MRX000
#define VIRIATUM_PLATFORM_CPU "mips"
#endif

// CPU BITS

#ifdef _WIN32
#define VIRIATUM_PLATFORM_CPU_BITS 32
#endif

#ifdef _WIN64
#define VIRIATUM_PLATFORM_CPU_BITS 64
#endif


// COMPILERS


#ifdef _MSC_VER
#define VIRIATUM_PLATFORM_MSC true
#define VIRIATUM_COMPILER "msvc"
#define VIRIATUM_COMPILER_VERSION _MSC_VER
#define VIRIATUM_COMPILER_VERSION_STRING "1.0.0"
#endif

#ifdef __GNUC__
#define VIRIATUM_PLATFORM_GCC true
#define VIRIATUM_COMPILER "gcc"
#define VIRIATUM_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define VIRIATUM_COMPILER_VERSION_STRING __VERSION__
#endif



// COMPILATION TIME

#define VIRIATUM_COMPILATION_DATE __DATE__
#define VIRIATUM_COMPILATION_TIME __TIME__