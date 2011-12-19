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

#include "../debug/debug.h"
#include "../structures/structures.h"

/**
 * Enumeration describing the various types
 * of possible arguments.
 */
typedef enum ArgumentType_e {
	/**
	 * An argument that contains only
	 * a key and so the value is considered
	 * to be null.
	 */
	SINGLE_ARGUMENT = 1,

	/**
	 * An argument that contains both
	 * a key and a value.
	 */
	VALUE_ARGUMENT
} ArgumentType;

/**
 * Structure defining an argument received
 * from an external (data) source.
 */
typedef struct Argument_t {
	/**
	 * The type of argument, to control
	 * if the value is defined or not.
	 */
	enum ArgumentType_e type;

	/**
	 * The key to the argument, this value
	 * should also describe the argument.
	 */
	char key[256];

	/**
	 * The value content of the argument.
	 * This value is represented as a string
	 * and must be parsed for interpretation.
	 */
	char value[1024];
} Argument;
