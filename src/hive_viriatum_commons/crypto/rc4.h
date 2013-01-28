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

#pragma once

#define RC4_PRINT_OFFSET 10000
#define RC4_BUFFER_SIZE 4096
#define RC4_TEST_MESSAGE "Lorem ipsum dolor sit amet, consectetur\
vestibulum, mauris et ultrices luctus, ligula turpis semper lectus, sit\
amet tincidunt arcu eros in massa. Sed lobortis tincidunt vestibulum.\
Integer aliquam suscipit nibh, sit amet volutpat enim tempus in.\
Curabitur nisl orci, bibendum sit amet porta sed, viverra laoreet massa.\
Praesent quis mi in arcu vulputate ornare. Duis sed volutpat magna.\
Duis suscipit leo sed neque varius aliquet. Phasellus eu justo erat, non\
sagittis sem. Donec magna orci, tristique nec pharetra sed, rutrum at\
eros. Etiam ultricies diam eget justo fermentum vestibulum. In hac\
habitasse platea dictumst. Etiam eget mi risus.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque\
vestibulum, mauris et ultrices luctus, ligula turpis semper lectus, sit\
amet tincidunt arcu eros in massa. Sed lobortis tincidunt vestibulum.\
Integer aliquam suscipit nibh, sit amet volutpat enim tempus in.\
Curabitur nisl orci, bibendum sit amet porta sed, viverra laoreet massa.\
Praesent quis mi in arcu vulputate ornare. Duis sed volutpat magna.\
Duis suscipit leo sed neque varius aliquet. Phasellus eu justo erat, non\
sagittis sem. Donec magna orci, tristique nec pharetra sed, rutrum at\
eros. Etiam ultricies diam eget justo fermentum vestibulum. In hac\
habitasse platea dictumst. Etiam eget mi risus.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque\
vestibulum, mauris et ultrices luctus, ligula turpis semper lectus, sit\
amet tincidunt arcu eros in massa. Sed lobortis tincidunt vestibulum.\
Integer aliquam suscipit nibh, sit amet volutpat enim tempus in.\
Curabitur nisl orci, bibendum sit amet porta sed, viverra laoreet massa.\
Praesent quis mi in arcu vulputate ornare. Duis sed volutpat magna.\
Duis suscipit leo sed neque varius aliquet. Phasellus eu justo erat, non\
sagittis sem. Donec magna orci, tristique nec pharetra sed, rutrum at\
eros. Etiam ultricies diam eget justo fermentum vestibulum. In hac\
habitasse platea dictumst. Etiam eget mi risus."

typedef struct rc4_t {
    char *key;
    unsigned char x;
    unsigned char y;
    unsigned char box[255];
} rc4;

void create_rc4(char *key, struct rc4_t **rc4_pointer);
void delete_rc4(struct rc4_t *rc4);
void cipher_md4(struct rc4_t *rc4, char *data, size_t size, char *data_c);
void crypt_md4(char *message, char *key, char **result_pointer);
