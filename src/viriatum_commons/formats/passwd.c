/*
 Hive Viriatum Commons
 Copyright (c) 2008-2019 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "passwd.h"

ERROR_CODE process_passwd_file(char *file_path, struct hash_map_t **passwd_pointer) {
    /* allocates space for th variable used to store the
    error code result from the various calls */
    ERROR_CODE return_value;

    /* allocates the space for the various size related
    variables for dimensions of arrays */
    size_t size;
    size_t index;
    size_t length;

    /* allocates space for the current (byte) value in
    iteration (for parsing loop) and for the current state */
    unsigned char current;
    enum passwd_state_e state;

    /* allocates space for the various pointers to be used
    durring the parsing of the passwd file */
    unsigned char *mark;
    unsigned char *buffer;
    unsigned char *pointer;

    /* allocates the space for the username and for the password
    to be parsed, note that the username is allocated in stack
    (static memory) as it's not going to be stored in map */
    char username[128];
    char *password;

    /* creates the "final" hash map that is going to be used to
    associated a username with a password and then allocates the
    internal structures for it (no pre-defined size) */
    struct hash_map_t *passwd;
    create_hash_map(&passwd, 0);

    /* reads the complete set of contents from the file with the
    provided path an in case an error occurs re-raises it */
    return_value = read_file(file_path, &buffer, &size);
    if(IS_ERROR_CODE(return_value)) {
        RAISE_ERROR_F(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem reading file %s",
            file_path
        );
    }

    /* sets the initial values for the mark pointer and for the
    state control flag, these represent the start of the loop */
    mark = buffer;
    state = PASSWD_USER;

    /* iterates over the buffer contents in its range to parse
    it and populate the passwd hash map structure */
    for(index = 0; index < size; index++) {
        /* retrieves the current byte in iteration and the pointer
        address to the current buffer position */
        current = buffer[index];
        pointer = &buffer[index];

        /* switches over the current state to operate over the current
        buffer using the proper operation */
        switch(state) {
            case PASSWD_USER:
                switch(current) {
                    case ':':
                    case '\n':
                        length = pointer - mark;
                        memcpy(username, mark, length);
                        username[length] = '\0';
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
                        password = (char *) MALLOC(length + 1);
                        memcpy(password, mark, length);
                        password[length] = '\0';
                        mark = pointer + 1;
                        set_value_string_hash_map(
                            passwd,
                            (unsigned char *) username,
                            password
                        );
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
