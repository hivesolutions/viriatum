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

#ifndef VIRIATUM_NO_THREADS

#include "../structures/structures.h"
#include "../system/system.h"

typedef struct thread_pool_task_t {
    int (*start_function)(void *arguments);
    int (*stop_function)(void *arguments);
    int (*pause_function)(void *arguments);
    int (*resume_function)(void *arguments);
} thread_pool_task;

typedef struct thread_pool_element_t {
    THREAD_HANDLE thread_handle;
    THREAD_IDENTIFIER thread_id;
} thread_pool_element;

typedef struct thread_pool_t {
    size_t number_threads;
    size_t scheduling_algorithm;
    size_t maximum_number_threads;
    size_t current_number_threads;
    CONDITION_HANDLE task_condition;
    CRITICAL_SECTION_HANDLE task_condition_lock;
    struct linked_list_t *worker_threads_list;
    struct linked_list_t *task_queue;
} thread_pool;

VIRIATUM_EXPORT_PREFIX void create_thread_pool(struct thread_pool_t **thread_pool_pointer, size_t number_threads, size_t scheduling_algorithm, size_t maximum_number_threads);
VIRIATUM_EXPORT_PREFIX void delete_thread_pool(struct thread_pool_t *thread_pool);
VIRIATUM_EXPORT_PREFIX void create_thread_pool_element(struct thread_pool_t *thread_pool);
VIRIATUM_EXPORT_PREFIX void insert_task_thread_pool(struct thread_pool_t *thread_pool, struct thread_pool_task_t *thread_pool_task);

/**
 * Thread than runs a new stage. It controls the frequency of the loading
 * and manages it.
 *
 * @param parameters The thread parameters.
 * @return The thread result.
 */
VIRIATUM_EXPORT_PREFIX THREAD_RETURN pool_runner_thread(THREAD_ARGUMENTS parameters);

#endif
