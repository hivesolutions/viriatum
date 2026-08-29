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

#include "http2.h"

#ifdef VIRIATUM_HTTP2

unsigned int decode_number_http2(const unsigned char *data) {
    /* gathers the four bytes of the value in the network order that
    the protocol uses for every one of its numbers */
    return ((unsigned int) data[0] << 24) | ((unsigned int) data[1] << 16) |
           ((unsigned int) data[2] << 8) | (unsigned int) data[3];
}

void encode_number_http2(unsigned char *buffer, unsigned int value) {
    /* spreads the four bytes of the value in the network order that
    the protocol uses for every one of its numbers */
    buffer[0] = (unsigned char) ((value >> 24) & 0xff);
    buffer[1] = (unsigned char) ((value >> 16) & 0xff);
    buffer[2] = (unsigned char) ((value >> 8) & 0xff);
    buffer[3] = (unsigned char) (value & 0xff);
}

void create_settings_http2(struct http2_settings_t *http2_settings) {
    /* sets the values that the specification defines as the initial
    ones, they are in effect until the peer announces its own */
    http2_settings->header_table_size = HPACK_TABLE_SIZE;
    http2_settings->enable_push = TRUE;
    http2_settings->max_concurrent_streams = HTTP2_MAX_CONCURRENT;
    http2_settings->initial_window_size = HTTP2_DEFAULT_WINDOW_SIZE;
    http2_settings->max_frame_size = HTTP2_DEFAULT_FRAME_SIZE;
    http2_settings->max_header_list_size = HPACK_MAX_HEADER_LIST_SIZE;
}

ERROR_CODE decode_frame_http2(unsigned char *data, size_t data_size, struct http2_frame_t *http2_frame) {
    /* the header of a frame is of a fixed size, without it there's
    nothing at all that may be decoded */
    if(data_size < HTTP2_HEADER_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    /* gathers the length out of the three bytes that carry it and
    then the type and the flags out of the two that follow */
    http2_frame->length = ((size_t) data[0] << 16) | ((size_t) data[1] << 8) | (size_t) data[2];
    http2_frame->type = data[3];
    http2_frame->flags = data[4];

    /* gathers the identifier of the stream, the most significant bit
    of the field is reserved and has to be ignored on reception */
    http2_frame->stream_id = decode_number_http2(&data[5]) & (unsigned int) HTTP2_MAX_STREAM_ID;

    /* references the payload only when the buffer holds the complete
    frame, the caller uses the absence of it to know that more data
    has to be gathered before the frame may be handled */
    if(data_size < HTTP2_HEADER_SIZE + http2_frame->length) {
        http2_frame->payload = NULL;
    } else {
        http2_frame->payload = &data[HTTP2_HEADER_SIZE];
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_frame_http2(unsigned char *buffer, size_t buffer_size, size_t length, unsigned char type, unsigned char flags, unsigned int stream_id) {
    /* verifies that there's room for the header itself, the payload
    is the responsibility of the caller */
    if(buffer_size < HTTP2_HEADER_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    /* a length that does not fit in the three bytes of the field is
    never writable and so it is refused */
    if(length > HTTP2_MAX_FRAME_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    /* spreads the length over the three bytes that carry it and then
    writes both the type and the flags */
    buffer[0] = (unsigned char) ((length >> 16) & 0xff);
    buffer[1] = (unsigned char) ((length >> 8) & 0xff);
    buffer[2] = (unsigned char) (length & 0xff);
    buffer[3] = type;
    buffer[4] = flags;

    /* writes the identifier of the stream, the reserved bit is left
    unset as the specification requires */
    encode_number_http2(&buffer[5], stream_id & (unsigned int) HTTP2_MAX_STREAM_ID);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE strip_padding_http2(struct http2_frame_t *http2_frame, unsigned char **payload, size_t *payload_size) {
    /* allocates space for the length of the padding, it sits in the
    first byte of the payload when the flag is present */
    size_t padding;

    /* a frame without the flag carries no padding at all, so the
    payload is the complete one */
    if(!(http2_frame->flags & HTTP2_FLAG_PADDED)) {
        *payload = http2_frame->payload;
        *payload_size = http2_frame->length;
        RAISE_NO_ERROR;
    }

    /* the byte that carries the length of the padding has to be
    present, an empty payload cannot be a padded one */
    if(http2_frame->length < 1) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

    /* a padding that is as long as the remainder of the payload, or
    longer, leaves no room for the content and is refused */
    padding = (size_t) http2_frame->payload[0];
    if(padding >= http2_frame->length) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

    /* moves the payload past the byte of the length and deducts both
    that byte and the padding itself from the size */
    *payload = &http2_frame->payload[1];
    *payload_size = http2_frame->length - 1 - padding;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_priority_http2(const unsigned char *data, size_t data_size, struct http2_priority_t *http2_priority) {
    /* allocates space for the field that carries both the exclusive
    flag and the identifier of the dependency */
    unsigned int dependency;

    /* the priority information is of a fixed size, a payload of a
    different size is a frame size error */
    if(data_size != HTTP2_PRIORITY_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    /* the most significant bit of the field is the exclusive flag
    and the remaining ones are the identifier of the dependency */
    dependency = decode_number_http2(data);
    http2_priority->exclusive = dependency & 0x80000000 ? TRUE : FALSE;
    http2_priority->dependency = dependency & (unsigned int) HTTP2_MAX_STREAM_ID;

    /* the weight travels deducted of a unit so that the complete
    range of it fits in a single byte, the range it describes goes
    up to two hundred and fifty six and so it is widened here */
    http2_priority->weight = (unsigned short) data[4] + 1;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decode_settings_http2(const unsigned char *data, size_t data_size, struct http2_settings_t *http2_settings) {
    /* allocates space for the iteration over the entries and for the
    identifier and the value of each one of them */
    size_t offset;
    unsigned short identifier;
    unsigned int value;

    /* the payload is a sequence of entries of a fixed size, a size
    that is not a multiple of it is a frame size error */
    if(data_size % HTTP2_SETTING_SIZE != 0) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    /* walks every one of the entries, an identifier that is not
    recognised is ignored rather than being refused */
    for(offset = 0; offset < data_size; offset += HTTP2_SETTING_SIZE) {
        identifier = (unsigned short) (((unsigned short) data[offset] << 8) | (unsigned short) data[offset + 1]);
        value = decode_number_http2(&data[offset + 2]);

        switch(identifier) {
            case HTTP2_SETTING_HEADER_TABLE_SIZE:
                http2_settings->header_table_size = (size_t) value;

                /* breaks the switch */
                break;

            case HTTP2_SETTING_ENABLE_PUSH:
                /* the setting is a flag, so a value that is neither
                of the two accepted ones is a protocol error */
                if(value > 1) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
                http2_settings->enable_push = value ? TRUE : FALSE;

                /* breaks the switch */
                break;

            case HTTP2_SETTING_MAX_CONCURRENT_STREAMS:
                http2_settings->max_concurrent_streams = (size_t) value;

                /* breaks the switch */
                break;

            case HTTP2_SETTING_INITIAL_WINDOW_SIZE:
                /* a window above the largest one that the protocol
                is able to represent is a flow control error */
                if(value > (unsigned int) HTTP2_MAX_WINDOW_SIZE) { RAISE_ERROR_S(HTTP2_FLOW_CONTROL_ERROR); }
                http2_settings->initial_window_size = (size_t) value;

                /* breaks the switch */
                break;

            case HTTP2_SETTING_MAX_FRAME_SIZE:
                /* the size of a frame is bounded on both ends, the
                lower one is the size every peer has to accept */
                if(value < HTTP2_DEFAULT_FRAME_SIZE || value > (unsigned int) HTTP2_MAX_FRAME_SIZE) {
                    RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR);
                }
                http2_settings->max_frame_size = (size_t) value;

                /* breaks the switch */
                break;

            case HTTP2_SETTING_MAX_HEADER_LIST_SIZE:
                http2_settings->max_header_list_size = (size_t) value;

                /* breaks the switch */
                break;

            default:
                /* an unknown setting is ignored, this is what allows
                the protocol to be extended without breaking a peer
                that does not know about the extension */
                break;
        }
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_settings_http2(unsigned char *buffer, size_t buffer_size, struct http2_settings_t *http2_settings, size_t *size) {
    /* allocates space for the position in the payload and for the
    error code of the writing of the header */
    size_t offset = HTTP2_HEADER_SIZE;
    ERROR_CODE return_value;

    /* the frame carries every one of the settings that this end is
    able to set, a peer that was told nothing of one of them keeps
    the value of the specification for it and the two ends would
    then disagree on what has been announced */
    if(buffer_size < HTTP2_HEADER_SIZE + HTTP2_SETTING_SIZE * 5) {
        RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR);
    }

    /* announces the number of streams that a peer is allowed to keep
    open at the same time, which bounds the memory of a connection */
    buffer[offset] = 0x00;
    buffer[offset + 1] = HTTP2_SETTING_MAX_CONCURRENT_STREAMS;
    encode_number_http2(&buffer[offset + 2], (unsigned int) http2_settings->max_concurrent_streams);
    offset += HTTP2_SETTING_SIZE;

    /* announces the size of the dynamic table of the decoder, the
    peer is never allowed to index beyond it */
    buffer[offset] = 0x00;
    buffer[offset + 1] = HTTP2_SETTING_HEADER_TABLE_SIZE;
    encode_number_http2(&buffer[offset + 2], (unsigned int) http2_settings->header_table_size);
    offset += HTTP2_SETTING_SIZE;

    /* announces the largest header list that this end accepts, which
    is the guard against the expansion of a small block */
    buffer[offset] = 0x00;
    buffer[offset + 1] = HTTP2_SETTING_MAX_HEADER_LIST_SIZE;
    encode_number_http2(&buffer[offset + 2], (unsigned int) http2_settings->max_header_list_size);
    offset += HTTP2_SETTING_SIZE;

    /* announces the window that every stream of this end starts its
    life with, which bounds what the peer sends before it is widened */
    buffer[offset] = 0x00;
    buffer[offset + 1] = HTTP2_SETTING_INITIAL_WINDOW_SIZE;
    encode_number_http2(&buffer[offset + 2], (unsigned int) http2_settings->initial_window_size);
    offset += HTTP2_SETTING_SIZE;

    /* announces the largest frame that this end accepts, one past it
    is refused as soon as the header of it arrives */
    buffer[offset] = 0x00;
    buffer[offset + 1] = HTTP2_SETTING_MAX_FRAME_SIZE;
    encode_number_http2(&buffer[offset + 2], (unsigned int) http2_settings->max_frame_size);
    offset += HTTP2_SETTING_SIZE;

    /* writes the header of the frame now that the size of the
    payload that follows it is known */
    return_value = encode_frame_http2(
        buffer,
        buffer_size,
        offset - HTTP2_HEADER_SIZE,
        HTTP2_SETTINGS,
        0x00,
        0
    );
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    /* sets the size with the one of the complete frame */
    *size = offset;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_value_http2(unsigned char *buffer, size_t buffer_size, unsigned char type, unsigned int stream_id, unsigned int value, size_t *size) {
    /* allocates space for the error code of the writing of the
    header of the frame */
    ERROR_CODE return_value;

    /* the payload of these frames is a single value, so the size of
    the complete frame is a constant one */
    if(buffer_size < HTTP2_HEADER_SIZE + 4) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    return_value = encode_frame_http2(buffer, buffer_size, 4, type, 0x00, stream_id);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    encode_number_http2(&buffer[HTTP2_HEADER_SIZE], value);

    /* sets the size with the one of the complete frame */
    *size = HTTP2_HEADER_SIZE + 4;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encode_goaway_http2(unsigned char *buffer, size_t buffer_size, unsigned int last_stream_id, unsigned int error, size_t *size) {
    /* allocates space for the error code of the writing of the
    header of the frame */
    ERROR_CODE return_value;

    /* the payload carries the last stream and the error code, the
    debug data that may follow them is never written by this end */
    if(buffer_size < HTTP2_HEADER_SIZE + 8) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

    return_value = encode_frame_http2(buffer, buffer_size, 8, HTTP2_GOAWAY, 0x00, 0);
    if(IS_ERROR_CODE(return_value)) { RAISE_AGAIN(return_value); }

    encode_number_http2(&buffer[HTTP2_HEADER_SIZE], last_stream_id & (unsigned int) HTTP2_MAX_STREAM_ID);
    encode_number_http2(&buffer[HTTP2_HEADER_SIZE + 4], error);

    /* sets the size with the one of the complete frame */
    *size = HTTP2_HEADER_SIZE + 8;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE verify_frame_http2(struct http2_frame_t *http2_frame, struct http2_settings_t *http2_settings) {
    /* a frame larger than what this end has announced is refused, the
    peer has been told the size it is allowed to use */
    if(http2_frame->length > http2_settings->max_frame_size) {
        RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR);
    }

    switch(http2_frame->type) {
        case HTTP2_DATA:
        case HTTP2_HEADERS:
        case HTTP2_CONTINUATION:
            /* these frames only ever belong to a stream, the value
            zero refers to the connection and is not one */
            if(http2_frame->stream_id == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

            /* breaks the switch */
            break;

        case HTTP2_PRIORITY:
            if(http2_frame->stream_id == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            if(http2_frame->length != HTTP2_PRIORITY_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

            /* breaks the switch */
            break;

        case HTTP2_RST_STREAM:
            if(http2_frame->stream_id == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            if(http2_frame->length != 4) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

            /* breaks the switch */
            break;

        case HTTP2_SETTINGS:
            /* the settings belong to the connection and never to a
            stream, and an acknowledgement carries no payload */
            if(http2_frame->stream_id != 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            if(http2_frame->flags & HTTP2_FLAG_ACK && http2_frame->length != 0) {
                RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR);
            }
            if(http2_frame->length % HTTP2_SETTING_SIZE != 0) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

            /* breaks the switch */
            break;

        case HTTP2_PUSH_PROMISE:
            /* a promise is made on the stream that has asked for the
            resource, so it never belongs to the connection */
            if(http2_frame->stream_id == 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }

            /* a promise carries the identifier of the stream it
            reserves, and the byte of the padding before it when the
            flag announces one, a payload shorter than that holds no
            promise at all */
            if(http2_frame->flags & HTTP2_FLAG_PADDED) {
                if(http2_frame->length < 5) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }
            } else {
                if(http2_frame->length < 4) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }
            }

            /* breaks the switch */
            break;

        case HTTP2_PING:
            if(http2_frame->stream_id != 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            if(http2_frame->length != HTTP2_PING_SIZE) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

            /* breaks the switch */
            break;

        case HTTP2_GOAWAY:
            if(http2_frame->stream_id != 0) { RAISE_ERROR_S(HTTP2_PROTOCOL_ERROR); }
            if(http2_frame->length < 8) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

            /* breaks the switch */
            break;

        case HTTP2_WINDOW_UPDATE:
            /* the update applies either to a stream or to the
            connection, so both of the identifiers are valid */
            if(http2_frame->length != 4) { RAISE_ERROR_S(HTTP2_FRAME_SIZE_ERROR); }

            /* breaks the switch */
            break;

        default:
            /* a frame of an unknown type is ignored, this is what
            allows the protocol to be extended */
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

#endif
