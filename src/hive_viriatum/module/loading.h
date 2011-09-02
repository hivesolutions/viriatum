/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

/**
 * Function used for starting a module.
 * During this initialization all the module
 * internal structures shall be initialized.
 */
typedef ERROR_CODE (*viriatumStartModule)(void);

/**
 * Function used for stopping a module.
 * During this destruction all the module
 * internal structures shall be closed and
 * destroyed.
 */
typedef ERROR_CODE (*viriatumStopModule)(void);

/**
 * Loads the module in the given path.
 * The loading of the module implies finding
 * the appropriate file and loading the symbols.
 *
 * @param modulePath The path to the file
 * containing the module.
 * @return The resulting error code.
 */
ERROR_CODE loadModule(unsigned char *modulePath);
