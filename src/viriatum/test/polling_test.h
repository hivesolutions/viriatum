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

#include "test_support.h"

/**
 * Tests the registering of a connection with the mechanism
 * the service waits through, together with the taking of it
 * out again once it is no longer being served.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_connection(void);

/**
 * Tests the registering and the unregistering of the interest
 * in reading from a connection, which is what the serving of
 * a message turns on and off as it goes.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_read(void);

/**
 * Tests the registering of the interest in writing to a
 * connection, which is held apart from the one of reading.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_write(void);

/**
 * Tests that a connection with something waiting on it is
 * handed back by the waiting and that the handler of the
 * reading is called for it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_event(void);

/**
 * Tests that a peer which goes away without saying so is
 * reported by the mechanism, either as something to read
 * that reads as nothing or as an error of the connection.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_closed(void);

/**
 * Tests that a descriptor which goes away behind the back
 * of the mechanism is reported rather than taking the
 * serving of everything else down with it.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_gone(void);

/**
 * Tests the connections that are pending an operation at the
 * beginning of a cycle, which are driven before the waiting.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_outstanding(void);

/**
 * Tests that a connection taken out of the service while it
 * still has an operation pending is not driven again on the
 * cycle that follows, by then it has already been released.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_polling_discarded(void);
