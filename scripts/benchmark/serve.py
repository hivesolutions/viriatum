#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Benchmark server that serves the application of the harness through
the embedded interface of the server, the counterpart of the command
lines that start the reference of each one of the interfaces, so that
the very same object is reached through the server and through the
reference of it.

The interface is named on the command line rather than guessed at, so
that the shape being measured is always the one that was asked for,
and the streaming variant is an application of its own rather than a
mode of the one beside it.

Run from the project root with:
    python scripts/benchmark/serve.py <interface> <port>

Arguments:
    interface   One of wsgi, asgi or asgi-stream
    port        The tcp port the server binds
"""

from collections.abc import Callable
from sys import argv
from typing import Any

import app

import viriatum


def main() -> None:
    interface = argv[1]
    port = int(argv[2])

    # the streaming variant is an application of its own rather than a
    # mode of the one beside it, the interface that carries it is
    # still the more recent of the two
    application: Callable[..., Any]
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
