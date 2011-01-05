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

#include "../hive_viriatum_commons/viriatum_commons.h"

int threadPoolTest(void *arguments) {
	/* retrieves the current thread identifier */
	THREAD_IDENTIFIER threadId = THREAD_GET_IDENTIFIER();

	/* prints an hello world message */
	printf("hello world from thread: %d", threadId);

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
	threadPoolTask->startFunction = threadPoolTest;

	/* creates the thread pool */
	createThreadPool(&threadPool, 15, 1, 30);

	/* iterates over the range of the index */
	for(index = 0; index < 100; index ++) {
		/* inserts the task in the thread pool */
		insertTaskThreadPool(threadPool, threadPoolTask);
	}
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

    struct HashMap_t *hashMap;

    struct ArrayList_t *arrayList;

    struct LinkedList_t *linkedList;

    char rabeton[] = "como vai a vida";

    /* allocates the "simple" buffer */
    char buffer[10240];

    char response[1000];

    char *buffer2 = malloc(1000000);
    size_t fileSize;

    unsigned int n;

    char *fileName;

    FILE *file;

    unsigned int value;

    unsigned int tobias = 123;

    unsigned int *matias;

    unsigned char *receiver;

    size_t receiverLength;

    if(argc > 1) {
        fileName = argv[1];
    } else {
        fileName = "C:\\Desert.jpg";
    }

	/* tests the thread pool */
	testThreadPool();

    /* encodes the value into base64 */
    encodeBase64((unsigned char *) rabeton, strlen(rabeton), &receiver, &receiverLength);

    /* creates the hash map */
    createHashMap(&hashMap, 0);

    /* sets and retrieves the value in the hash map */
    setHashMap(hashMap, 123123, (void **) &tobias);
    getHashMap(hashMap, 123123, (void **) &matias);

    /* creates the array list */
    createArrayList(&arrayList, sizeof(unsigned int), 0);

    /* sets and retrieves the value in the array list */
    setArrayList(arrayList, 0, (void **) &tobias);
    getArrayList(arrayList, 0, (void **) &matias);

    /* creates the linked list */
    createLinkedList(&linkedList);

    /* adds some element to the linked list */
    appendLinkedList(linkedList, (void *) 12);
    appendLinkedList(linkedList, (void *) 13);

    /* retrieves an element from the linked list */
    getLinkedList(linkedList, 1, (void **) &value);

    file = fopen(fileName, "rb");

    fseek (file , 0 , SEEK_END);
    fileSize = ftell (file);
    rewind (file);

    fread(buffer2, 1, fileSize, file);

    sprintf(response, "HTTP/1.1 200 OK\r\nServer: viriatum/1.0.0\r\nContent-Length: %d\r\n\r\n", fileSize);

    /* initializes the socket infrastructure */
    SOCKET_INITIALIZE(&socketData);

    /* creates the socket for the given types */
    socketHandle = SOCKET_CREATE(SOCKET_INTERNET_TYPE, SOCKET_PACKET_TYPE, SOCKET_PROTOCOL_TCP);

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

    /* iterates continuously */
    while(1) {
        /* calculates the size of the socket address */
        clientSocketAddressSize = sizeof(socketAddress);

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
        n = SOCKET_SEND(clientSocketHandle, buffer2, fileSize, 0);

        /* closes the client socket */
        SOCKET_CLOSE(clientSocketHandle);
    }

    /* runs the socket finish */
    SOCKET_FINISH();

    /* returns zero (valid) */
    return 0;
}
