/*
 Hive Viriatum Commons
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "sha1.h"

void sha1(unsigned char *buffer, unsigned int bufferLength, unsigned char *result) {
    /* allocates space for the sha1 context */
    struct sha1Context_t sha1Context;

    /* initializes the sha1 context, then updates it
    with the provided buffer and after that finalizes
    it retrieving the result */
    initSha1(&sha1Context);
    updateSha1(&sha1Context, buffer, bufferLength);
    finalSha1(&sha1Context, result);
}

void initSha1(struct sha1Context_t *context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

void updateSha1(struct sha1Context_t *context, const unsigned char *data, const size_t len) {
    size_t i, j;

    j = (context->count[0] >> 3) & 63;
    if((context->count[0] += len << 3) < (len << 3)) { context->count[1]++; }
    context->count[1] += (len >> 29);
    if((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        _transformSha1(context->state, context->buffer);
        for(; i + 63 < len; i += 64) { _transformSha1(context->state, data + i); }
        j = 0;
    }
    else {
        i = 0;
    }

    memcpy(&context->buffer[j], &data[i], len - i);
}

void finalSha1(struct sha1Context_t *context, unsigned char *digest) {
    unsigned int i;
    unsigned char finalcount[8];

    for(i = 0; i < 8; i++) { finalcount[i] = (unsigned char) ((context->count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255); }

    updateSha1(context, (unsigned char *)"\200", 1);
    while((context->count[0] & 504) != 448) { updateSha1(context, (unsigned char *)"\0", 1); }
    updateSha1(context, finalcount, 8);
    for(i = 0; i < SHA1_DIGEST_SIZE; i++) { digest[i] = (unsigned char) ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255); }

    /* wipes the current variables, no more usage
    is possible (permanent delete) */
    i = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(finalcount, 0, 8);
}

void _transformSha1(unsigned int state[5], const unsigned char buffer[64]) {
    unsigned int a, b, c, d, e;
    union sha1Block_t *block;

    block = (union sha1Block_t *) buffer;

    /* copies context state to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* runs four rounds of operation twenty operations each */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    /* adds the working vars back into context state */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    /* wipes out all the variable to avoid any reusage
    of it (private usage) */
    a = b = c = d = e = 0;
}
