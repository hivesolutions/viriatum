#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.

"""
Minimal application of the more recent interface, driven by the suite
of the module so that the loading of it may be verified from the side
of the project that is written in C.

Answers every request the same way, with the method and the path it
was reached through written into the body, so that the harness is
able to tell one request from another without the application ever
having to carry state of its own.

Carries the three shapes of a callable that the module tells apart,
the single one of the more recent version, the pair of the older and
the class that stands for the very same pair.
"""

from collections.abc import Awaitable, Callable
from typing import Any

Scope = dict[str, Any]
""" The map of the connection as the interface builds it, which says
what kind of exchange is being served """

Message = dict[str, Any]
""" A single event of the interface, the ones that carry the answer
and the ones that carry what the client sent """

Receive = Callable[[], Awaitable[Message]]
""" The operation that reads the next event of the client """

Send = Callable[[Message], Awaitable[None]]
""" The operation that writes the next event of the answer """


async def application(scope: Scope, receive: Receive, send: Send) -> None:
    method = scope.get("method", "GET")
    path = scope.get("path", "/")

    body = ("method=%s\npath=%s\n" % (method, path)).encode("utf-8")

    await send(
        dict(
            type="http.response.start",
            status=200,
            headers=[
                (b"content-type", b"text/plain"),
                (b"content-length", str(len(body)).encode("utf-8")),
                (b"x-test", b"viriatum"),
            ],
        )
    )
    await send(dict(type="http.response.body", body=body))


def legacy(scope: Scope) -> Callable[..., Any]:
    # the older of the two shapes, the scope reaching the application
    # apart from the callables that carry the exchange
    async def application(receive: Receive, send: Send) -> None:
        body = b"legacy\n"
        await send(
            dict(
                type="http.response.start",
                status=200,
                headers=[(b"content-length", str(len(body)).encode("utf-8"))],
            )
        )
        await send(dict(type="http.response.body", body=body))

    return application


class Application:
    """
    The very same older shape written as a class, the building of the
    instance taking the scope and the calling of it the callables.
    """

    def __init__(self, scope: Scope) -> None:
        self.scope = scope

    async def __call__(self, receive: Receive, send: Send) -> None:
        body = b"class\n"
        await send(
            dict(
                type="http.response.start",
                status=200,
                headers=[(b"content-length", str(len(body)).encode("utf-8"))],
            )
        )
        await send(dict(type="http.response.body", body=body))


number = 42
""" A value that is not a callable at all, so that the refusal of one
may be driven against something the module is able to reach """


class Instance:
    """
    An application that is already an instance, the calling of it
    taking the pair of callables, which is the single shape of the
    more recent version written as an object.
    """

    async def __call__(self, scope: Scope, receive: Receive, send: Send) -> None:
        await send(dict(type="http.response.start", status=200, headers=[]))
        await send(dict(type="http.response.body", body=b"instance\n"))


instance = Instance()
""" The instance itself, which is what an application of that shape
is handed over as """


def marked_single(scope: Scope, receive: Receive, send: Send) -> None:
    # carries the marker of the adaptation helpers rather than the
    # shape, so that the reading of one ahead of any inspection is
    # what decides, the shape of it saying the opposite
    pass


marked_single._asgi_single_callable = True  # type: ignore[attr-defined]


async def marked_double(scope: Scope, receive: Receive, send: Send) -> None:
    # the same the other way around, a coroutine function that would
    # be read as the single shape and is marked as the double one
    pass


marked_double._asgi_double_callable = True  # type: ignore[attr-defined]
