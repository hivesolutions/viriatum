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

#include "../structures/linked_list.h"

typedef struct condition_t {
    unsigned int lock_count;
    CRITICAL_SECTION wait_critical_section;
    CRITICAL_SECTION lock_critical_section;
    struct linked_list_t *wait_set;
} condition;

VIRIATUM_EXPORT_PREFIX void create_condition(struct condition_t **condition_pointer);
VIRIATUM_EXPORT_PREFIX void delete_condition(struct condition_t *condition);
VIRIATUM_EXPORT_PREFIX void lock_condition(struct condition_t *condition);
VIRIATUM_EXPORT_PREFIX void unlock_condition(struct condition_t *condition);
VIRIATUM_EXPORT_PREFIX void wait_condition(struct condition_t *condition);
VIRIATUM_EXPORT_PREFIX void notify_condition(struct condition_t *condition);

VIRIATUM_NO_EXPORT_PREFIX void _push_condition(struct condition_t *condition, HANDLE *wait_event_pointer);
VIRIATUM_NO_EXPORT_PREFIX void _pop_condition(struct condition_t *condition, HANDLE *wait_event_pointer);
VIRIATUM_NO_EXPORT_PREFIX unsigned char _test_lock_owner_condition(struct condition_t *condition);

#endif

#endif
