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

#include "simple_test.h"

int threadPoolStartFunctionTest(void *arguments) {
    /* retrieves the current thread identifier */
    THREAD_IDENTIFIER threadId = THREAD_GET_IDENTIFIER();

    /* prints an hello world message */
    V_DEBUG_F("hello world from thread: %lu\n", (unsigned long) threadId);

    /* sleeps for a while */
    SLEEP(10);

    /* returns valid */
    return 0;
}

void testThreadPool() {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the thread pool */
    struct ThreadPool_t *threadPool;

    /* allocates space for the thread pool task */
    struct ThreadPoolTask_t *threadPoolTask = (struct ThreadPoolTask_t *) malloc(sizeof(struct ThreadPoolTask_t));

    /* sets the start function */
    threadPoolTask->startFunction = threadPoolStartFunctionTest;

    /* prints a debug message */
    V_DEBUG("Creating a thread pool\n");

    /* creates the thread pool */
    createThreadPool(&threadPool, 15, 1, 30);

    /* iterates over the range of the index */
    for(index = 0; index < 100; index ++) {
        /* prints a debug message */
        V_DEBUG("Inserting task in thread pool\n");

        /* inserts the task in the thread pool */
        insertTaskThreadPool(threadPool, threadPoolTask);
    }
}

void testLinkedList() {
    /* allocates space for the value */
    void *value;

    /* allocates space for the linked list */
    struct LinkedList_t *linkedList;

    /* creates the linked list */
    createLinkedList(&linkedList);

    /* adds some element to the linked list */
    appendValueLinkedList(linkedList, (void *) 1);
    appendValueLinkedList(linkedList, (void *) 2);
    appendValueLinkedList(linkedList, (void *) 3);

    /* retrieves a value from the linked list */
    getValueLinkedList(linkedList, 1, &value);

    /* removes a value from the linked list */
    removeValueLinkedList(linkedList, (void *) 1);

    /* removes an element from the linked list */
    removeIndexLinkedList(linkedList, 1);

    /* pops a value from the linked list */
    popValueLinkedList(linkedList, &value);

    /* pops a value from the linked list */
    popValueLinkedList(linkedList, &value);

    /* deletes the linked list */
    deleteLinkedList(linkedList);
}

void testArrayList() {
    /* allocates space for the element */
    unsigned int element = 1;

    /* allocates space for the element pointer */
    unsigned int *elementPointer;

    /* allocates space for the array list */
    struct ArrayList_t *arrayList;

    /* creates the array list */
    createArrayList(&arrayList, sizeof(unsigned int), 0);

    /* sets and retrieves the value in the array list */
    setArrayList(arrayList, 0, (void **) &element);
    getArrayList(arrayList, 0, (void **) &elementPointer);

    /* deletes the array list */
    deleteArrayList(arrayList);
}

void testHashMap() {
    /* allocates space for the element */
    void *element;

    /* allocates space for the hash map */
    struct HashMap_t *hashMap;

    /* creates the hash map */
    createHashMap(&hashMap, 0);

    /* sets and retrieves the value in the hash map */
    setValueHashMap(hashMap, 123123, (void *) 1);
    getValueHashMap(hashMap, 123123, &element);

    /* deletes the hash map */
    deleteHashMap(hashMap);
}

void testBase64() {
    /* allocates space for the buffer */
    char buffer[] = "hello world";

    /* allocates space for the encoded buffer */
    unsigned char *encodedBuffer;

    /* allocates space for the encoded buffer length */
    size_t encodedBufferLength;

    /* allocates space for the decoded buffer */
    unsigned char *decodedBuffer;

    /* allocates space for the decoded buffer length */
    size_t decodedBufferLength;

    /* encodes the value into base64 */
    encodeBase64((unsigned char *) buffer, strlen(buffer), &encodedBuffer, &encodedBufferLength);

    /* decodes the value from base64 */
    decodeBase64(encodedBuffer, encodedBufferLength, &decodedBuffer, &decodedBufferLength);

    /* releases the encoded buffer */
    free(encodedBuffer);

    /* releases the decoded buffer */
    free(decodedBuffer);
}

void runSimpleTests() {
    /* tests the thread pool */
    testThreadPool();

    /* tests the linked list */
    testLinkedList();

    /* tests the array list */
    testArrayList();

    /* tests the hash map */
    testHashMap();

    /* tests the base 64 encoder */
    testBase64();
}
