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

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "../hive_viriatum_commons/viriatum_commons.h"

int threadPoolStartFunctionTest(void *arguments) {
	/* retrieves the current thread identifier */
	THREAD_IDENTIFIER threadId = THREAD_GET_IDENTIFIER();

	/* prints an hello world message */
	printf("hello world from thread: %d\n", threadId);

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

	/* creates the thread pool */
	createThreadPool(&threadPool, 15, 1, 30);

	/* iterates over the range of the index */
	for(index = 0; index < 100; index ++) {
		/* inserts the task in the thread pool */
		insertTaskThreadPool(threadPool, threadPoolTask);
	}
}

void testLinkedList() {
	/* allocates space for the element */
    unsigned int element;

	/* allocates space for the linked list */
	struct LinkedList_t *linkedList;

	/* creates the linked list */
    createLinkedList(&linkedList);

    /* adds some element to the linked list */
    appendLinkedList(linkedList, (void *) 1);
    appendLinkedList(linkedList, (void *) 2);

    /* retrieves an element from the linked list */
    getLinkedList(linkedList, 1, (void **) &element);

	/* removes an element from the linked list */
    removeLinkedList(linkedList, 1);

	/* pops an element from the linked list */
	popLinkedList(linkedList, (void **) &element);

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

void readFile(unsigned char *filePath, unsigned char **bufferPointer, size_t *fileSizePointer) {
	/* allocates space for the file */
    FILE *file;
	
	/* allocates space for the file size */
	size_t fileSize;

	/* allocates space for the file buffer */
	unsigned char *fileBuffer;

	/* opens the file */
    file = fopen((char *) filePath, "rb");

	/* seeks the file until the end */
    fseek(file, 0, SEEK_END);

	/* retrieves the file size */
    fileSize = ftell (file);

	/* seeks the file until the beginning */
	fseek(file, 0, SEEK_SET);

	/* allocates space for the file buffer */
	fileBuffer = (unsigned char *) malloc(fileSize);

	/* reads the file contents */
    fread(fileBuffer, 1, fileSize, file);

	/* sets the buffer as the buffer pointer */
	*bufferPointer = fileBuffer;

	/* sets the file size as the file size pointer */
	*fileSizePointer = fileSize;
}

void testHashMap() {
	/* allocates space for the first element */
	unsigned int firstElement = 1;

	/* allocates space for the second element */
	unsigned int secondElement = 1;

	/* allocates space for the hash map */
    struct HashMap_t *hashMap;

    /* creates the hash map */
    createHashMap(&hashMap, 0);

    /* sets and retrieves the value in the hash map */
    setHashMap(hashMap, 123123, (void **) &firstElement);
    getHashMap(hashMap, 123123, (void **) &secondElement);

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

    /* encodes the value into base64 */
    encodeBase64((unsigned char *) buffer, strlen(buffer), &encodedBuffer, &encodedBufferLength);

	/* releases the encoded buffer */
	free(encodedBuffer);
}

void runTests() {
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

int main(int argc, char *argv[]) {
    /* allocates the socket data */
    SOCKET_DATA socketData;

    /* allocates the socket handle */
    SOCKET_HANDLE socketHandle;

    /* allocates the socket address structure */
    SOCKET_ADDRESS_INPUT socketAddress;

    /* allocates the client socket handle */
    SOCKET_HANDLE clientSocketHandle;

    /* allocates the client socket address structure */
    SOCKET_ADDRESS clientSocketAddress;

    /* allocates the client socket address size */
    unsigned int clientSocketAddressSize;

    /* allocates the result */
    SOCKET_ERROR_CODE result;

    /* allocates the "simple" buffer */
    char buffer[10240];

    char response[1000];

    unsigned int n;

	/* allocates space for the file path */
    unsigned char *filePath;

	/* allocates space for the file buffer */
	size_t fileSize;

	/* allocates space for the file buffer */
    unsigned char *fileBuffer;

	char optionValue;

	/* allocates space for the sockets set */
	fd_set socketsSet;

	/* allocates space for the select timeout value */
	struct timeval selectTimeout;

	/* in case the number of arguments is bigger than one */
    if(argc > 1) {
		/* sets the file path as the first argument */
        filePath = (unsigned char *) argv[1];
    } else {
		/* sets the file path as a static file */
        filePath = (unsigned char *) "C:\\Desert.jpg";
    }

	/* runs the tests */
	runTests();

	/* reads the file */
	readFile(filePath, &fileBuffer, &fileSize);

	/* writes the http static headers to the response */
    sprintf(response, "HTTP/1.1 200 OK\r\nServer: viriatum/1.0.0\r\nContent-Length: %d\r\n\r\n", fileSize);

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the socket for the given types */
    socketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

	/* sets the option value to one */
	optionValue = 1,

	/* sets the socket reuse address option in the socket */
	SOCKET_SET_OPTIONS(socketHandle, SOCKET_OPTIONS_LEVEL_SOCKET, SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET, optionValue);

    /* tests the socket for errors */
    if(SOCKET_TEST_SOCKET(socketHandle)) {
        /* retrieves the socket error code */
        SOCKET_ERROR_CODE socketErrorCode = SOCKET_GET_ERROR_CODE(socketHandle);

        /* prints the error */
        printf("Problem creating socket: %d", socketErrorCode);

        /* runs the socket finish */
        SOCKET_FINISH();

        /* returns immediately */
        return 0;
    }

    /* sets the socket address attributes */
    socketAddress.sin_family = SOCKET_INTERNET_TYPE;
    socketAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
    socketAddress.sin_port = htons(8080);

    /* binds the socket */
    result = SOCKET_BIND(socketHandle, socketAddress);

    /* in case there was an error binding the socket */
    if(SOCKET_TEST_ERROR(result)) {
        /* retrieves the binding error code */
        SOCKET_ERROR_CODE bindingErrorCode = SOCKET_GET_ERROR_CODE(result);

        /* prints the error */
        printf("Problem binding socket: %d", bindingErrorCode);

        /* closes the socket */
        SOCKET_CLOSE(socketHandle);

        /* runs the socket finish */
        SOCKET_FINISH();

        /* returns immediately */
        return 0;
    }

    /* listens for a socket change */
    SOCKET_LISTEN(socketHandle);

    /* calculates the size of the socket address */
	clientSocketAddressSize = sizeof(socketAddress);

    /* iterates continuously */
    while(1) {
		/* zeros the sockets set */
		FD_ZERO(&socketsSet);

		/* adds the socket handle to the sockets set */
		FD_SET(socketHandle, &socketsSet);

		/* sets the timeout value */
		selectTimeout.tv_sec = 1;
		selectTimeout.tv_usec = 0;

		select(1, &socketsSet, NULL, NULL, &selectTimeout);

        /* accepts the socket */
        clientSocketHandle = SOCKET_ACCEPT(socketHandle, &clientSocketAddress, clientSocketAddressSize);

        /* reveives from the client socket */
        n = SOCKET_RECEIVE(clientSocketHandle, buffer, 10240, 0);

        /* in case no bytes are sent */
        if(n < 0) {
            /* prints an error message */
            printf("error in receive");
        }

        /* sends the data */
        n = SOCKET_SEND(clientSocketHandle, response, strlen(response), 0);
        n = SOCKET_SEND(clientSocketHandle, fileBuffer, fileSize, 0);

        /* closes the client socket */
        SOCKET_CLOSE(clientSocketHandle);
    }

    /* runs the socket finish */
    SOCKET_FINISH();

    /* returns zero (valid) */
    return 0;
}
