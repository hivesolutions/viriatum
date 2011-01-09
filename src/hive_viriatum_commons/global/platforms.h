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

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define VIRIATUM_PLATFORM_WIN32 true
#define VIRIATUM_PLATFORM_STRING "win32"
#endif

#if defined(linux) || defined(__linux)
#define VIRIATUM_PLATFORM_LINUX true
#define VIRIATUM_PLATFORM_STRING "linux"
#endif

#if defined(__APPLE__) && defined(__MACH__)
#if TARGET_OS_IPHONE
#define VIRIATUM_PLATFORM_IPHONE true
#else
#define VIRIATUM_PLATFORM_MACOSX true
#endif
#define VIRIATUM_PLATFORM_STRING "darwin"
#endif

#if defined(__minix)
#define VIRIATUM_PLATFORM_MINIX true
#define VIRIATUM_PLATFORM_STRING "minix"
#endif

#if defined(__FreeBSD__)
#define VIRIATUM_PLATFORM_FREEBSD true
#define VIRIATUM_PLATFORM_STRING "freebsd"
#endif

#ifdef unix
#define VIRIATUM_PLATFORM_UNIX true
#endif

#ifdef __MINGW32__
#define VIRIATUM_PLATFORM_MINGW true
#define VIRIATUM_PLATFORM_STRING "mingw32"
#endif

// CPU

// CPU ARCHITECTURE

#if defined(__arm__)
#define VIRIATUM_PLATFORM_CPU "arm"
#define VIRIATUM_PLATFORM_CPU_ARM true
#endif

#if defined(__sparc__)
#define VIRIATUM_PLATFORM_CPU "sparc"
#define VIRIATUM_PLATFORM_CPU_SPARC true
#endif

#if defined(__hppa__)
#define VIRIATUM_PLATFORM_CPU "hp/pa risc"
#define VIRIATUM_PLATFORM_CPU_HPPA true
#endif

#if defined(__ppc__) || defined(_M_MPPC) || defined(_M_PPC)
#define VIRIATUM_PLATFORM_CPU "powerpc"
#define VIRIATUM_PLATFORM_CPU_POWERPC true
#endif

#if defined(__mips__) || defined(_M_MRX000)
#define VIRIATUM_PLATFORM_CPU "mips"
#define VIRIATUM_PLATFORM_CPU_MIPS true
#endif

#if defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_X86_) || defined(_M_IX86)
#define VIRIATUM_PLATFORM_CPU "intel x86"
#define VIRIATUM_PLATFORM_CPU_X86 true
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64)
#define VIRIATUM_PLATFORM_CPU "intel x64"
#define VIRIATUM_PLATFORM_CPU_X64 true
#endif

#if defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64)
#define VIRIATUM_PLATFORM_CPU "intel ia64"
#define VIRIATUM_PLATFORM_CPU_IA64 true
#endif

// CPU BITS

#define VIRIATUM_PLATFORM_CPU_BITS sizeof(void *) * 8

// CPU ENDIANESS

#if defined(VIRIATUM_PLATFORM_CPU_ARM) || defined(VIRIATUM_PLATFORM_CPU_SPARC) ||\
	defined(VIRIATUM_PLATFORM_CPU_HPPA) || defined(VIRIATUM_PLATFORM_CPU_POWERPC) || defined(VIRIATUM_PLATFORM_CPU_MIPS)
#define VIRIATUM_BIG_ENDIAN true
#else
#define VIRIATUM_LITTLE_ENDIAN true
#endif

// WIN32 API

#ifdef _WIN32
#define VIRIATUM_PLATFORM_CPU_WIN_API_BITS 32
#endif

#ifdef _WIN64
#define VIRIATUM_PLATFORM_CPU_WIN_API_BITS 64
#endif

// COMPILER

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

// COMPILATION

#define VIRIATUM_COMPILATION_DATE __DATE__
#define VIRIATUM_COMPILATION_TIME __TIME__