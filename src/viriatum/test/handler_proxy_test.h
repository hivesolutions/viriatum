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

#include "../handlers/handler_proxy.h"
#include "../http/http_parser.h"
#include "test_support.h"

/**
 * Tests the gathering of the request of a client into the buffer
 * that is handed to the upstream, including the growing of it past
 * the size it starts its life with.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_proxy_request(void);

/**
 * Tests the writing of the response of an upstream into the one of
 * the client, verifying that it is encoded in the protocol that is
 * serving the client rather than in the one of the upstream.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_proxy_response(void);

/**
 * Tests the response of a request that reaches no upstream at all,
 * the client is told that the gateway is the one at fault.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_proxy_gateway(void);

/**
 * Tests the response of an upstream as it travels on the wire,
 * taken apart by the parser of the backend and written into the
 * response of the client of the proxy.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_proxy_upstream(void);

/**
 * Tests the setting, the resetting and the unsetting of the
 * handler, together with the matching of the location that names
 * the upstream a message is handed to.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_handler_proxy_handler(void);
