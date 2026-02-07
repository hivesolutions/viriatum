/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../util/string_util.h"

#ifdef VIRIATUM_PLATFORM_ANDROID
#include <android/log.h>
#define PRINTF(format) __android_log_print(ANDROID_LOG_INFO, "logging", format)
#define PRINTF_F(format, ...) __android_log_print(ANDROID_LOG_INFO, "logging", format, __VA_ARGS__)
#define PRINTF_E(format) __android_log_print(ANDROID_LOG_ERROR, "logging", format)
#define PRINTF_E_F(format, ...) __android_log_print(ANDROID_LOG_ERROR, "logging", format, __VA_ARGS__)
#define PRINT_FLUSH() fflush(stdout)
#else
#define PRINTF(format) printf(format)
#define PRINTF_F(format, ...) printf(format, __VA_ARGS__)
#define PRINTF_E(format) fprintf(stderr, format);
#define PRINTF_E_F(format, ...) fprintf(stderr, format, __VA_ARGS__);
#define PRINT_FLUSH() fflush(stdout)
#endif

#ifdef VIRIATUM_DEBUG
#define V_MESSAGE(level) PRINTF_F("[%s] [%s:%d] ", level, base_string_value((unsigned char *) __FILE__), __LINE__)
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
#define V_DEBUG_F(format, ...) dump_multiple(format, __VA_ARGS__)

#define V_INFO(format) dump(format)
#define V_INFO_F(format, ...) dump_multiple(format, __VA_ARGS__)
#endif

#define V_WARNING(format) V_MESSAGE("WARNING"); PRINTF(format)
#define V_WARNING_F(format, ...) V_MESSAGE("WARNING"); PRINTF_F(format, __VA_ARGS__)

#define V_ERROR(format) V_MESSAGE("ERROR"); PRINTF(format)
#define V_ERROR_F(format, ...) V_MESSAGE("ERROR"); PRINTF_F(format, __VA_ARGS__)

#define V_PRINT(format) PRINTF(format)
#define V_PRINT_F(format, ...) PRINTF_F(format, __VA_ARGS__)

VIRIATUM_EXPORT_PREFIX void debug(const char *format, ...);
