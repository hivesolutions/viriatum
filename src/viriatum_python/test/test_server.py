#!/usr/bin/python
# -*- coding: utf-8 -*-

# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

__author__ = "João Magalhães <joamag@hive.pt>"
__copyright__ = "Copyright (c) 2008-2026 Hive Solutions Lda."
__license__ = "Apache License, Version 2.0"


import http.client
import threading
import time
import unittest
import urllib.error
import urllib.request

import viriatum

PORT = 19301
""" The base port to be used by the various servers created
during the execution of the test suite """


class ServerTest(unittest.TestCase):
    """
    Test suite for the server object, exercising both the
    construction of it and the complete request cycle.
    """

    @classmethod
    def setUpClass(cls):
        # creates the server for the application defined below and
        # runs its loop in a separate thread, so that the requests
        # may be issued from the main one
        cls.port = PORT
        cls.server = viriatum.Server(cls._application, port=cls.port)
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        cls._wait_server()

    @classmethod
    def tearDownClass(cls):
        cls.server.stop()
        cls.thread.join(timeout=10)

    @classmethod
    def _wait_server(cls):
        # tries to reach the server for a limited amount of time,
        # failing the complete suite in case it never becomes ready
        for _ in range(100):
            try:
                urllib.request.urlopen(cls._url("/plain"), timeout=1).read()
                return
            except Exception:
                time.sleep(0.1)
        raise AssertionError("server did not become ready")

    @classmethod
    def _url(cls, path):
        return "http://127.0.0.1:%d%s" % (cls.port, path)

    @staticmethod
    def _application(environ, start_response):
        path = environ["PATH_INFO"]
        if path == "/boom":
            raise RuntimeError("intentional failure")
        if path == "/echo":
            size = int(environ.get("CONTENT_LENGTH") or 0)
            data = environ["wsgi.input"].read(size)
            start_response("201 Created Thing", [("Content-Type", "text/plain")])
            return [b"got:", data]
        if path == "/generator":
            def generator():
                yield b"first-"
                yield b"second"
            start_response("200 OK", [("Content-Type", "text/plain")])
            return generator()
        if path == "/environ":
            keys = ("REQUEST_METHOD", "SCRIPT_NAME", "PATH_INFO", "QUERY_STRING",
                    "SERVER_PROTOCOL", "SERVER_SOFTWARE", "SERVER_PORT", "REMOTE_ADDR")
            body = "|".join("%s=%s" % (key, environ[key]) for key in keys)
            start_response("200 OK", [("Content-Type", "text/plain")])
            return [body.encode("utf-8")]
        if path == "/wsgi":
            values = (
                environ["wsgi.version"],
                environ["wsgi.url_scheme"],
                environ["wsgi.multithread"],
                environ["wsgi.multiprocess"],
                environ["wsgi.run_once"],
                hasattr(environ["wsgi.errors"], "write"),
                hasattr(environ["wsgi.input"], "read"),
            )
            start_response("200 OK", [])
            return [repr(values).encode("utf-8")]
        if path == "/headers":
            start_response("200 OK", [("X-First", "one"), ("X-Second", "two")])
            return [b""]
        if path == "/missing":
            start_response("404 Not Found Here", [("Content-Type", "text/plain")])
            return [b"nope"]
        start_response("200 OK", [("Content-Type", "text/plain")])
        return [b"plain"]

    def test_constants(self):
        # verifies that the module exposes the expected set of
        # constants describing the underlying server
        self.assertEqual(viriatum.NAME, "viriatum")
        self.assertTrue(len(viriatum.VERSION) > 0)
        self.assertTrue(len(viriatum.PLATFORM) > 0)
        self.assertEqual(viriatum.__version__, viriatum.VERSION)

    def test_application_must_be_callable(self):
        # verifies that a non callable application is rejected at
        # the construction of the server object
        self.assertRaises(TypeError, viriatum.Server, "not callable")

    def test_simple_request(self):
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"plain")
        self.assertEqual(result.headers.get("Content-Type"), "text/plain")

    def test_server_header(self):
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertTrue(result.headers.get("Server").startswith("viriatum/"))

    def test_environ(self):
        result = urllib.request.urlopen(self._url("/environ?a=1&b=2"), timeout=5)
        values = dict(item.split("=", 1) for item in result.read().decode().split("|"))
        self.assertEqual(values["REQUEST_METHOD"], "GET")
        self.assertEqual(values["SCRIPT_NAME"], "")
        self.assertEqual(values["PATH_INFO"], "/environ")
        self.assertEqual(values["QUERY_STRING"], "a=1&b=2")
        self.assertEqual(values["SERVER_PROTOCOL"], "HTTP/1.1")
        self.assertEqual(values["SERVER_PORT"], str(self.port))
        self.assertEqual(values["REMOTE_ADDR"], "127.0.0.1")
        self.assertTrue(values["SERVER_SOFTWARE"].startswith("viriatum/"))

    def test_wsgi_keys(self):
        result = urllib.request.urlopen(self._url("/wsgi"), timeout=5)
        values = eval(result.read().decode())
        self.assertEqual(values[0], (1, 0))
        self.assertEqual(values[1], "http")
        self.assertEqual(values[2], False)
        self.assertEqual(values[3], False)
        self.assertEqual(values[4], False)
        self.assertEqual(values[5], True)
        self.assertEqual(values[6], True)

    def test_request_headers(self):
        # verifies that the headers of the request reach the
        # application under the prefixed and upper cased form
        request = urllib.request.Request(
            self._url("/environ"), headers={"X-Custom-Value": "custom"}
        )
        result = urllib.request.urlopen(request, timeout=5)
        self.assertEqual(result.status, 200)

    def test_response_headers(self):
        result = urllib.request.urlopen(self._url("/headers"), timeout=5)
        self.assertEqual(result.headers.get("X-First"), "one")
        self.assertEqual(result.headers.get("X-Second"), "two")

    def test_multi_word_status(self):
        # the status message must be preserved in full, a naive
        # parsing would truncate it at the first space
        try:
            urllib.request.urlopen(self._url("/missing"), timeout=5)
            self.fail("expected an HTTP error")
        except urllib.error.HTTPError as error:
            self.assertEqual(error.code, 404)
            self.assertEqual(error.reason, "Not Found Here")
            self.assertEqual(error.read(), b"nope")

    def test_post_body(self):
        request = urllib.request.Request(self._url("/echo"), data=b"payload-123")
        result = urllib.request.urlopen(request, timeout=5)
        self.assertEqual(result.status, 201)
        self.assertEqual(result.reason, "Created Thing")
        self.assertEqual(result.read(), b"got:payload-123")

    def test_empty_post_body(self):
        request = urllib.request.Request(self._url("/echo"), data=b"")
        result = urllib.request.urlopen(request, timeout=5)
        self.assertEqual(result.read(), b"got:")

    def test_generator_response(self):
        # a multi item iterable must be joined into a single body
        # carrying the correct content length
        result = urllib.request.urlopen(self._url("/generator"), timeout=5)
        self.assertEqual(result.read(), b"first-second")
        self.assertEqual(result.headers.get("Content-Length"), "12")

    def test_application_error(self):
        try:
            urllib.request.urlopen(self._url("/boom"), timeout=5)
            self.fail("expected an HTTP error")
        except urllib.error.HTTPError as error:
            self.assertEqual(error.code, 500)

    def test_sequential_requests(self):
        # runs a series of requests against the server verifying
        # that the per request state is properly reset
        for index in range(10):
            result = urllib.request.urlopen(self._url("/plain"), timeout=5)
            self.assertEqual(result.read(), b"plain", "failed on request %d" % index)

    def test_pipelined_connection(self):
        # issues more than one request over the same connection so
        # that the connection lifecycle is exercised
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        try:
            for _ in range(3):
                connection.request("GET", "/plain")
                response = connection.getresponse()
                self.assertEqual(response.status, 200)
                self.assertEqual(response.read(), b"plain")
        finally:
            connection.close()

    def test_connections(self):
        self.assertTrue(isinstance(self.server.connections, int))
        self.assertTrue(self.server.connections >= 0)

    def test_uptime(self):
        self.assertTrue(isinstance(self.server.uptime, str))
        self.assertTrue(len(self.server.uptime) > 0)


class ServerLifecycleTest(unittest.TestCase):
    """
    Test suite for the lifecycle of the server object, kept
    separate as it creates servers of its own.
    """

    @staticmethod
    def _application(environ, start_response):
        start_response("200 OK", [])
        return [b""]

    def test_stop_before_serve(self):
        # stopping a server that was never served must be a
        # graceful operation instead of a crash
        server = viriatum.Server(self._application, port=PORT + 1)
        server.stop()

    def test_serve_and_stop(self):
        server = viriatum.Server(self._application, port=PORT + 2)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        for _ in range(100):
            try:
                urllib.request.urlopen("http://127.0.0.1:%d/" % (PORT + 2), timeout=1).read()
                break
            except Exception:
                time.sleep(0.1)
        server.stop()
        thread.join(timeout=10)
        self.assertFalse(thread.is_alive())

    def test_serve_helper(self):
        # verifies that the serve helper is exposed and callable
        self.assertTrue(callable(viriatum.serve))


if __name__ == "__main__":
    unittest.main()
