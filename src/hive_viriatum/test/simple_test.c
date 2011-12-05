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
    struct ThreadPoolTask_t *threadPoolTask = (struct ThreadPoolTask_t *) MALLOC(sizeof(struct ThreadPoolTask_t));

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
    removeValueLinkedList(linkedList, (void *) 1, 1);

    /* removes an element from the linked list */
    removeIndexLinkedList(linkedList, 1, 1);

    /* pops a value from the linked list */
    popValueLinkedList(linkedList, (void **) &value, 1);

    /* pops a value from the linked list */
    popValueLinkedList(linkedList, (void **) &value, 1);

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
    createHashMap(&hashMap, 3);

    /* sets and retrieves the value in the hash map */
    setValueHashMap(hashMap, 1, (void *) 1);
    getValueHashMap(hashMap, 1, &element);

    /* sets and retrieves the value in the hash map */
    setValueHashMap(hashMap, 2, (void *) 2);
    getValueHashMap(hashMap, 2, &element);

    /* sets and retrieves the value in the hash map,
    (thi set should for re-sizing) */
    setValueHashMap(hashMap, 3, (void *) 3);
    getValueHashMap(hashMap, 3, &element);

    /* sets and retrieves the value (using a string)
    in the hash map */
    setValueStringHashMap(hashMap, (unsigned char *) "test", (void *) 4);
    getValueStringHashMap(hashMap, (unsigned char *) "test", (void **) &element);

    /* deletes the hash map */
    deleteHashMap(hashMap);
}

void testStringBuffer() {
    /* allocates space for the string buffer */
    struct StringBuffer_t *stringBuffer;

    /* allocates the space for the string to
    hold the various joined values */
    unsigned char *stringValue;

    /* creates the string buffer */
    createStringBuffer(&stringBuffer);

    /* adds a set of string to the string buffer */
    appendStringBuffer(stringBuffer, (unsigned char *) "hello");
    appendStringBuffer(stringBuffer, (unsigned char *) " ");
    appendStringBuffer(stringBuffer, (unsigned char *) "world");

    /* "joins" the string buffer values into a single
    value (from the internal string list) */
    joinStringBuffer(stringBuffer, &stringValue);

    /* releases the string value (string) */
    FREE(stringValue);

    /* deletes the string buffer */
    deleteStringBuffer(stringBuffer);
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
    FREE(encodedBuffer);

    /* releases the decoded buffer */
    FREE(decodedBuffer);
}

void testHuffman() {
    /* allocates space for the huffman */
    struct Huffman_t *huffman;

    /* creates the huffman (encoder) */
    createHuffman(&huffman);

    /* generates the huffman table */
    generateTableHuffman(huffman, NULL);

    /* deletes the huffman (encoder) */
    deleteHuffman(huffman);
}

void testBitStream() {
    /* allocates space for the file stream */
    struct FileStream_t *fileStream;

    /* allocates space for the bit stream */
    struct BitStream_t *bitStream;

    /* creates the file stream */
    createFileStream(&fileStream, (unsigned char *) "bit_stream.txt", (unsigned char *) "wb");

    /* creates the bit stream */
    createBitStream(&bitStream, fileStream->stream);

    /* opens the bit stream */
    openBitStream(bitStream);

    /* writes the 0100 bit set to the bit stream
    and then writes the 0001 bit set */
    writeByteBitStream(bitStream, 0x04, 4);
    writeByteBitStream(bitStream, 0x01, 4);

    /* checks if the written 8 bits are
    01000001 (0x41) the correct value */
    assert(bitStream->buffer[0] == 0x41);

    /* writes a the 0100 bit set to the bit stream */
    writeByteBitStream(bitStream, 0x04, 4);

    /* closes the bit stream */
    closeBitStream(bitStream);

    /* deletes the bit stream */
    deleteBitStream(bitStream);

    /* deletes the file stream */
    deleteFileStream(fileStream);
}

void testFileStream() {
    /* allocates space for the file stream */
    struct FileStream_t *fileStream;

    /* allocates space for the stream */
    struct Stream_t *stream;

    /* allocates some space for the test buffer */
    unsigned char buffer[128];

    /* creates the file stream */
    createFileStream(&fileStream, (unsigned char *) "hello.txt", (unsigned char *) "wb");

    /* retrieves the stream from the file stream, in order
    to be able to use the "normal" stream functions */
    stream = fileStream->stream;

    /* opens the stream */
    stream->open(stream);

    /* writes some data to the stream */
    stream->write(stream, (unsigned char *) "hello world", 11);

    /* close the stream */
    stream->close(stream);

    /* creates the file stream */
    createFileStream(&fileStream, (unsigned char *) "hello.txt", (unsigned char *) "rb");

    /* retrieves the stream from the file stream, in order
    to be able to use the "normal" stream functions */
    stream = fileStream->stream;

    /* opens the stream */
    stream->open(stream);

    /* reads some data from the stream */
    stream->read(stream, buffer, 11);

    /* sets the end of string character in the buffer */
    buffer[11] = '\0';

    /* close the stream */
    stream->close(stream);

    /* compares the read string */
    assert(strcmp((char *) "hello world", (char *) buffer) == 0);

    /* deletes the file stream */
    deleteFileStream(fileStream);
}

void testTemplateHandler() {
    /* allocates space for the template handler */
    struct TemplateHandler_t *templateHandler;

    /* creates the template handler */
    createTemplateHandler(&templateHandler);

    /* processes the file as a template handler */
    processTemplateHandler(templateHandler, (unsigned char *) "test.tpl");

    /* deletes the template handler */
    deleteTemplateHandler(templateHandler);
}

void testQuicksort() {
    /* allocates space for the template handler */
    int list[10] = { 2, 4, 1, 2, 3, 5, 5, 3, 4, 1 };

    /* sorts the sequence according to the compare function */
    sortQuicksort((void **) list, 0, 10, _compare);
}

void testQuicksortLinkedList() {
    /* allocates space for the linked list */
    struct LinkedList_t *linkedList;

    /* creates the linked list */
    createLinkedList(&linkedList);

    /* adds some element to the linked list */
    appendValueLinkedList(linkedList, (void *) 2);
    appendValueLinkedList(linkedList, (void *) 3);
    appendValueLinkedList(linkedList, (void *) 1);

    /* retrieves a value from the linked list */
    sortLinkedList(linkedList, _compare);

    /* deletes the linked list */
    deleteLinkedList(linkedList);
}

void testMd5() {
    /* allocates space for the md5 result */
    char result[16];

    /* calculates the md5 hash value into the result */
    md5((unsigned char *) "Hello World", 11, result);
}

void testCrc32() {
    /* calculates the crc32 hash value and returns it */
    crc32((unsigned char *) "Hello World", 11);
}

int _compare(void *first, void *second) {
    /* in case the first element is smaller
    than the second element */
    if(first < second) {
        /* returns negative value */
        return -1;
    }
    /* in case the first value is larger
    than the second element */
    else if(first > second) {
        /* returns positive value */
        return 1;
    }

    /* returns zero value (equals) */
    return 0;
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

    /* tests the string buffer */
    testStringBuffer();

    /* tests the base 64 encoder */
    testBase64();

    /* tests the huffman encoder */
    testHuffman();

    /* tests the bit stream */
    testBitStream();

    /* tests the file stream */
    testFileStream();

    /* tests the template handler */
    testTemplateHandler();

    /* tests the quick sort algorithm */
    testQuicksort();
    testQuicksortLinkedList();

    /* tests the md5 hash function */
    testMd5();

    /* tests the crc32 hash function */
    testCrc32();
}
