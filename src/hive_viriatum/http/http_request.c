/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "http_request.h"

void createHttpRequest(struct HttpRequest_t **httpRequestPointer) {
    /* retrieves the http request size */
    size_t httpRequestSize = sizeof(struct HttpRequest_t);

    /* allocates space for the http request */
    struct HttpRequest_t *httpRequest = (struct HttpRequest_t *) malloc(httpRequestSize);

    /* sets the http request received data size */
    httpRequest->receivedDataSize = 0;

    /* sets the http request start line loaded */
    httpRequest->startLineLoaded = 0;

    /* sets the http request headers loaded */
    httpRequest->headersLoaded = 0;

    /* sets the http request in the http request pointer */
    *httpRequestPointer = httpRequest;
}

void deleteHttpRequest(struct HttpRequest_t *httpRequest) {
    /* releases the http request */
    free(httpRequest);
}

#define TESTFUNC_PARAMS_NAMED const uint8_t *needle, size_t nlen, const uint8_t *haystack, size_t hlen, size_t *ccnt
#define INC_CCNT (*ccnt)++
#define MISALIGNED_SIZET(X) ((size_t)(X) & (sizeof(size_t) - 1))
#define ALIGNED_SIZET(X) (!MISALIGNED_SIZET(X))

static inline int32_t internal_str_cmp(const uint8_t *s1, const uint8_t *s2, size_t len, size_t *ccnt) {
    // Nothing to compare, so let's return directly
    if (len == 0) {
        return (0);
    }

    // If s1 and s2 are aligned, and we need to compare something that is
    // at least equal or bigger to word size, we can speed up things a bit
    if((len >= sizeof(size_t)) && (ALIGNED_SIZET(s1)) && (ALIGNED_SIZET(s2))) {
        // If s1p and s2p are word-aligned, compare them one word at a time
        const size_t *a1 = (const size_t *)s1;
        const size_t *a2 = (const size_t *)s2;

        INC_CCNT;
        while ((len >= sizeof(size_t)) && (*a1 == *a2)) {
            len -= sizeof(size_t);
            if (len) {
                INC_CCNT;
                a1++;
                a2++;
            }
        }

        // Either difference detected or not enough bytes left, so search byte-wise
        s1 = (const uint8_t *)a1;
        s2 = (const uint8_t *)a2;
    }

    // Normal byte-wise compare
    INC_CCNT;
    while((len) && (*s1 == *s2)) {
        len--;
        if (len) {
            INC_CCNT;
            s1++;
            s2++;
        }
    }

    return (*s1 - *s2);
}

// Memory usage: 2 * size_t (counters) = 16 b
// O(hlen * nlen) worstcase, expected around 2 * nlen
static size_t naive_strstr(TESTFUNC_PARAMS_NAMED) {
    // Searching
    for (size_t j = 0, w = hlen - nlen; j <= w; j++) {
        if (internal_str_cmp(needle, haystack + j, nlen, ccnt) == 0) {
            return (j);
        }
    }

    return (SIZE_MAX);
}


//const uint8_t *needle, size_t nlen, const uint8_t *haystack, size_t hlen, size_t *cc

void parseDataHttpRequest(struct HttpRequest_t *httpRequest, unsigned char *data, size_t dataSize) {
    /**
     * NOTE: durring the parsing the index (and rest of the state) of
     *       the find must be saved during the various parsing iterations
     *
     * 1. must find the first \r\n to load the first line of the http message
     * 2. sets the startLineLoaded flag
     * 3. tries to find the \r\n\r\n end to load the headers
     * 4. sets the headersLoaded flag
     */
    size_t value;
    size_t position;

    /* in case the start line is not yet loaded */
    if(!httpRequest->startLineLoaded) {
        position = naive_strstr("\r\n", 2, data, dataSize, &value);

        printf("POSITION %d, '%c'", (int) position, data[position]);
    }

    printf("ola mundo\n");
}
