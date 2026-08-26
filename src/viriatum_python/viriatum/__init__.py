#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
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
__copyright__ = "Copyright (c) 2008-2026 Hive Solutions Lda."
__license__ = "Apache License, Version 2.0"

from ._viriatum import (
    Server,
    NAME,
    VERSION,
    PLATFORM,
    COMPILER,
    COMPILATION_DATE,
    COMPILATION_TIME,
)

__version__ = VERSION


def serve(application, host="0.0.0.0", port=8080, www_root=None):
    """
    Creates a server for the provided WSGI application and runs
    its loop until the server is stopped.

    :type application: Callable
    :param application: The WSGI application to be served.
    :type host: String
    :param host: The host to which the server should bind.
    :type port: int
    :param port: The TCP port to be listened by the server.
    :type www_root: String
    :param www_root: The root directory for static file serving.
    :rtype: Server
    :return: The server that has just finished its serving.
    """

    server = Server(application, host=host, port=port, www_root=www_root)
    server.serve_forever()
    return server


__all__ = [
    "Server",
    "NAME",
    "VERSION",
    "PLATFORM",
    "COMPILER",
    "COMPILATION_DATE",
    "COMPILATION_TIME",
    "serve",
]
