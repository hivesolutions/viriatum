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

#include "hpack.h"

/**
 * The sequence of bytes that opens every HTTP/2 connection, a
 * client sends it before any frame and it is the only way to
 * recognise the protocol when no negotiation has taken place.
 */
#define HTTP2_PREFACE "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
#define HTTP2_PREFACE_SIZE 24

/**
 * The size in bytes of the header that opens every frame, it
 * carries the length, the type, the flags and the identifier
 * of the stream that the frame belongs to.
 */
#define HTTP2_HEADER_SIZE 9

/**
 * The identifiers of the two protocols that may be negotiated
 * for a connection, the cleartext form of HTTP/2 is only ever
 * used through the prior knowledge of the client.
 */
#define HTTP2_ALPN "h2"
#define HTTP2_ALPN_CLEAR "h2c"
#define HTTP11_ALPN "http/1.1"

/**
 * The bounds of the size of a frame, the smallest one is the
 * value that a peer is required to accept and the largest one
 * is what the length field of the header is able to hold.
 */
#define HTTP2_DEFAULT_FRAME_SIZE 16384
#define HTTP2_MAX_FRAME_SIZE 16777215

/**
 * The size of the flow control window that both the connection
 * and every one of the streams start with, together with the
 * largest value that a window is allowed to reach.
 */
#define HTTP2_DEFAULT_WINDOW_SIZE 65535
#define HTTP2_MAX_WINDOW_SIZE 2147483647

/**
 * The number of streams that this end allows a peer to keep
 * open at the same time, an excess stream is refused instead of
 * being served, so that the memory of a connection is bounded.
 */
#define HTTP2_MAX_CONCURRENT 128

/**
 * The largest value that the identifier of a stream is allowed
 * to take, it is carried in thirty one bits as the remaining
 * one of the field is reserved.
 */
#define HTTP2_MAX_STREAM_ID 2147483647

/**
 * The size in bytes of a single entry of the payload of a
 * settings frame, the identifier and the value of it.
 */
#define HTTP2_SETTING_SIZE 6

/**
 * The size of the frame that announces the settings of one end, it
 * carries every one of the values that this end is able to set.
 */
#define HTTP2_SETTINGS_SIZE (HTTP2_HEADER_SIZE + HTTP2_SETTING_SIZE * 5)

/**
 * The size in bytes of the payload of a ping frame, the
 * specification fixes it and a different one is an error.
 */
#define HTTP2_PING_SIZE 8

/**
 * The size in bytes of the priority information, either the
 * complete payload of a priority frame or the part of a headers
 * frame that precedes the block.
 */
#define HTTP2_PRIORITY_SIZE 5

/**
 * The default weight of a stream, the value carried on the wire
 * is the weight deducted of one.
 */
#define HTTP2_DEFAULT_WEIGHT 16

/**
 * Enumeration defining the types of frame that the
 * specification describes, the value is the one that
 * travels in the header of the frame.
 */
typedef enum http2_frame_type_e {
    HTTP2_DATA = 0x00,
    HTTP2_HEADERS = 0x01,
    HTTP2_PRIORITY = 0x02,
    HTTP2_RST_STREAM = 0x03,
    HTTP2_SETTINGS = 0x04,
    HTTP2_PUSH_PROMISE = 0x05,
    HTTP2_PING = 0x06,
    HTTP2_GOAWAY = 0x07,
    HTTP2_WINDOW_UPDATE = 0x08,
    HTTP2_CONTINUATION = 0x09
} http2_frame_type;

/**
 * Enumeration defining the flags that a frame is able to
 * carry, the meaning of a value depends on the type of the
 * frame that carries it.
 */
typedef enum http2_flag_e {
    HTTP2_FLAG_END_STREAM = 0x01,
    HTTP2_FLAG_ACK = 0x01,
    HTTP2_FLAG_END_HEADERS = 0x04,
    HTTP2_FLAG_PADDED = 0x08,
    HTTP2_FLAG_PRIORITY = 0x20
} http2_flag;

/**
 * Enumeration defining the identifiers of the settings that a
 * peer is able to change, an identifier that is not one of
 * these must be ignored rather than refused.
 */
typedef enum http2_setting_e {
    HTTP2_SETTING_HEADER_TABLE_SIZE = 0x01,
    HTTP2_SETTING_ENABLE_PUSH = 0x02,
    HTTP2_SETTING_MAX_CONCURRENT_STREAMS = 0x03,
    HTTP2_SETTING_INITIAL_WINDOW_SIZE = 0x04,
    HTTP2_SETTING_MAX_FRAME_SIZE = 0x05,
    HTTP2_SETTING_MAX_HEADER_LIST_SIZE = 0x06
} http2_setting;

/**
 * Enumeration defining the error codes of the protocol, they
 * are the values carried by both the reset of a stream and the
 * termination of a connection.
 * These are also the values that the operations of this module
 * return, the absence of an error is the value zero and so it
 * agrees with the convention of the error codes of the project.
 */
typedef enum http2_error_e {
    HTTP2_NO_ERROR = 0x00,
    HTTP2_PROTOCOL_ERROR = 0x01,
    HTTP2_INTERNAL_ERROR = 0x02,
    HTTP2_FLOW_CONTROL_ERROR = 0x03,
    HTTP2_SETTINGS_TIMEOUT = 0x04,
    HTTP2_STREAM_CLOSED = 0x05,
    HTTP2_FRAME_SIZE_ERROR = 0x06,
    HTTP2_REFUSED_STREAM = 0x07,
    HTTP2_CANCEL = 0x08,
    HTTP2_COMPRESSION_ERROR = 0x09,
    HTTP2_CONNECT_ERROR = 0x0a,
    HTTP2_ENHANCE_YOUR_CALM = 0x0b,
    HTTP2_INADEQUATE_SECURITY = 0x0c,
    HTTP2_HTTP_1_1_REQUIRED = 0x0d
} http2_error;

/**
 * Structure describing the header of a frame together with a
 * reference to its payload, the payload is only set when the
 * complete frame is present in the buffer that was decoded.
 */
typedef struct http2_frame_t {
    /**
     * The size in bytes of the payload of the frame, it does
     * not account for the header that precedes it.
     */
    size_t length;

    /**
     * The type of the frame, one of the values of the type
     * enumeration, an unknown one has to be ignored.
     */
    unsigned char type;

    /**
     * The flags of the frame, their meaning depends on the
     * type of the frame that carries them.
     */
    unsigned char flags;

    /**
     * The identifier of the stream the frame belongs to, the
     * value zero refers to the connection itself.
     */
    unsigned int stream_id;

    /**
     * Reference to the payload of the frame inside the buffer
     * that has been decoded, it is unset when the buffer does
     * not yet hold the complete frame.
     */
    unsigned char *payload;
} http2_frame;

/**
 * Structure holding the set of settings of one of the ends of
 * a connection, it starts at the values that the specification
 * defines and changes as the peer sends its own.
 */
typedef struct http2_settings_t {
    size_t header_table_size;
    char enable_push;
    size_t max_concurrent_streams;
    size_t initial_window_size;
    size_t max_frame_size;
    size_t max_header_list_size;
} http2_settings;

/**
 * Structure describing the priority of a stream, both the
 * stream it depends on and the weight it carries inside that
 * dependency.
 */
typedef struct http2_priority_t {
    /**
     * The identifier of the stream this one depends on, the
     * value zero refers to the root of the tree.
     */
    unsigned int dependency;

    /**
     * The weight of the stream among its siblings, the value
     * on the wire is this one deducted of a unit, so the range
     * of it goes from one up to two hundred and fifty six and
     * does not fit in a single byte.
     */
    unsigned short weight;

    /**
     * Flag controlling if the dependency is an exclusive one,
     * meaning that the siblings become children of it.
     */
    char exclusive;
} http2_priority;

/**
 * Reads a thirty two bit value out of the provided buffer, the
 * protocol carries every number in network order.
 *
 * @param data The buffer to read the value from, it must hold
 * at least four bytes.
 * @return The value that has been read.
 */
unsigned int decode_number_http2(const unsigned char *data);

/**
 * Writes a thirty two bit value into the provided buffer in the
 * network order that the protocol uses.
 *
 * @param buffer The buffer to write the value into, it must
 * hold at least four bytes.
 * @param value The value to be written.
 */
void encode_number_http2(unsigned char *buffer, unsigned int value);

/**
 * Populates the provided settings with the values that the
 * specification defines as the initial ones.
 *
 * @param http2_settings The settings to be populated.
 */
void create_settings_http2(struct http2_settings_t *http2_settings);

/**
 * Decodes the header of a frame out of the provided buffer, the
 * payload of the frame is only referenced when the buffer holds
 * the complete frame, otherwise it is left unset so that the
 * caller knows that more data is required.
 *
 * @param data The buffer containing the frame.
 * @param data_size The size in bytes of the provided buffer.
 * @param http2_frame The frame to be populated.
 * @return The resulting error code.
 */
ERROR_CODE decode_frame_http2(unsigned char *data, size_t data_size, struct http2_frame_t *http2_frame);

/**
 * Writes the header of a frame into the provided buffer, the
 * payload is written by the caller right after it.
 *
 * @param buffer The buffer to write the header into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param length The size in bytes of the payload that follows.
 * @param type The type of the frame being written.
 * @param flags The flags of the frame being written.
 * @param stream_id The stream the frame belongs to.
 * @return The resulting error code.
 */
ERROR_CODE encode_frame_http2(unsigned char *buffer, size_t buffer_size, size_t length, unsigned char type, unsigned char flags, unsigned int stream_id);

/**
 * Removes the padding of a frame that carries it, the padding
 * exists for the data, the headers and the push promise frames
 * and its length sits in the first byte of the payload.
 *
 * @param http2_frame The frame to have the padding removed.
 * @param payload The variable to be set with the position of
 * the payload that follows the padding length.
 * @param payload_size The variable to be set with the size of
 * the payload once the padding is deducted.
 * @return The resulting error code.
 */
ERROR_CODE strip_padding_http2(struct http2_frame_t *http2_frame, unsigned char **payload, size_t *payload_size);

/**
 * Decodes the priority information out of the provided buffer,
 * it is either the complete payload of a priority frame or the
 * part of a headers frame that precedes the block.
 *
 * @param data The buffer containing the priority information.
 * @param data_size The size in bytes of the provided buffer.
 * @param http2_priority The priority to be populated.
 * @return The resulting error code.
 */
ERROR_CODE decode_priority_http2(const unsigned char *data, size_t data_size, struct http2_priority_t *http2_priority);

/**
 * Applies the payload of a settings frame over the provided
 * settings, an identifier that is not recognised is ignored as
 * the specification requires.
 *
 * @param data The payload of the settings frame.
 * @param data_size The size in bytes of the payload.
 * @param http2_settings The settings to be changed.
 * @return The resulting error code.
 */
ERROR_CODE decode_settings_http2(const unsigned char *data, size_t data_size, struct http2_settings_t *http2_settings);

/**
 * Writes a settings frame carrying the values of this end that
 * differ from the ones the specification defines.
 *
 * @param buffer The buffer to write the frame into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param http2_settings The settings to be announced.
 * @param size The variable to be set with the size in bytes of
 * the frame that has been written.
 * @return The resulting error code.
 */
ERROR_CODE encode_settings_http2(unsigned char *buffer, size_t buffer_size, struct http2_settings_t *http2_settings, size_t *size);

/**
 * Writes a frame that carries a single thirty two bit value as
 * its complete payload, which is the shape of both the reset of
 * a stream and the update of a window.
 *
 * @param buffer The buffer to write the frame into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param type The type of the frame being written.
 * @param stream_id The stream the frame belongs to.
 * @param value The value to be carried by the payload.
 * @param size The variable to be set with the size in bytes of
 * the frame that has been written.
 * @return The resulting error code.
 */
ERROR_CODE encode_value_http2(unsigned char *buffer, size_t buffer_size, unsigned char type, unsigned int stream_id, unsigned int value, size_t *size);

/**
 * Writes a goaway frame, which closes a connection and tells
 * the peer the identifier of the last stream that this end has
 * processed, so that the peer knows what to retry.
 *
 * @param buffer The buffer to write the frame into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param last_stream_id The last stream that has been handled.
 * @param error The error code that closes the connection.
 * @param size The variable to be set with the size in bytes of
 * the frame that has been written.
 * @return The resulting error code.
 */
ERROR_CODE encode_goaway_http2(unsigned char *buffer, size_t buffer_size, unsigned int last_stream_id, unsigned int error, size_t *size);

/**
 * Verifies that the provided frame is a coherent one for its
 * type, covering the identifier of the stream it carries and
 * the size that the specification fixes for it.
 *
 * @param http2_frame The frame to be verified.
 * @param http2_settings The settings of this end, they carry
 * the largest size a frame is allowed to take.
 * @return The resulting error code.
 */
ERROR_CODE verify_frame_http2(struct http2_frame_t *http2_frame, struct http2_settings_t *http2_settings);
