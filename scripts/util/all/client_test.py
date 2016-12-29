#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2017 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

__author__ = "João Magalhães <joamag@hive.pt>"
""" The author(s) of the module """

__version__ = "1.0.0"
""" The version of the module """

__revision__ = "$LastChangedRevision$"
""" The revision number of the module """

__date__ = "$LastChangedDate$"
""" The last change date of the module """

__copyright__ = "Copyright (c) 2008-2017-2012 Hive Solutions Lda."
""" The copyright for the module """

__license__ = "Apache License, Version 2.0"
""" The license for the module """

import time
import socket

HOST = "127.0.0.1"
""" The name of the host to connect to, this
value may be a dns name or an ip address """

PORT = 9090
""" The target tcp port for the connection """

PATH = "/index.html"
""" The path of the file that is going to be
retrieved from the target server """

SIZE = 10024
""" The size of the buffer to be used in the
reading of the response value """

WRITE_SLEEP = 0.25
""" The ammount of time to be used between write
operations (this should ensure message separation) """

SIMPLE = [
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "\r\n" % (PATH, HOST)
]
""" The simple example sequence of data
string (to be used for simple testing) """

COMPLEX = [
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "\r\n" % (PATH, HOST),
    "GET %s HTTP/1.1\r\n"
    "Connection: keep-alive\r\n"
    "Hos" % PATH,
    "t: %s\r\n"
    "\r\n"
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "\r\n" % (HOST, PATH, HOST)
]
""" The complex example sequence of data
string (to be used for cutted string testing) """

PIPELINING = [
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "\r\n" % (PATH, HOST, PATH, HOST, PATH, HOST)
]
""" The pipelining example sequence of data
string (to be used for long pipellined string
testing) one string containing three messages """

def call(messages):
    # creates the socket object and connects it to the
    # target host and port
    _socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    _socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    _socket.connect((HOST, PORT))

    # iterates over each of the message to be
    # sent and sends them to the server side, note
    # that a delay time is used to separate messages
    for message in messages:
        _socket.send(message)
        print "Sent:", message
        time.sleep(WRITE_SLEEP)

    # receives the complete chunk of responses
    # from the server side and then breaks
    # the loop on connection closed
    while True:
        data = _socket.recv(SIZE)
        print "Received:", data
        if not data: break

    # closes the socket, no more data is transmitted
    _socket.close()

if __name__ == "__main__":
    # runs the call operation on the complete set
    # of calls to be processed
    call(SIMPLE + COMPLEX + PIPELINING)
