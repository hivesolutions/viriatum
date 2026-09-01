/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
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

/**
 * The levels a message may be written under, a message reaching the
 * output only once the level of it is at or above the one that the
 * process has been set to.
 * The debug level is only ever produced by a build that carries the
 * debug messages, a release one writes none of them whatever the
 * level it has been set to.
 */
typedef enum logging_level_e {
    LOGGING_DEBUG = 1,
    LOGGING_INFO,
    LOGGING_WARNING,
    LOGGING_ERROR
} logging_level;

/* ANSI colour escape sequences used to decorate log level labels
and the date prefix when the terminal supports colour output */
#define V_COLOR_RESET "\x1b[0m"
#define V_COLOR_DATE "\x1b[37m"
#define V_COLOR_CONTEXT "\x1b[1;37m"
#define V_COLOR_DEBUG "\x1b[36m"
#define V_COLOR_INFO "\x1b[32m"
#define V_COLOR_WARNING "\x1b[33m"
#define V_COLOR_ERROR "\x1b[31m"

/* prints the level label surrounded by colour escapes when the
terminal supports it, or as plain text otherwise */
#define V_LEVEL(color, level) \
    use_color_logging() ? PRINTF_F("%s%s%s", color, level, V_COLOR_RESET) : PRINTF_F("%s", level)

#ifdef VIRIATUM_DEBUG
#define V_MESSAGE(level, color) \
    print_date_logging();       \
    V_LEVEL(color, level);      \
    PRINTF_F(" [%s:%d] ", base_string_value((unsigned char *) __FILE__), __LINE__)
#define V_MESSAGE_CONTEXT(level, color, context) \
    print_date_logging();                        \
    V_LEVEL(color, level);                       \
    use_color_logging() ? PRINTF_F(" [%s:%d] [%s%s%s] ", base_string_value((unsigned char *) __FILE__), __LINE__, V_COLOR_CONTEXT, context, V_COLOR_RESET) : PRINTF_F(" [%s:%d] [%s] ", base_string_value((unsigned char *) __FILE__), __LINE__, context)
#endif

#ifndef VIRIATUM_DEBUG
#define V_MESSAGE(level, color) \
    print_date_logging();       \
    V_LEVEL(color, level);      \
    PRINTF_F("%s", " ")
#define V_MESSAGE_CONTEXT(level, color, context) \
    print_date_logging();                        \
    V_LEVEL(color, level);                       \
    use_color_logging() ? PRINTF_F(" [%s%s%s] ", V_COLOR_CONTEXT, context, V_COLOR_RESET) : PRINTF_F(" [%s] ", context)
#endif

#ifdef VIRIATUM_DEBUG
#define V_DEBUG(format)                            \
    do {                                           \
        if(is_level_logging(LOGGING_DEBUG)) {      \
            V_MESSAGE("[DEBUG]  ", V_COLOR_DEBUG); \
            PRINTF(format);                        \
        }                                          \
    } while(0)
#define V_DEBUG_F(format, ...)                     \
    do {                                           \
        if(is_level_logging(LOGGING_DEBUG)) {      \
            V_MESSAGE("[DEBUG]  ", V_COLOR_DEBUG); \
            PRINTF_F(format, __VA_ARGS__);         \
        }                                          \
    } while(0)
#define V_DEBUG_CTX(context, format)                                \
    do {                                                            \
        if(is_level_logging(LOGGING_DEBUG)) {                       \
            V_MESSAGE_CONTEXT("[DEBUG]  ", V_COLOR_DEBUG, context); \
            PRINTF(format);                                         \
        }                                                           \
    } while(0)
#define V_DEBUG_CTX_F(context, format, ...)                         \
    do {                                                            \
        if(is_level_logging(LOGGING_DEBUG)) {                       \
            V_MESSAGE_CONTEXT("[DEBUG]  ", V_COLOR_DEBUG, context); \
            PRINTF_F(format, __VA_ARGS__);                          \
        }                                                           \
    } while(0)

#define V_INFO(format)                            \
    do {                                          \
        if(is_level_logging(LOGGING_INFO)) {      \
            V_MESSAGE("[INFO]   ", V_COLOR_INFO); \
            PRINTF(format);                       \
        }                                         \
    } while(0)
#define V_INFO_F(format, ...)                     \
    do {                                          \
        if(is_level_logging(LOGGING_INFO)) {      \
            V_MESSAGE("[INFO]   ", V_COLOR_INFO); \
            PRINTF_F(format, __VA_ARGS__);        \
        }                                         \
    } while(0)
#define V_INFO_CTX(context, format)                                \
    do {                                                           \
        if(is_level_logging(LOGGING_INFO)) {                       \
            V_MESSAGE_CONTEXT("[INFO]   ", V_COLOR_INFO, context); \
            PRINTF(format);                                        \
        }                                                          \
    } while(0)
#define V_INFO_CTX_F(context, format, ...)                         \
    do {                                                           \
        if(is_level_logging(LOGGING_INFO)) {                       \
            V_MESSAGE_CONTEXT("[INFO]   ", V_COLOR_INFO, context); \
            PRINTF_F(format, __VA_ARGS__);                         \
        }                                                          \
    } while(0)
#endif

#ifndef VIRIATUM_DEBUG
#define V_DEBUG(format) dump(format)
#define V_DEBUG_F(format, ...) dump_multiple(format, __VA_ARGS__)
#define V_DEBUG_CTX(context, format) dump_context(context, format)
#define V_DEBUG_CTX_F(context, format, ...) dump_context_multiple(context, format, __VA_ARGS__)

#define V_INFO(format)                            \
    do {                                          \
        if(is_level_logging(LOGGING_INFO)) {      \
            V_MESSAGE("[INFO]   ", V_COLOR_INFO); \
            PRINTF(format);                       \
        }                                         \
    } while(0)
#define V_INFO_F(format, ...)                     \
    do {                                          \
        if(is_level_logging(LOGGING_INFO)) {      \
            V_MESSAGE("[INFO]   ", V_COLOR_INFO); \
            PRINTF_F(format, __VA_ARGS__);        \
        }                                         \
    } while(0)
#define V_INFO_CTX(context, format)                                \
    do {                                                           \
        if(is_level_logging(LOGGING_INFO)) {                       \
            V_MESSAGE_CONTEXT("[INFO]   ", V_COLOR_INFO, context); \
            PRINTF(format);                                        \
        }                                                          \
    } while(0)
#define V_INFO_CTX_F(context, format, ...)                         \
    do {                                                           \
        if(is_level_logging(LOGGING_INFO)) {                       \
            V_MESSAGE_CONTEXT("[INFO]   ", V_COLOR_INFO, context); \
            PRINTF_F(format, __VA_ARGS__);                         \
        }                                                          \
    } while(0)
#endif

#define V_WARNING(format)                            \
    do {                                             \
        if(is_level_logging(LOGGING_WARNING)) {      \
            V_MESSAGE("[WARNING]", V_COLOR_WARNING); \
            PRINTF(format);                          \
        }                                            \
    } while(0)
#define V_WARNING_F(format, ...)                     \
    do {                                             \
        if(is_level_logging(LOGGING_WARNING)) {      \
            V_MESSAGE("[WARNING]", V_COLOR_WARNING); \
            PRINTF_F(format, __VA_ARGS__);           \
        }                                            \
    } while(0)
#define V_WARNING_CTX(context, format)                                \
    do {                                                              \
        if(is_level_logging(LOGGING_WARNING)) {                       \
            V_MESSAGE_CONTEXT("[WARNING]", V_COLOR_WARNING, context); \
            PRINTF(format);                                           \
        }                                                             \
    } while(0)
#define V_WARNING_CTX_F(context, format, ...)                         \
    do {                                                              \
        if(is_level_logging(LOGGING_WARNING)) {                       \
            V_MESSAGE_CONTEXT("[WARNING]", V_COLOR_WARNING, context); \
            PRINTF_F(format, __VA_ARGS__);                            \
        }                                                             \
    } while(0)

#define V_ERROR(format)                            \
    do {                                           \
        if(is_level_logging(LOGGING_ERROR)) {      \
            V_MESSAGE("[ERROR]  ", V_COLOR_ERROR); \
            PRINTF(format);                        \
        }                                          \
    } while(0)
#define V_ERROR_F(format, ...)                     \
    do {                                           \
        if(is_level_logging(LOGGING_ERROR)) {      \
            V_MESSAGE("[ERROR]  ", V_COLOR_ERROR); \
            PRINTF_F(format, __VA_ARGS__);         \
        }                                          \
    } while(0)
#define V_ERROR_CTX(context, format)                                \
    do {                                                            \
        if(is_level_logging(LOGGING_ERROR)) {                       \
            V_MESSAGE_CONTEXT("[ERROR]  ", V_COLOR_ERROR, context); \
            PRINTF(format);                                         \
        }                                                           \
    } while(0)
#define V_ERROR_CTX_F(context, format, ...)                         \
    do {                                                            \
        if(is_level_logging(LOGGING_ERROR)) {                       \
            V_MESSAGE_CONTEXT("[ERROR]  ", V_COLOR_ERROR, context); \
            PRINTF_F(format, __VA_ARGS__);                          \
        }                                                           \
    } while(0)

#define V_PRINT(format) PRINTF(format)
#define V_PRINT_F(format, ...) PRINTF_F(format, __VA_ARGS__)
#define V_PRINT_C(color, format)                                   \
    do {                                                           \
        if(use_color_logging()) { PRINTF_F("%s", color); }         \
        PRINTF(format);                                            \
        if(use_color_logging()) { PRINTF_F("%s", V_COLOR_RESET); } \
    } while(0)
#define V_PRINT_CF(color, format, ...)                             \
    do {                                                           \
        if(use_color_logging()) { PRINTF_F("%s", color); }         \
        PRINTF_F(format, __VA_ARGS__);                             \
        if(use_color_logging()) { PRINTF_F("%s", V_COLOR_RESET); } \
    } while(0)

VIRIATUM_EXPORT_PREFIX void set_level_logging(enum logging_level_e level);
VIRIATUM_EXPORT_PREFIX enum logging_level_e get_level_logging(void);
VIRIATUM_EXPORT_PREFIX int is_level_logging(enum logging_level_e level);
VIRIATUM_EXPORT_PREFIX int use_color_logging(void);
VIRIATUM_EXPORT_PREFIX void print_date_logging(void);
VIRIATUM_EXPORT_PREFIX void debug(const char *format, ...);
