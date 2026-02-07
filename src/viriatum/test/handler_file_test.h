/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

/**
 * Tests the handler file context creation
 * and default value initialization.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_context();

/**
 * Tests the handler file url callback including
 * query string stripping and path traversal rejection.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_url();

/**
 * Tests the handler file header field callback
 * for correct recognition of known HTTP headers.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_header_field();

/**
 * Tests the handler file header value callback
 * for correct storage of header values in context.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_file_header_value();
