/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "simple_test.h"
#include "handler_default_test.h"
#include "handler_dispatch_test.h"
#include "handler_file_test.h"
#include "handler_proxy_test.h"
#include "polling_test.h"
#include "hpack_test.h"
#include "http2_test.h"
#include "runner_test.h"
#include "service_test.h"
#include "stream_http2_test.h"
#include "websocket_test.h"

#ifndef VIRIATUM_NO_THREADS
#ifdef VIRIATUM_THREAD_SAFE
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

const char *test_thread_pool(void) {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the thread pool */
    struct thread_pool_t *thread_pool;

    /* allocates space for the thread pool task */
    struct thread_pool_task_t *thread_pool_task =
        (struct thread_pool_task_t *) MALLOC(sizeof(struct thread_pool_task_t));

    /* sets the start function */
    thread_pool_task->start_function = thread_pool_start_function_test;

    /* prints a debug message */
    V_DEBUG("Creating a thread pool\n");

    /* creates the thread pool */
    create_thread_pool(&thread_pool, 15, 1, 30);

    /* iterates over the range of the index */
    for(index = 0; index < 100; index++) {
        /* prints a debug message */
        V_DEBUG("Inserting task in thread pool\n");

        /* inserts the task in the thread pool */
        insert_task_thread_pool(thread_pool, thread_pool_task);
    }

    /* both the pool and the task are deliberately leaked, the
    delete operation of the pool neither signals nor joins the
    worker threads and so releasing them here would leave the
    running workers waiting on a closed condition and calling a
    start function that has already been released */

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}
#endif
#endif

const char *test_linked_list(void) {
    /* allocates space for the value that's going
    to be used for temporary storage */
    void *value;

    /* allocates space for the linked list */
    struct linked_list_t *linked_list;

    /* creates the linked list */
    create_linked_list(&linked_list);

    /* adds some elements to the linked list */
    append_value_linked_list(linked_list, (void *) 1);
    append_value_linked_list(linked_list, (void *) 2);
    append_value_linked_list(linked_list, (void *) 3);

    /* retrieves a value from the linked list and
    verifies that it contains the expected value */
    get_value_linked_list(linked_list, 1, &value);
    V_ASSERT(value == (void *) 2);

    /* removes a value from the linked list */
    remove_value_linked_list(linked_list, (void *) 1, TRUE);

    /* removes an element from the linked list */
    remove_index_linked_list(linked_list, 1, TRUE);

    /* pops two values from the linked list */
    pop_value_linked_list(linked_list, (void **) &value, TRUE);
    V_ASSERT(value == (void *) 2);
    pop_value_linked_list(linked_list, (void **) &value, TRUE);
    V_ASSERT(value == NULL);

    /* appends vome elements to the front of the linked list,
    then pops them out again */
    append_front_value_linked_list(linked_list, (void *) 4);
    append_front_value_linked_list(linked_list, (void *) 5);
    pop_value_linked_list(linked_list, (void **) &value, TRUE);
    V_ASSERT(value == (void *) 5);
    pop_value_linked_list(linked_list, (void **) &value, TRUE);
    V_ASSERT(value == (void *) 4);

    /* deletes the linked list */
    delete_linked_list(linked_list);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_linked_list_stress(void) {
    /* allocates space for the index to be used in the iteration
    for the temporary value pointer variable and for the pointer
    that is going to be used for the linked list */
    size_t index;
    void *value;
    struct linked_list_t *linked_list;

    /* creates the linked list structure and starts the long
    iteration that is going to append and then pop elements from
    the linked list (stressing the creation of nodes) */
    create_linked_list(&linked_list);
    for(index = 0; index < 100000000; index++) {
        append_value_linked_list(linked_list, (void *) 1);
        append_value_linked_list(linked_list, (void *) 2);
        append_value_linked_list(linked_list, (void *) 3);
        pop_value_linked_list(linked_list, (void **) &value, TRUE);
        pop_value_linked_list(linked_list, (void **) &value, TRUE);
        pop_value_linked_list(linked_list, (void **) &value, TRUE);
    }
    delete_linked_list(linked_list);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_linked_list_big(void) {
    /* allocates space for the index to be used in the iteration
    for the temporary value pointer variable and for the pointer
    that is going to be used for the linked list */
    size_t index;
    void *value;
    struct linked_list_t *linked_list;

    /* creates the linked list structure and starts the long
    iterations that are going to append and then pop elements from
    the linked list (stressing the creation of nodes) */
    create_linked_list(&linked_list);
    for(index = 0; index < 1000000; index++) {
        append_value_linked_list(linked_list, (void *) 1);
        append_value_linked_list(linked_list, (void *) 2);
        append_value_linked_list(linked_list, (void *) 3);
    }
    for(index = 0; index < 1000000; index++) {
        pop_value_linked_list(linked_list, (void **) &value, TRUE);
        pop_value_linked_list(linked_list, (void **) &value, TRUE);
        pop_value_linked_list(linked_list, (void **) &value, TRUE);
    }
    delete_linked_list(linked_list);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_array_list(void) {
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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_hash_map(void) {
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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_sort_map(void) {
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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_priority_queue(void) {
    /* allocates the space for the temporary value pointer
    and for the the priority queue structure */
    void *value;
    struct priority_queue_t *priority_queue;

    /* creates the priority queue and pushes the various
    values that are meant to be sorted to the queue so that
    these values are placed in the correct positions, the
    comparator that is going to be used is the default one */
    create_priority_queue(&priority_queue, _compare);
    push_priority_queue(priority_queue, (void *) 3);
    push_priority_queue(priority_queue, (void *) 2);
    push_priority_queue(priority_queue, (void *) 4);
    push_priority_queue(priority_queue, (void *) 1);
    push_priority_queue(priority_queue, (void *) 3);

    /* pops the various values from the queue and verifies that
    they are now sorted in the correct order */
    pop_priority_queue(priority_queue, &value);
    V_ASSERT(value == (void *) 1);
    pop_priority_queue(priority_queue, &value);
    V_ASSERT(value == (void *) 2);
    pop_priority_queue(priority_queue, &value);
    V_ASSERT(value == (void *) 3);
    pop_priority_queue(priority_queue, &value);
    V_ASSERT(value == (void *) 3);
    pop_priority_queue(priority_queue, &value);
    V_ASSERT(value == (void *) 4);

    /* deletes the priority queue structure as its no longer going
    to be used for the storage (avoids memory leaking) */
    delete_priority_queue(priority_queue);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_string_buffer(void) {
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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_linked_buffer(void) {
    /* allocates space for the linked buffer */
    struct linked_buffer_t *linked_buffer;

    /* allocates the space for the buffer to
    hold the various joined values */
    unsigned char *buffer_value;

    /* creates the linked buffer */
    create_linked_buffer(&linked_buffer);

    /* adds a set of strings to the string buffer */
    append_linked_buffer(linked_buffer, (void *) "hello", 5, FALSE);
    append_linked_buffer(linked_buffer, (void *) " ", 1, FALSE);
    append_linked_buffer(linked_buffer, (void *) "world", 5, FALSE);

    /* "joins" the linked buffer values into a single
    value (from the internal buffer list) */
    join_linked_buffer(linked_buffer, &buffer_value);

    /* releases the buffer value */
    FREE(buffer_value);

    /* deletes the linked buffer */
    delete_linked_buffer(linked_buffer);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_base64(void) {
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
    encode_base64(
        (unsigned char *) buffer,
        strlen(buffer),
        &encoded_buffer,
        &encoded_buffer_length
    );

    /* decodes the value from base64 */
    decode_base64(
        encoded_buffer,
        encoded_buffer_length,
        &decoded_buffer,
        &decoded_buffer_length
    );

    /* releases both the encoded and the decoded buffer
    to avoid any memory leak */
    FREE(encoded_buffer);
    FREE(decoded_buffer);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_bencoding(void) {
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

    /* gathers the number of allocations that are outstanding before
    anything is built, so that the release of everything may be
    verified against it once the test is over */
    size_t allocated = ALLOCATIONS;

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

    /* deletes the hash map structure and the list structure
    to avoid any memory leaking */
    delete_hash_map(map);
    delete_linked_list(list);

    /* prints the type structure into the standard output and then
    releases its memory recursively */
    print_type(type);
    V_PRINT("\n");
    free_type(type);

    /* releases the memory from the encoded buffer, this was
    created during the encoding using bencoding */
    FREE(encoded_buffer);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the test started at all */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_bit_stream(void) {
    /* allocates space for the byte value that is going
    to be used in the read based tezt of the bit stream */
    unsigned char byte;

    /* allocates space for both the file and the
    bit based streams, that are going to be used
    for the test of the bit stream infra-structure */
    struct file_stream_t *file_stream;
    struct bit_stream_t *bit_stream;

    /* gathers the number of allocations that are outstanding before
    anything is built, so that the release of everything may be
    verified against it once the test is over */
    size_t allocated = ALLOCATIONS;

    /* creates the file stream that is going to be used
    as the underlying stream for the bit stream */
    create_file_stream(
        &file_stream,
        (unsigned char *) "bit_stream.bin",
        (unsigned char *) "wb"
    );

    /* creates the bit stream using the file stream as the
    underlying stream structure to be used */
    create_bit_stream(&bit_stream, file_stream->stream);

    /* opens the bit stream so that operations may start
    to be triggered for the bit stream */
    open_bit_stream(bit_stream);

    /* writes the 0100 bit set to the bit stream
    and then writes the 0001 bit set */
    write_byte_bit_stream(bit_stream, 0x04, 4);
    write_byte_bit_stream(bit_stream, 0x01, 4);

    /* checks if the written 8 bits are
    01000001 (0x41) the correct value */
    V_ASSERT(bit_stream->buffer[0] == 0x41);

    /* writes a the 1000 bit set to the bit stream
    and then writes the 0010 bit set creating a new
    byte value in the bit stream */
    write_byte_bit_stream(bit_stream, 0x08, 4);
    write_byte_bit_stream(bit_stream, 0x02, 4);

    /* verifies if the expected 10000010 (0x82)
    value is current set in the output buffer */
    V_ASSERT(bit_stream->buffer[1] == 0x82);

    /* writes a partial stream of bit values that
    would create an extra (pending) bits situation
    and then verifies that the values are the expected
    ones in a series of assertions */
    write_byte_bit_stream(bit_stream, 0x08, 6);
    write_byte_bit_stream(bit_stream, 0x02, 6);
    write_byte_bit_stream(bit_stream, 0x02, 4);
    V_ASSERT(bit_stream->buffer[2] == 0x20);
    V_ASSERT(bit_stream->buffer[3] == 0x22);

    /* closes the bit stream and then deletes the references
    to both the but and the file stream */
    close_bit_stream(bit_stream);
    delete_bit_stream(bit_stream);
    delete_file_stream(file_stream);

    /* re-creates the previous file stream in read mode so that
    it may be used in the testing of the read operations in the
    bit stream (these operations will include seek) */
    create_file_stream(
        &file_stream,
        (unsigned char *) "bit_stream.bin",
        (unsigned char *) "rb"
    );

    /* creates the bit stream using the file stream as the
    underlying stream structure to be used */
    create_bit_stream(&bit_stream, file_stream->stream);
    open_bit_stream(bit_stream);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x04);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x01);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 8);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x04);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x01);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 2);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 2);
    V_ASSERT(byte == 0x01);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x08);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x02);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 6);
    V_ASSERT(byte == 0x08);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 6);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 6);
    V_ASSERT(byte == 0x08);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 6);
    V_ASSERT(byte == 0x02);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 6);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 6);
    V_ASSERT(byte == 0x02);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 12);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 6);
    V_ASSERT(byte == 0x08);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 6);
    V_ASSERT(byte == 0x02);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 4);
    V_ASSERT(byte == 0x02);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 3);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 3);
    V_ASSERT(byte == 0x02);

    /* seeks back the stream a bit so that the values may be
    tested again for coherence (complex operation) */
    seek_bit_stream(bit_stream, 32);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 3);
    V_ASSERT(byte == 0x02);

    /* reads a partial byte from the bit stream and verifies
    that the value is the expected one (according to spec) */
    read_byte_bit_stream(bit_stream, &byte, 5);
    V_ASSERT(byte == 0x01);

    /* closes the bit stream and then deletes the references
    to both the but and the file stream */
    close_bit_stream(bit_stream);
    delete_bit_stream(bit_stream);
    delete_file_stream(file_stream);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the test started at all */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_stream(void) {
    /* allocates space for the file stream */
    struct file_stream_t *file_stream;

    /* allocates space for the stream */
    struct stream_t *stream;

    /* allocates some space for the test buffer */
    unsigned char buffer[128];

    /* gathers the number of allocations that are outstanding before
    anything is built, so that the release of everything may be
    verified against it once the test is over */
    size_t allocated = ALLOCATIONS;

    /* creates the file stream */
    create_file_stream(
        &file_stream,
        (unsigned char *) "hello.txt",
        (unsigned char *) "wb"
    );

    /* retrieves the stream from the file stream, in order
    to be able to use the "normal" stream functions */
    stream = file_stream->stream;

    /* opens the stream */
    stream->open(stream);

    /* writes some data to the stream */
    stream->write(stream, (unsigned char *) "hello world", 11);

    /* close the stream and deletes the file stream that was used
    for the writing, the reading is driven through another one */
    stream->close(stream);
    delete_file_stream(file_stream);

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
    V_ASSERT(strcmp((char *) "hello world", (char *) buffer) == 0);

    /* deletes the file stream */
    delete_file_stream(file_stream);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the test started at all */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_memory_stream(void) {
    /* allocates the space for the local stream
    pointers and for the buffer that is going to
    be used for the testing (strings) */
    struct stream_t *stream;
    struct memory_stream_t *memory_stream;
    unsigned char buffer[256];

    /* gathers the number of allocations that are outstanding before
    anything is built, so that the release of everything may be
    verified against it once the test is over */
    size_t allocated = ALLOCATIONS;

    /* creates the memory stream structure starting
    the values contained in it */
    create_memory_stream(&memory_stream);

    /* extracts the inner stream structure from the
    memory stream and uses it to open stream (reference
    based operations) */
    stream = memory_stream->stream;
    stream->open(stream);

    /* writes a simple string into the stream and then
    seeks back the stream to the original position and
    then tries to read the same string from the stream */
    stream->write(stream, (unsigned char *) "hello world", 11);
    stream->seek(stream, 0);
    stream->read(stream, buffer, 11);

    /* closes the stream avoiding any extra operation in it
    and deletes the memory stream (avoiding any memory leaks) */
    stream->close(stream);
    delete_memory_stream(memory_stream);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the test started at all */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_huffman(void) {
    /* allocates space for both the file stream that is
    going to be used in the reading process and for the
    huffman structure to be used in the test */
    struct file_stream_t *in_stream;
    struct file_stream_t *out_stream;
    struct huffman_t *huffman;

    /* gathers the number of allocations that are outstanding before
    anything is built, so that the release of everything may be
    verified against it once the test is over */
    size_t allocated = ALLOCATIONS;

    /* creates the file stream that is going to be used for
    the testing of the huffman infra-structure, this is the
    input stream from which the data will be read (source) */
    create_file_stream(
        &in_stream,
        (unsigned char *) "bit_stream.bin",
        (unsigned char *) "rb"
    );

    /* creates the file that is going to be used to output the
    huffman encoded file, opening under the write mode */
    create_file_stream(
        &out_stream,
        (unsigned char *) "bit_stream.bin.huffman",
        (unsigned char *) "wb"
    );

    /* creates the huffman (encoder) and then runs the
    generation of the huffman table in it, after the generation
    of the table runs the generation of the prefix tables */
    create_huffman(&huffman);
    generate_table_huffman(huffman, in_stream->stream);
    generate_prefix_huffman(huffman);

    /* runs the huffman encoder creating the bit stream
    with the compressed contents provided by the input stream
    that was just loaded as the input */
    encode_huffman(huffman, in_stream->stream, out_stream->stream);

    /* deletes both the output and the input huffman file streams
    to avoid any memory leak (would create problems) */
    delete_file_stream(out_stream);
    delete_file_stream(in_stream);

    /* creates the file stream that is going to be used as the input
    for the decoding process this stream should contain an huffman
    encoded file to be decompressed */
    create_file_stream(
        &in_stream,
        (unsigned char *) "bit_stream.bin.huffman",
        (unsigned char *) "rb"
    );

    /* creates and opens the file that is going to be used as the output
    for the decoding process in the huffman infra-structure */
    create_file_stream(
        &out_stream,
        (unsigned char *) "bit_stream.bin.decoded",
        (unsigned char *) "wb"
    );

    /* runs the decoding process using the provided input and output
    streams, this is a very long operation that may consume a lot of
    resouces and take some time to complete */
    decode_huffman(huffman, in_stream->stream, out_stream->stream);

    /* deletes both the output and the input streams and then removes
    the current huffman structure as it's not goign to be required anymore */
    delete_file_stream(out_stream);
    delete_file_stream(in_stream);
    delete_huffman(huffman);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the test started at all */
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

/* the file that the tests of the template engine, of the handler and
of the cache write and parse, it is written into the directory the
process is running from and taken out of it once a test is done */
#define TEMPLATE_TEST_PATH "./viriatum_template_test.tpl"

/* the path that stands in for a template that is not there, no
file is ever written under it and opening it is meant to fail */
#define TEMPLATE_TEST_GONE "./viriatum_template_gone.tpl"

/* the contents that the file above is written with, a single tag
that prints the name assigned to it between two runs of text */
#define TEMPLATE_TEST_CONTENTS "<p>${out value=name /}</p>"

/* another set of contents of the very same length as the one
above, so that a file replaced by it is told apart from the one
that was there by nothing but the tree parsed out of it */
#define TEMPLATE_TEST_OTHER "<b>${out value=name /}</b>"

/* a longer set of contents, so that a file written over with it
is told apart from the one that was there by the size alone */
#define TEMPLATE_TEST_LONGER "<div>${out value=name /}</div>"

/* the template that goes through every one of the tags, the value
of a name, the walking of a list and the condition on the type of
an entry, which is what the listing of a directory is built on */
#define TEMPLATE_TEST_PAGE                                 \
    "<h1>${out value=title /}</h1>"                        \
    "${foreach item=entry from=entries}"                   \
    "<li>${if item=entry.type value=1 operator=eq}D${/if}" \
    "${out value=entry.name /}</li>"                       \
    "${/foreach}"                                          \
    "<p>${out value=count /}</p>"

/* the template whose tags carry none of the parameters they need,
every one of them is left out of the page and the text around them
is what the page ends up being made of */
#define TEMPLATE_TEST_MALFORMED "<a>${out /}${out/}${foreach item=entry /}${if item=entry.type /}</a>"

/* the number of events that the collector of the engine is able to
hold and the size of the text each of them carries, a parsing that
reports more than that fails rather than writing past the end */
#define TEMPLATE_TEST_MAX_EVENTS 32
#define TEMPLATE_TEST_MAX_DATA 64

/**
 * Collects the events that the engine reports while parsing, each
 * of them as a line of text made of the name of the event and the
 * data it carries, so that the sequence of them may be compared
 * against the one a template is expected to produce.
 */
typedef struct template_collector_t {
    size_t count;
    char events[TEMPLATE_TEST_MAX_EVENTS][TEMPLATE_TEST_MAX_DATA];
} template_collector;

static ERROR_CODE _collect_template_test(struct template_engine_t *template_engine, const char *name, const unsigned char *pointer, size_t size) {
    /* retrieves the collector out of the context of the engine and
    then verifies that there's still room for one more event */
    struct template_collector_t *collector = (struct template_collector_t *) template_engine->context;

    if(collector->count >= TEMPLATE_TEST_MAX_EVENTS) { RAISE_ERROR_S(RUNTIME_EXCEPTION_ERROR_CODE); }
    if(strlen(name) + size + 2 > TEMPLATE_TEST_MAX_DATA) { RAISE_ERROR_S(RUNTIME_EXCEPTION_ERROR_CODE); }

    /* writes the event as the name and the data it carries, the
    engine hands the data over without a termination */
    SPRINTF(
        collector->events[collector->count],
        TEMPLATE_TEST_MAX_DATA,
        "%s:%.*s",
        name,
        (int) size,
        (const char *) pointer
    );
    collector->count++;

    /* raises no error */
    RAISE_NO_ERROR;
}

static ERROR_CODE _text_begin_template_test(struct template_engine_t *template_engine) {
    return _collect_template_test(template_engine, "text_begin", (const unsigned char *) "", 0);
}

static ERROR_CODE _text_end_template_test(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    return _collect_template_test(template_engine, "text_end", pointer, size);
}

static ERROR_CODE _tag_begin_template_test(struct template_engine_t *template_engine) {
    return _collect_template_test(template_engine, "tag_begin", (const unsigned char *) "", 0);
}

static ERROR_CODE _tag_close_begin_template_test(struct template_engine_t *template_engine) {
    return _collect_template_test(template_engine, "tag_close_begin", (const unsigned char *) "", 0);
}

static ERROR_CODE _tag_end_template_test(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    return _collect_template_test(template_engine, "tag_end", pointer, size);
}

static ERROR_CODE _tag_name_template_test(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    return _collect_template_test(template_engine, "tag_name", pointer, size);
}

static ERROR_CODE _parameter_template_test(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    return _collect_template_test(template_engine, "parameter", pointer, size);
}

static ERROR_CODE _parameter_value_template_test(struct template_engine_t *template_engine, const unsigned char *pointer, size_t size) {
    return _collect_template_test(template_engine, "parameter_value", pointer, size);
}

static ERROR_CODE _fail_template_test(struct template_engine_t *template_engine) {
    /* stands in for a callback that is unable to handle what the
    engine reports, the parsing is expected to fail along with it */
    RAISE_ERROR_S(RUNTIME_EXCEPTION_ERROR_CODE);
}

/**
 * Creates the settings that send every one of the events of the
 * engine to the collector that the context of the engine points at.
 *
 * @param template_settings_pointer The pointer to the settings that
 * have been created.
 */
static void _create_settings_template_test(struct template_settings_t **template_settings_pointer) {
    struct template_settings_t *template_settings;

    create_template_settings(&template_settings);
    template_settings->on_text_begin = _text_begin_template_test;
    template_settings->on_text_end = _text_end_template_test;
    template_settings->on_tag_begin = _tag_begin_template_test;
    template_settings->on_tag_close_begin = _tag_close_begin_template_test;
    template_settings->on_tag_end = _tag_end_template_test;
    template_settings->on_tag_name = _tag_name_template_test;
    template_settings->on_parameter = _parameter_template_test;
    template_settings->on_parameter_value = _parameter_value_template_test;

    *template_settings_pointer = template_settings;
}

/**
 * Creates an entry of the shape the listing of a directory hands a
 * template, a map carrying the name of the entry and the type of it.
 *
 * @param entry_pointer The pointer to the entry that has been created.
 * @param name The name the entry is going to carry.
 * @param type The type the entry is going to carry.
 */
static void _create_entry_template_test(struct type_t **entry_pointer, char *name, int type) {
    struct hash_map_t *map;
    struct type_t *value;
    struct type_t *entry;

    create_hash_map(&map, 8);
    create_type(&value, STRING_TYPE);
    value->value.value_string = name;
    set_value_string_hash_map(map, (unsigned char *) "name", (void *) value);
    create_type(&value, INTEGER_TYPE);
    value->value.value_int = type;
    set_value_string_hash_map(map, (unsigned char *) "type", (void *) value);
    create_type(&entry, MAP_TYPE);
    entry->value.value_map = map;

    *entry_pointer = entry;
}

/**
 * Releases an entry created by the above, together with the values
 * it carries and the map that holds them.
 *
 * @param entry The entry to be released.
 */
static void _delete_entry_template_test(struct type_t *entry) {
    struct hash_map_t *map = entry->value.value_map;
    struct type_t *value;

    get_value_string_hash_map(map, (unsigned char *) "name", (void **) &value);
    delete_type(value);
    get_value_string_hash_map(map, (unsigned char *) "type", (void **) &value);
    delete_type(value);
    delete_hash_map(map);
    delete_type(entry);
}

/**
 * Renders the template under the provided path out of the provided
 * cache with the single name the templates of the tests print, and
 * copies the page that results into the provided buffer.
 *
 * @param template_cache The cache the template is rendered out of.
 * @param file_path The path of the template to be rendered.
 * @param buffer The buffer the page is copied into.
 * @param size The size in bytes of the provided buffer.
 */
static void _render_template_test(struct template_cache_t *template_cache, char *file_path, char *buffer, size_t size) {
    struct template_handler_t *template_handler;

    create_template_handler(&template_handler);
    assign_string_template_handler(template_handler, (unsigned char *) "name", "viriatum");
    process_cache_template_handler(template_handler, template_cache, (unsigned char *) file_path);
    SPRINTF(buffer, size, "%s", (char *) template_handler->string_value);
    delete_template_handler(template_handler);
}

const char *test_template_engine(void) {
    /* allocates space for the engine, for the settings that send
    its events to the collector and for the collector itself */
    struct template_engine_t *template_engine;
    struct template_settings_t *template_settings;
    struct template_collector_t collector;
    ERROR_CODE error;

    create_template_engine(&template_engine);
    _create_settings_template_test(&template_settings);
    template_engine->context = (void *) &collector;

    /* a file is parsed exactly the way its contents would be, the
    reading of it whole into memory changes nothing of the events */
    write_file((char *) TEMPLATE_TEST_PATH, (unsigned char *) "a ${out value=x /} b", 20);
    collector.count = 0;
    error = process_template_engine(template_engine, template_settings, (unsigned char *) TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 9);
    V_ASSERT_EQ_S(collector.events[0], "text_begin:");
    V_ASSERT_EQ_S(collector.events[1], "text_end:a ");
    V_ASSERT_EQ_S(collector.events[2], "tag_begin:");
    V_ASSERT_EQ_S(collector.events[3], "tag_name:out");
    V_ASSERT_EQ_S(collector.events[4], "parameter:value");
    V_ASSERT_EQ_S(collector.events[5], "parameter_value:x");
    V_ASSERT_EQ_S(collector.events[6], "tag_end:${out value=x /}");
    V_ASSERT_EQ_S(collector.events[7], "text_begin:");
    V_ASSERT_EQ_S(collector.events[8], "text_end: b");

    /* a file that is not there is not an error, the parsing simply
    reports nothing at all, which is what leaves a page empty */
    collector.count = 0;
    error = process_template_engine(template_engine, template_settings, (unsigned char *) TEMPLATE_TEST_GONE);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 0);

    /* a callback that fails takes the parsing of a file down with it
    and the failure is reported rather than swallowed */
    template_settings->on_text_begin = _fail_template_test;
    error = process_template_engine(template_engine, template_settings, (unsigned char *) TEMPLATE_TEST_PATH);
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    delete_template_settings(template_settings);
    delete_template_engine(template_engine);
    remove(TEMPLATE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_engine_buffer(void) {
    /* allocates space for the engine, for the settings that send
    its events to the collector and for the collector itself */
    struct template_engine_t *template_engine;
    struct template_settings_t *template_settings;
    struct template_collector_t collector;
    ERROR_CODE error;

    create_template_engine(&template_engine);
    _create_settings_template_test(&template_settings);
    template_engine->context = (void *) &collector;

    /* a run of text and nothing else is reported as a single text,
    opened before the first character and closed after the last */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "hello", 5);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 2);
    V_ASSERT_EQ_S(collector.events[0], "text_begin:");
    V_ASSERT_EQ_S(collector.events[1], "text_end:hello");

    /* a value between quotes is handed over with the quotes and with
    the spaces within them, the text on either side of the tag is
    reported even when there is nothing in it */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${out value=\"a b\" /}", 20);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 9);
    V_ASSERT_EQ_S(collector.events[1], "text_end:");
    V_ASSERT_EQ_S(collector.events[3], "tag_name:out");
    V_ASSERT_EQ_S(collector.events[4], "parameter:value");
    V_ASSERT_EQ_S(collector.events[5], "parameter_value:\"a b\"");
    V_ASSERT_EQ_S(collector.events[6], "tag_end:${out value=\"a b\" /}");
    V_ASSERT_EQ_S(collector.events[7], "text_begin:");

    /* a tag that opens a context and the one that closes it are told
    apart, the closing one being reported as such before its end */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${foreach item=e from=l}x${/foreach}", 36);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 17);
    V_ASSERT_EQ_S(collector.events[3], "tag_name:foreach");
    V_ASSERT_EQ_S(collector.events[4], "parameter:item");
    V_ASSERT_EQ_S(collector.events[5], "parameter_value:e");
    V_ASSERT_EQ_S(collector.events[6], "parameter:from");
    V_ASSERT_EQ_S(collector.events[7], "parameter_value:l");
    V_ASSERT_EQ_S(collector.events[8], "tag_end:${foreach item=e from=l}");
    V_ASSERT_EQ_S(collector.events[10], "text_end:x");
    V_ASSERT_EQ_S(collector.events[11], "tag_begin:");
    V_ASSERT_EQ_S(collector.events[12], "tag_close_begin:");
    V_ASSERT_EQ_S(collector.events[13], "tag_name:foreach");
    V_ASSERT_EQ_S(collector.events[14], "tag_end:${/foreach}");
    V_ASSERT_EQ_S(collector.events[16], "text_end:");

    /* a slash inside a value is part of the value, the look ahead
    past it finding no brace to close the tag with */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${out value=a/b /}", 18);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 9);
    V_ASSERT_EQ_S(collector.events[5], "parameter_value:a/b");
    V_ASSERT_EQ_S(collector.events[6], "tag_end:${out value=a/b /}");

    /* a tag that closes right after its name, with or without the
    slash of a single tag, still reports the name, one that went
    without would be rendered through a node with no name at all */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${out/}", 7);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 7);
    V_ASSERT_EQ_S(collector.events[3], "tag_name:out");
    V_ASSERT_EQ_S(collector.events[4], "tag_end:${out/}");

    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${out}", 6);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 7);
    V_ASSERT_EQ_S(collector.events[3], "tag_name:out");
    V_ASSERT_EQ_S(collector.events[4], "tag_end:${out}");

    /* a tag that closes on the very last character of the buffer is
    reported whole, the parsing used to stop one character short of
    the end and lose whatever that character was closing */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${out value=1 /}", 16);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 9);
    V_ASSERT_EQ_S(collector.events[5], "parameter_value:1");
    V_ASSERT_EQ_S(collector.events[6], "tag_end:${out value=1 /}");
    V_ASSERT_EQ_S(collector.events[8], "text_end:");

    /* a buffer with nothing in it opens and closes an empty text and
    reads nothing at all past its end */
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "", 0);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(collector.count, 2);
    V_ASSERT_EQ_S(collector.events[0], "text_begin:");
    V_ASSERT_EQ_S(collector.events[1], "text_end:");

    /* a callback that fails takes the parsing down with it */
    template_settings->on_tag_begin = _fail_template_test;
    collector.count = 0;
    error = process_buffer_template_engine(template_engine, template_settings, (unsigned char *) "${out value=1 /}", 16);
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    delete_template_settings(template_settings);
    delete_template_engine(template_engine);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache(void) {
    /* allocates space for the cache and for the index to be used in
    the walking of the entries it is made of */
    size_t index;
    struct template_cache_t *template_cache;

    /* creates the cache and verifies that every one of its entries
    starts out holding no template at all */
    create_template_cache(&template_cache);
    V_ASSERT_NOT_NULL(template_cache);
    V_ASSERT_NOT_NULL(template_cache->entries);

    for(index = 0; index < CACHE_SIZE_TEMPLATE_HANDLER; index++) {
        V_ASSERT_EQ_I(template_cache->entries[index].descriptor, -1);
        V_ASSERT_EQ_U(template_cache->entries[index].size, 0);
        V_ASSERT_NULL(template_cache->entries[index].root);
        V_ASSERT_NULL(template_cache->entries[index].nodes);
    }

    delete_template_cache(template_cache);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_acquire(void) {
    /* allocates space for the cache, for the entries that the
    acquiring of the very same path hands back and for the root of
    the tree the first of them is holding */
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *first;
    struct template_cache_entry_t *second;
    struct template_node_t *root;
    size_t allocated = ALLOCATIONS;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);

    /* the first acquisition opens the file, parses it into a tree
    and learns both its size and the moment of the last write */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &first
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_NOT_NULL(first);
    V_ASSERT_EQ_S((char *) first->path, TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_U(first->size, sizeof(TEMPLATE_TEST_CONTENTS) - 1);
    V_ASSERT(first->descriptor != -1);
    V_ASSERT(first->checked > 0);
    V_ASSERT_NOT_NULL(first->root);
    V_ASSERT_NOT_NULL(first->nodes);
    V_ASSERT_EQ_I(first->root->type, TEMPLATE_NODE_ROOT);

    /* the second one falls on the very same entry and hands back the
    tree that was already parsed rather than parsing the file again */
    root = first->root;
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &second
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_P(first, second);
    V_ASSERT_EQ_P(second->root, root);
    V_ASSERT_EQ_I(first->descriptor, second->descriptor);

    /* the tree, the entries and the cache are all gone together, so
    the number of outstanding allocations is the one it was before */
    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_missing(void) {
    /* allocates space for the cache, for the entry that the acquiring
    of a template that is not there would have handed back and for the
    index of the slot that such a path falls on */
    size_t index;
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry = NULL;
    ERROR_CODE error;

    create_template_cache(&template_cache);

    /* a template that does not exist raises rather than handing back
    an entry that describes nothing at all */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_GONE,
        &entry
    );
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(entry);
    RESET_ERROR;

    /* a directory under the path is not a template either, on the
    platforms that open one for reading it is the reading of it that
    fails and on the others the opening, either way the entry it fell
    on is left empty rather than half way through describing it */
    error = acquire_template_cache(template_cache, (unsigned char *) ".", &entry);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(entry);
    RESET_ERROR;
    index = _calculate_string_hash_map((unsigned char *) ".") % CACHE_SIZE_TEMPLATE_HANDLER;
    V_ASSERT_EQ_I(template_cache->entries[index].descriptor, -1);
    V_ASSERT_NULL(template_cache->entries[index].root);

    delete_template_cache(template_cache);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_changed(void) {
    /* allocates space for the cache, for the entry that describes the
    template before and after it has been written over and for the
    page that is rendered out of it */
    char page[64];
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);

    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(entry->size, sizeof(TEMPLATE_TEST_CONTENTS) - 1);
    _render_template_test(template_cache, (char *) TEMPLATE_TEST_PATH, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<p>viriatum</p>");

    /* the file is written over in place, which keeps the very same
    descriptor reaching it, so an entry that went on trusting the
    tree it parsed would render the page as it used to be, and the
    writing itself has to succeed while the cache is holding the
    file open, which is not something every platform allows */
    error = write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_LONGER,
        sizeof(TEMPLATE_TEST_LONGER) - 1
    );
    V_ASSERT_EQ_U(error, 0);

    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(entry->size, sizeof(TEMPLATE_TEST_LONGER) - 1);
    _render_template_test(template_cache, (char *) TEMPLATE_TEST_PATH, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<div>viriatum</div>");

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_collision(void) {
    /* allocates space for the cache and for the entries of the two
    templates that are made to fall on the very same slot */
    size_t index;
    size_t taken;
    char path[VIRIATUM_MAX_PATH_SIZE];
    char page[64];
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);

    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    taken = _calculate_string_hash_map((unsigned char *) TEMPLATE_TEST_PATH) %
            CACHE_SIZE_TEMPLATE_HANDLER;

    /* looks for a second path that falls on the very same slot as the
    first one, which is what a cache of this shape has instead of a
    chain and what makes one of the templates give way to the other */
    for(index = 0; index < 100000; index++) {
        SPRINTF(path, VIRIATUM_MAX_PATH_SIZE, "./viriatum_template_%d.tpl", (int) index);
        if(_calculate_string_hash_map((unsigned char *) path) % CACHE_SIZE_TEMPLATE_HANDLER == taken) {
            break;
        }
    }
    V_ASSERT(index < 100000);

    write_file(
        path,
        (unsigned char *) TEMPLATE_TEST_LONGER,
        sizeof(TEMPLATE_TEST_LONGER) - 1
    );

    /* the second of them takes the slot over and is described by it,
    a cache that handed back the entry of the first would be rendering
    one template under the name of another */
    error = acquire_template_cache(template_cache, (unsigned char *) path, &entry);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_S((char *) entry->path, path);
    V_ASSERT_EQ_U(entry->size, sizeof(TEMPLATE_TEST_LONGER) - 1);
    _render_template_test(template_cache, path, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<div>viriatum</div>");

    /* and the first of them takes it back again, describing itself
    and never what had displaced it */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_S((char *) entry->path, TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_U(entry->size, sizeof(TEMPLATE_TEST_CONTENTS) - 1);
    _render_template_test(template_cache, (char *) TEMPLATE_TEST_PATH, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<p>viriatum</p>");

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);
    remove(path);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_clear(void) {
    /* allocates space for the cache and for the entry that is going
    to be emptied out of it */
    size_t index;
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(entry->descriptor != -1);
    V_ASSERT_NOT_NULL(entry->root);

    /* the clearing closes every file that was being held and releases
    every tree, an entry left with either behind is a leak */
    clear_template_cache(template_cache);
    for(index = 0; index < CACHE_SIZE_TEMPLATE_HANDLER; index++) {
        V_ASSERT_EQ_I(template_cache->entries[index].descriptor, -1);
        V_ASSERT_NULL(template_cache->entries[index].root);
        V_ASSERT_NULL(template_cache->entries[index].nodes);
    }

    /* and the cache goes on working afterwards, the clearing empties
    it rather than taking it out of use */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(entry->descriptor != -1);
    V_ASSERT_NOT_NULL(entry->root);

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_long(void) {
    /* allocates space for the cache and for the path that is longer
    than an entry of it is able to carry */
    char path[VIRIATUM_MAX_PATH_SIZE * 2];
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry = NULL;
    ERROR_CODE error;

    create_template_cache(&template_cache);

    /* a path that does not fit inside an entry is refused rather than
    being copied past the end of the buffer that is meant to hold it */
    memset(path, 'a', sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    error = acquire_template_cache(template_cache, (unsigned char *) path, &entry);
    V_ASSERT(IS_ERROR_CODE(error));
    V_ASSERT_NULL(entry);
    RESET_ERROR;

    delete_template_cache(template_cache);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_expired(void) {
    /* allocates space for the cache, for the entry that is made to
    look older than the time it is trusted for and for the page that
    is rendered out of it */
    char page[64];
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);

    /* the entry is made to look as though it had been sitting there
    since well before the time it is trusted for, which is what sends
    the next acquisition to look at the path again */
    entry->checked = 0;

    /* the file has not moved on, so the page comes out exactly as it
    did and only the moment the entry was looked at is renewed */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT(entry->checked > 0);
    V_ASSERT(entry->descriptor != -1);
    V_ASSERT_NOT_NULL(entry->root);
    _render_template_test(template_cache, (char *) TEMPLATE_TEST_PATH, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<p>viriatum</p>");

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_replaced(void) {
    /* allocates space for the cache, for the entry of the template
    that another one is put in the place of and for the page that is
    rendered out of it */
    char page[64];
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    _render_template_test(template_cache, (char *) TEMPLATE_TEST_PATH, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<p>viriatum</p>");

    /* another template of the very same length is put in the place
    of the one that is held, which neither the size nor the moment of
    the last write is able to tell apart, so only a look at the path
    is, the removal has to have gone through or the writing lands on
    the very same file and there would be nothing to tell apart */
    V_ASSERT_EQ_I(remove(TEMPLATE_TEST_PATH), 0);
    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_OTHER,
        sizeof(TEMPLATE_TEST_OTHER) - 1
    );
    entry->checked = 0;

    /* the template that is now under the path is parsed in place of
    the one that was, and the page is rendered out of it */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(entry->size, sizeof(TEMPLATE_TEST_OTHER) - 1);
    _render_template_test(template_cache, (char *) TEMPLATE_TEST_PATH, page, sizeof(page));
    V_ASSERT_EQ_S(page, "<b>viriatum</b>");

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_cache_stale(void) {
    /* allocates space for the cache and for the entry whose file is
    taken out from underneath it */
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);

    /* the file that the entry is holding is closed behind its back
    and the entry is left pointing at a descriptor that reaches
    nothing, which is the state the cache has to answer for rather
    than go on rendering a template it can no longer ask about */
    CLOSE_READ(entry->descriptor);
    entry->descriptor = -2;

    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    /* the descriptor is unset by hand as the cache is no longer able
    to close what it was left holding, the tree it still holds is
    released along with the cache */
    entry->descriptor = -1;

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_handler(void) {
    /* allocates space for the template handler, for the list of the
    entries that the template walks and for the entries themselves */
    struct template_handler_t *template_handler;
    struct linked_list_t *entries;
    struct type_t *first;
    struct type_t *second;
    size_t allocated = ALLOCATIONS;

    /* writes the template that goes through every one of the tags
    and builds the entries that the walking of the list is fed */
    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_PAGE,
        sizeof(TEMPLATE_TEST_PAGE) - 1
    );
    create_linked_list(&entries);
    _create_entry_template_test(&first, "alpha", 1);
    _create_entry_template_test(&second, "beta", 2);
    append_value_linked_list(entries, (void *) first);
    append_value_linked_list(entries, (void *) second);

    /* creates the template handler then uses it to process the
    template file, the page carries the value of every name, one run
    of the list per entry and the mark of the condition on the one
    entry whose type matches it */
    create_template_handler(&template_handler);
    assign_string_template_handler(template_handler, (unsigned char *) "title", "Title");
    assign_list_template_handler(template_handler, (unsigned char *) "entries", entries);
    assign_integer_template_handler(template_handler, (unsigned char *) "count", 2);
    process_template_handler(template_handler, (unsigned char *) TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_S(
        (char *) template_handler->string_value,
        "<h1>Title</h1><li>Dalpha</li><li>beta</li><p>2</p>"
    );
    delete_template_handler(template_handler);

    /* a template that is not there leaves the handler with an empty
    result rather than an error, the one that builds a page out of it
    is what warns about the absence and falls back */
    create_template_handler(&template_handler);
    process_template_handler(template_handler, (unsigned char *) TEMPLATE_TEST_GONE);
    V_ASSERT_EQ_S((char *) template_handler->string_value, "");
    delete_template_handler(template_handler);

    /* a template whose tags carry none of the parameters they need
    renders the text around them and nothing for the tags themselves,
    rather than taking the rendering down over the missing values */
    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_MALFORMED,
        sizeof(TEMPLATE_TEST_MALFORMED) - 1
    );
    create_template_handler(&template_handler);
    process_template_handler(template_handler, (unsigned char *) TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_S((char *) template_handler->string_value, "<a></a>");
    delete_template_handler(template_handler);

    /* every one of the structures that has been built is gone, so
    the number of outstanding allocations is the one it was before
    the test started at all */
    _delete_entry_template_test(first);
    _delete_entry_template_test(second);
    delete_linked_list(entries);
    remove(TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_handler_cache(void) {
    /* allocates space for the cache, for the entry that describes the
    template within it, for the handler and for the root of the tree
    that the first page is rendered out of */
    struct template_cache_t *template_cache;
    struct template_cache_entry_t *entry;
    struct template_handler_t *template_handler;
    struct template_node_t *root;
    size_t allocated = ALLOCATIONS;
    ERROR_CODE error;

    write_file(
        (char *) TEMPLATE_TEST_PATH,
        (unsigned char *) TEMPLATE_TEST_CONTENTS,
        sizeof(TEMPLATE_TEST_CONTENTS) - 1
    );

    create_template_cache(&template_cache);

    /* the page is rendered out of the cache with the names that were
    assigned to the handler, exactly as it would be out of the file */
    create_template_handler(&template_handler);
    assign_string_template_handler(template_handler, (unsigned char *) "name", "first");
    process_cache_template_handler(template_handler, template_cache, (unsigned char *) TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_S((char *) template_handler->string_value, "<p>first</p>");
    delete_template_handler(template_handler);

    /* a second page out of the very same template costs no parsing,
    the tree that the cache holds is the one it is rendered from, and
    the names of one handler never reach the page of another */
    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    root = entry->root;

    create_template_handler(&template_handler);
    assign_string_template_handler(template_handler, (unsigned char *) "name", "second");
    process_cache_template_handler(template_handler, template_cache, (unsigned char *) TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_S((char *) template_handler->string_value, "<p>second</p>");
    delete_template_handler(template_handler);

    error = acquire_template_cache(
        template_cache,
        (unsigned char *) TEMPLATE_TEST_PATH,
        &entry
    );
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_P(entry->root, root);

    /* a template that is not there leaves the handler with an empty
    result rather than an error, the very same way the processing of
    the file itself does, so that the fallback of the page is reached */
    create_template_handler(&template_handler);
    process_cache_template_handler(template_handler, template_cache, (unsigned char *) TEMPLATE_TEST_GONE);
    V_ASSERT_EQ_S((char *) template_handler->string_value, "");
    delete_template_handler(template_handler);

    delete_template_cache(template_cache);
    remove(TEMPLATE_TEST_PATH);
    V_ASSERT_EQ_U(ALLOCATIONS, allocated);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_quicksort(void) {
    /* allocates space for the template handler */
    size_t list[10] = {2, 4, 1, 2, 3, 5, 5, 3, 4, 1};
    size_t index;

    /* sorts the sequence according to the compare function
    the algorithm to be used in the sorting is the quicksort */
    sort_quicksort((void **) &list, 0, 10, _compare);

    /* the sequence is left in a non decreasing order, the end of
    it is one past the last element and is never looked at */
    for(index = 1; index < 10; index++) {
        V_ASSERT(list[index - 1] <= list[index]);
    }

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_quicksort_linked_list(void) {
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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_crc_32(void) {
    /* allocates space for the crc32 result */
    unsigned long result;

    /* the expected crc32 for "Hello World" is 4a17b156 */
    unsigned long expected = 0x4a17b156;

    /* calculates the crc32 hash value */
    result = crc_32((unsigned char *) "Hello World", 11);

    /* verifies that the computed checksum matches the
    expected value */
    V_ASSERT_M(result == expected, "crc32 of 'Hello World' mismatch");

    /* verifies the crc32 for an empty string,
    the expected value is 00000000 */
    {
        unsigned long empty_expected = 0x00000000;
        result = crc_32((unsigned char *) "", 0);
        V_ASSERT_M(result == empty_expected, "crc32 of empty string mismatch");
    }

    /* verifies the crc32 for "abc" to exercise
    a different input length */
    {
        unsigned long abc_expected = 0x352441c2;
        result = crc_32((unsigned char *) "abc", 3);
        V_ASSERT_M(result == abc_expected, "crc32 of 'abc' mismatch");
    }

    /* verifies the crc32 for a single byte input */
    {
        unsigned long single_expected = 0xe8b7be43;
        result = crc_32((unsigned char *) "a", 1);
        V_ASSERT_M(result == single_expected, "crc32 of 'a' mismatch");
    }

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_md5(void) {
    /* allocates space for the md5 result */
    unsigned char result[MD5_DIGEST_SIZE];

    /* the expected md5 digest for "Hello World" is
    b10a8db164e0754105b7a99be72e3fe5 */
    unsigned char expected[] = {
        0xb1, 0x0a, 0x8d, 0xb1, 0x64, 0xe0, 0x75, 0x41,
        0x05, 0xb7, 0xa9, 0x9b, 0xe7, 0x2e, 0x3f, 0xe5
    };

    /* calculates the md5 hash value into the result */
    md5((unsigned char *) "Hello World", 11, result);

    /* verifies that the computed digest matches the
    expected value byte by byte */
    V_ASSERT_HEX(result, expected, MD5_DIGEST_SIZE);

    /* verifies the md5 digest for an empty string,
    the expected value is d41d8cd98f00b204e9800998ecf8427e */
    {
        unsigned char empty_expected[] = {
            0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
            0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e
        };
        md5((unsigned char *) "", 0, result);
        V_ASSERT_HEX(result, empty_expected, MD5_DIGEST_SIZE);
    }

    /* verifies the md5 digest for a longer string to
    exercise the multi-block code path */
    {
        unsigned char long_expected[] = {
            0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
            0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72
        };
        md5(
            (unsigned char *) "abc",
            3,
            result
        );
        V_ASSERT_HEX(result, long_expected, MD5_DIGEST_SIZE);
    }

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_sha1(void) {
    /* allocates space for the sha1 result */
    unsigned char result[SHA1_DIGEST_SIZE];

    /* the expected sha1 digest for "Hello World" is
    0a4d55a8d778e5022fab701977c5d840bbc486d0 */
    unsigned char expected[] = {
        0x0a, 0x4d, 0x55, 0xa8, 0xd7, 0x78, 0xe5, 0x02,
        0x2f, 0xab, 0x70, 0x19, 0x77, 0xc5, 0xd8, 0x40,
        0xbb, 0xc4, 0x86, 0xd0
    };

    /* calculates the sha1 hash value into the result */
    sha1((unsigned char *) "Hello World", 11, result);

    /* verifies that the computed digest matches the
    expected value byte by byte */
    V_ASSERT_HEX(result, expected, SHA1_DIGEST_SIZE);

    /* verifies the sha1 digest for an empty string,
    the expected value is da39a3ee5e6b4b0d3255bfef95601890afd80709 */
    {
        unsigned char empty_expected[] = {
            0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d,
            0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90,
            0xaf, 0xd8, 0x07, 0x09
        };
        sha1((unsigned char *) "", 0, result);
        V_ASSERT_HEX(result, empty_expected, SHA1_DIGEST_SIZE);
    }

    /* verifies the sha1 digest for "abc" to exercise
    a different input length */
    {
        unsigned char abc_expected[] = {
            0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a,
            0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c,
            0x9c, 0xd0, 0xd8, 0x9d
        };
        sha1((unsigned char *) "abc", 3, result);
        V_ASSERT_HEX(result, abc_expected, SHA1_DIGEST_SIZE);
    }

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_is_path_safe(void) {
    /* verifies that safe paths are accepted */
    V_ASSERT(is_path_safe((unsigned char *) "/index.html") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "/css/style.css") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "/") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "/path/to/file.txt") == 1);

    /* verifies that filenames containing ".." as part of a
    larger name are not falsely rejected */
    V_ASSERT(is_path_safe((unsigned char *) "/..hidden") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "/file..txt") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "/path/..name/file") == 1);
    V_ASSERT(is_path_safe((unsigned char *) "/path/name../file") == 1);

    /* verifies that basic path traversal attempts are rejected */
    V_ASSERT(is_path_safe((unsigned char *) "/../etc/passwd") == 0);
    V_ASSERT(is_path_safe((unsigned char *) "/path/../secret") == 0);
    V_ASSERT(is_path_safe((unsigned char *) "/path/to/../../secret") == 0);
    V_ASSERT(is_path_safe((unsigned char *) "..") == 0);
    V_ASSERT(is_path_safe((unsigned char *) "../etc/passwd") == 0);

    /* verifies that trailing ".." is rejected */
    V_ASSERT(is_path_safe((unsigned char *) "/path/..") == 0);

    /* verifies that backslash traversals are rejected */
    V_ASSERT(is_path_safe((unsigned char *) "/path\\..\\secret") == 0);
    V_ASSERT(is_path_safe((unsigned char *) "\\..\\etc\\passwd") == 0);

    /* verifies that traversal with query string is rejected */
    V_ASSERT(is_path_safe((unsigned char *) "/path/..?foo=bar") == 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_normalize_path(void) {
    /* allocates space for the path buffer to be
    used in the normalize path tests */
    char path[VIRIATUM_MAX_PATH_SIZE];

    /* verifies that forward slashes are normalized
    to the platform separator on Unix systems */
    memcpy(path, "path/to/file", 13);
    normalize_path(path);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp(path, "path\\to\\file") == 0);
#else
    V_ASSERT(strcmp(path, "path/to/file") == 0);
#endif

    /* verifies that backslashes are normalized
    to the platform separator */
    memcpy(path, "path\\to\\file", 13);
    normalize_path(path);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp(path, "path\\to\\file") == 0);
#else
    V_ASSERT(strcmp(path, "path/to/file") == 0);
#endif

    /* verifies that mixed separators are normalized
    to the platform separator */
    memcpy(path, "path/to\\file", 13);
    normalize_path(path);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp(path, "path\\to\\file") == 0);
#else
    V_ASSERT(strcmp(path, "path/to/file") == 0);
#endif

    /* verifies that an empty path is handled
    correctly without any modifications */
    memcpy(path, "", 1);
    normalize_path(path);
    V_ASSERT(strcmp(path, "") == 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_count_file(void) {
    /* allocates space for the size that the counting reports and
    for the error that it raises when it is unable to */
    size_t file_size = 0;
    ERROR_CODE error;

    /* writes a file of a size that is known, which is the one the
    counting of it is expected to come back with */
    write_file((char *) "./viriatum_count_file_test.txt", (unsigned char *) "viriatum", 8);

    error = count_file((char *) "./viriatum_count_file_test.txt", &file_size);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(file_size, 8);

    /* a file that is not there is reported as an error rather than
    as a file of no size at all, the handler tells the two apart by
    exactly this and answers with a not found for the first */
    error = count_file((char *) "./viriatum_count_file_gone.txt", &file_size);
    V_ASSERT(IS_ERROR_CODE(error));
    RESET_ERROR;

    remove("./viriatum_count_file_test.txt");

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_is_directory_file(void) {
    /* allocates space for the flag that the checking reports and
    for the error that it raises when it is unable to */
    unsigned int is_directory = 2;
    ERROR_CODE error;

    /* the directory the process is running from is one, which is
    the very question the handler asks before serving a path */
    error = is_directory_file((char *) ".", &is_directory);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(is_directory, 1);

    /* a file is not a directory, whatever else it may be */
    write_file((char *) "./viriatum_directory_test.txt", (unsigned char *) "viriatum", 8);
    error = is_directory_file((char *) "./viriatum_directory_test.txt", &is_directory);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(is_directory, 0);
    remove("./viriatum_directory_test.txt");

    /* a path that is not there is not a directory either, rather
    than an error, the handler goes on to answer the absence of it */
    is_directory = 2;
    error = is_directory_file((char *) "./viriatum_directory_gone", &is_directory);
    V_ASSERT_EQ_U(error, 0);
    V_ASSERT_EQ_U(is_directory, 0);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_join_path_file(void) {
    /* allocates space for the joined path buffer
    to be used in the join path tests */
    char joined[VIRIATUM_MAX_PATH_SIZE];

    /* verifies that two path components are joined
    with the correct platform separator */
    join_path_file("path", "file", joined);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp(joined, "path\\file") == 0);
#else
    V_ASSERT(strcmp(joined, "path/file") == 0);
#endif

    /* verifies that a trailing separator in the base
    path does not add a duplicate separator */
#ifdef VIRIATUM_PLATFORM_WIN32
    join_path_file("path\\", "file", joined);
    V_ASSERT(strcmp(joined, "path\\file") == 0);
#else
    join_path_file("path/", "file", joined);
    V_ASSERT(strcmp(joined, "path/file") == 0);
#endif

    /* verifies that joining with an empty name
    produces just the base path with separator */
    join_path_file("path", "", joined);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(strcmp(joined, "path\\") == 0);
#else
    V_ASSERT(strcmp(joined, "path/") == 0);
#endif

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_absolute_path_file(void) {
    /* allocates space for the path buffer to be
    used in the absolute path resolution tests */
    char path[VIRIATUM_MAX_PATH_SIZE];
    ERROR_CODE error;

    /* verifies that a relative path is resolved into
    an absolute path with normalization enabled */
    SPRINTF(path, VIRIATUM_MAX_PATH_SIZE, "%s", ".");
    error = absolute_path_file(path, TRUE);
    V_ASSERT(error == 0);
#ifdef VIRIATUM_PLATFORM_WIN32
    V_ASSERT(path[1] == ':');
#else
    V_ASSERT(path[0] == '/');
#endif

    /* verifies that the resolved path does not contain
    the relative reference anymore */
    V_ASSERT(strcmp(path, ".") != 0);

    /* verifies that resolving an invalid path returns
    an error code indicating failure, note that on Windows
    _fullpath does not validate existence so this test
    is only applicable on Unix systems */
#ifndef VIRIATUM_PLATFORM_WIN32
    SPRINTF(path, VIRIATUM_MAX_PATH_SIZE, "%s", "/nonexistent_viriatum_test_path_12345");
    error = absolute_path_file(path, TRUE);
    V_ASSERT(IS_ERROR_CODE(error));
#endif

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

int _compare(void *first, void *second) {
    /* in case the first element is smaller
    than the second element returns a negative
    value indicating the smaller than */
    if(first < second) { return -1; }

    /* in case the first value is larger
    than the second element a positive value
    is return indicating the larger than */
    else if(first > second) {
        return 1;
    }

    /* returns zero value (equals) */
    return 0;
}

/* the table that describes the complete suite of tests for the
commons infra-structure and for the server itself, the tags group
the entries so that a part of the suite may be selected without
every one of the names having to be provided to the runner */
static struct test_entry_t _simple_entries[] = {
#ifndef VIRIATUM_NO_THREADS
#ifdef VIRIATUM_THREAD_SAFE
    V_TEST_T(test_thread_pool, "thread"),
#endif
#endif
    V_TEST_T(test_runner_format_message, "runner"),
    V_TEST_T(test_runner_assert_values, "runner"),
    V_TEST_T(test_runner_match_name, "runner"),
    V_TEST_T(test_runner_match_tags, "runner"),
    V_TEST_T(test_runner_match_entry, "runner"),
    V_TEST_T(test_runner_options, "runner"),
    V_TEST_T(test_runner_run_suite, "runner"),
    V_TEST_T(test_runner_run_kinds, "runner"),
    V_TEST_T(test_runner_list_suite, "runner"),
    V_TEST_T(test_runner_status_label, "runner"),
    V_TEST_T(test_runner_escape_xml, "runner"),
    V_TEST_T(test_runner_write_text, "runner"),
    V_TEST_T(test_runner_write_tap, "runner"),
    V_TEST_T(test_runner_write_junit, "runner"),
    V_TEST_T(test_runner_write_markdown, "runner"),
    V_TEST_T(test_runner_write_report, "runner"),
    V_TEST_T(test_linked_list, "structures"),
    V_TEST_T(test_linked_list_stress, "structures"),
    V_TEST_T(test_linked_list_big, "structures"),
    V_TEST_T(test_array_list, "structures"),
    V_TEST_T(test_hash_map, "structures"),
    V_TEST_T(test_sort_map, "structures"),
    V_TEST_T(test_priority_queue, "structures"),
    V_TEST_T(test_string_buffer, "structures"),
    V_TEST_T(test_linked_buffer, "structures"),
    V_TEST_T(test_base64, "encoding"),
    V_TEST_T(test_bencoding, "encoding"),
    V_TEST_T(test_bit_stream, "stream"),
    V_TEST_T(test_file_stream, "stream"),
    V_TEST_T(test_memory_stream, "stream"),
    V_TEST_T(test_huffman, "compression"),
    V_TEST_T(test_template_engine, "template"),
    V_TEST_T(test_template_engine_buffer, "template"),
    V_TEST_T(test_template_cache, "template"),
    V_TEST_T(test_template_cache_acquire, "template"),
    V_TEST_T(test_template_cache_missing, "template"),
    V_TEST_T(test_template_cache_changed, "template"),
    V_TEST_T(test_template_cache_collision, "template"),
    V_TEST_T(test_template_cache_clear, "template"),
    V_TEST_T(test_template_cache_long, "template"),
    V_TEST_T(test_template_cache_expired, "template"),
    V_TEST_T(test_template_cache_replaced, "template"),
    V_TEST_T(test_template_cache_stale, "template"),
    V_TEST_T(test_template_handler, "template"),
    V_TEST_T(test_template_handler_cache, "template"),
    V_TEST_T(test_quicksort, "sorting"),
    V_TEST_T(test_quicksort_linked_list, "sorting"),
    V_TEST_T(test_crc_32, "checksum"),
    V_TEST_T(test_md5, "checksum"),
    V_TEST_T(test_sha1, "checksum"),
    V_TEST_T(test_is_path_safe, "path"),
    V_TEST_T(test_normalize_path, "path"),
    V_TEST_T(test_count_file, "path"),
    V_TEST_T(test_is_directory_file, "path"),
    V_TEST_T(test_join_path_file, "path"),
    V_TEST_T(test_absolute_path_file, "path"),
    V_TEST_T(test_handler_file_context, "handler"),
    V_TEST_C(test_handler_file_url, "handler", setup_handler_file_test, cleanup_handler_file_test),
    V_TEST_T(test_handler_file_header_field, "handler"),
    V_TEST_T(test_handler_file_header_value, "handler"),
    V_TEST_T(test_handler_file_response, "handler"),
    V_TEST_T(test_handler_file_range, "handler"),
    V_TEST_T(test_handler_file_missing, "handler"),
    V_TEST_T(test_handler_file_missing_template, "handler"),
    V_TEST_T(test_handler_file_gone, "handler"),
    V_TEST_T(test_handler_file_directory, "handler"),
    V_TEST_T(test_handler_file_listing, "handler"),
    V_TEST_T(test_handler_file_listing_changed, "handler"),
    V_TEST_T(test_handler_file_listing_template, "handler"),
    V_TEST_T(test_handler_file_path, "handler"),
    V_TEST_T(test_handler_file_location, "handler"),
    V_TEST_T(test_handler_file_handler, "handler"),
    V_TEST_T(test_handler_file_push, "handler"),
    V_TEST_T(test_file_cache, "handler"),
    V_TEST_T(test_file_cache_acquire, "handler"),
    V_TEST_T(test_file_cache_missing, "handler"),
    V_TEST_T(test_file_cache_changed, "handler"),
    V_TEST_T(test_file_cache_collision, "handler"),
    V_TEST_T(test_file_cache_clear, "handler"),
    V_TEST_T(test_file_cache_open, "handler"),
    V_TEST_T(test_file_cache_long, "handler"),
    V_TEST_T(test_file_cache_expired, "handler"),
    V_TEST_T(test_file_cache_replaced, "handler"),
    V_TEST_T(test_file_cache_rewritten, "handler"),
    V_TEST_T(test_file_cache_stale, "handler"),
    V_TEST_T(test_handler_default_response, "handler"),
    V_TEST_T(test_handler_default_close, "handler"),
    V_TEST_T(test_handler_default_stream, "handler"),
    V_TEST_T(test_handler_default_persistence, "handler"),
    V_TEST_T(test_handler_proxy_request, "handler"),
    V_TEST_T(test_handler_proxy_response, "handler"),
    V_TEST_T(test_handler_proxy_reuse, "handler"),
    V_TEST_T(test_handler_proxy_gateway, "handler"),
    V_TEST_T(test_handler_proxy_upstream, "handler"),
    V_TEST_T(test_handler_proxy_handler, "handler"),
#ifdef VIRIATUM_HTTP2
    V_TEST_T(test_hpack_table, "hpack"),
    V_TEST_T(test_hpack_table_resize, "hpack"),
    V_TEST_T(test_hpack_table_insert, "hpack"),
    V_TEST_T(test_hpack_table_find, "hpack"),
    V_TEST_T(test_hpack_integer, "hpack"),
    V_TEST_T(test_hpack_string, "hpack"),
    V_TEST_T(test_hpack_decode_request, "hpack"),
    V_TEST_T(test_hpack_decode_request_huffman, "hpack"),
    V_TEST_T(test_hpack_decode_response, "hpack"),
    V_TEST_T(test_hpack_decode_errors, "hpack"),
    V_TEST_T(test_hpack_decode_limits, "hpack"),
    V_TEST_T(test_hpack_encode, "hpack"),
    V_TEST_T(test_hpack_huffman, "hpack"),
    V_TEST_T(test_hpack_huffman_errors, "hpack"),
    V_TEST_T(test_http2_number, "http2"),
    V_TEST_T(test_http2_settings, "http2"),
    V_TEST_T(test_http2_decode_frame, "http2"),
    V_TEST_T(test_http2_encode_frame, "http2"),
    V_TEST_T(test_http2_padding, "http2"),
    V_TEST_T(test_http2_priority, "http2"),
    V_TEST_T(test_http2_decode_settings, "http2"),
    V_TEST_T(test_http2_encode_frames, "http2"),
    V_TEST_T(test_http2_verify_frame, "http2"),
    V_TEST_T(test_http2_connection, "http2"),
    V_TEST_T(test_http2_connection_streams, "http2"),
    V_TEST_T(test_http2_connection_priority, "http2"),
    V_TEST_T(test_http2_connection_push, "http2"),
    V_TEST_T(test_http2_connection_window, "http2"),
    V_TEST_T(test_http2_connection_settings, "http2"),
    V_TEST_T(test_http2_connection_frames, "http2"),
    V_TEST_T(test_http2_connection_states, "http2"),
    V_TEST_T(test_http2_connection_headers, "http2"),
    V_TEST_T(test_http2_connection_fields, "http2"),
    V_TEST_T(test_http2_connection_dispatch, "http2"),
    V_TEST_T(test_http2_connection_continuation, "http2"),
    V_TEST_T(test_http2_connection_trailers, "http2"),
    V_TEST_T(test_http2_connection_closed, "http2"),
    V_TEST_T(test_http2_connection_data, "http2"),
    V_TEST_T(test_http2_connection_errors, "http2"),
    V_TEST_T(test_http2_connection_read, "http2"),
    V_TEST_T(test_http2_connection_preface, "http2"),
    V_TEST_T(test_http2_connection_detect, "http2"),
    V_TEST_T(test_http2_connection_response, "http2"),
    V_TEST_T(test_http2_connection_complete, "http2"),
    V_TEST_T(test_http2_connection_error, "http2"),
    V_TEST_T(test_http2_connection_flow, "http2"),
    V_TEST_T(test_http2_connection_split, "http2"),
    V_TEST_T(test_http2_connection_schedule, "http2"),
    V_TEST_T(test_http2_connection_length, "http2"),
    V_TEST_T(test_http2_alpn, "http2"),
#endif
    V_TEST_T(test_websocket_accept_key, "websocket"),
    V_TEST_T(test_websocket_parse_frame, "websocket"),
    V_TEST_T(test_websocket_build_frame, "websocket"),
    V_TEST_T(test_websocket_build_close, "websocket"),
    V_TEST_T(test_websocket_is_control, "websocket"),
    V_TEST_T(test_websocket_close_code, "websocket"),
    V_TEST_T(test_dispatch_handler_context_keepalive, "handler"),
    V_TEST_T(test_dispatch_handler_response, "handler"),
    V_TEST_T(test_delete_service, "service"),
    V_TEST_T(test_create_service_options, "service"),
    V_TEST_T(test_base_path_service, "service"),
    V_TEST_T(test_bundled_path_service, "service"),
    V_TEST_T(test_calculate_options_service, "service"),
    V_TEST_T(test_calculate_locations_service, "service"),
    V_TEST_T(test_open_close_service, "service"),
    V_TEST_T(test_open_service_busy, "service"),
    V_TEST_T(test_file_options_service, "service"),
    V_TEST_T(test_arguments_options_service, "service"),
    V_TEST_T(test_polling_service, "service"),
    V_TEST_T(test_polling_connection, "polling"),
    V_TEST_T(test_polling_read, "polling"),
    V_TEST_T(test_polling_write, "polling"),
    V_TEST_T(test_polling_event, "polling"),
    V_TEST_T(test_polling_closed, "polling"),
    V_TEST_T(test_polling_gone, "polling"),
    V_TEST_T(test_polling_outstanding, "polling"),
    V_TEST_T(test_polling_discarded, "polling"),
    V_TEST_T(test_flags_service, "service"),
    V_TEST_T(test_ran_service, "service")
};

void create_simple_suite(struct test_suite_t *suite) {
    suite->name = "simple_tests";
    suite->entries = _simple_entries;
    suite->count = V_TEST_COUNT(_simple_entries);
    suite->setup = NULL;
    suite->teardown = NULL;
}

ERROR_CODE run_simple_tests(struct test_options_t *options) {
    struct test_suite_t suite;
    ERROR_CODE return_value;
    create_simple_suite(&suite);
    return_value = run_test_suite(&suite, options);
    RAISE_AGAIN(return_value);
}
