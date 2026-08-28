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

#include "websocket_test.h"

const char *test_websocket_accept_key(void) {
    /* allocates space for the error code returned by the various
    calls and for the buffer receiving the accept value */
    ERROR_CODE error;
    unsigned char accept_key[VIRIATUM_WEBSOCKET_ACCEPT_SIZE];
    unsigned char small[8];

    /* tests that the example of the specification produces the
    expected accept value for the handshake response */
    error = accept_key_websocket(
        (unsigned char *) "dGhlIHNhbXBsZSBub25jZQ==",
        accept_key,
        sizeof(accept_key)
    );
    V_ASSERT(error == 0);
    V_ASSERT(strcmp((char *) accept_key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") == 0);

    /* tests that an empty key is rejected, as it may never be the
    result of the encoding of a valid nonce */
    error = accept_key_websocket(
        (unsigned char *) "",
        accept_key,
        sizeof(accept_key)
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a buffer that is unable to receive the complete
    accept value is rejected instead of being overflown */
    error = accept_key_websocket(
        (unsigned char *) "dGhlIHNhbXBsZSBub25jZQ==",
        small,
        sizeof(small)
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_websocket_parse_frame(void) {
    /* allocates space for the error code, for the frame structure
    and for the various buffers carrying the frames to be parsed */
    ERROR_CODE error;
    struct websocket_frame_t websocket_frame;
    size_t index;
    unsigned char buffer[16];
    unsigned char extended[200];

    /* the single frame masked text message carrying the hello
    payload, as provided by the specification */
    unsigned char hello[] = {
        0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d,
        0x7f, 0x9f, 0x4d, 0x51, 0x58
    };

    /* tests that a buffer that does not even carry the mandatory
    part of the header is reported as an incomplete frame */
    memcpy(buffer, hello, 1);
    error = parse_frame_websocket(buffer, 1, &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == 0);

    /* tests that the example of the specification is parsed into
    the expected payload, unmasked in place */
    memcpy(buffer, hello, sizeof(hello));
    error = parse_frame_websocket(buffer, sizeof(hello), &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == sizeof(hello));
    V_ASSERT(websocket_frame.fin == TRUE);
    V_ASSERT(websocket_frame.masked == TRUE);
    V_ASSERT(websocket_frame.opcode == WEBSOCKET_OPCODE_TEXT);
    V_ASSERT(websocket_frame.payload_size == 5);
    V_ASSERT(memcmp(websocket_frame.payload, "Hello", 5) == 0);

    /* tests that a frame whose payload is not completely available
    is reported as an incomplete one, nothing is unmasked */
    memcpy(buffer, hello, sizeof(hello));
    error = parse_frame_websocket(buffer, sizeof(hello) - 1, &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == 0);

    /* tests that a frame whose masking key is not completely
    available is also reported as an incomplete one */
    memcpy(buffer, hello, sizeof(hello));
    error = parse_frame_websocket(buffer, 4, &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == 0);

    /* tests that a frame that is not masked is rejected, every
    frame sent by a client is required to be masked */
    memcpy(buffer, hello, sizeof(hello));
    buffer[1] &= 0x7f;
    error = parse_frame_websocket(buffer, sizeof(hello), &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a frame carrying a set reserved bit is rejected
    as no extension is ever negotiated by the server */
    memcpy(buffer, hello, sizeof(hello));
    buffer[0] |= 0x40;
    error = parse_frame_websocket(buffer, sizeof(hello), &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a frame carrying an unknown operation code is
    rejected, as it may not be properly handled */
    memcpy(buffer, hello, sizeof(hello));
    buffer[0] = 0x83;
    error = parse_frame_websocket(buffer, sizeof(hello), &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a fragmented control frame is rejected, the
    specification forbids the fragmentation of them */
    memcpy(buffer, hello, sizeof(hello));
    buffer[0] = 0x09;
    error = parse_frame_websocket(buffer, sizeof(hello), &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a control frame carrying an oversized payload is
    rejected, they are bounded by the base length field */
    extended[0] = 0x88;
    extended[1] = 0xfe;
    extended[2] = 0x00;
    extended[3] = 0x7e;
    error = parse_frame_websocket(extended, 4, &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a frame whose extended length is not completely
    available is reported as an incomplete one */
    extended[0] = 0x82;
    extended[1] = 0xfe;
    error = parse_frame_websocket(extended, 3, &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == 0);

    /* builds a binary frame carrying a payload of one hundred and
    thirty bytes, forcing the two byte length variant */
    extended[0] = 0x82;
    extended[1] = 0xfe;
    extended[2] = 0x00;
    extended[3] = 0x82;
    extended[4] = 0x01;
    extended[5] = 0x02;
    extended[6] = 0x03;
    extended[7] = 0x04;
    for(index = 0; index < 130; index++) {
        extended[8 + index] = (unsigned char) (0x40 ^ extended[4 + index % 4]);
    }
    error = parse_frame_websocket(extended, 138, &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == 138);
    V_ASSERT(websocket_frame.opcode == WEBSOCKET_OPCODE_BINARY);
    V_ASSERT(websocket_frame.payload_size == 130);
    V_ASSERT(websocket_frame.payload[0] == 0x40);
    V_ASSERT(websocket_frame.payload[129] == 0x40);

    /* tests that a two byte length that would fit the base variant is
    rejected, the specification mandates the minimal encoding */
    extended[0] = 0x82;
    extended[1] = 0xfe;
    extended[2] = 0x00;
    extended[3] = 0x01;
    error = parse_frame_websocket(extended, 12, &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that an eight byte length that would fit the two byte
    variant is rejected for the very same reason */
    extended[0] = 0x82;
    extended[1] = 0xff;
    for(index = 0; index < 8; index++) { extended[2 + index] = 0x00; }
    extended[9] = 0xff;
    error = parse_frame_websocket(extended, 20, &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* tests that a frame whose eight byte length is not completely
    available is reported as an incomplete one */
    extended[0] = 0x82;
    extended[1] = 0xff;
    error = parse_frame_websocket(extended, 6, &websocket_frame);
    V_ASSERT(error == 0);
    V_ASSERT(websocket_frame.size == 0);

    /* tests that a frame announcing a payload beyond the maximum
    allowed one is rejected instead of being buffered */
    extended[0] = 0x82;
    extended[1] = 0xff;
    for(index = 0; index < 8; index++) { extended[2 + index] = 0x00; }
    extended[5] = 0xff;
    error = parse_frame_websocket(extended, 14, &websocket_frame);
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_websocket_build_frame(void) {
    /* allocates space for the error code and for the buffer that
    is going to receive the built frame */
    ERROR_CODE error;
    unsigned char *buffer;
    size_t buffer_size;
    unsigned char payload[70000];
    size_t index;

    /* tests that a small payload takes the base length variant
    and that the frame is never masked by the server */
    error = build_frame_websocket(
        WEBSOCKET_OPCODE_TEXT,
        TRUE,
        (unsigned char *) "Hello",
        5,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == 7);
    V_ASSERT(buffer[0] == 0x81);
    V_ASSERT(buffer[1] == 0x05);
    V_ASSERT(memcmp(&buffer[2], "Hello", 5) == 0);
    FREE(buffer);

    /* tests that a non final frame has the corresponding bit unset
    and that an empty payload is properly handled */
    error = build_frame_websocket(
        WEBSOCKET_OPCODE_CONTINUATION,
        FALSE,
        NULL,
        0,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == 2);
    V_ASSERT(buffer[0] == 0x00);
    V_ASSERT(buffer[1] == 0x00);
    FREE(buffer);

    /* tests that a payload beyond the base length variant takes the
    two byte one, the boundary is at one hundred and twenty six */
    for(index = 0; index < sizeof(payload); index++) {
        payload[index] = (unsigned char) (index & 0xff);
    }
    error = build_frame_websocket(
        WEBSOCKET_OPCODE_BINARY,
        TRUE,
        payload,
        126,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == 130);
    V_ASSERT(buffer[1] == 126);
    V_ASSERT(buffer[2] == 0x00);
    V_ASSERT(buffer[3] == 0x7e);
    FREE(buffer);

    /* tests that a payload beyond the two byte length variant takes
    the eight byte one, the boundary is at sixty five thousand */
    error = build_frame_websocket(
        WEBSOCKET_OPCODE_BINARY,
        TRUE,
        payload,
        70000,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == 70010);
    V_ASSERT(buffer[1] == 127);
    V_ASSERT(buffer[7] == 0x01);
    V_ASSERT(buffer[8] == 0x11);
    V_ASSERT(buffer[9] == 0x70);
    FREE(buffer);

    /* tests that a payload beyond the maximum allowed one is
    rejected instead of being allocated */
    error = build_frame_websocket(
        WEBSOCKET_OPCODE_BINARY,
        TRUE,
        payload,
        VIRIATUM_WEBSOCKET_MAX_PAYLOAD + 1,
        &buffer,
        &buffer_size
    );
    V_ASSERT(IS_ERROR_CODE(error));

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_websocket_build_close(void) {
    /* allocates space for the error code, for the buffer receiving
    the frame and for the oversized reason to be truncated */
    ERROR_CODE error;
    unsigned char *buffer;
    size_t buffer_size;
    char reason[256];
    size_t index;

    /* tests that a close frame carrying both a code and a reason
    lays them out in the expected order */
    error = build_close_websocket(
        WEBSOCKET_CLOSE_NORMAL,
        "bye",
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == 7);
    V_ASSERT(buffer[0] == 0x88);
    V_ASSERT(buffer[1] == 0x05);
    V_ASSERT(buffer[2] == 0x03);
    V_ASSERT(buffer[3] == 0xe8);
    V_ASSERT(memcmp(&buffer[4], "bye", 3) == 0);
    FREE(buffer);

    /* tests that a close frame without a reason carries only the
    two bytes taken by the close code */
    error = build_close_websocket(
        WEBSOCKET_CLOSE_ERROR,
        NULL,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == 4);
    V_ASSERT(buffer[1] == 0x02);
    V_ASSERT(buffer[2] == 0x03);
    V_ASSERT(buffer[3] == 0xf3);
    FREE(buffer);

    /* tests that an oversized reason is truncated so that the
    resulting payload still fits a control frame */
    for(index = 0; index < sizeof(reason) - 1; index++) { reason[index] = 'a'; }
    reason[sizeof(reason) - 1] = '\0';
    error = build_close_websocket(
        WEBSOCKET_CLOSE_PROTOCOL,
        reason,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == VIRIATUM_WEBSOCKET_MAX_CONTROL + 2);
    V_ASSERT(buffer[1] == VIRIATUM_WEBSOCKET_MAX_CONTROL);
    FREE(buffer);

    /* tests that an oversized reason is never cut in the middle of a
    code point, the single character before the sequences makes the
    truncation fall inside one of them, so it is dropped as a whole
    and the resulting payload is two bytes shorter than the maximum */
    reason[0] = 'a';
    for(index = 1; index < sizeof(reason) - 3; index += 3) {
        reason[index] = (char) 0xe6;
        reason[index + 1] = (char) 0x97;
        reason[index + 2] = (char) 0xa5;
    }
    reason[sizeof(reason) - 1] = '\0';
    error = build_close_websocket(
        WEBSOCKET_CLOSE_PROTOCOL,
        reason,
        &buffer,
        &buffer_size
    );
    V_ASSERT(error == 0);
    V_ASSERT(buffer_size == VIRIATUM_WEBSOCKET_MAX_CONTROL);
    V_ASSERT(buffer[1] == VIRIATUM_WEBSOCKET_MAX_CONTROL - 2);
    V_ASSERT(((unsigned char) buffer[buffer_size - 1] & 0xc0) == 0x80);
    FREE(buffer);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_websocket_is_control(void) {
    /* tests that the data operation codes are not classified as
    control ones, they are the fragmentable ones */
    V_ASSERT(is_control_websocket(WEBSOCKET_OPCODE_CONTINUATION) == FALSE);
    V_ASSERT(is_control_websocket(WEBSOCKET_OPCODE_TEXT) == FALSE);
    V_ASSERT(is_control_websocket(WEBSOCKET_OPCODE_BINARY) == FALSE);

    /* tests that the control operation codes are properly detected
    as such, they are the ones with the high bit set */
    V_ASSERT(is_control_websocket(WEBSOCKET_OPCODE_CLOSE) == TRUE);
    V_ASSERT(is_control_websocket(WEBSOCKET_OPCODE_PING) == TRUE);
    V_ASSERT(is_control_websocket(WEBSOCKET_OPCODE_PONG) == TRUE);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_websocket_close_code(void) {
    /* allocates space for the payload carrying the close code to
    be extracted from it */
    unsigned char payload[] = {0x03, 0xf3};

    /* tests that a payload that does not carry a code is reported
    through the reserved no status value */
    V_ASSERT(close_code_websocket(NULL, 0) == WEBSOCKET_CLOSE_NONE);
    V_ASSERT(close_code_websocket(payload, 1) == WEBSOCKET_CLOSE_NONE);

    /* tests that a payload carrying a code has it unpacked from the
    first two bytes of it, laid out in network order */
    V_ASSERT(close_code_websocket(payload, 2) == WEBSOCKET_CLOSE_ERROR);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
