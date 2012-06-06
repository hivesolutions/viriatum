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

void create_thread_pool(struct thread_pool_t **thread_pool_pointer, size_t number_threads, size_t scheduling_algorithm, size_t maximum_number_threads) {
    /* allocates space for the index */
    size_t index;

    /* retrieves the thread pool size */
    size_t thread_pool_size = sizeof(struct thread_pool_t);

    /* allocates space for the thread pool */
    struct thread_pool_t *thread_pool = (struct thread_pool_t *) MALLOC(thread_pool_size);

    /* sets the number of threads */
    thread_pool->number_threads = number_threads;

    /* sets the scheduling algorithm */
    thread_pool->scheduling_algorithm = scheduling_algorithm;

    /* sets the maximum number of threads */
    thread_pool->maximum_number_threads = maximum_number_threads;

    /* initializes the thread pool current number of threads */
    thread_pool->current_number_threads = 0;

    /* creates the task condition */
    CONDITION_CREATE(thread_pool->task_condition);

    /* creates the task condition lock */
    CRITICAL_SECTION_CREATE(thread_pool->task_condition_lock);

    /* creates the worker threads list */
    create_linked_list(&thread_pool->worker_threads_list);

    /* creates the task queue */
    create_linked_list(&thread_pool->task_queue);

    /* iterates over the number of threas */
    for(index = 0; index < number_threads; index++) {
        /* creates a thread pool elememt for the thread pool */
        create_thread_pool_element(thread_pool);
    }

    /* sets the thread pool in the thread pool pointer */
    *thread_pool_pointer = thread_pool;
}

void delete_thread_pool(struct thread_pool_t *thread_pool) {
    /* closes the task condition */
    CONDITION_CLOSE(thread_pool->task_condition);

    /* delete the worker threads list */
    delete_linked_list(thread_pool->worker_threads_list);

    /* deletes the task queue */
    delete_linked_list(thread_pool->task_queue);

    /* releases the thread pool */
    FREE(thread_pool);
}

void create_thread_pool_element(struct thread_pool_t *thread_pool) {
    /* allocates space for the thread id */
    THREAD_IDENTIFIER thread_id;

    /* retrieves the thread pool element size */
    size_t thread_pool_element_size = sizeof(struct thread_pool_element_t);

    /* allocates space for the thread pool element */
    struct thread_pool_element_t *thread_pool_element = (struct thread_pool_element_t *) MALLOC(thread_pool_element_size);

    /* creates the engine runnner thread */
    THREAD_HANDLE thread_handle = THREAD_CREATE_BASE(thread_id, pool_runner_thread, (THREAD_ARGUMENTS) thread_pool);

    /* sets the thread pool element attributes */
    thread_pool_element->thread_handle = thread_handle;
    thread_pool_element->thread_id = thread_id;

    /* adds the thread pool element to the thread pool worker threads list */
    append_value_linked_list(thread_pool->worker_threads_list, (void *) thread_pool_element);

    /* increments the current number of threads */
    thread_pool->current_number_threads++;
}

void insert_task_thread_pool(struct thread_pool_t *thread_pool, struct thread_pool_task_t *thread_pool_task) {
    /* locks the task condition lock */
    CONDITION_LOCK(thread_pool->task_condition, thread_pool->task_condition_lock);

    /* adds the the thread pool task to the task queue */
    append_value_linked_list(thread_pool->task_queue, thread_pool_task);

    /* signals the task condition */
    CONDITION_SIGNAL(thread_pool->task_condition);

    /* unlock the task condition lock */
    CONDITION_UNLOCK(thread_pool->task_condition, thread_pool->task_condition_lock);
}

THREAD_RETURN pool_runner_thread(THREAD_ARGUMENTS parameters) {
    /* allocates space for the work thread task */
    struct thread_pool_task_t *work_thread_task;

    /* retrieves the thread pool from the arguments */
    struct thread_pool_t *thread_pool = (struct thread_pool_t *) parameters;

    /* ierates continuously */
    while(1) {
        /* locks the task condition lock */
        CONDITION_LOCK(thread_pool->task_condition, thread_pool->task_condition_lock);

        /* iterates while the task queue is empty */
        while(thread_pool->task_queue->size == 0) {
            /* waits for the task condition */
            CONDITION_WAIT(thread_pool->task_condition, thread_pool->task_condition_lock);
        }

        /* retrieves the work thread task */
        pop_value_linked_list(thread_pool->task_queue, (void **) &work_thread_task, 0);

        /* unlock the task condition lock */
        CONDITION_UNLOCK(thread_pool->task_condition, thread_pool->task_condition_lock);

        /* calls the start function */
        work_thread_task->start_function(NULL);
    }

    /* returns valid */
    return 0;
}

#endif
