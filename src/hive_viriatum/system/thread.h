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

#ifdef VIRIATUM_PLATFORM_WIN32
#define THREAD_VALID_RETURN_VALUE 0
#define THREAD_INVALID_RETURN_VALUE 1
#define THREAD_RETURN DWORD WINAPI
#define THREAD_ARGUMENTS LPVOID
#define THREAD_HANDLE HANDLE
#define THREAD_IDENTIFIER DWORD
#define THREAD_REFERENCE THREAD_HANDLE
#define THREAD_CREATE(threadAttributes, stackSize, startAddress, parameter, creationFlags, threadId) CreateThread(threadAttributes, stackSize, startAddress, (THREAD_ARGUMENTS) parameter, creationFlags, &threadId)
#define THREAD_CREATE_BASE(threadId, startAddress, parameter) THREAD_CREATE(NULL, 0, startAddress, (THREAD_ARGUMENTS) parameter, 0, threadId)
#define THREAD_JOIN(threadHandle) WaitForSingleObject(threadHandle, INFINITE)
#define THREAD_JOIN_BASE(threadHandle, threadIdentifier) THREAD_JOIN(threadHandle)
#define THREAD_CLOSE(threadHandle) CloseHandle(threadHandle)
#define MUTEX_HANDLE HANDLE
#define MUTEX_CREATE(mutexHandle) mutexHandle = CreateMutex(NULL, false, NULL)
#define MUTEX_LOCK(mutexHandle) WaitForSingleObject(mutexHandle, INFINITE)
#define MUTEX_UNLOCK(mutexHandle) ReleaseMutex(mutexHandle)
#define MUTEX_CLOSE(mutexHandle) CloseHandle(mutexHandle)
#define EVENT_HANDLE HANDLE
#define EVENT_CREATE(eventHandle) eventHandle = CreateEvent(NULL, true, false, NULL)
#define EVENT_WAIT(eventHandle) WaitForSingleObject(eventHandle, INFINITE)
#define EVENT_SIGNAL(eventHandle) SetEvent(eventHandle)
#define EVENT_RESET(eventHandle) ResetEvent(eventHandle)
#define EVENT_CLOSE(eventHandle) CloseHandle(eventHandle)
#define CRITICAL_SECTION_HANDLE CRITICAL_SECTION
#define CRITICAL_SECTION_CREATE(criticalSectionHandle) InitializeCriticalSection(&criticalSectionHandle)
#define CRITICAL_SECTION_ENTER(criticalSectionHandle) EnterCriticalSection(&criticalSectionHandle)
#define CRITICAL_SECTION_LEAVE(criticalSectionHandle) LeaveCriticalSection(&criticalSectionHandle)
#define CRITICAL_SECTION_CLOSE(criticalSectionHandle) DeleteCriticalSection(&criticalSectionHandle)
#define CONDITION_HANDLE CONDITION_VARIABLE
#define CONDITION_CREATE(conditionHandle) InitializeConditionVariable(&conditionHandle)
#define CONDITION_WAIT(conditionHandle, criticalSectionHandle) SleepConditionVariableCS(&conditionHandle, &criticalSectionHandle, INFINITE)
#define CONDITION_SIGNAL(conditionHandle) WakeConditionVariable(&conditionHandle)
#define CONDITION_CLOSE(conditionHandle)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
typedef struct EventHandle_t {
    pthread_cond_t event;
    pthread_mutex_t mutex;
} EventHandle;
#define THREAD_VALID_RETURN_VALUE 0
#define THREAD_INVALID_RETURN_VALUE 0
#define THREAD_RETURN void *
#define THREAD_ARGUMENTS void *
#define THREAD_HANDLE int
#define THREAD_IDENTIFIER pthread_t
#define THREAD_REFERENCE THREAD_IDENTIFIER
#define THREAD_CREATION_FUNCTION CreateThread
#define THREAD_CREATE(threadId, creationFlags, startAddress, parameter) pthread_create(&threadId, creationFlags, startAddress, (THREAD_ARGUMENTS) parameter)
#define THREAD_CREATE_BASE(threadId, startAddress, parameter) THREAD_CREATE(threadId, NULL, startAddress, parameter)
#define THREAD_JOIN(threadIdentifier) pthread_join(threadIdentifier, NULL)
#define THREAD_JOIN_BASE(threadHandle, threadIdentifier) THREAD_JOIN(threadIdentifier)
#define THREAD_CLOSE(threadHandle)
#define MUTEX_HANDLE pthread_mutex_t *
#define MUTEX_CREATE(mutexHandle) mutexHandle = (MUTEX_HANDLE) malloc(sizeof(pthread_mutex_t));\
    pthread_mutex_init(mutexHandle, NULL)
#define MUTEX_LOCK(mutexHandle) pthread_mutex_lock(mutexHandle)
#define MUTEX_UNLOCK(mutexHandle) pthread_mutex_unlock(mutexHandle)
#define MUTEX_CLOSE(mutexHandle) pthread_mutex_destroy(mutexHandle);\
    free(mutexHandle)
#define EVENT_HANDLE EventHandle_t *
#define EVENT_CREATE(eventHandle) eventHandle = (EVENT_HANDLE) malloc(sizeof(EventHandle_t));\
    pthread_mutex_init(&eventHandle->mutex, NULL);\
    pthread_cond_init(&eventHandle->event, NULL)
#define EVENT_WAIT(eventHandle) pthread_mutex_lock(&eventHandle->mutex);\
    pthread_cond_wait(&eventHandle->event, &eventHandle->mutex);\
    pthread_mutex_unlock(&eventHandle->mutex)
#define EVENT_SIGNAL(eventHandle) pthread_cond_signal(&eventHandle->event)
#define EVENT_RESET(eventHandle)
#define EVENT_CLOSE(eventHandle) pthread_mutex_destroy(&eventHandle->mutex);\
    pthread_cond_destroy(&eventHandle->event);\
    free(eventHandle)
#define CRITICAL_SECTION_HANDLE MUTEX_HANDLE
#define CRITICAL_SECTION_CREATE(criticalSectionHandle) MUTEX_CREATE(criticalSectionHandle)
#define CRITICAL_SECTION_ENTER(criticalSectionHandle) MUTEX_LOCK(criticalSectionHandle)
#define CRITICAL_SECTION_LEAVE(criticalSectionHandle) MUTEX_UNLOCK(criticalSectionHandle)
#define CRITICAL_SECTION_CLOSE(criticalSectionHandle) MUTEX_CLOSE(criticalSectionHandle)
#define CONDITION_HANDLE pthread_cond_t *
#define CONDITION_CREATE(conditionHandle) conditionHandle = (CONDITION_HANDLE) malloc(sizeof(pthread_cond_t));\
pthread_cond_init(conditionHandle, NULL)
#define CONDITION_WAIT(conditionHandle, criticalSectionHandle) pthread_cond_wait(conditionHandle, criticalSectionHandle)
#define CONDITION_SIGNAL(conditionHandle) pthread_cond_signal(conditionHandle)
#define CONDITION_CLOSE(conditionHandle) pthread_cond_destroy(conditionHandle);\
free(conditionHandle)
#endif
