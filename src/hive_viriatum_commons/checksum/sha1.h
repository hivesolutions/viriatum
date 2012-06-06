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

#pragma once

/**
 * The size of the buffer resulting
 * from the sha1 digest calculation.
 */
#define SHA1_DIGEST_SIZE 20

#define ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#ifdef VIRIATUM_BIG_ENDIAN
#define BLK_0(i) block->l[i]
#else
#define BLK_0(i) (block->l[i] = (ROL(block->l[i], 24) & 0xff00ff00) \
    |(ROL(block->l[i], 8) & 0x00ff00ff))
#endif
#define BLK(i) (block->l[i & 15] = ROL(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] \
    ^block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

#define R0(v, w, x, y, z, i) z += ((w & (x ^ y)) ^ y) + BLK_0(i) + 0x5a827999 + ROL(v, 5); w = ROL(w, 30);
#define R1(v, w, x, y, z, i) z += ((w & (x ^ y)) ^ y) + BLK(i) + 0x5a827999 + ROL(v, 5); w = ROL(w, 30);
#define R2(v, w, x, y, z, i) z += (w ^ x ^ y) + BLK(i) + 0x6ed9eba1 + ROL(v, 5); w = ROL(w, 30);
#define R3(v, w, x, y, z, i) z += (((w | x) & y) | (w & x)) + BLK(i) + 0x8f1bbcdc + ROL(v, 5); w = ROL(w, 30);
#define R4(v, w, x, y, z, i) z += ( w ^ x ^ y) + BLK(i) + 0xca62c1d6 + ROL(v, 5); w = ROL(w, 30);

typedef struct sha1_context_t {
    unsigned int state[5];
    unsigned int count[2];
    unsigned char  buffer[64];
} sha1_context;

typedef union sha1_block_t {
    unsigned char c[64];
    unsigned int l[16];
} sha1_block;

VIRIATUM_EXPORT_PREFIX void sha1(unsigned char *buffer, unsigned int buffer_length, unsigned char *result);
VIRIATUM_EXPORT_PREFIX void init_sha1(struct sha1_context_t *context);
VIRIATUM_EXPORT_PREFIX void update_sha1(struct sha1_context_t *context, const unsigned char *data, const size_t size);
VIRIATUM_EXPORT_PREFIX void final_sha1(struct sha1_context_t *context, unsigned char *digest);
VIRIATUM_EXPORT_PREFIX void _transform_sha1(unsigned int state[5], const unsigned char buffer[64]);
