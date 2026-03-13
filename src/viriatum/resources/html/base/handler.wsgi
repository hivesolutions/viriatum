# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Example WSGI handler for Viriatum Web Server.
# Accessible at /wsgi_demo when the wsgi handler is configured.

import platform
import viriatum_wsgi

def application(environ, start_response):
    body = (
        "            _      _       _\n"
        " __   ___ _(_)_ __(_) __ _| |_ _   _ _ __ ___\n"
        " \\ \\ / / | '_| | |/ _` | __| | | | '_ ` _ \\\n"
        "  \\ V /| | | | | | (_| | |_| |_| | | | | | |\n"
        "   \\_/ |_|_| |_|_|\\__,_|\\__|\\__,_|_| |_| |_|\n"
        "\n"
        "  engine:      %s (wsgi handler)\n"
        "  version:     %s\n"
        "  platform:    %s\n"
        "  description: %s\n"
        "\n"
        "  uptime:      %s\n"
        "  connections: %d\n"
        "  method:      %s %s\n"
        "\n"
        "  runtime:     Python %s\n"
        "  compiler:    %s / %s\n"
        "  compiled:    %s @ %s\n"
        "  flags:       %s\n"
        "  modules:     %s\n"
        % (
            viriatum_wsgi.NAME,
            viriatum_wsgi.VERSION,
            viriatum_wsgi.PLATFORM,
            viriatum_wsgi.DESCRIPTION,
            viriatum_wsgi.uptime(),
            viriatum_wsgi.connections(),
            environ.get("REQUEST_METHOD", "?"),
            environ.get("PATH_INFO", "/"),
            platform.python_version(),
            viriatum_wsgi.COMPILER,
            viriatum_wsgi.COMPILER_VERSION,
            viriatum_wsgi.COMPILATION_DATE,
            viriatum_wsgi.COMPILATION_TIME,
            viriatum_wsgi.COMPILATION_FLAGS,
            viriatum_wsgi.modules(),
        )
    )
    body = body.encode("utf-8")
    start_response("200 OK", [
        ("Content-Type", "text/plain"),
        ("Content-Length", str(len(body)))
    ])
    return [body]
