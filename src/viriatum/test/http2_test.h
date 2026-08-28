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

#include "../http/http2.h"
#include "test_support.h"

/**
 * Tests the reading and the writing of the thirty two bit
 * values that the protocol carries in network order.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_number(void);

/**
 * Tests that a fresh set of settings carries the values that
 * the specification defines as the initial ones.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_settings(void);

/**
 * Tests the decoding of the header of a frame, including the
 * reserved bit of the stream and the buffer that does not yet
 * hold the complete frame.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_decode_frame(void);

/**
 * Tests the writing of the header of a frame, including the
 * refusal of a length that does not fit the field.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_encode_frame(void);

/**
 * Tests the removal of the padding of a frame, including the
 * padding that leaves no room for the content.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_padding(void);

/**
 * Tests the decoding of the priority information, both the
 * exclusive dependency and the offset of the weight.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_priority(void);

/**
 * Tests the application of a settings payload, covering every
 * one of the settings and the values that are refused.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_decode_settings(void);

/**
 * Tests the writing of the frames that this end produces, the
 * settings, the single valued ones and the goaway.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_encode_frames(void);

/**
 * Tests the coherence checks of a frame, covering the stream
 * a frame is allowed to belong to and the size that the
 * specification fixes for each of the types.
 *
 * @return A message describing the execution of
 * the unit test should describe possible errors.
 */
const char *test_http2_verify_frame(void);
