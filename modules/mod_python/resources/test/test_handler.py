#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.

"""
Minimal application of the older interface, driven by the suite of
the module so that the output of it may be verified from the side of
the project that is written in C.

Answers every request the same way, with the method and the path it
was reached through written into the body, so that the harness is
able to tell one request from another without the application ever
having to carry state of its own.
"""

from collections.abc import Callable
from typing import Any

Environ = dict[str, Any]
""" The map of the request as the interface builds it, one entry per
part of the request and of the environment """

StartResponse = Callable[[str, list[tuple[str, str]]], Any]
""" The operation that opens the answer, taking the status and the
fields that travel with it """


def application(environ: Environ, start_response: StartResponse) -> list[bytes]:
    method = environ.get("REQUEST_METHOD", "GET")
    path = environ.get("PATH_INFO", "/")

    body = ("method=%s\npath=%s\n" % (method, path)).encode("utf-8")

    headers = [
        ("Content-Type", "text/plain"),
        ("Content-Length", str(len(body))),
        ("X-Test", "viriatum"),
    ]

    start_response("200 OK", headers)
    return [body]
