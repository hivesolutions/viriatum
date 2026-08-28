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

#include "../system/service.h"

/**
 * The magic value that is concatenated with the key sent by the
 * client in order to produce the accept value of the handshake,
 * as defined by the websocket specification.
 */
#define VIRIATUM_WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/**
 * The only version of the websocket protocol that is currently
 * supported by the server (the one of the final specification).
 */
#define VIRIATUM_WEBSOCKET_VERSION "13"

/**
 * The size of the buffer that must be provided for the reception
 * of the accept value, the encoded digest takes 28 characters.
 */
#define VIRIATUM_WEBSOCKET_ACCEPT_SIZE 32

/**
 * The maximum size of the payload of a single frame, anything
 * beyond this value is rejected as a protocol violation.
 */
#define VIRIATUM_WEBSOCKET_MAX_PAYLOAD 16777216

/**
 * The maximum size of the payload of a control frame, imposed by
 * the specification so that they are never fragmented.
 */
#define VIRIATUM_WEBSOCKET_MAX_CONTROL 125

/**
 * The maximum size taken by the header of a frame, reached by the
 * masked variant of the extended (64 bits) length one.
 */
#define VIRIATUM_WEBSOCKET_MAX_HEADER 14

/**
 * Enumeration describing the complete set of operation codes
 * defined by the websocket specification.
 */
typedef enum websocket_opcode_e {
    WEBSOCKET_OPCODE_CONTINUATION = 0x0,
    WEBSOCKET_OPCODE_TEXT = 0x1,
    WEBSOCKET_OPCODE_BINARY = 0x2,
    WEBSOCKET_OPCODE_CLOSE = 0x8,
    WEBSOCKET_OPCODE_PING = 0x9,
    WEBSOCKET_OPCODE_PONG = 0xa
} websocket_opcode;

/**
 * Enumeration describing the close codes that may be sent by the
 * server as part of the closing handshake.
 */
typedef enum websocket_close_e {
    WEBSOCKET_CLOSE_NORMAL = 1000,
    WEBSOCKET_CLOSE_GOING_AWAY = 1001,
    WEBSOCKET_CLOSE_PROTOCOL = 1002,
    WEBSOCKET_CLOSE_UNSUPPORTED = 1003,
    WEBSOCKET_CLOSE_NONE = 1005,
    WEBSOCKET_CLOSE_INVALID = 1007,
    WEBSOCKET_CLOSE_TOO_LARGE = 1009,
    WEBSOCKET_CLOSE_ERROR = 1011
} websocket_close;

/**
 * Structure describing a single frame as decoded from the stream
 * of data received from the client, the payload points into the
 * buffer that has been provided for the parsing.
 */
typedef struct websocket_frame_t {
    /**
     * Flag controlling if the current frame is the final one of
     * the message that it belongs to.
     */
    char fin;

    /**
     * The operation code of the frame, one of the values of the
     * operation code enumeration.
     */
    unsigned char opcode;

    /**
     * Flag controlling if the payload of the frame was masked,
     * mandatory for every frame sent by a client, together with
     * the key that has been used for the masking of it.
     */
    char masked;
    unsigned char mask[4];

    /**
     * The payload of the frame, already unmasked in place, and
     * the number of bytes that it takes.
     */
    unsigned char *payload;
    size_t payload_size;

    /**
     * The complete number of bytes taken by the frame, header
     * included, set to zero while the frame is incomplete.
     */
    size_t size;
} websocket_frame;

VIRIATUM_EXPORT_PREFIX ERROR_CODE accept_key_websocket(const unsigned char *key, unsigned char *accept_key, size_t accept_key_size);
VIRIATUM_EXPORT_PREFIX ERROR_CODE parse_frame_websocket(unsigned char *buffer, size_t buffer_size, struct websocket_frame_t *websocket_frame);
VIRIATUM_EXPORT_PREFIX ERROR_CODE build_frame_websocket(unsigned char opcode, char fin, const unsigned char *payload, size_t payload_size, unsigned char **buffer_pointer, size_t *buffer_size_pointer);
VIRIATUM_EXPORT_PREFIX ERROR_CODE build_close_websocket(unsigned short code, const char *reason, unsigned char **buffer_pointer, size_t *buffer_size_pointer);
VIRIATUM_EXPORT_PREFIX char is_control_websocket(unsigned char opcode);
VIRIATUM_EXPORT_PREFIX unsigned short close_code_websocket(const unsigned char *payload, size_t payload_size);
