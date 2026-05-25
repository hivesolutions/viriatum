# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Minimal WSGI handler for mod_wsgi test suite.
# Returns a simple text response with predictable content
# so that the C test harness can verify the output.


def application(environ, start_response):
    method = environ.get("REQUEST_METHOD", "GET")
    path = environ.get("PATH_INFO", "/")

    body = ("method=%s\npath=%s\n" % (method, path)).encode("utf-8")

    headers = [
        ("Content-Type", "text/plain"),
        ("Content-Length", str(len(body))),
        ("X-Test", "viriatum")
    ]

    start_response("200 OK", headers)
    return [body]
