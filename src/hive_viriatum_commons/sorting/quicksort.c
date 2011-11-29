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

#include "quicksort.h"

void sortQuicksort(int sequence[], int beginning, int end) {
    /* retrieves the initial pivor, left and right elements */
    int pivot = sequence[beginning];
    int left = beginning + 1;
    int right = end;

    /* in case the end reference "overflows"
    the beginig reference (end of recursion) */
    if(end <= beginning + 1) {
        /* returns immediately */
        return;
    }

    while(left < right) {
        if(sequence[left] <= pivot) {
            left++;
        }
        else {
            _swapQuicksort(&sequence[left], &sequence[--right]);
        }
    }

    _swapQuicksort(&sequence[--left], &sequence[beginning]);
    sortQuicksort(sequence, beginning, left);
    sortQuicksort(sequence, right, end);
}

void _swapQuicksort(int *first, int *second) {
    /* saves the first element to the accumulator
    and then swaps both elements using it (accumulator) */
    int accumulator = *first;
    *first = *second;
    *second = accumulator;
}
