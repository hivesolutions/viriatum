/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "simple_test.h"

#ifndef VIRIATUM_NO_THREADS

int thread_pool_start_function_test(void *arguments) {
    /* retrieves the current thread identifier */
    THREAD_IDENTIFIER thread_id = THREAD_GET_IDENTIFIER();

    /* prints an hello world message */
    V_DEBUG_F("hello world from thread: %lu\n", (unsigned long) thread_id);

    /* sleeps for a while */
    SLEEP(10);

    /* returns valid */
    return 0;
}

void test_thread_pool() {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the thread pool */
    struct thread_pool_t *thread_pool;

    /* allocates space for the thread pool task */
    struct thread_pool_task_t *thread_pool_task = (struct thread_pool_task_t *) MALLOC(sizeof(struct thread_pool_task_t));

    /* sets the start function */
    thread_pool_task->start_function = thread_pool_start_function_test;

    /* prints a debug message */
    V_DEBUG("Creating a thread pool\n");

    /* creates the thread pool */
    create_thread_pool(&thread_pool, 15, 1, 30);

    /* iterates over the range of the index */
    for(index = 0; index < 100; index ++) {
        /* prints a debug message */
        V_DEBUG("Inserting task in thread pool\n");

        /* inserts the task in the thread pool */
        insert_task_thread_pool(thread_pool, thread_pool_task);
    }
}

#endif

void test_linked_list() {
    /* allocates space for the value */
    void *value;

    /* allocates space for the linked list */
    struct linked_list_t *linked_list;

    /* creates the linked list */
    create_linked_list(&linked_list);

    /* adds some element to the linked list */
    append_value_linked_list(linked_list, (void *) 1);
    append_value_linked_list(linked_list, (void *) 2);
    append_value_linked_list(linked_list, (void *) 3);

    /* retrieves a value from the linked list */
    get_value_linked_list(linked_list, 1, &value);

    /* removes a value from the linked list */
    remove_value_linked_list(linked_list, (void *) 1, 1);

    /* removes an element from the linked list */
    remove_index_linked_list(linked_list, 1, 1);

    /* pops a value from the linked list */
    pop_value_linked_list(linked_list, (void **) &value, 1);

    /* pops a value from the linked list */
    pop_value_linked_list(linked_list, (void **) &value, 1);

    /* deletes the linked list */
    delete_linked_list(linked_list);
}

void test_array_list() {
    /* allocates space for the element */
    unsigned int element = 1;

    /* allocates space for the element pointer */
    unsigned int *element_pointer;

    /* allocates space for the array list */
    struct array_list_t *array_list;

    /* creates the array list */
    create_array_list(&array_list, sizeof(unsigned int), 0);

    /* sets and retrieves the value in the array list */
    set_array_list(array_list, 0, (void **) &element);
    get_array_list(array_list, 0, (void **) &element_pointer);

    /* deletes the array list */
    delete_array_list(array_list);
}

void test_hash_map() {
    /* allocates space for the element */
    void *element;

    /* allocates space for the hash map */
    struct hash_map_t *hash_map;

    /* creates the hash map */
    create_hash_map(&hash_map, 3);

    /* sets and retrieves the value in the hash map */
    set_value_hash_map(hash_map, 1, NULL, (void *) 1);
    get_value_hash_map(hash_map, 1, NULL, &element);

    /* sets and retrieves the value in the hash map */
    set_value_hash_map(hash_map, 2, NULL, (void *) 2);
    get_value_hash_map(hash_map, 2, NULL, &element);

    /* sets and retrieves the value in the hash map,
    (thi set should for re-sizing) */
    set_value_hash_map(hash_map, 3, NULL, (void *) 3);
    get_value_hash_map(hash_map, 3, NULL, &element);

    /* sets and retrieves the value (using a string)
    in the hash map */
    set_value_string_hash_map(hash_map, (unsigned char *) "test", (void *) 4);
    get_value_string_hash_map(hash_map, (unsigned char *) "test", (void **) &element);

    /* deletes the hash map */
    delete_hash_map(hash_map);
}

void test_sort_map() {
    /* allocates space for the element */
    void *element;

    /* allocates space for the sort map */
    struct sort_map_t *sort_map;

    /* creates the sort map */
    create_sort_map(&sort_map, 3);

    /* sets and retrieves the value in the sort map */
    set_value_sort_map(sort_map, 1, NULL, (void *) 1);
    get_value_sort_map(sort_map, 1, NULL, &element);

    /* sets and retrieves the value in the sort map */
    set_value_sort_map(sort_map, 2, NULL, (void *) 2);
    get_value_sort_map(sort_map, 2, NULL, &element);

    /* sets and retrieves the value in the sort map,
    (thi set should for re-sizing) */
    set_value_sort_map(sort_map, 3, NULL, (void *) 3);
    get_value_sort_map(sort_map, 3, NULL, &element);

    /* sets and retrieves the value (using a string)
    in the sort map */
    set_value_string_sort_map(sort_map, (unsigned char *) "test", (void *) 4);
    get_value_string_sort_map(sort_map, (unsigned char *) "test", (void **) &element);

    /* deletes the sort map */
    delete_sort_map(sort_map);
}

void test_string_buffer() {
    /* allocates space for the string buffer */
    struct string_buffer_t *string_buffer;

    /* allocates the space for the string to
    hold the various joined values */
    unsigned char *string_value;

    /* creates the string buffer */
    create_string_buffer(&string_buffer);

    /* adds a set of strings to the string buffer */
    append_string_buffer(string_buffer, (unsigned char *) "hello");
    append_string_buffer(string_buffer, (unsigned char *) " ");
    append_string_buffer(string_buffer, (unsigned char *) "world");

    /* "joins" the string buffer values into a single
    value (from the internal string list) */
    join_string_buffer(string_buffer, &string_value);

    /* releases the string value (string) */
    FREE(string_value);

    /* deletes the string buffer */
    delete_string_buffer(string_buffer);
}

void test_linked_buffer() {
    /* allocates space for the linked buffer */
    struct linked_buffer_t *linked_buffer;

    /* allocates the space for the buffer to
    hold the various joined values */
    unsigned char *buffer_value;

    /* creates the linked buffer */
    create_linked_buffer(&linked_buffer);

    /* adds a set of strings to the string buffer */
    append_linked_buffer(linked_buffer, (void *) "hello", 5, 0);
    append_linked_buffer(linked_buffer, (void *) " ", 1, 0);
    append_linked_buffer(linked_buffer, (void *) "world", 5, 0);

    /* "joins" the linked buffer values into a single
    value (from the internal buffer list) */
    join_linked_buffer(linked_buffer, &buffer_value);

    /* releases the buffer value */
    FREE(buffer_value);

    /* deletes the linked buffer */
    delete_linked_buffer(linked_buffer);
}

void test_base64() {
    /* allocates space for the buffer */
    char buffer[] = "hello world";

    /* allocates space for the encoded buffer */
    unsigned char *encoded_buffer;

    /* allocates space for the encoded buffer length */
    size_t encoded_buffer_length;

    /* allocates space for the decoded buffer */
    unsigned char *decoded_buffer;

    /* allocates space for the decoded buffer length */
    size_t decoded_buffer_length;

    /* encodes the value into base64 */
    encode_base64((unsigned char *) buffer, strlen(buffer), &encoded_buffer, &encoded_buffer_length);

    /* decodes the value from base64 */
    decode_base64(encoded_buffer, encoded_buffer_length, &decoded_buffer, &decoded_buffer_length);

    /* releases the encoded buffer */
    FREE(encoded_buffer);

    /* releases the decoded buffer */
    FREE(decoded_buffer);
}

void test_bencoding() {
    /* allocates space for the various type references
    and values and also dor the sequence structures */
    struct type_t *type;
    struct type_t _map_type;
    struct type_t _list_type;
    struct type_t _string_type;
    struct type_t _integer_type;
    struct hash_map_t *map;
    struct linked_list_t *list;

    /* allocates space for the encoded buffer reference
    and for the encoded buffer length integer value */
    unsigned char *encoded_buffer;
    size_t encoded_buffer_length;

    /* creates the sequence structures (map and list), initializing
    them in the simple (empty) way */
    create_hash_map(&map, 0);
    create_linked_list(&list);

    /* creates the various type structures from the internal raw values
    of them, uses the appropriate constructor functions */
    _map_type = map_type(map);
    _list_type = list_type(list);
    _string_type = string_type("world");
    _integer_type = integer_type(1234);

    /* adds the integer value to the list */
    append_value_linked_list(list, (void *) &_integer_type);

    /* sets the top level hash map values */
    set_value_string_hash_map(map, (unsigned char *) "hello", (void *) &_string_type);
    set_value_string_hash_map(map, (unsigned char *) "_hello", (void *) &_list_type);

    /* encodes the top level map type into the encoded buffer
    and then decodes it from the the encoded buffer back to
    a type structure reference */
    encode_bencoding(&_map_type, &encoded_buffer, &encoded_buffer_length);
    decode_bencoding(encoded_buffer, encoded_buffer_length, &type);

    /* deletes the hash map structure and the list strucuture
    to avoid any memory leaking */
    delete_hash_map(map);
    delete_linked_list(list);

    /* prints the type structure into the standard output and then
    releases its memory recursively */
    print_type(type);
    free_type(type);

    /* releases the memory from the encoded buffer, this was
    created during the encoding using bencoding */
    FREE(encoded_buffer);
}

void test_huffman() {
    /* allocates space for the huffman */
    struct huffman_t *huffman;

    /* creates the huffman (encoder) */
    create_huffman(&huffman);

    /* generates the huffman table */
    generate_table_huffman(huffman, NULL);

    /* deletes the huffman (encoder) */
    delete_huffman(huffman);
}

void test_bit_stream() {
    /* allocates space for the file stream */
    struct file_stream_t *file_stream;

    /* allocates space for the bit stream */
    struct bit_stream_t *bit_stream;

    /* creates the file stream */
    create_file_stream(&file_stream, (unsigned char *) "bit_stream.txt", (unsigned char *) "wb");

    /* creates the bit stream */
    create_bit_stream(&bit_stream, file_stream->stream);

    /* opens the bit stream */
    open_bit_stream(bit_stream);

    /* writes the 0100 bit set to the bit stream
    and then writes the 0001 bit set */
    write_byte_bit_stream(bit_stream, 0x04, 4);
    write_byte_bit_stream(bit_stream, 0x01, 4);

    /* checks if the written 8 bits are
    01000001 (0x41) the correct value */
    assert(bit_stream->buffer[0] == 0x41);

    /* writes a the 0100 bit set to the bit stream */
    write_byte_bit_stream(bit_stream, 0x04, 4);

    /* closes the bit stream */
    close_bit_stream(bit_stream);

    /* deletes the bit stream */
    delete_bit_stream(bit_stream);

    /* deletes the file stream */
    delete_file_stream(file_stream);
}

void test_file_stream() {
    /* allocates space for the file stream */
    struct file_stream_t *file_stream;

    /* allocates space for the stream */
    struct stream_t *stream;

    /* allocates some space for the test buffer */
    unsigned char buffer[128];

    /* creates the file stream */
    create_file_stream(&file_stream, (unsigned char *) "hello.txt", (unsigned char *) "wb");

    /* retrieves the stream from the file stream, in order
    to be able to use the "normal" stream functions */
    stream = file_stream->stream;

    /* opens the stream */
    stream->open(stream);

    /* writes some data to the stream */
    stream->write(stream, (unsigned char *) "hello world", 11);

    /* close the stream */
    stream->close(stream);

    /* creates the file stream */
    create_file_stream(&file_stream, (unsigned char *) "hello.txt", (unsigned char *) "rb");

    /* retrieves the stream from the file stream, in order
    to be able to use the "normal" stream functions */
    stream = file_stream->stream;

    /* opens the stream */
    stream->open(stream);

    /* reads some data from the stream */
    stream->read(stream, buffer, 11);

    /* sets the end of string character in the buffer */
    buffer[11] = '\0';

    /* close the stream */
    stream->close(stream);

    /* compares the read string */
    assert(strcmp((char *) "hello world", (char *) buffer) == 0);

    /* deletes the file stream */
    delete_file_stream(file_stream);
}

void test_template_handler() {
    /* allocates space for the template handler */
    struct template_handler_t *template_handler;

    /* creates the template handler */
    create_template_handler(&template_handler);

    /* processes the file as a template handler */
    process_template_handler(template_handler, (unsigned char *) "test.tpl");

    /* deletes the template handler */
    delete_template_handler(template_handler);
}

void test_quicksort() {
    /* allocates space for the template handler */
    int list[10] = { 2, 4, 1, 2, 3, 5, 5, 3, 4, 1 };

    /* sorts the sequence according to the compare function */
    sort_quicksort((void **) list, 0, 10, _compare);
}

void test_quicksort_linked_list() {
    /* allocates space for the linked list */
    struct linked_list_t *linked_list;

    /* creates the linked list */
    create_linked_list(&linked_list);

    /* adds some element to the linked list */
    append_value_linked_list(linked_list, (void *) 2);
    append_value_linked_list(linked_list, (void *) 3);
    append_value_linked_list(linked_list, (void *) 1);

    /* retrieves a value from the linked list */
    sort_linked_list(linked_list, _compare);

    /* deletes the linked list */
    delete_linked_list(linked_list);
}

void test_crc32() {
    /* calculates the crc32 hash value and returns it */
    crc32((unsigned char *) "Hello World", 11);
}

void test_md5() {
    /* allocates space for the md5 result */
    unsigned char result[MD5_DIGEST_SIZE];

    /* calculates the md5 hash value into the result */
    md5((unsigned char *) "Hello World", 11, result);
}

void test_sha1() {
    /* allocates space for the sha1 result */
    unsigned char result[SHA1_DIGEST_SIZE];

    /* calculates the sha1 hash value into the result */
    sha1((unsigned char *) "Hello World", 11, result);
}

int _compare(void *first, void *second) {
    /* in case the first element is smaller
    than the second element */
    if(first < second) {
        /* returns negative value */
        return -1;
    }
    /* in case the first value is larger
    than the second element */
    else if(first > second) {
        /* returns positive value */
        return 1;
    }

    /* returns zero value (equals) */
    return 0;
}

void run_simple_tests() {
    #ifndef VIRIATUM_NO_THREADS
    /* tests the thread pool */
    test_thread_pool();
    #endif

    /* tests the linked list */
    test_linked_list();

    /* tests the array list */
    test_array_list();

    /* tests the hash map */
    test_hash_map();

    /* tests the sort map */
    test_sort_map();

    /* tests the string buffer */
    test_string_buffer();

    /* tests the linked buffer */
    test_linked_buffer();

    /* tests the base 64 encoder */
    test_base64();

    /* tests the bencoding encoder */
    test_bencoding();

    /* tests the huffman encoder */
    test_huffman();

    /* tests the bit stream */
    test_bit_stream();

    /* tests the file stream */
    test_file_stream();

    /* tests the template handler */
    test_template_handler();

    /* tests the quick sort algorithm */
    test_quicksort();
    test_quicksort_linked_list();

    /* tests the crc32 hash function */
    test_crc32();

    /* tests the md5 hash function */
    test_md5();

    /* tests the sha1 hash function */
    test_sha1();
}
