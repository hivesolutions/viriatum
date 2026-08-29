#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
The application that every one of the python interfaces is driven
against, the very same object being served by the server and by the
reference that stands for its interface, so that what ends up being
measured is the interface and never the application behind it.
"""

BODY = b"viriatum"
""" The body that the plain applications answer with, a small and
fixed one so that the cost of producing it never shows up in the
figure of the interface that carried it """

CHUNKS = 8
""" The number of parts the streaming application breaks its body
into, enough of them that the sending of a part is exercised more
than once per request """

HEADERS = [("Content-Type", "text/plain"), ("Content-Length", str(len(BODY)))]
""" The fields of the response, the length is always among them so
that no interface is forced into a chunked framing the other one is
not paying the cost of """


def wsgi_app(environ, start_response):
    """
    The application of the older of the two interfaces, answering
    with a fixed body and the smallest set of fields that a complete
    response is able to carry.

    :type environ: Dictionary
    :param environ: The map of the request as the interface builds it.
    :type start_response: Callable
    :param start_response: The operation that starts the response.
    :rtype: List
    :return: The sequence of the parts of the body.
    """

    start_response("200 OK", HEADERS)
    return [BODY]


async def asgi_app(scope, receive, send):
    """
    The application of the more recent of the two interfaces, the
    counterpart of the one above and answering exactly the same way.

    :type scope: Dictionary
    :param scope: The map of the request as the interface builds it.
    :type receive: Callable
    :param receive: The operation that reads an event of the client.
    :type send: Callable
    :param send: The operation that writes an event of the response.
    """

    await send(
        {
            "type": "http.response.start",
            "status": 200,
            "headers": [
                (name.lower().encode("utf-8"), value.encode("utf-8"))
                for name, value in HEADERS
            ],
        }
    )
    await send({"type": "http.response.body", "body": BODY, "more_body": False})


async def asgi_stream_app(scope, receive, send):
    """
    The streaming counterpart of the application above, sending the
    body in several parts instead of a single one, which is the shape
    an application that produces its answer as it goes ends up taking.

    :type scope: Dictionary
    :param scope: The map of the request as the interface builds it.
    :type receive: Callable
    :param receive: The operation that reads an event of the client.
    :type send: Callable
    :param send: The operation that writes an event of the response.
    """

    await send(
        {
            "type": "http.response.start",
            "status": 200,
            "headers": [
                (b"content-type", b"text/plain"),
                (b"content-length", str(len(BODY) * CHUNKS).encode("utf-8")),
            ],
        }
    )
    for index in range(CHUNKS):
        await send(
            {
                "type": "http.response.body",
                "body": BODY,
                "more_body": index < CHUNKS - 1,
            }
        )
