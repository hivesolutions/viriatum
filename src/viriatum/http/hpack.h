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

#pragma once

/**
 * The number of entries of the static table defined by
 * the appendix A of RFC 7541, an index below or equal to
 * this value refers to the static table and one above it
 * refers to the dynamic one.
 */
#define HPACK_STATIC_SIZE 61

/**
 * The default and also maximum size in bytes of the dynamic
 * table, this is the value that gets advertised to the peer
 * through the header table size setting and as such the peer
 * is never allowed to grow the table beyond it.
 */
#define HPACK_TABLE_SIZE 4096

/**
 * The overhead in bytes that the specification accounts for
 * every entry of the dynamic table, on top of the sizes of
 * the name and of the value of the header field.
 */
#define HPACK_ENTRY_OVERHEAD 32

/**
 * The maximum number of entries that the dynamic table may
 * hold, an entry can never be smaller than the overhead and
 * so this is the tightest bound that the table size allows.
 */
#define HPACK_MAX_ENTRIES (HPACK_TABLE_SIZE / HPACK_ENTRY_OVERHEAD)

/**
 * The maximum accumulated size in bytes of the header list of
 * a single message, a block that decodes to more than this is
 * refused. This is the main memory exhaustion vector of the
 * protocol, as a small compressed block may expand into a very
 * large header list.
 */
#define HPACK_MAX_HEADER_LIST_SIZE 32768

/**
 * The maximum size in bytes of the name and of the value of a
 * single header field, a field that decodes to more than this
 * is refused before any space is committed for it.
 */
#define HPACK_MAX_NAME_SIZE VIRIATUM_MAX_HEADER_SIZE
#define HPACK_MAX_VALUE_SIZE VIRIATUM_MAX_HEADER_V_SIZE

/**
 * The number of bits of the longest code of the Huffman table
 * of the appendix B of RFC 7541 and the symbol that encodes
 * the end of string condition.
 */
#define HPACK_HUFFMAN_MAX_BITS 30
#define HPACK_HUFFMAN_EOS 256

/**
 * Structure describing a single header field, both the name
 * and the value are referenced and not owned, so they remain
 * valid only for as long as the buffer that carries them.
 */
typedef struct hpack_header_t {
    unsigned char *name;
    size_t name_size;
    unsigned char *value;
    size_t value_size;
} hpack_header;

/**
 * Structure describing a single entry of the dynamic table,
 * unlike a header field the entry owns the memory of both the
 * name and the value, released once it is evicted.
 */
typedef struct hpack_entry_t {
    unsigned char *name;
    size_t name_size;
    unsigned char *value;
    size_t value_size;

    /**
     * The size of the entry as accounted by the specification,
     * the sum of the sizes of the name and of the value plus
     * the constant overhead of an entry.
     */
    size_t size;
} hpack_entry;

/**
 * Structure describing the dynamic table of one direction of a
 * connection, the entries are kept in a ring so that both the
 * insertion of a new entry and the eviction of the oldest one
 * are constant in time.
 */
typedef struct hpack_table_t {
    /**
     * The ring of entries, the newest entry sits at the head
     * and the oldest one at the head deducted of the count.
     */
    struct hpack_entry_t entries[HPACK_MAX_ENTRIES];

    /**
     * The position in the ring of the newest entry, only
     * meaningful when the table is not empty.
     */
    size_t head;

    /**
     * The number of entries currently held by the table.
     */
    size_t count;

    /**
     * The accumulated size in bytes of the entries currently
     * held, as accounted by the specification.
     */
    size_t size;

    /**
     * The maximum size in bytes that the table is allowed to
     * reach, changed by a dynamic table size update and never
     * above the size that has been advertised to the peer.
     */
    size_t max_size;
} hpack_table;

/**
 * Callback invoked for each one of the header fields that the
 * decoding of a header block produces, returning an error code
 * from it aborts the decoding.
 */
typedef ERROR_CODE (*hpack_callback)(void *parameters, struct hpack_header_t *hpack_header);

/**
 * Constructor of the HPACK dynamic table.
 *
 * @param hpack_table_pointer The pointer to the HPACK table to be constructed.
 */
ERROR_CODE create_hpack_table(struct hpack_table_t **hpack_table_pointer);

/**
 * Destructor of the HPACK dynamic table, releases every one of
 * the entries that the table is still holding.
 *
 * @param hpack_table The HPACK table to be destroyed.
 */
ERROR_CODE delete_hpack_table(struct hpack_table_t *hpack_table);

/**
 * Changes the maximum size of the provided dynamic table,
 * evicting as many of the oldest entries as required for the
 * table to fit in the new size.
 *
 * @param hpack_table The HPACK table to be resized.
 * @param max_size The new maximum size in bytes of the table.
 * @return The resulting error code.
 */
ERROR_CODE resize_hpack_table(struct hpack_table_t *hpack_table, size_t max_size);

/**
 * Inserts the provided header field at the head of the dynamic
 * table, evicting the oldest entries until the new one fits.
 * An entry that is larger than the complete table empties it
 * and is then not inserted, as the specification requires.
 *
 * @param hpack_table The HPACK table to be inserted into.
 * @param hpack_header The header field to be inserted.
 * @return The resulting error code.
 */
ERROR_CODE insert_hpack_table(struct hpack_table_t *hpack_table, struct hpack_header_t *hpack_header);

/**
 * Retrieves the header field sitting at the provided index of
 * the combined address space of the static and of the dynamic
 * tables, the index is the one used on the wire and as such is
 * based at one.
 *
 * @param hpack_table The HPACK table to retrieve the field from.
 * @param index The index of the field in the combined space.
 * @param hpack_header The structure to be populated with the
 * name and the value of the field.
 * @return The resulting error code.
 */
ERROR_CODE get_hpack_table(struct hpack_table_t *hpack_table, size_t index, struct hpack_header_t *hpack_header);

/**
 * Searches both the static and the dynamic tables for the
 * provided header field, preferring an entry that matches both
 * the name and the value over one that matches only the name.
 *
 * @param hpack_table The HPACK table to be searched.
 * @param hpack_header The header field to be searched for.
 * @param index The variable to be set with the index found, it
 * is set to zero when there's no match at all.
 * @return The value one in case the value has also matched and
 * the value zero in case only the name has.
 */
char find_hpack_table(struct hpack_table_t *hpack_table, struct hpack_header_t *hpack_header, size_t *index);

/**
 * Decodes an integer from the provided buffer using the prefix
 * based representation of the section 5.1 of RFC 7541, an
 * encoding whose continuation would overflow the target type is
 * refused instead of being wrapped around.
 *
 * @param data The buffer containing the encoded integer.
 * @param data_size The size in bytes of the provided buffer.
 * @param offset The position in the buffer to read from, it is
 * updated with the position that follows the integer.
 * @param prefix The number of bits of the prefix, between one
 * and eight.
 * @param value The variable to be set with the decoded value.
 * @return The resulting error code.
 */
ERROR_CODE decode_integer_hpack(const unsigned char *data, size_t data_size, size_t *offset, unsigned char prefix, size_t *value);

/**
 * Encodes an integer into the provided buffer using the prefix
 * based representation of the section 5.1 of RFC 7541.
 *
 * @param buffer The buffer to write the encoded integer into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param offset The position in the buffer to write at, it is
 * updated with the position that follows the integer.
 * @param prefix The number of bits of the prefix, between one
 * and eight.
 * @param flags The bits that sit above the prefix and that
 * identify the representation being written.
 * @param value The value to be encoded.
 * @return The resulting error code.
 */
ERROR_CODE encode_integer_hpack(unsigned char *buffer, size_t buffer_size, size_t *offset, unsigned char prefix, unsigned char flags, size_t value);

/**
 * Decodes a string literal from the provided buffer, handling
 * both the raw and the Huffman coded forms as described by the
 * section 5.2 of RFC 7541.
 *
 * @param data The buffer containing the encoded string.
 * @param data_size The size in bytes of the provided buffer.
 * @param offset The position in the buffer to read from, it is
 * updated with the position that follows the string.
 * @param buffer The buffer to write the decoded string into.
 * @param buffer_size The size in bytes of the target buffer.
 * @param string_size The variable to be set with the size of
 * the decoded string.
 * @return The resulting error code.
 */
ERROR_CODE decode_string_hpack(const unsigned char *data, size_t data_size, size_t *offset, unsigned char *buffer, size_t buffer_size, size_t *string_size);

/**
 * Encodes a string literal into the provided buffer, the
 * Huffman coded form is used whenever it is the shorter of the
 * two representations.
 *
 * @param buffer The buffer to write the encoded string into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param offset The position in the buffer to write at, it is
 * updated with the position that follows the string.
 * @param value The string to be encoded.
 * @param value_size The size in bytes of the string.
 * @return The resulting error code.
 */
ERROR_CODE encode_string_hpack(unsigned char *buffer, size_t buffer_size, size_t *offset, const unsigned char *value, size_t value_size);

/**
 * Decodes a complete header block, calling the provided
 * callback once for each one of the header fields it contains
 * and updating the dynamic table along the way.
 *
 * @param hpack_table The dynamic table of the decoding side.
 * @param data The buffer containing the header block.
 * @param data_size The size in bytes of the header block.
 * @param callback The callback to be invoked for each field.
 * @param parameters The opaque value handed to the callback.
 * @return The resulting error code.
 */
ERROR_CODE decode_hpack(struct hpack_table_t *hpack_table, const unsigned char *data, size_t data_size, hpack_callback callback, void *parameters);

/**
 * Encodes a single header field into the provided buffer,
 * using an indexed representation whenever the field is already
 * present in one of the tables.
 *
 * @param hpack_table The dynamic table of the encoding side.
 * @param buffer The buffer to write the encoded field into.
 * @param buffer_size The size in bytes of the provided buffer.
 * @param offset The position in the buffer to write at, it is
 * updated with the position that follows the field.
 * @param hpack_header The header field to be encoded.
 * @param indexing The value one in case the field may be added
 * to the dynamic table and the value zero otherwise.
 * @return The resulting error code.
 */
ERROR_CODE encode_hpack(struct hpack_table_t *hpack_table, unsigned char *buffer, size_t buffer_size, size_t *offset, struct hpack_header_t *hpack_header, char indexing);

/**
 * Decodes a Huffman coded buffer using the canonical code of
 * the appendix B of RFC 7541, both a padding that is not the
 * prefix of the end of string symbol and the presence of the
 * symbol itself are refused.
 *
 * @param data The buffer containing the coded bytes.
 * @param data_size The size in bytes of the coded buffer.
 * @param buffer The buffer to write the decoded bytes into.
 * @param buffer_size The size in bytes of the target buffer.
 * @param result_size The variable to be set with the size of
 * the decoded buffer.
 * @return The resulting error code.
 */
ERROR_CODE decode_huffman_hpack(const unsigned char *data, size_t data_size, unsigned char *buffer, size_t buffer_size, size_t *result_size);

/**
 * Encodes the provided buffer using the Huffman code of the
 * appendix B of RFC 7541, padding the last byte with the prefix
 * of the end of string symbol.
 *
 * @param data The buffer containing the bytes to be coded.
 * @param data_size The size in bytes of the provided buffer.
 * @param buffer The buffer to write the coded bytes into.
 * @param buffer_size The size in bytes of the target buffer.
 * @param result_size The variable to be set with the size of
 * the coded buffer.
 * @return The resulting error code.
 */
ERROR_CODE encode_huffman_hpack(const unsigned char *data, size_t data_size, unsigned char *buffer, size_t buffer_size, size_t *result_size);

/**
 * Measures the size in bytes that the Huffman coded form of the
 * provided buffer would occupy, used to decide between the two
 * representations of a string literal.
 *
 * @param data The buffer containing the bytes to be measured.
 * @param data_size The size in bytes of the provided buffer.
 * @return The size in bytes of the coded form.
 */
size_t size_huffman_hpack(const unsigned char *data, size_t data_size);
