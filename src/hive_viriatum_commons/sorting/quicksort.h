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
 * Structure defining the elements
 * of a "sortable" element.
 * This kind of element allows the sorting
 * of a sequence and maintains a "back-reference"
 * to the "original" element.
 */
typedef struct SortElement_t {
    /**
     * The value to be used for comparision
     * in the sorting algorithm.
     */
    int value;

    /**
     * The reference to the "original" value
     * of the element.
     */
    void *reference;
} SortElement;

/**
 * Sorts the given sequence in the given range,
 * using the quicksort algorithm.
 *
 * @param sequence The sequence (array) to be sorted
 * the elements in it must conform with the sort element
 * specification.
 * @param beginning The initial element for the current
 * iteration.
 * @param end The final element for the current
 * iteration.
 */
void sortQuicksort(int sequence[], int beginning, int end);

/**
 * Swaps the values (via reference) of two given elements.
 *
 * @param first The reference to the first element to be
 * swapped.
 * @param second The reference to the second element to be
 * swapped.
 */
void _swapQuicksort(int *first, int *second);
