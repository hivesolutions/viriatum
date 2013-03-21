#!/bin/bash
# -*- coding: utf-8 -*-

HOST=${HOST-localhost}
PORT=${PORT-9090}

pathoc -p $PORT $HOST 'get:/:b@10:h"Content-Length"="x"'
pathoc -p $PORT $HOST 'get:/:i4,"\r"'
pathoc -p $PORT $HOST 'get:/:h"h\r\n"="x"'
pathoc -p $PORT $HOST 'get:"/\0"'
pathoc -p $PORT $HOST 'get:/:i16," "'
pathoc -p $PORT $HOST 'get:/:h"Host"="n\r\0"'
pathoc -p $PORT $HOST 'get:/:b@100,ascii:ir,@1'
