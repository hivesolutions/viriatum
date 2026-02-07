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

/**
 * The date as year, month and day for the
 * start of the compilation process.
 */
#define VIRIATUM_COMPILATION_DATE __DATE__

/**
 * The time as a string containing hour
 * minute and second for the start of the
 * compilation process.
 */
#define VIRIATUM_COMPILATION_TIME __TIME__

/**
 * The string containing the description
 * of the flags used in the compilation.
 */
#ifdef CFLAGS
#define VIRIATUM_COMPILATION_FLAGS CFLAGS
#else
#define VIRIATUM_COMPILATION_FLAGS ""
#endif
