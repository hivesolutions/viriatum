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

#define ETAG_H "ETag"
#define HOST_H "Host"
#define COOKIE_H "Cookie"
#define SERVER_H "Server"
#define LOCATION_H "Location"
#define LOCATION_H "Location"
#define CONNECTION_H "Connection"
#define CONTENT_TYPE_H "Content-Type"
#define ACCEPT_RANGES_H "Accept-Ranges"
#define CONTENT_RANGE_H "Content-Range"
#define CACHE_CONTROL_H "Cache-Control"
#define CONTENT_LENGTH_H "Content-Length"
#define WWW_AUTHENTICATE_H "WWW-Authenticate"
#define TRANSFER_ENCODING_H "Transfer-Encoding"
#define ACCESS_CONTROL_ORIGIN_H "Access-Control-Allow-Origin"
#define ACCESS_CONTROL_METHODS_H "Access-Control-Allow-Methods"
#define ACCESS_CONTROL_HEADERS_H "Access-Control-Allow-Headers"

/**
 * The scheme values under which a message may be
 * received, these are static strings and as such
 * must never be released.
 */
#define HTTP_SCHEME "http"
#define HTTPS_SCHEME "https"

/**
 * The array defining the various strings indicating
 * the respective HTTP protocol version according to
 * the pre-defined enumeration.
 */
static const char *http_version_strings[4] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1",
    "HTTP/2.0"
};

/**
 * The array defining the various strings indicating the
 * numeric part of the respective HTTP protocol version,
 * these are the values expected by the surfaces that report
 * the version on its own, as is the case for the scope of
 * an ASGI application.
 */
static const char *http_version_numbers[4] = {
    "0.9",
    "1.0",
    "1.1",
    "2"
};

/**
 * Enumeration defining the various possible
 * and "recognizable" HTTP header types, this
 * may be used to provided static reference.
 */
typedef enum http_header_e {
    UNDEFINED_HEADER = 1,
    CONTENT_TYPE,
    CONTENT_LENGTH,
    COOKIE,
    HOST,
    ETAG,
    CACHE_CONTROL,
    AUTHORIZATION,
    RANGE
} http_header;

/**
 * Enumeration defining the various possible
 * and "recognizable" protocol versions for
 * the HTTP protocol.
 */
typedef enum http_version_e {
    HTTP09 = 1,
    HTTP10,
    HTTP11,
    HTTP20
} http_version;

/**
 * Enumeration defining the various possible
 * methods for an HTTP request.
 */
typedef enum http_request_method_e {
    GET_REQUEST_METHOD = 1,
    POST_REQUEST_METHOD
} http_request_method;

/**
 * Enumeration defining the various possible
 * keep alive situations (modes).
 */
typedef enum http_keep_alive_e {
    KEEP_CLOSE = 1,
    KEEP_ALIVE
} http_keep_alive;

/**
 * Enumeration defining the various possible
 * HTTP cache control strategies.
 */
typedef enum http_cache_e {
    NO_CACHE = 1,
    MAX_AGE
} http_cache;

typedef struct http_header_value_t {
    char name[VIRIATUM_MAX_HEADER_SIZE];
    char value[VIRIATUM_MAX_HEADER_V_SIZE];
    size_t name_size;
    size_t value_size;
} http_header_value;

typedef struct http_headers_t {
    struct http_header_value_t values[VIRIATUM_MAX_HEADER_COUNT];
    size_t count;
} http_headers_value;

/**
 * Structure describing a message as it is seen by an
 * handler, the structure is populated by the protocol
 * specific layer, either the HTTP/1.1 parser or the
 * HTTP/2 session, and is the only message oriented
 * structure that an handler is meant to interact with.
 * Under HTTP/1.1 there's one of these structures per
 * connection, as only one message is in transit at a
 * given time, under HTTP/2 there's one per stream.
 */
typedef struct http_request_t {
    /**
     * The version of the protocol that has delivered the
     * message, this value is used by the response writing
     * operations to decide on the encoding to be used.
     */
    enum http_version_e version;

    /**
     * The method of the request according to the values of
     * the http_method enumeration, kept as an integer so
     * that the complete set of methods remains
     * representable.
     */
    unsigned char method;

    /**
     * The status code of the message, only meaningful for
     * a structure that describes a response, as is the case
     * for the upstream side of the proxy handler.
     */
    unsigned short status_code;

    /**
     * The set of control flags for the message, the bits are
     * the ones defined by the http_flags enumeration so that
     * an handler reads them in the same way no matter the
     * protocol that has delivered the message.
     */
    unsigned char flags;

    /**
     * The identifier of the stream that carries the message,
     * the value is zero under HTTP/1.1, where a connection
     * carries one message at a time, and an odd value for a
     * client initiated HTTP/2 stream.
     */
    unsigned int stream_id;

    /**
     * Flag controlling if a trailer section is expected to be
     * received once the body of the message is complete, only
     * ever set for a chunked HTTP/1.1 message or for an HTTP/2
     * stream whose header block is not the final one.
     */
    char trailers;

    /**
     * The scheme under which the message has been received,
     * either http or https, this is a static string and as
     * such must never be released.
     */
    const char *scheme;

    /**
     * The authority of the message, meaning the host and the
     * optional port, populated from the host header under
     * HTTP/1.1 and from the authority pseudo header under
     * HTTP/2.
     */
    unsigned char authority[VIRIATUM_MAX_PATH_SIZE];

    /**
     * The complete path of the message, including the query
     * part, populated from the request line under HTTP/1.1 and
     * from the path pseudo header under HTTP/2.
     */
    unsigned char path[VIRIATUM_MAX_URL_SIZE];

    /**
     * The size in bytes of the path, kept together with it so
     * that the append operation does not have to measure the
     * string on every one of the fragments it receives.
     */
    size_t path_size;

    /**
     * The original content length value, this value is not
     * changed across the parsing of the message and overcomes
     * the issue created by the reset of the transient value
     * kept by the parser.
     */
    size_t content_length;

    /**
     * Reference to the handler specific context structure,
     * owned by the currently active handler.
     * Each handler stores its own context type (eg: file
     * handler stores handler_file_context, proxy handler
     * stores handler_proxy_context).
     * This value is set to NULL on message initialization
     * and should be managed by the handler during the
     * request life-cycle.
     */
    void *context;

    /**
     * Unstructured reference to a pointer, this may be used
     * to maintain references to upper objects. Example usage
     * would include reference to the connection objects.
     */
    void *parameters;
} http_request;

/**
 * Constructor of the HTTP request.
 *
 * @param http_request_pointer The pointer to the HTTP request to be constructed.
 */
void create_http_request(struct http_request_t **http_request_pointer);

/**
 * Destructor of the HTTP request.
 *
 * @param http_request The HTTP request to be destroyed.
 */
void delete_http_request(struct http_request_t *http_request);

/**
 * Resets the provided HTTP request to its initial state, so
 * that it may be re-used for the handling of a new message
 * in the same connection, note that the scheme, the context
 * and the parameters references are preserved as they are
 * owned by the layers above the message itself.
 *
 * @param http_request The HTTP request to be reset.
 */
void reset_http_request(struct http_request_t *http_request);

/**
 * Appends the provided data to the path of the given HTTP
 * request, this is meant to be used by a protocol layer that
 * receives the path in a fragmented fashion, as is the case
 * for the HTTP/1.1 parser. The data that would overflow the
 * buffer of the path is discarded, keeping the string always
 * properly terminated.
 *
 * @param http_request The HTTP request to have the path
 * appended with the provided data.
 * @param data The buffer containing the fragment of the path
 * to be appended.
 * @param data_size The size in bytes of the fragment of the
 * path to be appended.
 */
void append_path_http_request(struct http_request_t *http_request, const unsigned char *data, size_t data_size);

/**
 * Retrieves the string representing the HTTP version for
 * the given HTTP version integer represented in the
 * enumeration.
 *
 * @param http_method The HTTP version integer to be "converted"
 * into string representation.
 * @return The string representation of the HTTP version.
 */
static __inline const char *get_http_version_string(enum http_version_e http_version) {
    return http_version_strings[http_version - 1];
}

/**
 * Retrieves the string representing the numeric part of the
 * HTTP version for the given HTTP version integer represented
 * in the enumeration.
 *
 * @param http_version The HTTP version integer to be "converted"
 * into its numeric string representation.
 * @return The numeric string representation of the HTTP version.
 */
static __inline const char *get_http_version_number(enum http_version_e http_version) {
    return http_version_numbers[http_version - 1];
}

/**
 * Converts the provided major and minor based version set
 * of integer values into a more standard representation
 * of the HTTP version values using enumerations.
 *
 * @param http_major The major version value for the HTTP
 * protocol to be used in the enumeration conversion.
 * @param http_minor The minor version value for the http
 * protocol  to be used in the enumeration conversion.
 * @return The standard enumeration oriented version of the
 * HTTP version value converted accordingly.
 */
static __inline enum http_version_e get_http_version(unsigned short http_major, unsigned short http_minor) {
    switch(http_major) {
        case 0:
            switch(http_minor) {
                case 9:
                    return HTTP09;
            }

            break;

        case 1:
            switch(http_minor) {
                case 0:
                    return HTTP10;

                case 1:
                    return HTTP11;
            }

            break;

        case 2:
            switch(http_minor) {
                case 0:
                    return HTTP20;
            }

            break;
    }

    return HTTP11;
}
