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

#include "thread_pool.h"

void createThreadPool(struct ThreadPool_t **threadPoolPointer, size_t numberThreads, size_t schedulingAlgorithm, size_t maximumNumberThreads) {
    /* allocates space for the index */
    size_t index;

    /* retrieves the thread pool size */
    size_t threadPoolSize = sizeof(struct ThreadPool_t);

    /* allocates space for the thread pool */
    struct ThreadPool_t *threadPool = (struct ThreadPool_t *) malloc(threadPoolSize);

    /* sets the number of threads */
    threadPool->numberThreads = numberThreads;

    /* sets the scheduling algorithm */
    threadPool->schedulingAlgorithm = schedulingAlgorithm;

    /* sets the maximum number of threads */
    threadPool->maximumNumberThreads = maximumNumberThreads;

    /* initializes the thread pool current number of threads */
    threadPool->currentNumberThreads = 0;

    /* creates the task condition */
    CONDITION_CREATE(threadPool->taskCondition);

    /* creates the task condition lock */
    CRITICAL_SECTION_CREATE(threadPool->taskConditionLock);

    /* creates the worker threads list */
    createLinkedList(&threadPool->workerThreadsList);

    /* creates the task queue */
    createLinkedList(&threadPool->taskQueue);

    /* iterates over the number of threas */
    for(index = 0; index < numberThreads; index++) {
        /* creates a thread pool elememt for the thread pool */
        createThreadPoolElement(threadPool);
    }

    /* sets the thread pool in the thread pool pointer */
    *threadPoolPointer = threadPool;
}

void deleteThreadPool(struct ThreadPool_t *threadPool) {
    /* closes the task condition */
    CONDITION_CLOSE(threadPool->taskCondition);

    /* delete the worker threads list */
    deleteLinkedList(threadPool->workerThreadsList);

    /* deletes the task queue */
    deleteLinkedList(threadPool->taskQueue);

    /* releases the thread pool */
    free(threadPool);
}

void createThreadPoolElement(struct ThreadPool_t *threadPool) {
    /* allocates space for the thread id */
    THREAD_IDENTIFIER threadId;

    /* retrieves the thread pool element size */
    size_t threadPoolElementSize = sizeof(struct ThreadPoolElement_t);

    /* allocates space for the thread pool element */
    struct ThreadPoolElement_t *threadPoolElement = (struct ThreadPoolElement_t *) malloc(threadPoolElementSize);

    /* creates the engine runnner thread */
    THREAD_HANDLE threadHandle = THREAD_CREATE_BASE(threadId, poolRunnerThread, (THREAD_ARGUMENTS) threadPool);

    /* sets the thread pool element attributes */
    threadPoolElement->threadHandle = threadHandle;
    threadPoolElement->threadId = threadId;

    /* adds the thread pool element to the thread pool worker threads list */
    appendValueLinkedList(threadPool->workerThreadsList, (void *) threadPoolElement);

    /* increments the current number of threads */
    threadPool->currentNumberThreads++;
}

void insertTaskThreadPool(struct ThreadPool_t *threadPool, struct ThreadPoolTask_t *threadPoolTask) {
    /* locks the task condition lock */
    CONDITION_LOCK(threadPool->taskCondition, threadPool->taskConditionLock);

    /* adds the the thread pool task to the task queue */
    appendValueLinkedList(threadPool->taskQueue, threadPoolTask);

    /* signals the task condition */
    CONDITION_SIGNAL(threadPool->taskCondition);

    /* unlock the task condition lock */
    CONDITION_UNLOCK(threadPool->taskCondition, threadPool->taskConditionLock);
}

/*
* Thread than runs a new stage. It controls the frequency of the loading
* and manages it.
*
* @param parameters The thread parameters.
* @return The thread result.
*/
THREAD_RETURN poolRunnerThread(THREAD_ARGUMENTS parameters) {
    /* allocates space for the work thread task */
    struct ThreadPoolTask_t *workThreadTask;

    /* retrieves the thread pool from the arguments */
    struct ThreadPool_t *threadPool = (struct ThreadPool_t *) parameters;

    /* ierates continuously */
    while(1) {
        /* locks the task condition lock */
        CONDITION_LOCK(threadPool->taskCondition, threadPool->taskConditionLock);

        /* iterates while the task queue is empty */
        while(threadPool->taskQueue->size == 0) {
            /* waits for the task condition */
            CONDITION_WAIT(threadPool->taskCondition, threadPool->taskConditionLock);
        }

        /* retrieves the work thread task */
        popValueLinkedList(threadPool->taskQueue, (void **) &workThreadTask);

        /* unlock the task condition lock */
        CONDITION_UNLOCK(threadPool->taskCondition, threadPool->taskConditionLock);

        /* calls the start function */
        workThreadTask->startFunction(NULL);
    }

    /* returns valid */
    return 0;
}
