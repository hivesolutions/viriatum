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

#include "../util/string_util.h"

#ifdef VIRIATUM_PLATFORM_ANDROID
#include <android/log.h>
#define PRINTF(format) __android_log_print(ANDROID_LOG_INFO, "logging", format)
#define PRINTF_F(format, ...) __android_log_print(ANDROID_LOG_INFO, "logging", format, __VA_ARGS__)
#else
#define PRINTF(format) printf(format)
#define PRINTF_F(format, ...) printf(format, __VA_ARGS__)
#endif

#ifdef VIRIATUM_DEBUG
#define V_MESSAGE(level) PRINTF_F("[%s] [%s:%d] ", level, baseStringValue((unsigned char *) __FILE__), __LINE__)
#endif

#ifndef VIRIATUM_DEBUG
#define V_MESSAGE(level) PRINTF_F("[%s] ", level)
#endif

#ifdef VIRIATUM_DEBUG
#define V_DEBUG(format) V_MESSAGE("DEBUG"); PRINTF(format)
#define V_DEBUG_F(format, ...) V_MESSAGE("DEBUG"); PRINTF_F(format, __VA_ARGS__)

#define V_INFO(format) V_MESSAGE("INFO"); PRINTF(format)
#define V_INFO_F(format, ...) V_MESSAGE("INFO"); PRINTF_F(format, __VA_ARGS__)
#endif

#ifndef VIRIATUM_DEBUG
#define V_DEBUG(format) dump(format)
#define V_DEBUG_F(format, ...) dumpMultiple(format, __VA_ARGS__)

#define V_INFO(format) dump(format)
#define V_INFO_F(format, ...) dumpMultiple(format, __VA_ARGS__)
#endif

#define V_WARNING(format) V_MESSAGE("WARNING"); PRINTF(format)
#define V_WARNING_F(format, ...) V_MESSAGE("WARNING"); PRINTF_F(format, __VA_ARGS__)

#define V_ERROR(format) V_MESSAGE("ERROR"); PRINTF(format)
#define V_ERROR_F(format, ...) V_MESSAGE("ERROR"); PRINTF_F(format, __VA_ARGS__)

#define V_PRINT(format) PRINTF(format)
#define V_PRINT_F(format, ...) PRINTF_F(format, __VA_ARGS__)

VIRIATUM_EXPORT_PREFIX void debug(const char *format, ...);
