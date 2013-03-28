/*
 Hive Viriatum Commons
 Copyright (C) 2008-2012 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "passwd.h"

ERROR_CODE process_passwd_file(char *file_path, struct hash_map_t **passwd_pointer) {
	size_t size;
	size_t index;
	size_t length;
	ERROR_CODE return_value;
	unsigned char current;

	unsigned char state;

	unsigned char *mark;
	unsigned char *buffer;
	unsigned char *pointer;

	char name[128];
	char *value;

    struct hash_map_t *passwd;
    
	create_hash_map(&passwd, 0);

	return_value = read_file(file_path, &buffer, &size);
    if(IS_ERROR_CODE(return_value)) {
		RAISE_ERROR_M(
		    RUNTIME_EXCEPTION_ERROR_CODE,
			(unsigned char *) "Problem reading file"
		);
	}

	mark = buffer;
	state = PASSWD_USER;

	for(index = 0; index < size; index++) {
		current = buffer[index];
		pointer = &buffer[index];

		switch(state) {
			case PASSWD_USER:
				switch(current) {
					case ':':
					case '\n':
						length = pointer - mark;
						memcpy(name, mark, length);
						name[length] = '\0';
						mark = pointer + 1;
						state = current == ':' ? PASSWD_PASSWORD : PASSWD_USER;
						break;
				}

				break;

			case PASSWD_PASSWORD:
				switch(current) {
					case ':':
					case '\n':
						length = pointer - mark;
						value = (char *) MALLOC(length + 1);
						memcpy(value, mark, length);
						value[length] = '\0';
						mark = pointer + 1;
						set_value_string_hash_map(passwd, name, value);
						state = current == ':' ? PASSWD_COMMENT : PASSWD_USER;
						break;
				}

				break;

			case PASSWD_COMMENT:
				switch(current) {
					case '\n':
						state = PASSWD_USER;
						mark = pointer + 1;
						break;
				}

				break;
		}
	}

	/* releases the buffer with the file contents used during
	the parsing of the passwd file (avoids memory leaks) */
	FREE(buffer);

	/* updates the passwd structure pointer with the currently
	used hash map structure containing the various username to
	password association, that may be used for verification */
	*passwd_pointer = passwd;
	RAISE_NO_ERROR;
}