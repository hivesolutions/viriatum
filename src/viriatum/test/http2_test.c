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

#include "stdafx.h"

#include "http2_test.h"

const char *test_http2_number(void) {
    /* allocates space for the buffer that carries the value in the
    network order that the protocol uses */
    unsigned char buffer[4];

    /* the largest value that a stream identifier is allowed to take
    survives the round trip untouched */
    encode_number_http2(buffer, (unsigned int) HTTP2_MAX_STREAM_ID);
    V_ASSERT_EQ_U(buffer[0], 0x7f);
    V_ASSERT_EQ_U(buffer[1], 0xff);
    V_ASSERT_EQ_U(buffer[2], 0xff);
    V_ASSERT_EQ_U(buffer[3], 0xff);
    V_ASSERT_EQ_U(decode_number_http2(buffer), (unsigned int) HTTP2_MAX_STREAM_ID);

    /* a value that uses the most significant bit is carried just as
    faithfully, the reading never treats it as a sign */
    encode_number_http2(buffer, 0x80000001);
    V_ASSERT_EQ_U(buffer[0], 0x80);
    V_ASSERT_EQ_U(buffer[3], 0x01);
    V_ASSERT_EQ_U(decode_number_http2(buffer), 0x80000001);

    /* the value zero is carried as four empty bytes */
    encode_number_http2(buffer, 0);
    V_ASSERT_EQ_U(buffer[0], 0);
    V_ASSERT_EQ_U(decode_number_http2(buffer), 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_settings(void) {
    /* allocates space for the settings that are going to receive the
    values of the specification */
    struct http2_settings_t http2_settings;

    /* creates the settings and verifies that every one of them holds
    the value that the specification defines */
    create_settings_http2(&http2_settings);
    V_ASSERT_EQ_U(http2_settings.header_table_size, HPACK_TABLE_SIZE);
    V_ASSERT(http2_settings.enable_push == TRUE);
    V_ASSERT_EQ_U(http2_settings.max_concurrent_streams, HTTP2_MAX_CONCURRENT);
    V_ASSERT_EQ_U(http2_settings.initial_window_size, HTTP2_DEFAULT_WINDOW_SIZE);
    V_ASSERT_EQ_U(http2_settings.max_frame_size, HTTP2_DEFAULT_FRAME_SIZE);
    V_ASSERT_EQ_U(http2_settings.max_header_list_size, HPACK_MAX_HEADER_LIST_SIZE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_decode_frame(void) {
    /* allocates space for the buffer that carries the frame and for
    the structure that receives the decoding of it */
    unsigned char buffer[16];
    struct http2_frame_t http2_frame;
    ERROR_CODE error;

    /* a complete frame carrying a payload of two bytes on the first
    stream that a client is allowed to open */
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x02;
    buffer[3] = HTTP2_DATA;
    buffer[4] = HTTP2_FLAG_END_STREAM;
    buffer[5] = 0x00;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x01;
    buffer[9] = 'o';
    buffer[10] = 'k';

    error = decode_frame_http2(buffer, 11, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.length, 2);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_DATA);
    V_ASSERT_EQ_U(http2_frame.flags, HTTP2_FLAG_END_STREAM);
    V_ASSERT_EQ_U(http2_frame.stream_id, 1);
    V_ASSERT_NOT_NULL(http2_frame.payload);
    V_ASSERT_MEM(http2_frame.payload, "ok", 2);

    /* a buffer that holds the header but not the complete payload
    leaves the payload unset, telling the caller to wait */
    error = decode_frame_http2(buffer, 10, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.length, 2);
    V_ASSERT_NULL(http2_frame.payload);

    /* a buffer smaller than the header itself carries nothing that
    may be decoded at all */
    error = decode_frame_http2(buffer, HTTP2_HEADER_SIZE - 1, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* the most significant bit of the stream field is reserved and
    has to be ignored rather than taken as part of the identifier */
    buffer[5] = 0xff;
    buffer[6] = 0xff;
    buffer[7] = 0xff;
    buffer[8] = 0xff;
    error = decode_frame_http2(buffer, 11, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.stream_id, (unsigned int) HTTP2_MAX_STREAM_ID);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_encode_frame(void) {
    /* allocates space for the buffer that receives the header of the
    frame being written */
    unsigned char buffer[16];
    struct http2_frame_t http2_frame;
    ERROR_CODE error;

    /* writes a header and verifies that decoding it produces the
    very same values that have been written */
    error = encode_frame_http2(buffer, sizeof(buffer), 4, HTTP2_WINDOW_UPDATE, 0x00, 3);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    buffer[9] = 0x00;
    buffer[10] = 0x00;
    buffer[11] = 0x00;
    buffer[12] = 0x01;
    error = decode_frame_http2(buffer, 13, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.length, 4);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_WINDOW_UPDATE);
    V_ASSERT_EQ_U(http2_frame.stream_id, 3);

    /* the reserved bit is never set on the wire, even when the value
    handed over carries it */
    error = encode_frame_http2(buffer, sizeof(buffer), 0, HTTP2_PING, 0x00, 0xffffffff);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(buffer[5], 0x7f);

    /* a length that does not fit in the three bytes of the field is
    refused rather than being truncated */
    error = encode_frame_http2(buffer, sizeof(buffer), HTTP2_MAX_FRAME_SIZE + 1, HTTP2_DATA, 0x00, 1);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* a buffer that has no room for the header is refused */
    error = encode_frame_http2(buffer, HTTP2_HEADER_SIZE - 1, 0, HTTP2_DATA, 0x00, 1);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_padding(void) {
    /* allocates space for the frame being stripped and for the
    payload that the operation reports */
    struct http2_frame_t http2_frame;
    unsigned char payload[8];
    unsigned char *result;
    size_t result_size;
    ERROR_CODE error;

    /* a frame without the flag carries no padding at all, so the
    payload it reports is the complete one */
    http2_frame.flags = 0x00;
    http2_frame.length = 3;
    http2_frame.payload = payload;
    payload[0] = 'a';
    payload[1] = 'b';
    payload[2] = 'c';
    error = strip_padding_http2(&http2_frame, &result, &result_size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(result_size, 3);
    V_ASSERT_EQ_P(result, payload);

    /* a padded frame reports the content that sits between the byte
    of the length and the padding itself */
    http2_frame.flags = HTTP2_FLAG_PADDED;
    http2_frame.length = 5;
    payload[0] = 0x02;
    payload[1] = 'a';
    payload[2] = 'b';
    payload[3] = 0x00;
    payload[4] = 0x00;
    error = strip_padding_http2(&http2_frame, &result, &result_size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(result_size, 2);
    V_ASSERT_MEM(result, "ab", 2);

    /* a padding that leaves no content at all is still valid, the
    byte of the length and the padding may fill the payload */
    http2_frame.length = 3;
    payload[0] = 0x02;
    error = strip_padding_http2(&http2_frame, &result, &result_size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(result_size, 0);

    /* a padding as long as the payload itself leaves no room for the
    byte that carries its own length and is refused */
    http2_frame.length = 3;
    payload[0] = 0x03;
    error = strip_padding_http2(&http2_frame, &result, &result_size);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* an empty payload cannot carry the byte that holds the length
    of the padding */
    http2_frame.length = 0;
    error = strip_padding_http2(&http2_frame, &result, &result_size);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a padding of no bytes at all is still valid, only the byte
    that carries the length is deducted */
    http2_frame.length = 3;
    payload[0] = 0x00;
    error = strip_padding_http2(&http2_frame, &result, &result_size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(result_size, 2);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_priority(void) {
    /* allocates space for the buffer that carries the priority and
    for the structure that receives the decoding of it */
    unsigned char buffer[HTTP2_PRIORITY_SIZE];
    struct http2_priority_t http2_priority;
    ERROR_CODE error;

    /* a plain dependency on the root of the tree carrying the
    default weight, which travels deducted of a unit */
    encode_number_http2(buffer, 0);
    buffer[4] = HTTP2_DEFAULT_WEIGHT - 1;
    error = decode_priority_http2(buffer, sizeof(buffer), &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_priority.dependency, 0);
    V_ASSERT_EQ_U(http2_priority.weight, HTTP2_DEFAULT_WEIGHT);
    V_ASSERT(http2_priority.exclusive == FALSE);

    /* an exclusive dependency sets the most significant bit of the
    field, which is not part of the identifier */
    encode_number_http2(buffer, 0x80000005);
    buffer[4] = 0xff;
    error = decode_priority_http2(buffer, sizeof(buffer), &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_priority.dependency, 5);
    V_ASSERT_EQ_U(http2_priority.weight, 256);
    V_ASSERT(http2_priority.exclusive == TRUE);

    /* the priority information is of a fixed size, anything else is
    a frame size error */
    error = decode_priority_http2(buffer, sizeof(buffer) - 1, &http2_priority);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_decode_settings(void) {
    /* allocates space for the payload of the frame and for the
    settings that it is going to change */
    unsigned char payload[HTTP2_SETTING_SIZE * 3];
    struct http2_settings_t http2_settings;
    ERROR_CODE error;

    /* creates the settings so that the changes are observed against
    the values of the specification */
    create_settings_http2(&http2_settings);

    /* an empty payload changes nothing at all, which is the shape of
    a settings frame that only opens the connection */
    error = decode_settings_http2(payload, 0, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_settings.max_frame_size, HTTP2_DEFAULT_FRAME_SIZE);

    /* changes the three settings that a peer most commonly sends,
    each entry carries the identifier and the value */
    payload[0] = 0x00;
    payload[1] = HTTP2_SETTING_HEADER_TABLE_SIZE;
    encode_number_http2(&payload[2], 2048);
    payload[6] = 0x00;
    payload[7] = HTTP2_SETTING_INITIAL_WINDOW_SIZE;
    encode_number_http2(&payload[8], 131072);
    payload[12] = 0x00;
    payload[13] = HTTP2_SETTING_MAX_FRAME_SIZE;
    encode_number_http2(&payload[14], 32768);
    error = decode_settings_http2(payload, sizeof(payload), &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_settings.header_table_size, 2048);
    V_ASSERT_EQ_U(http2_settings.initial_window_size, 131072);
    V_ASSERT_EQ_U(http2_settings.max_frame_size, 32768);

    /* the remaining two settings are carried in the very same way */
    payload[1] = HTTP2_SETTING_ENABLE_PUSH;
    encode_number_http2(&payload[2], 0);
    payload[7] = HTTP2_SETTING_MAX_CONCURRENT_STREAMS;
    encode_number_http2(&payload[8], 42);
    payload[13] = HTTP2_SETTING_MAX_HEADER_LIST_SIZE;
    encode_number_http2(&payload[14], 8192);
    error = decode_settings_http2(payload, sizeof(payload), &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT(http2_settings.enable_push == FALSE);
    V_ASSERT_EQ_U(http2_settings.max_concurrent_streams, 42);
    V_ASSERT_EQ_U(http2_settings.max_header_list_size, 8192);

    /* an identifier that is not recognised is ignored rather than
    refused, which is what allows the protocol to be extended */
    payload[1] = 0xff;
    encode_number_http2(&payload[2], 1);
    error = decode_settings_http2(payload, HTTP2_SETTING_SIZE, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* a payload whose size is not a multiple of the size of an entry
    cannot be walked and is a frame size error */
    error = decode_settings_http2(payload, HTTP2_SETTING_SIZE + 1, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* the push setting is a flag, so a value that is neither of the
    two accepted ones is a protocol error */
    payload[1] = HTTP2_SETTING_ENABLE_PUSH;
    encode_number_http2(&payload[2], 2);
    error = decode_settings_http2(payload, HTTP2_SETTING_SIZE, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* a window above the largest one the protocol represents is a
    flow control error */
    payload[1] = HTTP2_SETTING_INITIAL_WINDOW_SIZE;
    encode_number_http2(&payload[2], 0x80000000);
    error = decode_settings_http2(payload, HTTP2_SETTING_SIZE, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FLOW_CONTROL_ERROR);

    /* a frame size below the one that every peer is required to
    accept is a protocol error */
    payload[1] = HTTP2_SETTING_MAX_FRAME_SIZE;
    encode_number_http2(&payload[2], HTTP2_DEFAULT_FRAME_SIZE - 1);
    error = decode_settings_http2(payload, HTTP2_SETTING_SIZE, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* and so is one above what the length field is able to hold */
    encode_number_http2(&payload[2], HTTP2_MAX_FRAME_SIZE + 1);
    error = decode_settings_http2(payload, HTTP2_SETTING_SIZE, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_encode_frames(void) {
    /* allocates space for the buffer that receives the frames and
    for the structures used to read them back */
    unsigned char buffer[64];
    struct http2_settings_t http2_settings;
    struct http2_settings_t decoded;
    struct http2_frame_t http2_frame;
    size_t size;
    ERROR_CODE error;

    /* creates the settings of this end and writes them as a frame,
    which is what opens a connection */
    create_settings_http2(&http2_settings);
    http2_settings.max_concurrent_streams = 64;
    error = encode_settings_http2(buffer, sizeof(buffer), &http2_settings, &size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(size, HTTP2_HEADER_SIZE + HTTP2_SETTING_SIZE * 3);

    /* the frame that has been written decodes into the very same
    values that have been announced */
    error = decode_frame_http2(buffer, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_SETTINGS);
    V_ASSERT_EQ_U(http2_frame.stream_id, 0);
    V_ASSERT_EQ_U(http2_frame.flags, 0);

    create_settings_http2(&decoded);
    error = decode_settings_http2(http2_frame.payload, http2_frame.length, &decoded);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(decoded.max_concurrent_streams, 64);
    V_ASSERT_EQ_U(decoded.header_table_size, HPACK_TABLE_SIZE);
    V_ASSERT_EQ_U(decoded.max_header_list_size, HPACK_MAX_HEADER_LIST_SIZE);

    /* a buffer that has no room for the complete frame is refused */
    error = encode_settings_http2(buffer, HTTP2_HEADER_SIZE, &http2_settings, &size);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* the reset of a stream carries the error code as its complete
    payload, which is the shape of a single valued frame */
    error = encode_value_http2(buffer, sizeof(buffer), HTTP2_RST_STREAM, 5, HTTP2_CANCEL, &size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(size, HTTP2_HEADER_SIZE + 4);
    error = decode_frame_http2(buffer, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_RST_STREAM);
    V_ASSERT_EQ_U(http2_frame.stream_id, 5);
    V_ASSERT_EQ_U(decode_number_http2(http2_frame.payload), HTTP2_CANCEL);

    /* a buffer that has no room for the value is refused */
    error = encode_value_http2(buffer, HTTP2_HEADER_SIZE + 3, HTTP2_RST_STREAM, 5, 0, &size);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* the goaway carries the last stream that has been handled
    together with the reason the connection is being closed */
    error = encode_goaway_http2(buffer, sizeof(buffer), 7, HTTP2_PROTOCOL_ERROR, &size);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(size, HTTP2_HEADER_SIZE + 8);
    error = decode_frame_http2(buffer, size, &http2_frame);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    V_ASSERT_EQ_U(http2_frame.type, HTTP2_GOAWAY);
    V_ASSERT_EQ_U(http2_frame.stream_id, 0);
    V_ASSERT_EQ_U(decode_number_http2(http2_frame.payload), 7);
    V_ASSERT_EQ_U(decode_number_http2(&http2_frame.payload[4]), HTTP2_PROTOCOL_ERROR);

    /* a buffer that has no room for the goaway is refused */
    error = encode_goaway_http2(buffer, HTTP2_HEADER_SIZE + 7, 7, 0, &size);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_http2_verify_frame(void) {
    /* allocates space for the frame being verified and for the
    settings that bound the size of it */
    struct http2_frame_t http2_frame;
    struct http2_settings_t http2_settings;
    ERROR_CODE error;

    /* creates the settings, they carry the largest size that a frame
    is allowed to take on this end */
    create_settings_http2(&http2_settings);

    /* a frame larger than what has been announced is refused before
    anything at all is done with it */
    http2_frame.type = HTTP2_DATA;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 1;
    http2_frame.length = HTTP2_DEFAULT_FRAME_SIZE + 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* the frames that belong to a stream are refused when they carry
    the identifier of the connection */
    http2_frame.length = 0;
    http2_frame.stream_id = 0;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    http2_frame.type = HTTP2_HEADERS;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    http2_frame.type = HTTP2_CONTINUATION;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    http2_frame.type = HTTP2_PUSH_PROMISE;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);

    /* the priority carries a payload of a fixed size and belongs to
    a stream, both of the conditions are verified */
    http2_frame.type = HTTP2_PRIORITY;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);
    http2_frame.stream_id = 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);
    http2_frame.length = HTTP2_PRIORITY_SIZE;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the reset of a stream carries a single value and belongs to a
    stream just the same */
    http2_frame.type = HTTP2_RST_STREAM;
    http2_frame.stream_id = 0;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);
    http2_frame.stream_id = 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);
    http2_frame.length = 4;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the settings belong to the connection and their payload is a
    sequence of entries of a fixed size */
    http2_frame.type = HTTP2_SETTINGS;
    http2_frame.length = 0;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);
    http2_frame.stream_id = 0;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    http2_frame.length = HTTP2_SETTING_SIZE + 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);

    /* an acknowledgement of the settings never carries a payload */
    http2_frame.flags = HTTP2_FLAG_ACK;
    http2_frame.length = HTTP2_SETTING_SIZE;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);
    http2_frame.length = 0;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the ping belongs to the connection and its payload is of a
    size that the specification fixes */
    http2_frame.type = HTTP2_PING;
    http2_frame.flags = 0x00;
    http2_frame.stream_id = 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);
    http2_frame.stream_id = 0;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);
    http2_frame.length = HTTP2_PING_SIZE;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the goaway belongs to the connection and carries at least the
    last stream and the error code */
    http2_frame.type = HTTP2_GOAWAY;
    http2_frame.stream_id = 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_PROTOCOL_ERROR);
    http2_frame.stream_id = 0;
    http2_frame.length = 7;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);
    http2_frame.length = 8;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* the update of a window applies to either a stream or the
    connection, so only its size is verified */
    http2_frame.type = HTTP2_WINDOW_UPDATE;
    http2_frame.length = 5;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_FRAME_SIZE_ERROR);
    http2_frame.length = 4;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);
    http2_frame.stream_id = 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* a frame of an unknown type is accepted so that it may be
    ignored, which is what allows the protocol to be extended */
    http2_frame.type = 0xff;
    http2_frame.length = 3;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* a data frame on a stream is the ordinary case and passes every
    one of the verifications */
    http2_frame.type = HTTP2_DATA;
    http2_frame.stream_id = 1;
    error = verify_frame_http2(&http2_frame, &http2_settings);
    V_ASSERT_EQ_U(error, HTTP2_NO_ERROR);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
