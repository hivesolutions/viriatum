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

 __author__    = Jo‹o Magalh‹es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

typedef struct sha1Context_t {
    unsigned int state[5];
    unsigned int count[2];
    unsigned char  buffer[64];
} sha1Context;

#define SHA1_DIGEST_SIZE 20

VIRIATUM_EXPORT_PREFIX void sha1(unsigned char *buffer, unsigned int bufferLength, unsigned char *result);
VIRIATUM_EXPORT_PREFIX void initSha1(struct sha1Context_t *context);
VIRIATUM_EXPORT_PREFIX void updateSha1(struct sha1Context_t *context, const unsigned char *data, const size_t len);
VIRIATUM_EXPORT_PREFIX void finalSha1(struct sha1Context_t *context, unsigned char *digest);
VIRIATUM_EXPORT_PREFIX void _transformSha1(unsigned int state[5], const unsigned char buffer[64]);
