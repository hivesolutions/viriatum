#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Serves the application of the harness through the embedded interface
of the server, the counterpart of the command lines that start the
reference of each one of the interfaces, so that the very same object
is reached through the server and through the reference of it.
"""

import sys

import app

import viriatum


def main():
    interface = sys.argv[1]
    port = int(sys.argv[2])

    # the streaming variant is an application of its own rather than a
    # mode of the one beside it, the interface that carries it is
    # still the more recent of the two
    if interface == "asgi-stream":
        application = app.asgi_stream_app
        interface = "asgi"
    elif interface == "asgi":
        application = app.asgi_app
    else:
        application = app.wsgi_app

    viriatum.serve(application, host="127.0.0.1", port=port, interface=interface)


if __name__ == "__main__":
    main()
