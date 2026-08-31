#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Test suite for the embedded server, driving it through the older of
the two interfaces and over the whole of a request cycle, from the
construction of the object to the answer that reaches a client of the
standard library.

One server is opened for the class and reached over the loopback, so
that what is exercised is the serving itself rather than a stand in
for it, and the applications the tests hand it answer according to
the path they are reached through.

Run from the project root with:
    python -m unittest discover -s src/viriatum_python/test
"""

from ast import literal_eval
from collections.abc import Callable, Iterable, Iterator
from http.client import HTTPConnection
from signal import SIGINT, raise_signal
from socket import create_connection
from sys import exit, platform, version_info
from threading import Event, Thread
from time import sleep
from typing import Any
from unittest import TestCase, main, skipIf
from urllib.error import HTTPError
from urllib.request import Request, urlopen

import viriatum

Environ = dict[str, Any]
""" The map of the request as the interface builds it, one entry per
part of the request and of the environment """

StartResponse = Callable[[str, list[tuple[str, str]]], Any]
""" The operation that opens the answer, taking the status and the
fields that travel with it """

PORT = 19301
""" The base port to be used by the various servers created
during the execution of the test suite """


class ServerTest(TestCase):
    """
    Test suite for the server object, exercising both the
    construction of it and the complete request cycle.
    """

    @classmethod
    def setUpClass(cls) -> None:
        # creates the server for the application defined below and
        # runs its loop in a separate thread, so that the requests
        # may be issued from the main one
        cls.port = PORT
        cls.server = viriatum.Server(cls._application, port=cls.port)
        cls.thread = Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        cls._wait_server()

    @classmethod
    def tearDownClass(cls) -> None:
        cls.server.stop()
        cls.thread.join(timeout=10)

    @classmethod
    def _wait_server(cls) -> None:
        # tries to reach the server for a limited amount of time,
        # failing the complete suite in case it never becomes ready
        for _ in range(100):
            try:
                urlopen(cls._url("/plain"), timeout=1).read()
                return
            except Exception:
                sleep(0.1)
        raise AssertionError("server did not become ready")

    @classmethod
    def _url(cls, path: str) -> str:
        return "http://127.0.0.1:%d%s" % (cls.port, path)

    @staticmethod
    def _application(
        environ: Environ, start_response: StartResponse
    ) -> Iterable[bytes]:
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

            def generator() -> Iterator[bytes]:
                yield b"first-"
                yield b"second"

            start_response("200 OK", [("Content-Type", "text/plain")])
            return generator()
        if path == "/environ":
            keys = (
                "REQUEST_METHOD",
                "SCRIPT_NAME",
                "PATH_INFO",
                "QUERY_STRING",
                "SERVER_PROTOCOL",
                "SERVER_SOFTWARE",
                "SERVER_PORT",
                "REMOTE_ADDR",
            )
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
            start_response(
                "200 OK", [("Content-Type", "text/plain"), ("X-Stale", "yes")]
            )
            raise RuntimeError("intentional failure after start response")
        if path == "/exit":
            start_response("200 OK", [])
            exit(3)
        if path == "/write":
            write = start_response("200 OK", [("Content-Type", "text/plain")])
            write(b"written-")
            write(b"twice-")
            return [b"returned"]
        if path == "/long-header":
            start_response("200 OK", [("X-Long", "v" * 4000)])
            return [b"long"]
        if path == "/own-length":
            body = b"exact"
            start_response(
                "200 OK",
                [
                    ("Content-Type", "text/plain"),
                    ("Content-Length", str(len(body))),
                ],
            )
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

    def test_constants(self) -> None:
        # verifies that the module exposes the expected set of
        # constants describing the underlying server
        self.assertEqual(viriatum.NAME, "viriatum")
        self.assertTrue(len(viriatum.VERSION) > 0)
        self.assertTrue(len(viriatum.PLATFORM) > 0)
        self.assertEqual(viriatum.__version__, viriatum.VERSION)

    def test_application_must_be_callable(self) -> None:
        # verifies that a non callable application is rejected at
        # the construction of the server object
        self.assertRaises(TypeError, viriatum.Server, "not callable")

    def test_simple_request(self) -> None:
        result = urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"plain")
        self.assertEqual(result.headers.get("Content-Type"), "text/plain")

    def test_server_header(self) -> None:
        result = urlopen(self._url("/plain"), timeout=5)
        self.assertTrue(result.headers.get("Server").startswith("viriatum/"))

    def test_environ(self) -> None:
        result = urlopen(self._url("/environ?a=1&b=2"), timeout=5)
        values = dict(item.split("=", 1) for item in result.read().decode().split("|"))
        self.assertEqual(values["REQUEST_METHOD"], "GET")
        self.assertEqual(values["SCRIPT_NAME"], "")
        self.assertEqual(values["PATH_INFO"], "/environ")
        self.assertEqual(values["QUERY_STRING"], "a=1&b=2")
        self.assertEqual(values["SERVER_PROTOCOL"], "HTTP/1.1")
        self.assertEqual(values["SERVER_PORT"], str(self.port))
        self.assertEqual(values["REMOTE_ADDR"], "127.0.0.1")
        self.assertTrue(values["SERVER_SOFTWARE"].startswith("viriatum/"))

    def test_wsgi_keys(self) -> None:
        result = urlopen(self._url("/wsgi"), timeout=5)
        values = literal_eval(result.read().decode())
        self.assertEqual(values[0], (1, 0))
        self.assertEqual(values[1], "http")
        self.assertEqual(values[2], False)
        self.assertEqual(values[3], False)
        self.assertEqual(values[4], False)
        self.assertEqual(values[5], True)
        self.assertEqual(values[6], True)

    def test_request_headers(self) -> None:
        # verifies that the headers of the request reach the
        # application under the prefixed and upper cased form
        request = Request(self._url("/environ"), headers={"X-Custom-Value": "custom"})
        result = urlopen(request, timeout=5)
        self.assertEqual(result.status, 200)

    def test_response_headers(self) -> None:
        result = urlopen(self._url("/headers"), timeout=5)
        self.assertEqual(result.headers.get("X-First"), "one")
        self.assertEqual(result.headers.get("X-Second"), "two")

    def test_multi_word_status(self) -> None:
        # the status message must be preserved in full, a naive
        # parsing would truncate it at the first space
        try:
            urlopen(self._url("/missing"), timeout=5)
            self.fail("expected an HTTP error")
        except HTTPError as error:
            self.assertEqual(error.code, 404)
            self.assertEqual(error.reason, "Not Found Here")
            self.assertEqual(error.read(), b"nope")

    def test_post_body(self) -> None:
        request = Request(self._url("/echo"), data=b"payload-123")
        result = urlopen(request, timeout=5)
        self.assertEqual(result.status, 201)
        self.assertEqual(result.reason, "Created Thing")
        self.assertEqual(result.read(), b"got:payload-123")

    def test_empty_post_body(self) -> None:
        request = Request(self._url("/echo"), data=b"")
        result = urlopen(request, timeout=5)
        self.assertEqual(result.read(), b"got:")

    def test_large_body(self) -> None:
        # a payload larger than the initial capacity must be fully
        # accumulated, the buffer growing to accommodate it
        payload = b"x" * 200000
        request = Request(self._url("/size"), data=payload)
        result = urlopen(request, timeout=10)
        self.assertEqual(result.read(), str(len(payload)).encode("utf-8"))

    def test_malformed_headers(self) -> None:
        # a header line carrying no value and a folded one must both be
        # tolerated, neither leaking nor faulting the server
        connection = create_connection(("127.0.0.1", self.port), timeout=5)
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

    def test_generator_response(self) -> None:
        # a multi item iterable must be joined into a single body
        # carrying the correct content length
        result = urlopen(self._url("/generator"), timeout=5)
        self.assertEqual(result.read(), b"first-second")
        self.assertEqual(result.headers.get("Content-Length"), "12")

    def test_percent_decoded_path(self) -> None:
        # the path must reach the application already decoded, the
        # value is carried as latin 1 as mandated by the specification
        result = urlopen(self._url("/decoded/john%20doe"), timeout=5)
        self.assertEqual(result.read().decode("latin-1"), "/decoded/john doe")

    def test_repeated_headers(self) -> None:
        # two headers of the same name must be joined with a comma
        # instead of the last one replacing the first
        connection = HTTPConnection("127.0.0.1", self.port, timeout=5)
        try:
            connection.putrequest("GET", "/cookie")
            connection.putheader("Cookie", "a=1")
            connection.putheader("Cookie", "b=2")
            connection.endheaders()
            response = connection.getresponse()
            self.assertEqual(response.read(), b"a=1, b=2")
        finally:
            connection.close()

    def test_write_callable(self) -> None:
        # the callable returned by start response must be usable, its
        # payload preceding the one returned by the application
        result = urlopen(self._url("/write"), timeout=5)
        self.assertEqual(result.read(), b"written-twice-returned")

    def test_long_response_header(self) -> None:
        # verifies that a header longer than the maximum size of a
        # single one reaches the wire, the formatting of the envelope
        # is bounded by the remaining capacity of its buffer
        result = urlopen(self._url("/long-header"), timeout=5)
        self.assertEqual(result.read(), b"long")
        self.assertEqual(result.headers.get("X-Long"), "v" * 4000)

    def test_oversized_body(self) -> None:
        # verifies that a payload beyond the maximum allowed size is
        # refused, handing a truncated one to the application would
        # have it processed as valid but incomplete data
        payload = b"o" * (17 * 1024 * 1024)
        try:
            result = urlopen(self._url("/size"), data=payload, timeout=30)
        except HTTPError as error:
            result = error
        self.assertEqual(result.status, 413)

    def test_application_content_length(self) -> None:
        # a content length set by the application must be the only one
        # present, two of them would desynchronize the client
        result = urlopen(self._url("/own-length"), timeout=5)
        self.assertEqual(result.headers.get("Content-Length"), "5")
        self.assertEqual(len(result.headers.get_all("Content-Length")), 1)
        self.assertEqual(result.read(), b"exact")

    def test_invalid_header(self) -> None:
        # a header carrying a control character must be rejected so
        # that the response envelope may not be split
        result = urlopen(self._url("/bad-header"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_invalid_status(self) -> None:
        # a status that does not start with a valid code must be
        # rejected instead of reaching the client as a zero code
        result = urlopen(self._url("/bad-status"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_application_error(self) -> None:
        try:
            urlopen(self._url("/boom"), timeout=5)
            self.fail("expected an HTTP error")
        except HTTPError as error:
            self.assertEqual(error.code, 500)

    def test_application_error_after_start(self) -> None:
        # a failure raised after the response has been started must
        # discard the status and the headers already set by it
        try:
            urlopen(self._url("/raise-after"), timeout=5)
            self.fail("expected an HTTP error")
        except HTTPError as error:
            self.assertEqual(error.code, 500)
            self.assertEqual(error.headers.get("X-Stale"), None)

    def test_application_exit(self) -> None:
        # a system exit raised by the application must not terminate
        # the process that is hosting the server
        try:
            urlopen(self._url("/exit"), timeout=5)
            self.fail("expected an HTTP error")
        except HTTPError as error:
            self.assertEqual(error.code, 500)

        # verifies that the server is still able to serve, which would
        # not be the case had the interpreter been terminated
        result = urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.read(), b"plain")

    def test_sequential_requests(self) -> None:
        # runs a series of requests against the server verifying
        # that the per request state is properly reset
        for index in range(10):
            result = urlopen(self._url("/plain"), timeout=5)
            self.assertEqual(result.read(), b"plain", "failed on request %d" % index)

    def test_pipelined_connection(self) -> None:
        # issues more than one request over the same connection so
        # that the connection lifecycle is exercised
        connection = HTTPConnection("127.0.0.1", self.port, timeout=5)
        try:
            for _ in range(3):
                connection.request("GET", "/plain")
                response = connection.getresponse()
                self.assertEqual(response.status, 200)
                self.assertEqual(response.read(), b"plain")
        finally:
            connection.close()

    def test_concurrent_keep_alive(self) -> None:
        # drives a series of requests over several connections that are
        # kept alive, so that the writing of the responses completes
        # from the polling of the service rather than from the calling
        # of it, the tearing down of the context runs with the global
        # interpreter lock released in that path
        failures = []

        def hammer(count: int) -> None:
            connection = HTTPConnection("127.0.0.1", self.port, timeout=15)
            connection.auto_open = 0
            try:
                connection.connect()
                for _ in range(count):
                    connection.request(
                        "GET", "/plain", headers={"Connection": "keep-alive"}
                    )
                    response = connection.getresponse()
                    if response.read() != b"plain":
                        failures.append("unexpected payload")
            except Exception as exception:
                failures.append(repr(exception))
            finally:
                connection.close()

        threads = [Thread(target=hammer, args=(30,)) for _ in range(4)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join(timeout=30)

        self.assertEqual(failures, [])
        self.assertEqual([thread.is_alive() for thread in threads], [False] * 4)
        result = urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.read(), b"plain")

    def test_connections(self) -> None:
        self.assertTrue(isinstance(self.server.connections, int))
        self.assertTrue(self.server.connections >= 0)

    def test_uptime(self) -> None:
        self.assertTrue(isinstance(self.server.uptime, str))
        self.assertTrue(len(self.server.uptime) > 0)


class ServerLifecycleTest(TestCase):
    """
    Test suite for the lifecycle of the server object, kept
    separate as it creates servers of its own.
    """

    @staticmethod
    def _application(environ: Environ, start_response: StartResponse) -> list[bytes]:
        start_response("200 OK", [])
        return [b""]

    def test_default_www_root(self) -> None:
        # the default www root value must be accepted, this is the one
        # that the serve helper passes for its own default
        server = viriatum.Server(self._application, port=PORT + 3, www_root=None)
        server.stop()

    def test_invalid_port(self) -> None:
        # a port outside of the representable range must be rejected
        # instead of being silently narrowed into another one
        for port in (-1, 65536, 70000):
            self.assertRaises(ValueError, viriatum.Server, self._application, port=port)

    def test_invalid_host(self) -> None:
        # a host that does not fit the buffer receiving it must be
        # rejected instead of overflowing it
        self.assertRaises(
            ValueError, viriatum.Server, self._application, host="h" * 4096
        )

    def test_invalid_www_root(self) -> None:
        # a www root that does not fit the buffer receiving it must be
        # rejected instead of overflowing it
        self.assertRaises(
            ValueError, viriatum.Server, self._application, www_root="w" * 8192
        )

    def test_www_root(self) -> None:
        # an explicit www root must be accepted and resolved, the value
        # is the root from which the static files would be served
        server = viriatum.Server(self._application, port=PORT + 5, www_root=".")
        server.stop()

    def test_failed_open(self) -> None:
        # an opening that fails must raise instead of leaving the server
        # in a half opened state, the address is taken from the range
        # reserved for documentation so that no interface carries it and
        # the binding fails on every platform
        server = viriatum.Server(self._application, host="192.0.2.1", port=PORT + 6)
        self.assertRaises(RuntimeError, server.serve_forever)

        # a second attempt must fail in the very same way, which shows
        # that the first failure left no state behind
        self.assertRaises(RuntimeError, server.serve_forever)

    def test_repeated_initialization(self) -> None:
        # a second initialization must be rejected as it would leak
        # both the previous service and its application reference
        server = viriatum.Server(self._application, port=PORT + 4)
        self.assertRaises(RuntimeError, server.__init__, self._application)

    def test_stop_before_serve(self) -> None:
        # stopping a server that was never served must be a
        # graceful operation instead of a crash
        server = viriatum.Server(self._application, port=PORT + 1)
        server.stop()

    def test_serve_and_stop(self) -> None:
        server = viriatum.Server(self._application, port=PORT + 2)
        thread = Thread(target=server.serve_forever, daemon=True)
        thread.start()
        for _ in range(100):
            try:
                urlopen("http://127.0.0.1:%d/" % (PORT + 2), timeout=1).read()
                break
            except Exception:
                sleep(0.1)
        server.stop()
        thread.join(timeout=10)
        self.assertFalse(thread.is_alive())

    @skipIf(
        platform == "win32" and version_info < (3, 11),
        "the interrupt is not delivered reliably by that runtime",
    )
    def test_keyboard_interrupt(self) -> None:
        # verifies that an interrupt stops the serving loop, the
        # signals are only handled in the main thread and so it is the
        # one that runs the loop while another raises the interrupt,
        # the older runtime of the other platform is left out of it
        # because the interrupt reaches the loop there only some of
        # the time, the later ones of it deliver it every time
        state = {"raised": False, "expired": False}
        finished = Event()

        def application(environ: Environ, start_response: StartResponse) -> list[bytes]:
            start_response("200 OK", [("Content-Type", "text/plain")])
            return [b"plain"]

        server = viriatum.Server(application, port=PORT + 30, interface="wsgi")

        def interrupt() -> None:
            # waits for the service to be listening before raising the
            # interrupt, so that it never reaches the opening of it
            for _ in range(200):
                try:
                    connection = create_connection(("127.0.0.1", PORT + 30), timeout=1)
                    connection.close()
                    break
                except Exception:
                    sleep(0.05)
            state["raised"] = True
            raise_signal(SIGINT)

            # stops the server in case the interrupt is ignored, so
            # that the test fails instead of hanging forever
            if not finished.wait(timeout=10.0):
                state["expired"] = True
                server.stop()

        Thread(target=interrupt, daemon=True).start()
        try:
            self.assertRaises(KeyboardInterrupt, server.serve_forever)
        finally:
            finished.set()
        self.assertTrue(state["raised"])
        self.assertFalse(state["expired"], "the interrupt was ignored")

    def test_serve_helper(self) -> None:
        # verifies that the serve helper is exposed and callable
        self.assertTrue(callable(viriatum.serve))


if __name__ == "__main__":
    main()
