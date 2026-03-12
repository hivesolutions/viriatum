# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Example WSGI handler for Viriatum Web Server.
# Accessible at /wsgi_demo when the wsgi handler is configured.

import sys
import platform

def application(environ, start_response):
    body = (
        "            _      _       _\n"
        " __   ___ _(_)_ __(_) __ _| |_ _   _ _ __ ___\n"
        " \\ \\ / / | '_| | |/ _` | __| | | | '_ ` _ \\\n"
        "  \\ V /| | | | | | (_| | |_| |_| | | | | | |\n"
        "   \\_/ |_|_| |_|_|\\__,_|\\__|\\__,_|_| |_| |_|\n"
        "\n"
        "  engine:   viriatum (wsgi handler)\n"
        "  runtime:  Python %s\n"
        "  platform: %s\n"
        "  method:   %s %s\n"
        "  status:   ok\n"
        % (
            platform.python_version(),
            platform.platform(),
            environ.get("REQUEST_METHOD", "?"),
            environ.get("PATH_INFO", "/"),
        )
    )
    body = body.encode("utf-8")
    start_response("200 OK", [
        ("Content-Type", "text/plain"),
        ("Content-Length", str(len(body)))
    ])
    return [body]
