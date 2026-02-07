# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Example WSGI handler for Viriatum Web Server.
# Accessible at /wsgi_demo when the wsgi handler is configured.

def application(environ, start_response):
    body = b"Hello World from Viriatum WSGI!\n"
    start_response("200 OK", [
        ("Content-Type", "text/plain"),
        ("Content-Length", str(len(body)))
    ])
    return [body]
