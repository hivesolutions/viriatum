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

#ifndef VIRIATUM_NO_THREADS

#ifdef VIRIATUM_PLATFORM_WIN32

#include "_thread_win32.h"

void create_condition(struct condition_t **condition_pointer) {
    /* retrieves the condition size */
    size_t condition_size = sizeof(struct condition_t);

    /* allocates space for the condition */
    struct condition_t *condition = (struct condition_t *) MALLOC(condition_size);

    /* starts the lock count */
    condition->lock_count = 0;

    /* initializes the wait critical section */
    InitializeCriticalSection(&condition->wait_critical_section);

    /* initializes the lock critical section */
    InitializeCriticalSection(&condition->lock_critical_section);

    /* creates the wait set */
    create_linked_list(&condition->wait_set);

    /* sets the condition in the condition pointer */
    *condition_pointer = condition;
}

void delete_condition(struct condition_t *condition) {
    /* deletes the wait set */
    delete_linked_list(condition->wait_set);

    /* deletes the lock critical section */
    DeleteCriticalSection(&condition->lock_critical_section);

    /* deletes the wait critical section */
    DeleteCriticalSection(&condition->wait_critical_section);

    /* releases the condition */
    FREE(condition);
}

void lock_condition(struct condition_t *condition) {
    /* enters the lock critical section */
    EnterCriticalSection(&condition->lock_critical_section);

    /* increments the condition lock count */
    condition->lock_count++;
}

void unlock_condition(struct condition_t *condition) {
    /* in case the test lock owner condition fails */
    if(!_test_lock_owner_condition(condition)) {
        SetLastError(ERROR_INVALID_FUNCTION);
    }

    /* decrements the condition lock count */
    condition->lock_count--;

    /* leaves the lock critical section */
    LeaveCriticalSection(&condition->lock_critical_section);
}

void wait_condition(struct condition_t *condition) {
    /* allocates the index */
    unsigned int index;

    /* allocates the wait event */
    HANDLE wait_event;

    /* allocates the wait result */
    DWORD wait_result;

    /* saves the current lock count */
    unsigned int current_lock_count = condition->lock_count;

    /* pushes the wait event */
    _push_condition(condition, &wait_event);

    /* in case the wait event is not set */
    if(wait_event == NULL) {
        /* returns immediately */
        return;
    }

    /* resets the lock count */
    condition->lock_count = 0;

    /* iterates over the current lock count */
    for(index = 0; index < current_lock_count; index++) {
        /* leaves the lock critical section */
        LeaveCriticalSection(&condition->lock_critical_section);
    }

    /* waits for the event to become signalled */
    wait_result = WaitForSingleObjectEx(wait_event, INFINITE, 0);

  // If the wait failed, store the last error because it will get
  // overwritten when acquiring the lock.
    /* DWORD dw_last_error;
     if( WAIT_FAILED == dw_wait_result )
        dw_last_error = ::GetLastError();*/

    /* iterates over the current lock count */
    for(index = 0; index < current_lock_count; index++) {
        /* enters the lock critical section */
        EnterCriticalSection(&condition->lock_critical_section);
    }

    /* restores the lock count */
    condition->lock_count = current_lock_count;

    /* in case the closing of the wait event failed */
    if(!CloseHandle(wait_event)) {
        /* returns in failure */
        //return WAIT_FAILED;
        return;
    }

  /*if( WAIT_FAILED == dw_wait_result )
    ::SetLastError(dw_last_error);*/

    /*return dw_wait_result;*/
    return;
}

void notify_condition(struct condition_t *condition) {
    /* allocates the wait event */
    HANDLE wait_event;

    /* pops the wait event */
    _pop_condition(condition, &wait_event);

    /* in case the wait event is not set */
    if(wait_event == NULL) {
        /* returns immediately */
        return;
    }

    /* signals the event */
    SetEvent(wait_event);
}

void _push_condition(struct condition_t *condition, HANDLE *wait_event_pointer) {
    /* creates the new event */
    HANDLE wait_event = CreateEvent(NULL, 0, 0, NULL);

    /* in case the creation of the wait event failed */
    if(wait_event == NULL) {
        /* returns immediately */
        return;
    }

    /* enters the wait critical section */
    EnterCriticalSection(&condition->wait_critical_section);

    /* adds the wait event to the wait set */
    append_value_linked_list(condition->wait_set, (void *) wait_event);

    /* leaves the wait critical section */
    LeaveCriticalSection(&condition->wait_critical_section);

    /* sets the wait event in the wait event pointer */
    *wait_event_pointer = wait_event;
}

void _pop_condition(struct condition_t *condition, HANDLE *wait_event_pointer) {
    /* allocates the wait event */
    HANDLE wait_event;

    /* enters the wait critical section */
    EnterCriticalSection(&condition->wait_critical_section);

    /* pops the wait event from the wait set */
    pop_value_linked_list(condition->wait_set, (void **) &wait_event, 0);

    /* leaves the wait critical section */
    LeaveCriticalSection(&condition->wait_critical_section);

    /* sets the wait event in the wait event pointer */
    *wait_event_pointer = wait_event;
}

unsigned char _test_lock_owner_condition(struct condition_t *condition) {
    /* tries to enter the lock critical section */
    BOOL try_lock_result = TryEnterCriticalSection(&condition->lock_critical_section);

    /* in case the lock is not successfully held */
    if(!try_lock_result) {
        /* returns invalid */
        return 0;
    }

    /* if we got the lock, but the lock count is zero, then nobody had it */
    if(condition->lock_count == 0) {
        /* asserts that the try lock result is valid */
        assert(try_lock_result);

        /* leaves the lock critical section */
        LeaveCriticalSection(&condition->lock_critical_section);

        /* returns invalid */
        return 0;
    }

    /* asserts that the try lock result is valid
    and that the lock count is greater than zero */
    assert(try_lock_result && condition->lock_count > 0);

    /* leaves the lock critical section */
    LeaveCriticalSection(&condition->lock_critical_section);

    /* returns valid */
    return 1;
}

#endif

#endif
