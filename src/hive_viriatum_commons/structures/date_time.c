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

#include "date_time.h"

void create_date_time(struct date_time_t **date_time_pointer) {
    /* retrieves the date time size */
    size_t date_time_size = sizeof(struct date_time_t);

    /* allocates space for the date time */
    struct date_time_t *date_time = (struct date_time_t *) MALLOC(date_time_size);

    /* sets the buffer in the date time pointer */
    *date_time_pointer = date_time;
}

void delete_date_time(struct date_time_t *date_time) {
    /* releases the data time */
    FREE(date_time);
}
