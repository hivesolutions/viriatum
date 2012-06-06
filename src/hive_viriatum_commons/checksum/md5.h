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
 * from the md5 digest calculation.
 */
#define MD5_DIGEST_SIZE 16

#define F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))

#define STEP(f, a, b, c, d, x, t, s)\
    (a) += f((b), (c), (d)) + (x) + (t);\
    (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));\
    (a) += (b);

#define SET(n) (*(unsigned int *)&pointer[(n) * 4])
#define GET(n) SET(n)

/**
 * Structure defining a context of md5
 * execution.
 * For every md5 execution a new context
 * must be created for temporary values
 * storage.
 */
typedef struct md5_context_t {
    unsigned int low;
    unsigned int high;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned char buffer[64];
    unsigned int block[16];
} md5_context;

VIRIATUM_EXPORT_PREFIX void md5(unsigned char *buffer, unsigned int buffer_length, unsigned char *result);
VIRIATUM_EXPORT_PREFIX void init_md5(struct md5_context_t *context);
VIRIATUM_EXPORT_PREFIX void update_md5(struct md5_context_t *context, void *data, unsigned long size);
VIRIATUM_EXPORT_PREFIX void final_md5(struct md5_context_t *context, unsigned char *result);
VIRIATUM_EXPORT_PREFIX void *_body_md5(struct md5_context_t *context, void *data, unsigned long size);
