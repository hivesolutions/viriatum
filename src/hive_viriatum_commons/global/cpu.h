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

/* cpu architecture */

#if defined(__arm__) || defined(__thumb__) || defined(_ARM)
#define VIRIATUM_PLATFORM_CPU "arm"
#define VIRIATUM_PLATFORM_CPU_ARM true
#endif

#if defined(__sparc__) || defined(__sparc) || defined(__sparcv8) || defined(__sparcv9)
#define VIRIATUM_PLATFORM_CPU "sparc"
#define VIRIATUM_PLATFORM_CPU_SPARC true
#endif

#if defined(__hppa__) || defined(__hppa)
#define VIRIATUM_PLATFORM_CPU "hp/pa risc"
#define VIRIATUM_PLATFORM_CPU_HPPA true
#endif

#if defined(__ppc__) || defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(_ARCH_PPC) || defined(_M_MPPC) || defined(_M_PPC)
#define VIRIATUM_PLATFORM_CPU "powerpc"
#define VIRIATUM_PLATFORM_CPU_POWERPC true
#endif

#if defined(__mips__) || defined(mips) || defined(__mips) || defined(__MIPS__) || defined(_M_MRX000)
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

/* cpu bits */

#define VIRIATUM_PLATFORM_CPU_BITS sizeof(void *) * 8

/* cpu endianess */

#if defined(VIRIATUM_PLATFORM_CPU_ARM) || defined(VIRIATUM_PLATFORM_CPU_SPARC) ||\
    defined(VIRIATUM_PLATFORM_CPU_HPPA) || defined(VIRIATUM_PLATFORM_CPU_POWERPC) || defined(VIRIATUM_PLATFORM_CPU_MIPS)
#define VIRIATUM_BIG_ENDIAN true
#else
#define VIRIATUM_LITTLE_ENDIAN true
#endif
