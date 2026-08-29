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

#include "../http/hpack.h"
#include "../http/http2.h"

/**
 * The largest input that the target accepts, an input above it
 * carries nothing that a smaller one does not already reach and
 * only makes the corpus heavier.
 */
#define FUZZ_MAX_SIZE 16384

/**
 * Gathers a header field that has been decoded and does nothing at
 * all with it, the decoding is what is under test rather than the
 * handling of what comes out of it.
 *
 * @param parameters The counter of the fields that reach it.
 * @param hpack_header The header field that has been decoded.
 * @return The resulting error code.
 */
static ERROR_CODE _collect_fuzz(void *parameters, struct hpack_header_t *hpack_header) {
    (*(size_t *) parameters)++;
    RAISE_NO_ERROR;
}

/**
 * Drives the decoders of the protocol over the provided bytes, they
 * are the two places that take apart what a peer controls and so the
 * ones where the memory safety of a from scratch implementation is
 * at stake.
 *
 * @param data The bytes that the peer is standing in for.
 * @param data_size The size in bytes of the provided input.
 * @return The value zero, the interface of the engine requires it.
 */
int LLVMFuzzerTestOneInput(const unsigned char *data, size_t data_size) {
    /* allocates space for the frame being decoded, for the values it
    carries and for the table of the decoding of a block */
    struct http2_frame_t http2_frame;
    struct http2_settings_t http2_settings;
    struct http2_priority_t http2_priority;
    struct hpack_table_t *hpack_table;
    unsigned char *payload;
    size_t payload_size;
    size_t count = 0;

    /* an input above the bound carries nothing that a smaller one
    does not already reach */
    if(data_size > FUZZ_MAX_SIZE) { return 0; }

    /* takes the frame apart, the header of it is of a fixed size and
    the payload only exists once the complete frame has arrived */
    if(decode_frame_http2((unsigned char *) data, data_size, &http2_frame) == 0 &&
       http2_frame.payload != NULL) {
        /* the padding is stripped before anything else looks at the
        payload, a length that lies is refused here */
        payload = http2_frame.payload;
        payload_size = http2_frame.length;
        strip_padding_http2(&http2_frame, &payload, &payload_size);

        /* the payload of a settings frame is a sequence of entries of
        a fixed size, a value out of range is refused */
        create_settings_http2(&http2_settings);
        decode_settings_http2(payload, payload_size, &http2_settings);

        /* the priority information is of a fixed size, the decoding
        of it is driven over whatever the payload holds */
        decode_priority_http2(payload, payload_size, &http2_priority);

        /* the frame is verified against the rules that bound the type
        of it, which is what a session does before handling it */
        create_settings_http2(&http2_settings);
        verify_frame_http2(&http2_frame, &http2_settings);
    }

    /* takes the very same bytes apart as a header block, the table of
    it starts empty so that a run never depends on another */
    create_hpack_table(&hpack_table);
    decode_hpack(hpack_table, data, data_size, _collect_fuzz, (void *) &count);
    delete_hpack_table(hpack_table);

    /* returns the value that the engine requires, the outcome of the
    decoding carries no meaning for it */
    return 0;
}
