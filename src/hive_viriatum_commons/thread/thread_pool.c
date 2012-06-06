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

#ifndef VIRIATUM_NO_THREADS

void create_thread_pool(struct ThreadPool_t **threadPoolPointer, size_t numberThreads, size_t schedulingAlgorithm, size_t maximumNumberThreads) {
    /* allocates space for the index */
    size_t index;

    /* retrieves the thread pool size */
    size_t threadPoolSize = sizeof(struct ThreadPool_t);

    /* allocates space for the thread pool */
    struct ThreadPool_t *thread_pool = (struct ThreadPool_t *) MALLOC(threadPoolSize);

    /* sets the number of threads */
    thread_pool->numberThreads = numberThreads;

    /* sets the scheduling algorithm */
    thread_pool->schedulingAlgorithm = schedulingAlgorithm;

    /* sets the maximum number of threads */
    thread_pool->maximumNumberThreads = maximumNumberThreads;

    /* initializes the thread pool current number of threads */
    thread_pool->currentNumberThreads = 0;

    /* creates the task condition */
    CONDITION_CREATE(thread_pool->taskCondition);

    /* creates the task condition lock */
    CRITICAL_SECTION_CREATE(thread_pool->taskConditionLock);

    /* creates the worker threads list */
    create_linked_list(&thread_pool->workerThreadsList);

    /* creates the task queue */
    create_linked_list(&thread_pool->taskQueue);

    /* iterates over the number of threas */
    for(index = 0; index < numberThreads; index++) {
        /* creates a thread pool elememt for the thread pool */
        createThreadPoolElement(thread_pool);
    }

    /* sets the thread pool in the thread pool pointer */
    *threadPoolPointer = thread_pool;
}

void deleteThreadPool(struct ThreadPool_t *thread_pool) {
    /* closes the task condition */
    CONDITION_CLOSE(thread_pool->taskCondition);

    /* delete the worker threads list */
    delete_linked_list(thread_pool->workerThreadsList);

    /* deletes the task queue */
    delete_linked_list(thread_pool->taskQueue);

    /* releases the thread pool */
    FREE(thread_pool);
}

void createThreadPoolElement(struct ThreadPool_t *thread_pool) {
    /* allocates space for the thread id */
    THREAD_IDENTIFIER thread_id;

    /* retrieves the thread pool element size */
    size_t threadPoolElementSize = sizeof(struct ThreadPoolElement_t);

    /* allocates space for the thread pool element */
    struct ThreadPoolElement_t *threadPoolElement = (struct ThreadPoolElement_t *) MALLOC(threadPoolElementSize);

    /* creates the engine runnner thread */
    THREAD_HANDLE thread_handle = THREAD_CREATE_BASE(thread_id, poolRunnerThread, (THREAD_ARGUMENTS) thread_pool);

    /* sets the thread pool element attributes */
    threadPoolElement->thread_handle = thread_handle;
    threadPoolElement->thread_id = thread_id;

    /* adds the thread pool element to the thread pool worker threads list */
    append_value_linked_list(thread_pool->workerThreadsList, (void *) threadPoolElement);

    /* increments the current number of threads */
    thread_pool->currentNumberThreads++;
}

void insert_task_thread_pool(struct ThreadPool_t *thread_pool, struct ThreadPoolTask_t *thread_pool_task) {
    /* locks the task condition lock */
    CONDITION_LOCK(thread_pool->taskCondition, thread_pool->taskConditionLock);

    /* adds the the thread pool task to the task queue */
    append_value_linked_list(thread_pool->taskQueue, thread_pool_task);

    /* signals the task condition */
    CONDITION_SIGNAL(thread_pool->taskCondition);

    /* unlock the task condition lock */
    CONDITION_UNLOCK(thread_pool->taskCondition, thread_pool->taskConditionLock);
}

THREAD_RETURN poolRunnerThread(THREAD_ARGUMENTS parameters) {
    /* allocates space for the work thread task */
    struct ThreadPoolTask_t *workThreadTask;

    /* retrieves the thread pool from the arguments */
    struct ThreadPool_t *thread_pool = (struct ThreadPool_t *) parameters;

    /* ierates continuously */
    while(1) {
        /* locks the task condition lock */
        CONDITION_LOCK(thread_pool->taskCondition, thread_pool->taskConditionLock);

        /* iterates while the task queue is empty */
        while(thread_pool->taskQueue->size == 0) {
            /* waits for the task condition */
            CONDITION_WAIT(thread_pool->taskCondition, thread_pool->taskConditionLock);
        }

        /* retrieves the work thread task */
        pop_value_linked_list(thread_pool->taskQueue, (void **) &workThreadTask, 0);

        /* unlock the task condition lock */
        CONDITION_UNLOCK(thread_pool->taskCondition, thread_pool->taskConditionLock);

        /* calls the start function */
        workThreadTask->start_function(NULL);
    }

    /* returns valid */
    return 0;
}

#endif
