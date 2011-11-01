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

#pragma once

/**
 * Test thread function to be used in the thread
 * pool test.
 *
 * @param arguments The pointer to the arguments to
 * the thread.
 * @return The result of the thread execution.
 */
int threadPoolStartFunctionTest(void *arguments);

/**
 * Tests the thread pool implementation.
 */
void testThreadPool();

/**
 * Tests the linked list structure.
 */
void testLinkedList();

/**
 * Tests the array list structure.
 */
void testArrayList();

/**
 * Tests the hash map structure.
 */
void testHashMap();

/**
 * Teste the base 64 encoding.
 */
void testBase64();

/**
 * Teste the huffman encoding.
 */
void testHuffman();

/**
 * Runs the set of simple tests in the current
 * test case.
 */
void runSimpleTests();
