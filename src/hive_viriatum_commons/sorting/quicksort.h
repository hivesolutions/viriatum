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

#include "base.h"

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
 * @param cmp The comparator method to be used for value
 * comparision.
 */
VIRIATUM_EXPORT_PREFIX void sortQuicksort(void **sequence, size_t beginning, size_t end, comparator cmp);

/**
 * Swaps the values (via reference) of two given elements.
 *
 * @param first The reference to the first element to be
 * swapped.
 * @param second The reference to the second element to be
 * swapped.
 */
VIRIATUM_EXPORT_PREFIX void _swapQuicksort(void **first, void **second);
