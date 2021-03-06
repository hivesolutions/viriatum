/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

/**
 * The carriage return character.
 */
#define CR '\r'

/**
 * The line feed character.
 */
#define LF '\n'

#define START_STATE (http_parser->type == HTTP_REQUEST ? STATE_START_REQ : STATE_START_RES)

#ifndef MIN
#define MIN(first, second) ((first) < (second) ? (first) : (second))
#endif

#define LOWER(byte) (unsigned char)(byte | 0x20)
#define TOKEN(byte) (tokens[(unsigned char) byte])
#define IS_ALPHA(byte) (LOWER(byte) >= 'a' && LOWER(byte) <= 'z')
#define IS_NUM(byte) ((byte) >= '0' && (byte) <= '9')
#define IS_ALPHANUM(byte) (IS_ALPHA(byte) || IS_NUM(byte))

#if HTTP_PARSER_STRICT
#define IS_URL_CHAR(byte) (normal_url_char[(unsigned char) (byte)])
#define IS_HOST_CHAR(byte) (IS_ALPHANUM(c) || (c) == '.' || (byte) == '-')
#else
#define IS_URL_CHAR(byte) (normal_url_char[(byte)] || ((byte) & 0x80))
#define IS_HOST_CHAR(byte) (IS_ALPHANUM(byte) || (byte) == '.' || (byte) == '-' || (byte) == '_')
#endif

#if HTTP_PARSER_STRICT
#define STRICT_CHECK(condition)\
    do {\
        if(condition) {\
            SET_ERRNO(HPE_STRICT);\
            goto error;\
        }\
    } while(0)
#define NEW_MESSAGE() (http_should_keep_alive(parser) ? START_STATE : STATE_DEAD)
#else
#define STRICT_CHECK(condition)
#define NEW_MESSAGE() START_STATE
#endif

#define HTTP_MARK(FOR)\
    do {\
        FOR##_mark = pointer;\
    } while(0)

#define HTTP_CALLBACK(FOR)\
    do {\
        if(http_settings->on_##FOR) {\
            if(http_settings->on_##FOR(http_parser) != 0) {\
                /*SET_ERRNO(HPE_CB_##FOR);*/\
                /*return (pointer - data);*/\
            }\
        }\
    } while(0)

#define HTTP_CALLBACK_DATA(FOR)\
    do {\
        if(FOR##_mark) {\
            if(http_settings->on_##FOR) {\
                if(http_settings->on_##FOR(http_parser, FOR##_mark, pointer - FOR##_mark) != 0) {\
                /*    SET_ERRNO(HPE_CB_##FOR);*/\
                    /*return (p - data);*/\
                }\
            }\
            FOR##_mark = NULL;\
        }\
    } while(0)

/**
 * The HTTP proxy connection string value.
 */
#define HTTP_PROXY_CONNECTION "proxy-connection"

/**
 * The HTTP connection string value.
 */
#define HTTP_CONNECTION "connection"

/**
 * The HTTP content length string value.
 */
#define HTTP_CONTENT_LENGTH "content-length"

/**
 * The HTTP transfer encoding string value.
 */
#define HTTP_TRANSFER_ENCODING "transfer-encoding"

/**
 * The HTTP upgrade string value.
 */
#define HTTP_UPGRADE "upgrade"

/**
 * The HTTP chunked string value.
 */
#define HTTP_CHUNKED "chunked"

/**
 * The HTTP keep alive string value.
 */
#define HTTP_KEEP_ALIVE "keep-alive"

/**
 * The HTTP close string value.
 */
#define HTTP_CLOSE "close"

struct http_parser_t;

static const char tokens[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
       ' ',      '!',     '"',     '#',     '$',     '%',     '&',    '\'',
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        0,       0,      '*',     '+',      0,      '-',     '.',     '/',
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
       '0',     '1',     '2',     '3',     '4',     '5',     '6',     '7',
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
       '8',     '9',      0,       0,       0,       0,       0,       0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        0,      'a',     'b',     'c',     'd',     'e',     'f',     'g',
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
       'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
       'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
       'x',     'y',     'z',      0,       0,       0,      '^',     '_',
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
       '`',     'a',     'b',     'c',     'd',     'e',     'f',     'g',
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
       'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
       'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
       'x',     'y',     'z',      0,      '|',     '}',     '~',       0
};

static const char unhex[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/**
 * Map defining the validation structure for
 * characters in the url.
 * Chracters that return 0 to this map are considered
 * to be out of the url characters range.
 */
static const unsigned char normal_url_char[256] = {
/*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
        0,       0,       0,       0,       0,       0,       0,       0,
/*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
        0,       1,       1,       0,       1,       1,       1,       1,
/*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
        1,       1,       1,       1,       1,       1,       1,       0,
/*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
        1,       1,       1,       1,       1,       1,       1,       1,
/*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
        1,       1,       1,       1,       1,       1,       1,       1,
/* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
        1,       1,       1,       1,       1,       1,       1,       1,
/* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
        1,       1,       1,       1,       1,       1,       1,       1,
/* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
        1,       1,       1,       1,       1,       1,       1,       0
};

/**
 * List of strings defining the various HTTP method
 * names.
 * This names may be used together with an enumeration
 * defining a sequence of HTTP methods.
 */
static const char *http_method_strings[24] = {
    "DELETE",
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "COPY",
    "LOCK",
    "MKCOL",
    "MOVE",
    "PROPFIND",
    "PROPPATCH",
    "UNLOCK",
    "REPORT",
    "MKACTIVITY",
    "CHECKOUT",
    "MERGE",
    "M-SEARCH",
    "NOTIFY",
    "SUBSCRIBE",
    "UNSUBSCRIBE",
    "PATCH"
};

/**
 * The "default" callback function to be used, without
 * any extra arguments.
 */
typedef ERROR_CODE (*http_callback) (struct http_parser_t *);

/**
 * Callback function type used for callbacks that require
 * "extra" data to be send as argument.
 */
typedef ERROR_CODE (*http_data_callback) (struct http_parser_t *, const unsigned char *, size_t);

/**
 * Callback function type used for callbacks that require
 * "extra" indexes to be send as argument.
 */
typedef ERROR_CODE (*http_index_callback) (struct http_parser_t *, size_t, size_t);

/**
 * Defines the various types of HTTP request.
 */
typedef enum http_request_type_e {
    HTTP_REQUEST = 1,
    HTTP_RESPONSE,
    HTTP_BOTH
} http_request_type;

/**
 * Defines the various HTTP request mehtods.
 * The methods are defined in a random order.
 */
typedef enum http_method_e {
    HTTP_DELETE = 1,
    HTTP_GET,
    HTTP_HEAD,
    HTTP_POST,
    HTTP_PUT,
    HTTP_CONNECT,
    HTTP_OPTIONS,
    HTTP_TRACE,
    HTTP_COPY,
    HTTP_LOCK,
    HTTP_MKCOL,
    HTTP_MOVE,
    HTTP_PROPFIND,
    HTTP_PROPPATCH,
    HTTP_UNLOCK,
    HTTP_REPORT,
    HTTP_MKACTIVITY,
    HTTP_CHECKOUT,
    HTTP_MERGE,
    HTTP_MSEARCH,
    HTTP_NOTIFY,
    HTTP_SUBSCRIBE,
    HTTP_UNSUBSCRIBE,
    HTTP_PATCH
} http_method;

typedef enum http_parser_state_e {
    STATE_DEAD = 1,
    STATE_START_REQ_OR_RES,

    STATE_RES_OR_RESP_H,
    STATE_START_RES,
    STATE_RES_H,
    STATE_RES_HT,
    STATE_RES_HTT,
    STATE_RES_HTTP,
    STATE_RES_FIRST_HTTP_MAJOR,
    STATE_RES_HTTP_MAJOR,
    STATE_RES_FIRST_HTTP_MINOR,
    STATE_RES_HTTP_MINOR,
    STATE_RES_FIRST_STATUS_CODE,
    STATE_RES_STATUS_CODE,
    STATE_RES_STATUS,
    STATE_RES_LINE_ALMOST_DONE,

    STATE_START_REQ,
    STATE_REQ_METHOD,
    STATE_REQ_SPACES_BEFORE_URL,
    STATE_REQ_SCHEMA,
    STATE_REQ_SCHEMA_SLASH,
    STATE_REQ_SCHEMA_SLASH_SLASH,
    STATE_REQ_HOST,
    STATE_REQ_PORT,
    STATE_REQ_PATH,
    STATE_REQ_QUERY_STRING_START,
    STATE_REQ_QUERY_STRING,
    STATE_REQ_FRAGMENT_START,
    STATE_REQ_FRAGMENT,
    STATE_REQ_HTTP_START,
    STATE_REQ_HTTP_H,
    STATE_REQ_HTTP_HT,
    STATE_REQ_HTTP_HTT,
    STATE_REQ_HTTP_HTTP,
    STATE_REQ_FIRST_HTTP_MAJOR,
    STATE_REQ_HTTP_MAJOR,
    STATE_REQ_FIRST_HTTP_MINOR,
    STATE_REQ_HTTP_MINOR,
    STATE_REQ_LINE_ALMOST_DONE,

    STATE_HEADER_FIELD_START,
    STATE_HEADER_FIELD,
    STATE_HEADER_VALUE_START,
    STATE_HEADER_VALUE,
    STATE_HEADER_VALUE_LWS,
    STATE_HEADER_ALMOST_DONE,

    STATE_CHUNK_SIZE_START,
    STATE_CHUNK_SIZE,
    STATE_CHUNK_PARAMETERS,
    STATE_CHUNK_SIZE_ALMOST_DONE,

    STATE_HEADERS_ALMOST_DONE,

    STATE_CHUNK_DATA,
    STATE_CHUNK_DATA_ALMOST_DONE,
    STATE_CHUNK_DATA_DONE,

    STATE_BODY_IDENTITY,
    STATE_BODY_IDENTITY_EOF
} http_parser_state;

typedef enum http_header_state_e {
    HEADER_STATE_GENERAL = 1,
    HEADER_STATE_C,
    HEADER_STATE_CO,
    HEADER_STATE_CON,

    HEADER_STATE_MATCHING_CONNECTION,
    HEADER_STATE_MATCHING_PROXY_CONNECTION,
    HEADER_STATE_MATCHING_CONTENT_LENGTH,
    HEADER_STATE_MATCHING_TRANSFER_ENCODING,
    HEADER_STATE_MATCHING_UPGRADE,

    HEADER_STATE_CONNECTION,
    HEADER_STATE_CONTENT_LENGTH,
    HEADER_STATE_TRANSFER_ENCODING,
    HEADER_STATE_UPGRADE,

    HEADER_STATE_MATCHING_TRANSFER_ENCODING_CHUNKED,
    HEADER_STATE_MATCHING_CONNECTION_KEEP_ALIVE,
    HEADER_STATE_MATCHING_CONNECTION_CLOSE,

    HEADER_STATE_TRANSFER_ENCODING_CHUNKED,
    HEADER_STATE_CONNECTION_KEEP_ALIVE,
    HEADER_STATE_CONNECTION_CLOSE
} http_header_state;

typedef enum http_flags_e {
    FLAG_CHUNKED = 1 << 0,
    FLAG_KEEP_ALIVE = 1 << 1,
    FLAG_CLOSE = 1 << 2,
    FLAG_TRAILING = 1 << 3,
    FLAG_UPGRADE = 1 << 4,
    FLAG_SKIPBODY = 1 << 5
} http_flags;

/**
 * Structure representing an HTTP parser
 * it contains information about parsing
 * including state, size contents and control flags.
 */
typedef struct http_parser_t {
    enum http_request_type_e type;
    unsigned char flags;
    enum http_parser_state_e state;
    enum http_header_state_e header_state;
    unsigned char index;
    size_t read_count;
    int content_length;
    unsigned short http_major;
    unsigned short http_minor;
    unsigned short status_code;
    unsigned char method;
    char upgrade;
    void *context;
    unsigned char *header_field_mark;
    unsigned char *header_value_mark;
    unsigned char *url_mark;

    /**
     * Unstructured reference to a pointer, this may
     * be used to maintain references to upper objects.
     * Example usage would include reference to the
     * connection objects.
     */
    void *parameters;

    /**
     * The original content length value, this
     * value is not changed across parsing requests.
     * Saving this value overcomes the issue created
     * by reseting the content value along the parsing
     * logic.
     */
    size_t _content_length;
} http_parser;

/**
 * Structure representing the various settings
 * to be used for parsing the HTTP message.
 */
typedef struct http_settings_t {
    http_callback on_message_begin;
    http_data_callback on_url;
    http_callback on_line;
    http_data_callback on_header_field;
    http_data_callback on_header_value;
    http_callback on_headers_complete;
    http_data_callback on_body;
    http_callback on_message_complete;
    http_data_callback on_path;
    http_index_callback on_location;
    http_data_callback on_virtual_url;
} http_settings;

/**
 * Constructor of the HTTP parser.
 *
 * @param http_parser_pointer The pointer to the HTTP parser to be constructed.
 * @param request If the parser is meant to be created for a request if not
 * set the parser assumes it's a response.
 */
void create_http_parser(struct http_parser_t **http_parser_pointer, char request);

/**
 * Destructor of the HTTP parser.
 *
 * @param http_parser The HTTP parser to be destroyed.
 */
void delete_http_parser(struct http_parser_t *http_parser);

/**
 * Constructor of the HTTP settings.
 *
 * @param http_settings_pointer The pointer to the HTTP settings to be constructed.
 */
void create_http_settings(struct http_settings_t **http_settings_pointer);

/**
 * Destructor of the HTTP settings.
 *
 * @param http_settings The HTTP settings to be destroyed.
 */
void delete_http_settings(struct http_settings_t *http_settings);

/**
 * Called to process a new data chunk in the context
 * of an HTTP parsing structure.
 * This function should be called whenever a new data
 * chunk is received.
 *
 * @param http_parser The HTTP parser object.
 * @param http_settings The HTTP settings for the processing.
 * @param data The data to be parsed.
 * @param data_size The size of the data to be parsed.
 * @return The number of bytes used during the processing.
 */
int process_data_http_parser(struct http_parser_t *http_parser, struct http_settings_t *http_settings, unsigned char *data, size_t data_size);

/**
 * Retrieves the string representing the HTTP method for
 * the given HTTP method integer represented in the
 * enumeration.
 *
 * @param http_method The HTTP method integer to be "converted"
 * into string representation.
 * @return The string representation of the HTTP method.
 */
static __inline const char *get_http_method_string(enum http_method_e http_method) {
    return http_method_strings[http_method - 1];
}
