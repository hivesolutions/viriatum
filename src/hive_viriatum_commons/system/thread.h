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

#ifdef VIRIATUM_PLATFORM_WIN32
#define THREAD_VALID_RETURN_VALUE 0
#define THREAD_INVALID_RETURN_VALUE 1
#define THREAD_RETURN DWORD WINAPI
#define THREAD_ARGUMENTS LPVOID
#define THREAD_HANDLE HANDLE
#define THREAD_IDENTIFIER DWORD
#define THREAD_REFERENCE THREAD_HANDLE
#define THREAD_CREATE(thread_attributes, stack_size, start_address, parameter, creation_flags, thread_id) CreateThread(thread_attributes, stack_size, start_address, (THREAD_ARGUMENTS) parameter, creation_flags, &thread_id)
#define THREAD_CREATE_BASE(thread_id, start_address, parameter) THREAD_CREATE(NULL, 0, start_address, (THREAD_ARGUMENTS) parameter, 0, thread_id)
#define THREAD_JOIN(thread_handle) WaitForSingleObject(thread_handle, INFINITE)
#define THREAD_JOIN_BASE(thread_handle, thread_identifier) THREAD_JOIN(thread_handle)
#define THREAD_CLOSE(thread_handle) CloseHandle(thread_handle)
#define THREAD_GET_IDENTIFIER() GetCurrentThreadId()
#define MUTEX_HANDLE HANDLE
#define MUTEX_CREATE(mutex_handle) mutex_handle = CreateMutex(NULL, 0, NULL)
#define MUTEX_LOCK(mutex_handle) WaitForSingleObject(mutex_handle, INFINITE)
#define MUTEX_UNLOCK(mutex_handle) ReleaseMutex(mutex_handle)
#define MUTEX_CLOSE(mutex_handle) CloseHandle(mutex_handle)
#define EVENT_HANDLE HANDLE
#define EVENT_CREATE(event_handle) event_handle = CreateEvent(NULL, 1, 0, NULL)
#define EVENT_WAIT(event_handle) WaitForSingleObject(event_handle, INFINITE)
#define EVENT_SIGNAL(event_handle) SetEvent(event_handle)
#define EVENT_RESET(event_handle) ResetEvent(event_handle)
#define EVENT_CLOSE(event_handle) CloseHandle(event_handle)
#define CRITICAL_SECTION_HANDLE CRITICAL_SECTION
#define CRITICAL_SECTION_CREATE(critical_section_handle) InitializeCriticalSection(&critical_section_handle)
#define CRITICAL_SECTION_ENTER(critical_section_handle) EnterCriticalSection(&critical_section_handle)
#define CRITICAL_SECTION_LEAVE(critical_section_handle) LeaveCriticalSection(&critical_section_handle)
#define CRITICAL_SECTION_CLOSE(critical_section_handle) DeleteCriticalSection(&critical_section_handle)
#define CONDITION_HANDLE struct condition_t*
#define CONDITION_CREATE(condition_handle) create_condition(&condition_handle)
#define CONDITION_LOCK(condition_handle, critical_section_handle) lock_condition(condition_handle)
#define CONDITION_UNLOCK(condition_handle, critical_section_handle) unlock_condition(condition_handle)
#define CONDITION_WAIT(condition_handle, critical_section_handle) wait_condition(condition_handle)
#define CONDITION_SIGNAL(condition_handle) notify_condition(condition_handle)
#define CONDITION_CLOSE(condition_handle) delete_condition(condition_handle)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#include <pthread.h>
typedef struct event_handle_t {
    pthread_cond_t event;
    pthread_mutex_t mutex;
} event_handle;
#define THREAD_VALID_RETURN_VALUE 0
#define THREAD_INVALID_RETURN_VALUE 0
#define THREAD_RETURN void *
#define THREAD_ARGUMENTS void *
#define THREAD_HANDLE int
#define THREAD_IDENTIFIER pthread_t
#define THREAD_REFERENCE THREAD_IDENTIFIER
#define THREAD_CREATION_FUNCTION CreateThread
#define THREAD_CREATE(thread_id, creation_flags, start_address, parameter) pthread_create(&thread_id, creation_flags, start_address, (THREAD_ARGUMENTS) parameter)
#define THREAD_CREATE_BASE(thread_id, start_address, parameter) THREAD_CREATE(thread_id, NULL, start_address, parameter)
#define THREAD_JOIN(thread_identifier) pthread_join(thread_identifier, NULL)
#define THREAD_JOIN_BASE(thread_handle, thread_identifier) THREAD_JOIN(thread_identifier)
#define THREAD_CLOSE(thread_handle)
#define THREAD_GET_IDENTIFIER() pthread_self()
#define MUTEX_HANDLE pthread_mutex_t *
#define MUTEX_CREATE(mutex_handle) mutex_handle = (MUTEX_HANDLE) MALLOC(sizeof(pthread_mutex_t));\
    pthread_mutex_init(mutex_handle, NULL)
#define MUTEX_LOCK(mutex_handle) pthread_mutex_lock(mutex_handle)
#define MUTEX_UNLOCK(mutex_handle) pthread_mutex_unlock(mutex_handle)
#define MUTEX_CLOSE(mutex_handle) pthread_mutex_destroy(mutex_handle);\
    FREE(mutex_handle)
#define EVENT_HANDLE EventHandle_t *
#define EVENT_CREATE(event_handle) event_handle = (EVENT_HANDLE) MALLOC(sizeof(EventHandle_t));\
    pthread_mutex_init(&event_handle->mutex, NULL);\
    pthread_cond_init(&event_handle->event, NULL)
#define EVENT_WAIT(event_handle) pthread_mutex_lock(&event_handle->mutex);\
    pthread_cond_wait(&event_handle->event, &event_handle->mutex);\
    pthread_mutex_unlock(&event_handle->mutex)
#define EVENT_SIGNAL(event_handle) pthread_cond_signal(&event_handle->event)
#define EVENT_RESET(event_handle)
#define EVENT_CLOSE(event_handle) pthread_mutex_destroy(&event_handle->mutex);\
    pthread_cond_destroy(&event_handle->event);\
    FREE(event_handle)
#define CRITICAL_SECTION_HANDLE MUTEX_HANDLE
#define CRITICAL_SECTION_CREATE(critical_section_handle) MUTEX_CREATE(critical_section_handle)
#define CRITICAL_SECTION_ENTER(critical_section_handle) MUTEX_LOCK(critical_section_handle)
#define CRITICAL_SECTION_LEAVE(critical_section_handle) MUTEX_UNLOCK(critical_section_handle)
#define CRITICAL_SECTION_CLOSE(critical_section_handle) MUTEX_CLOSE(critical_section_handle)
#define CONDITION_HANDLE pthread_cond_t *
#define CONDITION_CREATE(condition_handle) condition_handle = (CONDITION_HANDLE) MALLOC(sizeof(pthread_cond_t));\
pthread_cond_init(condition_handle, NULL)
#define CONDITION_LOCK(condition_handle, critical_section_handle) CRITICAL_SECTION_ENTER(critical_section_handle)
#define CONDITION_UNLOCK(condition_handle, critical_section_handle) CRITICAL_SECTION_LEAVE(critical_section_handle)
#define CONDITION_WAIT(condition_handle, critical_section_handle) pthread_cond_wait(condition_handle, critical_section_handle);
#define CONDITION_SIGNAL(condition_handle) pthread_cond_signal(condition_handle)
#define CONDITION_CLOSE(condition_handle) pthread_cond_destroy(condition_handle);\
    FREE(condition_handle)
#endif

#endif
