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

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "md5.h"

/**
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))

/**
 * The MD5 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, t, s)\
    (a) += f((b), (c), (d)) + (x) + (t);\
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));\
    (a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned
 * memory accesses is just an optimization.  Nothing will break if it
 * doesn't work.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
    (*(MD5_u32plus *)&ptr[(n) * 4])
#define GET(n) \
    SET(n)
#else
#define SET(n)\
    (context->block[(n)] =\
    (MD5_u32plus)ptr[(n) * 4] |\
    ((MD5_u32plus)ptr[(n) * 4 + 1] << 8) |\
    ((MD5_u32plus)ptr[(n) * 4 + 2] << 16) |\
    ((MD5_u32plus)ptr[(n) * 4 + 3] << 24))
#define GET(n)\
    (context->block[(n)])
#endif

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There are no alignment requirements.
 */
static void *body(MD5_context *context, void *data, unsigned long size) {
    unsigned char *ptr;
    MD5_u32plus a, b, c, d;
    MD5_u32plus saved_a, saved_b, saved_c, saved_d;

    ptr = data;

    a = context->a;
    b = context->b;
    c = context->c;
    d = context->d;

    do {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

		/* executes the round 1 computation */
        STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
        STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
        STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
        STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
        STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
        STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
        STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
        STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
        STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
        STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
        STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
        STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
        STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
        STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
        STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
        STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

		/* executes the round 2 computation */
        STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
        STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
        STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
        STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
        STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
        STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
        STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
        STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
        STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
        STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
        STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
        STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
        STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
        STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
        STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
        STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

		/* executes the round 3 computation */
        STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
        STEP(H, d, a, b, c, GET(8), 0x8771f681, 11)
        STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
        STEP(H, b, c, d, a, GET(14), 0xfde5380c, 23)
        STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
        STEP(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
        STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
        STEP(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
        STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
        STEP(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
        STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
        STEP(H, b, c, d, a, GET(6), 0x04881d05, 23)
        STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
        STEP(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
        STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
        STEP(H, b, c, d, a, GET(2), 0xc4ac5665, 23)

		/* executes the round 4 computation */
        STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
        STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
        STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
        STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
        STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
        STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
        STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
        STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
        STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
        STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
        STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
        STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
        STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
        STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
        STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
        STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;

        ptr += 64;
    } while(size -= 64);

    context->a = a;
    context->b = b;
    context->c = c;
    context->d = d;

    return ptr;
}

void MD5_Init(MD5_context *context) {
    context->a = 0x67452301;
    context->b = 0xefcdab89;
    context->c = 0x98badcfe;
    context->d = 0x10325476;

    context->lo = 0;
    context->hi = 0;
}

void MD5_Update(MD5_context *context, void *data, unsigned long size) {
    MD5_u32plus saved_lo;
    unsigned long used, free;

    saved_lo = context->lo;
	if((context->lo = (saved_lo + size) & 0x1fffffff) < saved_lo) {
        context->hi++;
	}
    context->hi += size >> 29;

    used = saved_lo & 0x3f;

    if(used) {
        free = 64 - used;

        if(size < free) {
            memcpy(&context->buffer[used], data, size);
            return;
        }

        memcpy(&context->buffer[used], data, free);
        data = (unsigned char *) data + free;
        size -= free;
        body(context, context->buffer, 64);
    }

    if (size >= 64) {
        data = body(context, data, size & ~ (unsigned long) 0x3f);
        size &= 0x3f;
    }

    memcpy(context->buffer, data, size);
}

void MD5_Final(unsigned char *result, MD5_context *context) {
    unsigned long used, free;

    used = context->lo & 0x3f;

    context->buffer[used++] = 0x80;

    free = 64 - used;

    if(free < 8) {
        memset(&context->buffer[used], 0, free);
        body(context, context->buffer, 64);
        used = 0;
        free = 64;
    }

    memset(&context->buffer[used], 0, free - 8);

    context->lo <<= 3;
    context->buffer[56] = context->lo;
    context->buffer[57] = context->lo >> 8;
    context->buffer[58] = context->lo >> 16;
    context->buffer[59] = context->lo >> 24;
    context->buffer[60] = context->hi;
    context->buffer[61] = context->hi >> 8;
    context->buffer[62] = context->hi >> 16;
    context->buffer[63] = context->hi >> 24;

    body(context, context->buffer, 64);

    result[0] = context->a;
    result[1] = context->a >> 8;
    result[2] = context->a >> 16;
    result[3] = context->a >> 24;
    result[4] = context->b;
    result[5] = context->b >> 8;
    result[6] = context->b >> 16;
    result[7] = context->b >> 24;
    result[8] = context->c;
    result[9] = context->c >> 8;
    result[10] = context->c >> 16;
    result[11] = context->c >> 24;
    result[12] = context->d;
    result[13] = context->d >> 8;
    result[14] = context->d >> 16;
    result[15] = context->d >> 24;

    memset(context, 0, sizeof(*context));
}
