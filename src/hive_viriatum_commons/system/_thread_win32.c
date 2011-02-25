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

#ifdef VIRIATUM_PLATFORM_WIN32

#include "_thread_win32.h"

void createCondition(struct Condition_t **conditionPointer) {
    /* retrieves the condition size */
    size_t conditionSize = sizeof(struct Condition_t);

    /* allocates space for the condition */
    struct Condition_t *condition = (struct Condition_t *) malloc(conditionSize);

    /* starts the lock count */
    condition->lockCount = 0;

    /* initializes the wait critical section */
    InitializeCriticalSection(&condition->waitCriticalSection);

    /* initializes the lock critical section */
    InitializeCriticalSection(&condition->lockCriticalSection);

    /* creates the wait set */
    createLinkedList(&condition->waitSet);

    /* sets the condition in the condition pointer */
    *conditionPointer = condition;
}

void deleteCondition(struct Condition_t *condition) {
    /* deletes the wait set */
    deleteLinkedList(condition->waitSet);

    /* deletes the lock critical section */
    DeleteCriticalSection(&condition->lockCriticalSection);

    /* deletes the wait critical section */
    DeleteCriticalSection(&condition->waitCriticalSection);

    /* releases the condition */
    free(condition);
}

void lockCondition(struct Condition_t *condition) {
    /* enters the lock critical section */
    EnterCriticalSection(&condition->lockCriticalSection);

    /* increments the condition lock count */
    condition->lockCount++;
}

void unlockCondition(struct Condition_t *condition) {
    /* in case the test lock owner condition fails */
    if(!_testLockOwnerCondition(condition)) {
        SetLastError(ERROR_INVALID_FUNCTION);
    }

    /* decrements the condition lock count */
    condition->lockCount--;

    /* leaves the lock critical section */
    LeaveCriticalSection(&condition->lockCriticalSection);
}

void waitCondition(struct Condition_t *condition) {
    /* allocates the index */
    unsigned int index;

    /* allocates the wait event */
    HANDLE waitEvent;

    /* allocates the wait result */
    DWORD waitResult;

    /* saves the current lock count */
    unsigned int currentLockCount = condition->lockCount;

    /* pushes the wait event */
    _pushCondition(condition, &waitEvent);

    /* in case the wait event is not set */
    if(waitEvent == NULL) {
        /* returns immediately */
        return;
    }

    /* resets the lock count */
    condition->lockCount = 0;

    /* iterates over the current lock count */
    for(index = 0; index < currentLockCount; index++) {
        /* leaves the lock critical section */
        LeaveCriticalSection(&condition->lockCriticalSection);
    }

    /* waits for the event to become signalled */
    waitResult = WaitForSingleObjectEx(waitEvent, INFINITE, 0);

  // If the wait failed, store the last error because it will get
  // overwritten when acquiring the lock.
    /* DWORD dwLastError;
      if( WAIT_FAILED == dwWaitResult )
        dwLastError = ::GetLastError();*/

    /* iterates over the current lock count */
    for(index = 0; index < currentLockCount; index++) {
        /* enters the lock critical section */
        EnterCriticalSection(&condition->lockCriticalSection);
    }

    /* restores the lock count */
    condition->lockCount = currentLockCount;

    /* in case the closing of the wait event failed */
    if(!CloseHandle(waitEvent)) {
        /* returns in failure */
        //return WAIT_FAILED;
        return;
    }

  /*if( WAIT_FAILED == dwWaitResult )
    ::SetLastError(dwLastError);*/

    /*return dwWaitResult;*/
    return;
}

void notifyCondition(struct Condition_t *condition) {
    /* allocates the wait event */
    HANDLE waitEvent;

    /* pops the wait event */
    _popCondition(condition, &waitEvent);

    /* in case the wait event is not set */
    if(waitEvent == NULL) {
        /* returns immediately */
        return;
    }

    /* signals the event */
    SetEvent(waitEvent);
}

void _pushCondition(struct Condition_t *condition, HANDLE *waitEventPointer) {
    /* creates the new event */
    HANDLE waitEvent = CreateEvent(NULL, 0, 0, NULL);

    /* in case the creation of the wait event failed */
    if(waitEvent == NULL) {
        /* returns immediately */
        return;
    }

    /* enters the wait critical section */
    EnterCriticalSection(&condition->waitCriticalSection);

    /* adds the wait event to the wait set */
    appendValueLinkedList(condition->waitSet, (void *) waitEvent);

    /* leaves the wait critical section */
    LeaveCriticalSection(&condition->waitCriticalSection);

    /* sets the wait event in the wait event pointer */
    *waitEventPointer = waitEvent;
}

void _popCondition(struct Condition_t *condition, HANDLE *waitEventPointer) {
    /* allocates the wait event */
    HANDLE waitEvent;

    /* enters the wait critical section */
    EnterCriticalSection(&condition->waitCriticalSection);

    /* pops the wait event from the wait set */
    popValueLinkedList(condition->waitSet, (void **) &waitEvent);

    /* leaves the wait critical section */
    LeaveCriticalSection(&condition->waitCriticalSection);

    /* sets the wait event in the wait event pointer */
    *waitEventPointer = waitEvent;
}

unsigned char _testLockOwnerCondition(struct Condition_t *condition) {
    /* tries to enter the lock critical section */
    BOOL tryLockResult = TryEnterCriticalSection(&condition->lockCriticalSection);

    /* in case the lock is not successfully held */
    if(!tryLockResult) {
        /* returns invalid */
        return 0;
    }

    // If we got the lock, but the lock count is zero, then nobody had it.
    //
    if(condition->lockCount == 0) {
        /* asserts that the try lock result is valid */
        assert(tryLockResult);

        /* leaves the lock critical section */
        LeaveCriticalSection(&condition->lockCriticalSection);

        /* returns invalid */
        return 0;
    }

    /* asserts that the try lock result is valid
    and that the lock count is greater than zero */
    assert(tryLockResult && condition->lockCount > 0);

    /* leaves the lock critical section */
    LeaveCriticalSection(&condition->lockCriticalSection);

    /* returns valid */
    return 1;
}

#endif
