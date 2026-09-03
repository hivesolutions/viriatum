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
 * The port to be bound by the service lifecycle test, an
 * unlikely value so that no conflict arises.
 */
#define VIRIATUM_TEST_PORT 19399

/**
 * The address used by the failed opening test, taken from the
 * range reserved for documentation so that it is assigned to
 * no interface and the binding to it always fails.
 */
#define VIRIATUM_TEST_ADDRESS "192.0.2.1"

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
 * Tests the resolving of the directory that the binary
 * of the process sits in.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_base_path_service(void);

/**
 * Tests the preferring of a directory that sits beside
 * the binary over the one built into the binary.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_bundled_path_service(void);

/**
 * Tests the resolving of the paths of a service, both
 * the ones taken from the tree and the ones that a web
 * root set on the options overrides.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_calculate_options_service(void);

/**
 * Tests the locations calculation against a service
 * that has no configuration currently loaded.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_calculate_locations_service(void);

/**
 * Tests the opening, polling and closing of a service
 * through the step based lifecycle functions.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_open_close_service(void);

/**
 * Tests that a failed opening of a service leaves it in
 * a properly closed state.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_open_service_busy(void);

/**
 * Tests the file based options loading from a directory
 * where no configuration file may be reached.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_file_options_service(void);

/**
 * Tests the options that the arguments of the command line carry,
 * including the one that turns the serving of the cleartext form
 * of the most recent version of the protocol off.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_arguments_options_service(void);

/**
 * Tests the flags that the banner of the startup carries, they
 * describe the build and decide what a connection looks at when it
 * opens.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
/**
 * Tests the naming of the mechanism that the service
 * waits on its connections through, which the status
 * page reads off the service to show which is in use.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_service(void);

const char *test_flags_service(void);

/**
 * Tests that stopping the service tolerates a service
 * that has not been initialized yet.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_ran_service(void);
