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

#include "../http/websocket.h"
#include "test_support.h"

/**
 * Tests the derivation of the accept value of the
 * handshake from the key provided by the client.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_websocket_accept_key(void);

/**
 * Tests the parsing of a frame including the various
 * length variants and the protocol violations.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_websocket_parse_frame(void);

/**
 * Tests the building of a frame verifying that the
 * smallest length variant is always used.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_websocket_build_frame(void);

/**
 * Tests the building of a close frame including the
 * truncation of an oversized reason.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_websocket_build_close(void);

/**
 * Tests the classification of the various operation
 * codes as either control or data ones.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_websocket_is_control(void);

/**
 * Tests the extraction of the close code from the
 * payload of a close frame.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_websocket_close_code(void);
