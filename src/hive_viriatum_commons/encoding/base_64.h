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
 * The padding character for base 64.
 */
#define PADDING_BASE_64 '='

/**
 * Encodes into base 64 the buffer with the given length into a new buffer
 * to be created.
 *
 * @param buffer The buffer to be encoded into base 64.
 * @param bufferLength The length of the buffer to be encoded into base64.
 * @param encodedBufferPointer The pointer to the created encoded buffer.
 * @param encodedBufferLengthPointer The length of the created encoded buffer.
 * @return The execution status.
 */
VIRIATUM_EXPORT_PREFIX int encodeBase64(unsigned char *buffer, size_t bufferLength, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer);

/**
 * Decodes from base 64 the encoded buffer with the given length into a new (decoded) buffer
 * to be created.
 *
 * @param encodedBuffer The encoded buffer to be decoded from base 64.
 * @param encodedBufferLength The length of the encoded buffer to be decoded from base64.
 * @param decodedBufferPointer The pointer to the created (decoder) buffer.
 * @param decodedBufferLengthPointer The length of the created (decoder) buffer.
 * @return The execution status.
 */
VIRIATUM_EXPORT_PREFIX int decodeBase64(unsigned char *encodedBuffer, size_t encodedBufferLength, unsigned char **decodedBufferPointer, size_t *decodedBufferLengthPointer);

/**
 * Calculates the encoded buffer length from the
 * given (decoded) buffer length.
 *
 * @param bufferLength The length of the (decoded) buffer.
 * @return The length of the encoded buffer.
 */
VIRIATUM_EXPORT_PREFIX size_t calculateEncodedBufferLengthBase64(size_t bufferLength);

/**
 * Calculates the decoded buffer length from the
 * given encoded buffer length.
 *
 * @param encodedBufferLength The length of the encoded buffer.
 * @param paddingCount The ammount of padding in the base 64 encoded string.
 * @return The length of the decoded buffer.
 */
VIRIATUM_EXPORT_PREFIX size_t calculateDecodedBufferLenghtBase64(size_t encodedBufferLength, size_t paddingCount);

/**
 * Encodes the given buffer into base64.
 * This function is for internal use.
 *
 * @param buffer The buffer to be encoded.
 * @param bufferLength The length of the buffer to be encoded.
 * @param encodedBuffer The target encoded buffer.
 * @param encodedBufferLength The target encoded buffer length.
 * @return The execution status.
 */
VIRIATUM_NO_EXPORT_PREFIX int _encodeBase64(unsigned char *buffer, size_t bufferLength, unsigned char *encodedBuffer, size_t encodedBufferLength);

/**
 * Decodes the given encoded buffer from base64.
 * This function is for internal use.
 *
 * @param encodedBuffer The encoded buffer to be decoded.
 * @param encodedBufferLength The length of the encoded buffer to be decoded.
 * @param buffer The target buffer.
 * @param bufferLength The target buffer length.
 * @param paddingCount The ammount of padding in the base 64 encoded string.
 * @return The execution status.
 */
VIRIATUM_NO_EXPORT_PREFIX int _decodeBase64(unsigned char *encodedBuffer, size_t encodedBufferLength, unsigned char *buffer, size_t bufferLength, size_t paddingCount);

/**
 * Allocates a new encoded buffer.
 * This function is for internal use.
 *
 * @param bufferLength The size of the buffer to be encoded.
 * @param encodedBufferPointer The pointer to the encoded buffer to be created.
 * @param encodedBufferLengthPointer The length of the encoded buffer to be created.
 * @return The execution status.
 */
VIRIATUM_NO_EXPORT_PREFIX int _allocateEncodedBuffer(size_t bufferLength, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer);

/**
 * Allocates a new decoded buffer.
 * This function is for internal use.
 *
 * @param encodedBufferLength The (encoded) buffer to be decoded.
 * @param decodedBufferPointer The pointer to the decoded buffer to be created.
 * @param decodedBufferLengthPointer The length of the decoded buffer to be created.
 * @param paddingCount The ammount of padding to be used.
 * @return The execution status.
 */
VIRIATUM_NO_EXPORT_PREFIX int _allocateDecodedBuffer(size_t encodedBufferLength, unsigned char **decodedBufferPointer, size_t *decodedBufferLengthPointer, size_t paddingCount);

/**
 * Retrieves the padding count for the given encoded buffer
 * and using the given encoded buffer length.
 *
 * @param encodedBuffer The encoded buffer to measure the padding.
 * @param encodedBufferLength The lenght of the encoded buffer.
 * @return The padding count for the given encoded buffer.
 */
VIRIATUM_NO_EXPORT_PREFIX unsigned int _getPaddingCount(unsigned char *encodedBuffer, size_t encodedBufferLength);

/**
 * Looks up the given value in the reverse
 * base 64 table and returns the value.
 *
 * @param value The value to obtain the reverse value.
 * @return The reverse value (result).
 */
VIRIATUM_NO_EXPORT_PREFIX unsigned char _lookupBase64(unsigned char value);

/**
 * Looks up the given value in the reverse
 * base 64 table and returns the value.
 * This method is much faster than the normal
 * implementation
 *
 * @param value The value to obtain the reverse value.
 * @return The reverse value (result).
 */
VIRIATUM_NO_EXPORT_PREFIX unsigned char _lookupFastBase64(unsigned char value);
