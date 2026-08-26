/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../base/entry.h"
#include "../system/service.h"
#include "test_support.h"

/**
 * Tests that the service deletion tolerates an
 * unset (null) service reference.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_delete_service(void);

/**
 * Tests the service options creation and the
 * default value initialization of every field.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_create_service_options(void);

/**
 * Tests the locations calculation against a service
 * that has no configuration currently loaded.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_calculate_locations_service(void);

/**
 * Tests the file based options loading from a directory
 * where no configuration file may be reached.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_options_service(void);

/**
 * Tests that stopping the service tolerates a service
 * that has not been initialized yet.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_ran_service(void);
