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
#include "handler_file_test.h"

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

void test_thread_pool() {
    /* allocates space for the index */
    unsigned int index;

    /* allocates space for the thread pool */
    struct thread_pool_t *thread_pool;

    /* allocates space for the thread pool task */
    struct thread_pool_task_t *thread_pool_task =\
        (struct thread_pool_task_t *) MALLOC(sizeof(struct thread_pool_task_t));

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
#endif

const char *test_linked_list() {
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

const char *test_linked_list_stress() {
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

const char *test_linked_list_big() {
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

const char *test_array_list() {
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

const char *test_hash_map() {
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

const char *test_sort_map() {
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

const char *test_priority_queue() {
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

const char *test_string_buffer() {
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

const char *test_linked_buffer() {
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

const char *test_base64() {
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

const char *test_bencoding() {
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

    /* deletes the hash map structure and the list structure
    to avoid any memory leaking */
    delete_hash_map(map);
    delete_linked_list(list);

    /* prints the type structure into the standard output and then
    releases its memory recursively */
    print_type(type); V_PRINT("\n");
    free_type(type);

    /* releases the memory from the encoded buffer, this was
    created during the encoding using bencoding */
    FREE(encoded_buffer);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_bit_stream() {
    /* allocates space for the byte value that is going
    to be used in the read based tezt of the bit stream */
    unsigned char byte;

    /* allocates space for both the file and the
    bit based streams, that are going to be used
    for the test of the bit stream infra-structure */
    struct file_stream_t *file_stream;
    struct bit_stream_t *bit_stream;

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
    it may be used in the testing of the read opertions in the
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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_file_stream() {
    /* allocates space for the file stream */
    struct file_stream_t *file_stream;

    /* allocates space for the stream */
    struct stream_t *stream;

    /* allocates some space for the test buffer */
    unsigned char buffer[128];

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
    V_ASSERT(strcmp((char *) "hello world", (char *) buffer) == 0);

    /* deletes the file stream */
    delete_file_stream(file_stream);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_memory_stream() {
    /* allocates the space for the local stream
    pointers and for the buffer that is going to
    be used for the testing (strings) */
    struct stream_t *stream;
    struct memory_stream_t *memory_stream;
    unsigned char buffer[256];

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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_huffman() {
    /* allocates space for both the file stream that is
    going to be used in the reading process and for the
    huffman structure to be used in the test */
    struct file_stream_t *in_stream;
    struct file_stream_t *out_stream;
    struct huffman_t *huffman;

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

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_template_handler() {
    /* allocates space for the template handler */
    struct template_handler_t *template_handler;

    /* creates the template handler then uses it to process
    the test template file and then deletes the template
    handler removing any memory resources */
    create_template_handler(&template_handler);
    process_template_handler(template_handler, (unsigned char *) "test.tpl");
    delete_template_handler(template_handler);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_quicksort() {
    /* allocates space for the template handler */
    size_t list[10] = { 2, 4, 1, 2, 3, 5, 5, 3, 4, 1 };

    /* sorts the sequence according to the compare function
    the algorithm to be used in the sorting is the quicksort */
    sort_quicksort((void **) &list, 0, 10, _compare);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_quicksort_linked_list() {
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

const char *test_crc_32() {
    /* calculates the crc32 hash value and returns it */
    crc_32((unsigned char *) "Hello World", 11);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_md5() {
    /* allocates space for the md5 result */
    unsigned char result[MD5_DIGEST_SIZE];

    /* calculates the md5 hash value into the result */
    md5((unsigned char *) "Hello World", 11, result);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_sha1() {
    /* allocates space for the sha1 result */
    unsigned char result[SHA1_DIGEST_SIZE];

    /* calculates the sha1 hash value into the result */
    sha1((unsigned char *) "Hello World", 11, result);

    /* returns the default value, nothing happened so there's
    nothing to report for this execution */
    return NULL;
}

const char *test_is_path_safe() {
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

int _compare(void *first, void *second) {
    /* in case the first element is smaller
    than the second element returns a negative
    value indicating the smaller than */
    if(first < second) { return -1; }

    /* in case the first value is larger
    than the second element a positive value
    is return indicating the larger than */
    else if(first > second) { return 1; }

    /* returns zero value (equals) */
    return 0;
}

void exec_simple_tests(struct test_case_t *test_case) {
#ifndef VIRIATUM_NO_THREADS
#ifdef VIRIATUM_THREAD_SAFE
    /* tests the thread pool */
    V_RUN_TEST(test_thread_pool);
#endif
#endif

    /* runs the complete suite of tests for the
    commons infra-structure this a long running
    blocking operation and so it may take some
    time for the complete execution */
    V_RUN_TEST(test_linked_list, test_case);
    V_RUN_TEST(test_linked_list_stress, test_case);
    V_RUN_TEST(test_linked_list_big, test_case);
    V_RUN_TEST(test_array_list, test_case);
    V_RUN_TEST(test_hash_map, test_case);
    V_RUN_TEST(test_sort_map, test_case);
    V_RUN_TEST(test_priority_queue, test_case);
    V_RUN_TEST(test_string_buffer, test_case);
    V_RUN_TEST(test_linked_buffer, test_case);
    V_RUN_TEST(test_base64, test_case);
    V_RUN_TEST(test_bencoding, test_case);
    V_RUN_TEST(test_bit_stream, test_case);
    V_RUN_TEST(test_file_stream, test_case);
    V_RUN_TEST(test_memory_stream, test_case);
    V_RUN_TEST(test_huffman, test_case);
    V_RUN_TEST(test_template_handler, test_case);
    V_RUN_TEST(test_quicksort, test_case);
    V_RUN_TEST(test_quicksort_linked_list, test_case);
    V_RUN_TEST(test_crc_32, test_case);
    V_RUN_TEST(test_md5, test_case);
    V_RUN_TEST(test_sha1, test_case);
    V_RUN_TEST(test_is_path_safe, test_case);
    V_RUN_TEST(test_handler_file_context, test_case);
    V_RUN_TEST(test_handler_file_url, test_case);
    V_RUN_TEST(test_handler_file_header_field, test_case);
    V_RUN_TEST(test_handler_file_header_value, test_case);
}

ERROR_CODE run_simple_tests() {
    ERROR_CODE return_value = run_test_case(exec_simple_tests, "simple_tests");
    RAISE_AGAIN(return_value);
}
