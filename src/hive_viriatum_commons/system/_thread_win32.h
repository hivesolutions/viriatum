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

#ifdef VIRIATUM_PLATFORM_WIN32

#include "../structures/linked_list.h"

typedef struct Condition_t {
    unsigned int lockCount;
    CRITICAL_SECTION waitCriticalSection;
    CRITICAL_SECTION lockCriticalSection;
    struct LinkedList_t *waitSet;
} Condition;

VIRIATUM_EXPORT_PREFIX void createCondition(struct Condition_t **conditionPointer);
VIRIATUM_EXPORT_PREFIX void deleteCondition(struct Condition_t *condition);
VIRIATUM_EXPORT_PREFIX void lockCondition(struct Condition_t *condition);
VIRIATUM_EXPORT_PREFIX void unlockCondition(struct Condition_t *condition);
VIRIATUM_EXPORT_PREFIX void waitCondition(struct Condition_t *condition);
VIRIATUM_EXPORT_PREFIX void notifyCondition(struct Condition_t *condition);

VIRIATUM_NO_EXPORT_PREFIX void _pushCondition(struct Condition_t *condition, HANDLE *waitEventPointer);
VIRIATUM_NO_EXPORT_PREFIX void _popCondition(struct Condition_t *condition, HANDLE *waitEventPointer);
VIRIATUM_NO_EXPORT_PREFIX unsigned char _testLockOwnerCondition(struct Condition_t *condition);

#endif
