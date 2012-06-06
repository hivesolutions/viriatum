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

/**
 * Structure defining the various
 * components of both a date and
 * a time description.
 */
typedef struct date_time_t {
    /**
     * The year component of the date.
     */
    unsigned short year;

    /**
     * The month component of the date.
     */
    unsigned short month;

    /**
     * The day component of the date.
     */
    unsigned short day;

    /**
     * The hour component of the time.
     */
    unsigned short hour;

    /**
     * The minute component of the time.
     */
    unsigned short minute;

    /**
     * The second component of the time.
     */
    unsigned short second;

    /**
     * The milisecond component of the time.
     */
    unsigned short milisecond;
} date_time;

VIRIATUM_EXPORT_PREFIX void create_date_time(struct date_time_t **date_time_pointer);
VIRIATUM_EXPORT_PREFIX void delete_date_time(struct date_time_t *data_time);
