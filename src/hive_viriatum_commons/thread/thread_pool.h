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

#include "../structures/structures.h"
#include "../system/system.h"

typedef struct ThreadPoolTask_t {
    int (*startFunction)(void *arguments);
    int (*stopFunction)(void *arguments);
    int (*pauseFunction)(void *arguments);
    int (*resumeFunction)(void *arguments);
} ThreadPoolTask;

typedef struct ThreadPoolElement_t {
    THREAD_HANDLE threadHandle;
    THREAD_IDENTIFIER threadId;
} ThreadPoolElement;

typedef struct ThreadPool_t {
    size_t numberThreads;
    size_t schedulingAlgorithm;
    size_t maximumNumberThreads;
    size_t currentNumberThreads;
    CONDITION_HANDLE taskCondition;
    CRITICAL_SECTION_HANDLE taskConditionLock;
    struct LinkedList_t *workerThreadsList;
    struct LinkedList_t *taskQueue;
} ThreadPool;

VIRIATUM_EXPORT_PREFIX void createThreadPool(struct ThreadPool_t **threadPoolPointer, size_t numberThreads, size_t schedulingAlgorithm, size_t maximumNumberThreads);
VIRIATUM_EXPORT_PREFIX void deleteThreadPool(struct ThreadPool_t *threadPool);
VIRIATUM_EXPORT_PREFIX void createThreadPoolElement(struct ThreadPool_t *threadPool);
VIRIATUM_EXPORT_PREFIX void insertTaskThreadPool(struct ThreadPool_t *threadPool, struct ThreadPoolTask_t *threadPoolTask);

VIRIATUM_EXPORT_PREFIX THREAD_RETURN poolRunnerThread(THREAD_ARGUMENTS parameters);
