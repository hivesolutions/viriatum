#!/usr/bin/python
# -*- coding: utf-8 -*-

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


def serve(application, host="0.0.0.0", port=8080, www_root=None, interface="auto"):
    """
    Creates a server for the provided WSGI or ASGI application
    and runs its loop until the server is stopped.

    :type application: Callable
    :param application: The WSGI or ASGI application to be served.
    :type host: String
    :param host: The host to which the server should bind.
    :type port: int
    :param port: The TCP port to be listened by the server.
    :type www_root: String
    :param www_root: The root directory for static file serving.
    :type interface: String
    :param interface: The interface to be used for the calling of
    the application, either auto, wsgi or asgi.
    :rtype: Server
    :return: The server that has just finished its serving.
    """

    server = Server(
        application,
        host=host,
        port=port,
        www_root=www_root,
        interface=interface
    )
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
