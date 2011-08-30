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

#ifdef VIRIATUM_DEBUG
#define V_DEBUG(format) printf("[DEBUG] "); printf(format)
#define V_DEBUG_F(format, ...) printf("[DEBUG] "); printf(format, __VA_ARGS__)
#endif

#ifndef VIRIATUM_DEBUG
#define V_DEBUG(format) dump(format)
#define V_DEBUG_F(format, ...) dumpMultiple(format, __VA_ARGS__)
#endif

#define V_WARNING(format) printf("[WARNING] "); printf(format)
#define V_WARNING_F(format, ...) printf("[WARNING] "); printf(format, __VA_ARGS__)

#define V_ERROR(format) printf("[ERROR] "); printf(format)
#define V_ERROR_F(format, ...) printf("[ERROR] "); printf(format, __VA_ARGS__)

#define V_PRINT(format) printf(format)
#define V_PRINT_F(format, ...) printf(format, __VA_ARGS__)

VIRIATUM_EXPORT_PREFIX void debug(const char *format, ...);
