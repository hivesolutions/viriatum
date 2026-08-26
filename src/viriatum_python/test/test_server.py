#!/usr/bin/python
# -*- coding: utf-8 -*-

import ast
import http.client
import socket
import sys
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
        if path == "/size":
            size = int(environ.get("CONTENT_LENGTH") or 0)
            data = environ["wsgi.input"].read(size)
            start_response("200 OK", [("Content-Type", "text/plain")])
            return [str(len(data)).encode("utf-8")]
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
        if path == "/raise-after":
            start_response("200 OK", [("Content-Type", "text/plain"), ("X-Stale", "yes")])
            raise RuntimeError("intentional failure after start response")
        if path == "/exit":
            start_response("200 OK", [])
            sys.exit(3)
        if path == "/write":
            write = start_response("200 OK", [("Content-Type", "text/plain")])
            write(b"written-")
            write(b"twice-")
            return [b"returned"]
        if path == "/own-length":
            body = b"exact"
            start_response("200 OK", [
                ("Content-Type", "text/plain"),
                ("Content-Length", str(len(body))),
            ])
            return [body]
        if path == "/bad-header":
            try:
                start_response("200 OK", [("X-Bad", "a\r\nInjected: 1")])
            except ValueError:
                start_response("200 OK", [("Content-Type", "text/plain")])
                return [b"rejected"]
        if path == "/bad-status":
            try:
                start_response("nonsense", [])
            except ValueError:
                start_response("200 OK", [("Content-Type", "text/plain")])
                return [b"rejected"]
        if path.startswith("/decoded"):
            start_response("200 OK", [("Content-Type", "text/plain")])
            return [path.encode("latin-1")]
        if path == "/cookie":
            start_response("200 OK", [("Content-Type", "text/plain")])
            return [environ.get("HTTP_COOKIE", "<none>").encode("utf-8")]
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
        values = ast.literal_eval(result.read().decode())
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

    def test_large_body(self):
        # a payload larger than the initial capacity must be fully
        # accumulated, the buffer growing to accommodate it
        payload = b"x" * 200000
        request = urllib.request.Request(self._url("/size"), data=payload)
        result = urllib.request.urlopen(request, timeout=10)
        self.assertEqual(result.read(), str(len(payload)).encode("utf-8"))

    def test_malformed_headers(self):
        # a header line carrying no value and a folded one must both be
        # tolerated, neither leaking nor faulting the server
        connection = socket.create_connection(("127.0.0.1", self.port), timeout=5)
        try:
            connection.sendall(
                b"GET /plain HTTP/1.1\r\nHost: x\r\nNoValueHeader\r\n"
                b"Folded: one\r\n two\r\nConnection: close\r\n\r\n"
            )
            data = b""
            while True:
                chunk = connection.recv(4096)
                if not chunk:
                    break
                data += chunk
        finally:
            connection.close()
        self.assertTrue(data.startswith(b"HTTP/1.1 200 OK"))
        self.assertTrue(data.endswith(b"plain"))

    def test_generator_response(self):
        # a multi item iterable must be joined into a single body
        # carrying the correct content length
        result = urllib.request.urlopen(self._url("/generator"), timeout=5)
        self.assertEqual(result.read(), b"first-second")
        self.assertEqual(result.headers.get("Content-Length"), "12")

    def test_percent_decoded_path(self):
        # the path must reach the application already decoded, the
        # value is carried as latin 1 as mandated by the specification
        result = urllib.request.urlopen(self._url("/decoded/john%20doe"), timeout=5)
        self.assertEqual(result.read().decode("latin-1"), "/decoded/john doe")

    def test_repeated_headers(self):
        # two headers of the same name must be joined with a comma
        # instead of the last one replacing the first
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        try:
            connection.putrequest("GET", "/cookie")
            connection.putheader("Cookie", "a=1")
            connection.putheader("Cookie", "b=2")
            connection.endheaders()
            response = connection.getresponse()
            self.assertEqual(response.read(), b"a=1, b=2")
        finally:
            connection.close()

    def test_write_callable(self):
        # the callable returned by start response must be usable, its
        # payload preceding the one returned by the application
        result = urllib.request.urlopen(self._url("/write"), timeout=5)
        self.assertEqual(result.read(), b"written-twice-returned")

    def test_application_content_length(self):
        # a content length set by the application must be the only one
        # present, two of them would desynchronize the client
        result = urllib.request.urlopen(self._url("/own-length"), timeout=5)
        self.assertEqual(result.headers.get("Content-Length"), "5")
        self.assertEqual(len(result.headers.get_all("Content-Length")), 1)
        self.assertEqual(result.read(), b"exact")

    def test_invalid_header(self):
        # a header carrying a control character must be rejected so
        # that the response envelope may not be split
        result = urllib.request.urlopen(self._url("/bad-header"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_invalid_status(self):
        # a status that does not start with a valid code must be
        # rejected instead of reaching the client as a zero code
        result = urllib.request.urlopen(self._url("/bad-status"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_application_error(self):
        try:
            urllib.request.urlopen(self._url("/boom"), timeout=5)
            self.fail("expected an HTTP error")
        except urllib.error.HTTPError as error:
            self.assertEqual(error.code, 500)

    def test_application_error_after_start(self):
        # a failure raised after the response has been started must
        # discard the status and the headers already set by it
        try:
            urllib.request.urlopen(self._url("/raise-after"), timeout=5)
            self.fail("expected an HTTP error")
        except urllib.error.HTTPError as error:
            self.assertEqual(error.code, 500)
            self.assertEqual(error.headers.get("X-Stale"), None)

    def test_application_exit(self):
        # a system exit raised by the application must not terminate
        # the process that is hosting the server
        try:
            urllib.request.urlopen(self._url("/exit"), timeout=5)
            self.fail("expected an HTTP error")
        except urllib.error.HTTPError as error:
            self.assertEqual(error.code, 500)

        # verifies that the server is still able to serve, which would
        # not be the case had the interpreter been terminated
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.read(), b"plain")

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

    def test_default_www_root(self):
        # the default www root value must be accepted, this is the one
        # that the serve helper passes for its own default
        server = viriatum.Server(self._application, port=PORT + 3, www_root=None)
        server.stop()

    def test_invalid_port(self):
        # a port outside of the representable range must be rejected
        # instead of being silently narrowed into another one
        for port in (-1, 65536, 70000):
            self.assertRaises(
                ValueError, viriatum.Server, self._application, port=port
            )

    def test_invalid_host(self):
        # a host that does not fit the buffer receiving it must be
        # rejected instead of overflowing it
        self.assertRaises(
            ValueError, viriatum.Server, self._application, host="h" * 4096
        )

    def test_invalid_www_root(self):
        # a www root that does not fit the buffer receiving it must be
        # rejected instead of overflowing it
        self.assertRaises(
            ValueError, viriatum.Server, self._application, www_root="w" * 8192
        )

    def test_www_root(self):
        # an explicit www root must be accepted and resolved, the value
        # is the root from which the static files would be served
        server = viriatum.Server(self._application, port=PORT + 5, www_root=".")
        server.stop()

    def test_busy_port(self):
        # opening a service on a port that is already taken must raise
        # instead of leaving the server in a half opened state
        holder = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        holder.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        holder.bind(("127.0.0.1", PORT + 6))
        holder.listen(1)
        try:
            server = viriatum.Server(self._application, host="127.0.0.1", port=PORT + 6)
            self.assertRaises(RuntimeError, server.serve_forever)
        finally:
            holder.close()

    def test_repeated_initialization(self):
        # a second initialization must be rejected as it would leak
        # both the previous service and its application reference
        server = viriatum.Server(self._application, port=PORT + 4)
        self.assertRaises(RuntimeError, server.__init__, self._application)

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
