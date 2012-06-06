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
 * Function used to compare two elements, returns positive in
 * case the firt element is greater than the second, negative
 * in case the opostie happens and zero in case they are equals.
 */
typedef int (*comparator) (void *, void *);

/**
 * Structure defining the elements
 * of a "sortable" element.
 * This kind of element allows the sorting
 * of a sequence and maintains a "back-reference"
 * to the "original" element.
 */
typedef struct sort_element_t {
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
} sort_element;
