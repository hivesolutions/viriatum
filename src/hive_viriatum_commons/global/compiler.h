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

/* compiler structure */

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
