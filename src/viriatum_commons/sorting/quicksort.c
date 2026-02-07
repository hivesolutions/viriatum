/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "quicksort.h"

void sort_quicksort(void **sequence, size_t beginning, size_t end, comparator cmp) {
    /* retrieves the initial pivor, left and right elements */
    void *pivot = sequence[beginning];
    size_t left = beginning + 1;
    size_t right = end;

    /* in case the end reference "overflows"
    the beginig reference (end of recursion) */
    if(end <= beginning + 1) {
        /* returns immediately */
        return;
    }

    /* iterates while the right index is greater
    than the left index */
    while(left < right) {
        /* in case the left element is smaller
        or equal to the pivot (left advance) */
        if(cmp(sequence[left], pivot) <= 0) {
            /* increments the left index */
            left++;
        }
        /* otherwise a swap should be done */
        else {
            /* swaps the left value with the right one */
            _swap_quicksort(&sequence[left], &sequence[--right]);
        }
    }

    /* swaps the left value with the beginning value */
    _swap_quicksort(&sequence[--left], &sequence[beginning]);

    /* runs the quicksort from the begining to the left
    and from the right to the end (bi-section) */
    sort_quicksort(sequence, beginning, left, cmp);
    sort_quicksort(sequence, right, end, cmp);
}

void _swap_quicksort(void **first, void **second) {
    /* saves the first element to the accumulator
    and then swaps both elements using it (accumulator) */
    void *accumulator = *first;
    *first = *second;
    *second = accumulator;
}
