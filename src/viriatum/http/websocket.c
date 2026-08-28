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

#include "websocket.h"

ERROR_CODE accept_key_websocket(const unsigned char *key, unsigned char *accept_key, size_t accept_key_size) {
    /* allocates space for the buffer that is going to hold the
    concatenation of the key and of the magic value, together with
    the digest and the encoded representation of it */
    unsigned char *buffer;
    unsigned char digest[20];
    unsigned char *encoded;
    size_t encoded_size;
    size_t key_size = strlen((char *) key);
    size_t buffer_size = key_size + sizeof(VIRIATUM_WEBSOCKET_GUID) - 1;

    /* verifies that the key provided by the client is not larger than
    a header may be, avoiding an unbounded allocation driven by it */
    if(key_size == 0 || key_size > VIRIATUM_MAX_HEADER_SIZE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Invalid websocket key"
        );
    }

    /* concatenates the key with the magic value and runs the digest
    over the resulting buffer, as defined by the specification */
    buffer = (unsigned char *) MALLOC(buffer_size);
    memcpy(buffer, key, key_size);
    memcpy(&buffer[key_size], VIRIATUM_WEBSOCKET_GUID, sizeof(VIRIATUM_WEBSOCKET_GUID) - 1);
    sha1(buffer, (unsigned int) buffer_size, digest);
    FREE(buffer);

    /* encodes the digest into its base 64 representation, the result
    of it is the value of the accept header of the response */
    encode_base64(digest, sizeof(digest), &encoded, &encoded_size);
    if(encoded_size >= accept_key_size) {
        FREE(encoded);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Buffer is too small for the accept key"
        );
    }

    /* copies the encoded value into the provided buffer closing it
    with the end of string character and releases the encoded one */
    memcpy(accept_key, encoded, encoded_size);
    accept_key[encoded_size] = '\0';
    FREE(encoded);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE parse_frame_websocket(unsigned char *buffer, size_t buffer_size, struct websocket_frame_t *websocket_frame) {
    /* allocates space for the various values that are gathered from
    the header of the frame and for the iteration index */
    size_t index;
    size_t offset = 2;
    unsigned long long payload_size;

    /* resets the frame so that an incomplete parsing is reported as
    a zero sized one, no data is consumed in that situation */
    memset(websocket_frame, 0, sizeof(struct websocket_frame_t));

    /* in case not even the mandatory part of the header is available
    the frame is still incomplete (more data is required) */
    if(buffer_size < 2) { RAISE_NO_ERROR; }

    /* unpacks the first byte of the header, the reserved bits must be
    unset as no extension is ever negotiated by the server */
    websocket_frame->fin = buffer[0] & 0x80 ? TRUE : FALSE;
    websocket_frame->opcode = buffer[0] & 0x0f;
    if(buffer[0] & 0x70) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Reserved bits are set in the frame"
        );
    }

    /* verifies that the operation code is one of the defined ones, an
    unknown one may not be handled and so it's a protocol violation */
    switch(websocket_frame->opcode) {
        case WEBSOCKET_OPCODE_CONTINUATION:
        case WEBSOCKET_OPCODE_TEXT:
        case WEBSOCKET_OPCODE_BINARY:
        case WEBSOCKET_OPCODE_CLOSE:
        case WEBSOCKET_OPCODE_PING:
        case WEBSOCKET_OPCODE_PONG:
            break;

        default:
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Unknown opcode in the frame"
            );
    }

    /* unpacks the second byte of the header, the mask flag must be set
    as every frame sent by a client is required to be masked */
    websocket_frame->masked = buffer[1] & 0x80 ? TRUE : FALSE;
    payload_size = (unsigned long long) (buffer[1] & 0x7f);
    if(websocket_frame->masked == FALSE) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Received an unmasked frame from the client"
        );
    }

    /* reads the extended variants of the length, the first one takes
    two bytes and the second one eight, both in network order */
    if(payload_size == 126) {
        if(buffer_size < offset + 2) { RAISE_NO_ERROR; }
        payload_size = ((unsigned long long) buffer[2] << 8) |
            (unsigned long long) buffer[3];
        offset += 2;
        if(payload_size < 126) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Length is not minimally encoded"
            );
        }
    } else if(payload_size == 127) {
        if(buffer_size < offset + 8) { RAISE_NO_ERROR; }
        payload_size = 0;
        for(index = 0; index < 8; index++) {
            payload_size = (payload_size << 8) |
                (unsigned long long) buffer[offset + index];
        }
        offset += 8;
        if(payload_size <= 65535) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Length is not minimally encoded"
            );
        }
    }

    /* verifies that the payload does not exceed the maximum allowed
    size, an oversized one is rejected instead of being buffered */
    if(payload_size > (unsigned long long) VIRIATUM_WEBSOCKET_MAX_PAYLOAD) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Frame payload is too large"
        );
    }

    /* a control frame may neither be fragmented nor carry a payload
    larger than the one that fits the base length field */
    if(is_control_websocket(websocket_frame->opcode) == TRUE) {
        if(websocket_frame->fin == FALSE) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Received a fragmented control frame"
            );
        }
        if(payload_size > VIRIATUM_WEBSOCKET_MAX_CONTROL) {
            RAISE_ERROR_M(
                RUNTIME_EXCEPTION_ERROR_CODE,
                (unsigned char *) "Control frame payload is too large"
            );
        }
    }

    /* in case the masking key is not completely available the frame
    is still incomplete and so more data is required */
    if(buffer_size < offset + 4) { RAISE_NO_ERROR; }
    memcpy(websocket_frame->mask, &buffer[offset], 4);
    offset += 4;

    /* in case the payload is not completely available the frame is
    still incomplete, note that nothing has been unmasked yet */
    if(buffer_size < offset + (size_t) payload_size) { RAISE_NO_ERROR; }

    /* unmasks the payload in place, the operation is its own inverse
    and so the buffer may not be parsed a second time */
    for(index = 0; index < (size_t) payload_size; index++) {
        buffer[offset + index] ^= websocket_frame->mask[index % 4];
    }

    /* sets the payload and the complete size of the frame, the latter
    is what marks the frame as a completely parsed one */
    websocket_frame->payload = &buffer[offset];
    websocket_frame->payload_size = (size_t) payload_size;
    websocket_frame->size = offset + (size_t) payload_size;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE build_frame_websocket(unsigned char opcode, char fin, const unsigned char *payload, size_t payload_size, unsigned char **buffer_pointer, size_t *buffer_size_pointer) {
    /* allocates space for the buffer that is going to receive the
    frame and for the offset at which the payload starts */
    unsigned char *buffer;
    size_t index;
    size_t offset = 2;

    /* verifies that the payload does not exceed the maximum allowed
    size, keeping the framing symmetrical with the parsing one */
    if(payload_size > VIRIATUM_WEBSOCKET_MAX_PAYLOAD) {
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Frame payload is too large"
        );
    }

    /* calculates the offset at which the payload starts, it grows with
    the variant of the length field that is required by the size */
    if(payload_size > 65535) { offset += 8; }
    else if(payload_size > 125) { offset += 2; }

    /* allocates the buffer for the complete frame and writes the first
    byte of the header into it, the frames sent by the server are never
    masked as required by the specification */
    buffer = (unsigned char *) MALLOC(offset + payload_size);
    buffer[0] = (unsigned char) ((fin == TRUE ? 0x80 : 0x00) | (opcode & 0x0f));

    /* writes the length of the payload using the smallest of the
    variants that is able to represent it */
    if(payload_size > 65535) {
        buffer[1] = 127;
        for(index = 0; index < 8; index++) {
            buffer[2 + index] = (unsigned char)
                ((unsigned long long) payload_size >> ((7 - index) * 8) & 0xff);
        }
    } else if(payload_size > 125) {
        buffer[1] = 126;
        buffer[2] = (unsigned char) (payload_size >> 8 & 0xff);
        buffer[3] = (unsigned char) (payload_size & 0xff);
    } else {
        buffer[1] = (unsigned char) payload_size;
    }

    /* copies the payload into the tail of the buffer and sets both
    the buffer and its size in the provided pointers */
    if(payload_size > 0) { memcpy(&buffer[offset], payload, payload_size); }
    *buffer_pointer = buffer;
    *buffer_size_pointer = offset + payload_size;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE build_close_websocket(unsigned short code, const char *reason, unsigned char **buffer_pointer, size_t *buffer_size_pointer) {
    /* allocates space for the payload of the close frame, it is bound
    by the maximum size of a control frame payload */
    unsigned char payload[VIRIATUM_WEBSOCKET_MAX_CONTROL];
    size_t reason_size = reason == NULL ? 0 : strlen(reason);

    /* truncates the reason so that the resulting payload fits the one
    of a control frame, the code takes the first two bytes of it */
    if(reason_size > VIRIATUM_WEBSOCKET_MAX_CONTROL - 2) {
        reason_size = VIRIATUM_WEBSOCKET_MAX_CONTROL - 2;

        /* walks back over the continuation bytes so that the reason is
        never cut in the middle of a code point, the specification
        requires it to be a valid utf-8 sequence */
        while(reason_size > 0 &&
            ((unsigned char) reason[reason_size] & 0xc0) == 0x80) {
            reason_size--;
        }
    }

    /* writes the close code in network order and copies the reason
    into the remaining part of the payload */
    payload[0] = (unsigned char) (code >> 8 & 0xff);
    payload[1] = (unsigned char) (code & 0xff);
    if(reason_size > 0) { memcpy(&payload[2], reason, reason_size); }

    /* builds the control frame carrying the payload that has just
    been created, a close frame is always a final one */
    return build_frame_websocket(
        WEBSOCKET_OPCODE_CLOSE,
        TRUE,
        payload,
        reason_size + 2,
        buffer_pointer,
        buffer_size_pointer
    );
}

char is_control_websocket(unsigned char opcode) {
    /* the control frames are the ones whose most significant bit of
    the operation code is set, as defined by the specification */
    return opcode & 0x08 ? TRUE : FALSE;
}

unsigned short close_code_websocket(const unsigned char *payload, size_t payload_size) {
    /* in case the payload does not carry a code the connection has
    been closed without one, reported through the reserved value */
    if(payload == NULL || payload_size < 2) { return WEBSOCKET_CLOSE_NONE; }

    /* unpacks the code from the first two bytes of the payload, they
    are laid out in network order as every other length is */
    return (unsigned short) (((unsigned short) payload[0] << 8) |
        (unsigned short) payload[1]);
}
