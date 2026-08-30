#!/usr/bin/python
# -*- coding: utf-8 -*-

import ast
import asyncio
import base64
import gc
import hashlib
import os
import shutil
import signal
import socket
import struct
import subprocess
import sys
import threading
import time
import unittest

import http.client
import urllib.error
import urllib.request

import viriatum

PORT = 19401
""" The base port to be used by the various servers created
during the execution of the test suite """

GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
""" The magic value that is concatenated with the key of the
handshake in order to produce the accept value of it """


class WebSocketClient(object):
    """
    Minimal websocket client, it performs the handshake and both
    frames and unframes the payloads according to the protocol.
    """

    def __init__(self, port, path="/ws", protocols=None):
        self.socket = socket.create_connection(("127.0.0.1", port), timeout=5)
        self.key = base64.b64encode(os.urandom(16)).decode("ascii")
        request = (
            "GET %s HTTP/1.1\r\n"
            "Host: 127.0.0.1\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: %s\r\n"
            "Sec-WebSocket-Version: 13\r\n"
        ) % (path, self.key)
        if protocols is not None:
            request += "Sec-WebSocket-Protocol: %s\r\n" % protocols
        self.socket.sendall((request + "\r\n").encode("ascii"))
        self.buffer = b""
        self.response = b""
        while b"\r\n\r\n" not in self.response:
            data = self.socket.recv(1024)
            if not data:
                break
            self.response += data
        self.response, _, self.buffer = self.response.partition(b"\r\n\r\n")

    @property
    def status(self):
        return int(self.response.split(b" ")[1])

    @property
    def accept(self):
        digest = hashlib.sha1((self.key + GUID).encode("ascii")).digest()
        return base64.b64encode(digest).decode("ascii")

    def header(self, name):
        for line in self.response.split(b"\r\n"):
            if line.lower().startswith(name.lower().encode("ascii") + b":"):
                return line.split(b":", 1)[1].strip().decode("ascii")
        return None

    def send(self, opcode, payload, mask=b"\x01\x02\x03\x04", fin=True):
        size = len(payload)
        header = bytes([(0x80 if fin else 0x00) | opcode])
        if size < 126:
            header += bytes([0x80 | size])
        elif size < 65536:
            header += bytes([0x80 | 126]) + struct.pack("!H", size)
        else:
            header += bytes([0x80 | 127]) + struct.pack("!Q", size)
        masked = bytes(payload[index] ^ mask[index % 4] for index in range(size))
        self.socket.sendall(header + mask + masked)

    def send_raw(self, data):
        self.socket.sendall(data)

    def receive(self):
        header = self._read(2)
        if header is None:
            return None, None
        opcode = header[0] & 0x0F
        size = header[1] & 0x7F
        if size == 126:
            size = struct.unpack("!H", self._read(2))[0]
        elif size == 127:
            size = struct.unpack("!Q", self._read(8))[0]
        return opcode, self._read(size) if size > 0 else b""

    def close(self):
        self.socket.close()

    def _read(self, size):
        data = self.buffer[:size]
        self.buffer = self.buffer[size:]
        while len(data) < size:
            chunk = self.socket.recv(size - len(data))
            if not chunk:
                return None
            data += chunk
        return data


class ServerCase(unittest.TestCase):
    """
    Base of the various suites, it owns the application that is
    served and the lifecycle of the server running it.
    """

    OFFSET = 0
    """ The offset that is added to the base port, so that each of
    the suites binds a port of its own """

    @classmethod
    def setUpClass(cls):
        # creates the server for the application defined below and
        # runs its loop in a separate thread, so that the requests
        # may be issued from the main one
        cls.port = PORT + cls.OFFSET
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
    async def _websocket(scope, receive, send):
        path = scope["path"]
        message = await receive()
        if message["type"] != "websocket.connect":
            raise RuntimeError("unexpected message %s" % message["type"])
        if path == "/ws-reject":
            await send({"type": "websocket.close", "code": 4000})
            return
        if path == "/ws-silent":
            return
        if path == "/ws-not-bytes":
            await send({"type": "websocket.accept"})
            try:
                await send({"type": "websocket.send", "bytes": "text"})
            except TypeError:
                await send({"type": "websocket.send", "text": "rejected"})
            return
        if path == "/ws-headers":
            await send(
                {
                    "type": "websocket.accept",
                    "headers": [(b"set-cookie", b"session=1"), (b"x-extra", b"yes")],
                }
            )
            return
        if path == "/ws-bad-header":
            try:
                await send(
                    {
                        "type": "websocket.accept",
                        "headers": [(b"x-bad", b"a\r\nInjected: 1")],
                    }
                )
            except ValueError:
                await send({"type": "websocket.accept"})
                await send({"type": "websocket.send", "text": "rejected"})
            return
        if path == "/ws-return":
            await send({"type": "websocket.accept"})
            return
        if path == "/ws-refused":
            failures = []
            try:
                await send({"type": "websocket.send", "text": "early"})
            except RuntimeError:
                failures.append("send-before-accept")
            try:
                await send({"type": "websocket.nonsense"})
            except ValueError:
                failures.append("unknown-message")
            try:
                await send({"type": "websocket.accept", "subprotocol": "a\r\nb"})
            except ValueError:
                failures.append("control-subprotocol")
            try:
                await send({"type": "websocket.accept", "subprotocol": "a" * 200})
            except ValueError:
                failures.append("long-subprotocol")
            await send({"type": "websocket.accept"})
            try:
                await send({"type": "websocket.accept"})
            except RuntimeError:
                failures.append("double-accept")
            await send({"type": "websocket.send", "text": ",".join(failures)})
            return
        if path == "/ws-scope":
            await send({"type": "websocket.accept"})
            await send(
                {
                    "type": "websocket.send",
                    "text": repr(
                        (
                            scope["type"],
                            scope["scheme"],
                            scope["path"],
                            scope["query_string"],
                            scope["subprotocols"],
                        )
                    ),
                }
            )
        elif path == "/ws-protocol":
            await send({"type": "websocket.accept", "subprotocol": "chat"})
        else:
            await send({"type": "websocket.accept"})
        while True:
            message = await receive()
            if message["type"] == "websocket.disconnect":
                return
            if path == "/ws-close":
                await send({"type": "websocket.close", "code": 4001, "reason": "done"})
                return
            if message.get("text") is not None:
                await send(
                    {"type": "websocket.send", "text": "echo:" + message["text"]}
                )
            else:
                await send(
                    {"type": "websocket.send", "bytes": b"bin:" + message["bytes"]}
                )

    @staticmethod
    async def _application(scope, receive, send):
        if scope["type"] == "websocket":
            await AsgiTest._websocket(scope, receive, send)
            return

        path = scope["path"]
        if path == "/boom":
            raise RuntimeError("intentional failure")
        if path == "/echo":
            message = await receive()
            await send(
                {
                    "type": "http.response.start",
                    "status": 201,
                    "headers": [(b"content-type", b"text/plain")],
                }
            )
            await send(
                {"type": "http.response.body", "body": b"got:" + message["body"]}
            )
            return
        if path == "/size":
            message = await receive()
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send(
                {
                    "type": "http.response.body",
                    "body": str(len(message["body"])).encode("utf-8"),
                }
            )
            return
        if path == "/stream":
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"content-type", b"text/plain")],
                }
            )
            for index in range(3):
                await send(
                    {
                        "type": "http.response.body",
                        "body": ("chunk%d-" % index).encode("utf-8"),
                        "more_body": True,
                    }
                )
            await send({"type": "http.response.body", "body": b"end"})
            return
        if path == "/scope":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send(
                {
                    "type": "http.response.body",
                    "body": repr(
                        (
                            scope["type"],
                            scope["asgi"],
                            scope["http_version"],
                            scope["method"],
                            scope["scheme"],
                            scope["path"],
                            scope["raw_path"],
                            scope["query_string"],
                            scope["root_path"],
                            scope["client"][0],
                            scope["server"][1],
                        )
                    ).encode("utf-8"),
                }
            )
            return
        if path == "/headers-in":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            values = [pair for pair in scope["headers"] if pair[0] == b"x-custom-value"]
            await send(
                {"type": "http.response.body", "body": repr(values).encode("utf-8")}
            )
            return
        if path == "/headers-out":
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"x-first", b"one"), (b"x-second", b"two")],
                }
            )
            await send({"type": "http.response.body", "body": b""})
            return
        if path == "/own-length":
            body = b"exact"
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [
                        (b"content-type", b"text/plain"),
                        (b"content-length", str(len(body)).encode("utf-8")),
                    ],
                }
            )
            await send({"type": "http.response.body", "body": body})
            return
        if path == "/start-only":
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"x-marker", b"present")],
                }
            )
            return
        if path == "/long-header":
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"x-long", b"v" * 4000)],
                }
            )
            await send({"type": "http.response.body", "body": b"long"})
            return
        if path == "/no-content":
            await send({"type": "http.response.start", "status": 204, "headers": []})
            await send({"type": "http.response.body", "body": b"ignored"})
            return
        if path == "/raise-after":
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"content-type", b"text/plain")],
                }
            )
            await send(
                {"type": "http.response.body", "body": b"partial", "more_body": True}
            )
            raise RuntimeError("intentional failure after start")
        if path == "/never-close":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send(
                {"type": "http.response.body", "body": b"open", "more_body": True}
            )
            return
        if path == "/exit":
            raise SystemExit(3)
        if path == "/send-not-dict":
            try:
                await send("nonsense")
            except TypeError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/send-no-type":
            try:
                await send({"nothing": True})
            except ValueError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/header-not-bytes":
            try:
                await send(
                    {
                        "type": "http.response.start",
                        "status": 200,
                        "headers": [("x-text", "value")],
                    }
                )
            except TypeError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/body-not-bytes":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            try:
                await send({"type": "http.response.body", "body": "text"})
            except TypeError:
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/short-header":
            try:
                await send(
                    {
                        "type": "http.response.start",
                        "status": 200,
                        "headers": [(b"only-one",)],
                    }
                )
            except IndexError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/similar-header":
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"x-content-type", b"one")],
                }
            )
            await send({"type": "http.response.body", "body": b"similar"})
            return
        if path == "/bad-header":
            try:
                await send(
                    {
                        "type": "http.response.start",
                        "status": 200,
                        "headers": [(b"x-bad", b"a\r\nInjected: 1")],
                    }
                )
            except ValueError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/bad-status":
            try:
                await send({"type": "http.response.start", "status": 99, "headers": []})
            except ValueError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/body-first":
            try:
                await send({"type": "http.response.body", "body": b"early"})
            except RuntimeError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/double-start":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            try:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
            except RuntimeError:
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/after-complete":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            send({"type": "http.response.body", "body": b"done"})
            try:
                send({"type": "http.response.body", "body": b"extra"})
            except RuntimeError:
                pass
            return
        if path == "/unknown-message":
            try:
                await send({"type": "http.nonsense"})
            except ValueError:
                await send(
                    {"type": "http.response.start", "status": 200, "headers": []}
                )
                await send({"type": "http.response.body", "body": b"rejected"})
            return
        if path == "/exhausted":
            await receive()
            try:
                await asyncio.wait_for(receive(), timeout=0.2)
                result = b"unexpected"
            except asyncio.TimeoutError:
                result = b"exhausted"
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send({"type": "http.response.body", "body": result})
            return
        if path == "/flood":
            await send({"type": "http.response.start", "status": 200, "headers": []})
            try:
                for _ in range(20):
                    await send(
                        {
                            "type": "http.response.body",
                            "body": b"z" * 100000,
                            "more_body": True,
                        }
                    )
                await send({"type": "http.response.body", "body": b""})
            except BaseException:
                pass
            return
        if path == "/slow":
            await asyncio.sleep(0.4)
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send({"type": "http.response.body", "body": b"slow"})
            return
        if path.startswith("/decoded"):
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send({"type": "http.response.body", "body": path.encode("utf-8")})
            return
        if path == "/missing":
            await send({"type": "http.response.start", "status": 404, "headers": []})
            await send({"type": "http.response.body", "body": b"nope"})
            return
        await send(
            {
                "type": "http.response.start",
                "status": 200,
                "headers": [(b"content-type", b"text/plain")],
            }
        )
        await send({"type": "http.response.body", "body": b"plain"})


class AsgiTest(ServerCase):
    """
    Test suite for the http scope of the asgi handler, exercising
    the complete request cycle of an application.
    """

    def test_interface_detection(self):
        # verifies that a coroutine function is detected as an asgi
        # application while a plain one is taken as a wsgi one
        async def application(scope, receive, send):
            pass

        def wsgi(environ, start_response):
            pass

        server = viriatum.Server(application, port=self.port + 90)
        self.assertTrue(server.asgi)
        server = viriatum.Server(wsgi, port=self.port + 91)
        self.assertFalse(server.asgi)

    def test_interface_detection_callable(self):
        # verifies that an instance whose call method is a coroutine
        # one is also detected as an asgi application
        class Application(object):
            async def __call__(self, scope, receive, send):
                pass

        server = viriatum.Server(Application(), port=self.port + 92)
        self.assertTrue(server.asgi)

    def test_interface_explicit(self):
        # verifies that the interface may be forced regardless of the
        # shape of the application that has been provided
        def wsgi(environ, start_response):
            pass

        server = viriatum.Server(wsgi, port=self.port + 93, interface="asgi")
        self.assertTrue(server.asgi)
        server = viriatum.Server(wsgi, port=self.port + 94, interface="wsgi")
        self.assertFalse(server.asgi)

    def test_interface_version(self):
        # verifies that the shape of the application selects the
        # version of the interface used for the calling of it
        def legacy(scope):
            async def application(receive, send):
                pass

            return application

        class LegacyClass(object):
            def __init__(self, scope):
                self.scope = scope

            async def __call__(self, receive, send):
                pass

        async def modern(scope, receive, send):
            pass

        server = viriatum.Server(legacy, port=self.port + 80, interface="asgi")
        self.assertTrue(server.double_callable)
        server = viriatum.Server(LegacyClass, port=self.port + 81, interface="asgi")
        self.assertTrue(server.double_callable)
        server = viriatum.Server(modern, port=self.port + 82, interface="asgi")
        self.assertFalse(server.double_callable)

    def test_interface_version_forced(self):
        # verifies that the version of the interface may be forced
        # regardless of the shape of the application
        async def application(scope, receive, send):
            pass

        server = viriatum.Server(application, port=self.port + 83, interface="asgi2")
        self.assertTrue(server.asgi)
        self.assertTrue(server.double_callable)
        server = viriatum.Server(application, port=self.port + 84, interface="asgi3")
        self.assertTrue(server.asgi)
        self.assertFalse(server.double_callable)

    def test_interface_markers(self):
        # verifies that the markers set by the adaptation helpers take
        # precedence over the inspection of the application
        async def single(scope, receive, send):
            pass

        def double(scope):
            pass

        single._asgi_double_callable = True
        double._asgi_single_callable = True

        server = viriatum.Server(single, port=self.port + 85, interface="asgi")
        self.assertTrue(server.double_callable)
        server = viriatum.Server(double, port=self.port + 86, interface="asgi")
        self.assertFalse(server.double_callable)

    def test_interface_markers_auto(self):
        # verifies that the markers are honoured by the automatic
        # interface as well, a wsgi application never carries them
        def double(scope):
            pass

        async def single(scope, receive, send):
            pass

        double._asgi_double_callable = True
        single._asgi_single_callable = True

        server = viriatum.Server(double, port=self.port + 87)
        self.assertTrue(server.asgi)
        self.assertTrue(server.double_callable)
        server = viriatum.Server(single, port=self.port + 88)
        self.assertTrue(server.asgi)
        self.assertFalse(server.double_callable)

    def test_interface_invalid(self):
        # verifies that an unknown interface is rejected at the
        # construction of the server object
        async def application(scope, receive, send):
            pass

        self.assertRaises(
            ValueError,
            viriatum.Server,
            application,
            port=self.port + 95,
            interface="nonsense",
        )

    def test_loop_creation_failure(self):
        # verifies that a failure in the creation of the event loop
        # is reported instead of leaving a half built server
        async def application(scope, receive, send):
            pass

        def new_event_loop():
            raise RuntimeError("intentional failure")

        original = asyncio.new_event_loop
        asyncio.new_event_loop = new_event_loop
        try:
            self.assertRaises(
                RuntimeError, viriatum.Server, application, port=self.port + 96
            )
        finally:
            asyncio.new_event_loop = original

    def test_simple_request(self):
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"plain")
        self.assertEqual(result.headers.get("Content-Type"), "text/plain")

    def test_scope(self):
        result = urllib.request.urlopen(self._url("/scope?a=1&b=2"), timeout=5)
        values = ast.literal_eval(result.read().decode())
        self.assertEqual(values[0], "http")
        self.assertEqual(values[1], {"version": "3.0", "spec_version": "2.3"})
        self.assertEqual(values[2], "1.1")
        self.assertEqual(values[3], "GET")
        self.assertEqual(values[4], "http")
        self.assertEqual(values[5], "/scope")
        self.assertEqual(values[6], b"/scope")
        self.assertEqual(values[7], b"a=1&b=2")
        self.assertEqual(values[8], "")
        self.assertEqual(values[9], "127.0.0.1")
        self.assertEqual(values[10], self.port)

    def test_scope_decoded_path(self):
        # verifies that the path reaches the application already
        # decoded while the raw one keeps the escapes
        result = urllib.request.urlopen(self._url("/decoded%2Da%20b"), timeout=5)
        self.assertEqual(result.read(), "/decoded-a b".encode("utf-8"))

    def test_scope_encoded_separator(self):
        # verifies that an encoded query separator is not able to
        # forge one, the split happens before the decoding
        result = urllib.request.urlopen(self._url("/decoded%3Fa=1"), timeout=5)
        self.assertEqual(result.read(), b"/decoded?a=1")

    def test_request_headers(self):
        # verifies that the headers of the request reach the
        # application as a sequence of lower cased byte pairs
        request = urllib.request.Request(
            self._url("/headers-in"), headers={"X-Custom-Value": "custom"}
        )
        result = urllib.request.urlopen(request, timeout=5)
        self.assertEqual(
            ast.literal_eval(result.read().decode()), [(b"x-custom-value", b"custom")]
        )

    def test_response_headers(self):
        result = urllib.request.urlopen(self._url("/headers-out"), timeout=5)
        self.assertEqual(result.headers.get("X-First"), "one")
        self.assertEqual(result.headers.get("X-Second"), "two")

    def test_receive_body(self):
        result = urllib.request.urlopen(self._url("/echo"), data=b"payload", timeout=5)
        self.assertEqual(result.status, 201)
        self.assertEqual(result.read(), b"got:payload")

    def test_receive_empty_body(self):
        result = urllib.request.urlopen(self._url("/size"), data=b"", timeout=5)
        self.assertEqual(result.read(), b"0")

    def test_receive_large_body(self):
        # verifies that a payload that spans several of the body
        # callbacks of the parser is properly accumulated
        payload = b"x" * 200000
        result = urllib.request.urlopen(self._url("/size"), data=payload, timeout=15)
        self.assertEqual(result.read(), str(len(payload)).encode("utf-8"))

    def test_receive_exhausted(self):
        # verifies that the request stream carries a single event, a
        # further receive blocks instead of producing a bogus one
        result = urllib.request.urlopen(self._url("/exhausted"), timeout=5)
        self.assertEqual(result.read(), b"exhausted")

    def test_streaming_response(self):
        # verifies that the various chunks are joined by the client
        # and that the chunked framing has been used for them
        result = urllib.request.urlopen(self._url("/stream"), timeout=5)
        self.assertEqual(result.read(), b"chunk0-chunk1-chunk2-end")
        self.assertEqual(result.headers.get("Transfer-Encoding"), "chunked")
        self.assertEqual(result.headers.get("Content-Length"), None)

    def test_streaming_keep_alive(self):
        # verifies that a response whose size is not known in advance
        # still bounds the message, so a connection that is kept alive
        # carries a second one after it rather than leaving the client
        # waiting for the rest of the first forever
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        connection.auto_open = 0
        connection.connect()
        connection.request("GET", "/stream")
        result = connection.getresponse()
        self.assertEqual(result.read(), b"chunk0-chunk1-chunk2-end")
        self.assertEqual(result.headers.get("Transfer-Encoding"), "chunked")
        self.assertEqual(result.headers.get("Content-Length"), None)

        # the message of the stream has been bounded by the framing of
        # the chunks, so the one that follows it is served just the
        # same on the very same connection
        connection.request("GET", "/plain")
        result = connection.getresponse()
        self.assertEqual(result.read(), b"plain")
        connection.close()

    def test_streaming_framing(self):
        # verifies that each of the chunks reaches the wire framed on
        # its own, which is what makes the response a streamed one
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        connection.putrequest("GET", "/stream", skip_accept_encoding=True)
        connection.endheaders()
        raw = connection.sock.recv(65536)
        while b"0\r\n\r\n" not in raw:
            data = connection.sock.recv(65536)
            if not data:
                break
            raw += data
        connection.close()
        body = raw.split(b"\r\n\r\n", 1)[1]
        self.assertTrue(body.startswith(b"7\r\nchunk0-\r\n"))
        self.assertTrue(body.endswith(b"3\r\nend\r\n0\r\n\r\n"))

    def test_own_length(self):
        # verifies that a content length set by the application
        # replaces the chunked framing of the payload
        result = urllib.request.urlopen(self._url("/own-length"), timeout=5)
        self.assertEqual(result.read(), b"exact")
        self.assertEqual(result.headers.get("Content-Length"), "5")
        self.assertEqual(result.headers.get("Transfer-Encoding"), None)

    def test_head_request(self):
        # verifies that the response of a head request carries no
        # payload and is not framed as a chunked one
        request = urllib.request.Request(self._url("/plain"), method="HEAD")
        result = urllib.request.urlopen(request, timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"")
        self.assertEqual(result.headers.get("Transfer-Encoding"), None)

    def test_start_only_response(self):
        # verifies that an application that starts the response and
        # returns without any payload still produces a complete
        # envelope, the client would otherwise receive no status line
        result = urllib.request.urlopen(self._url("/start-only"), timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"")
        self.assertEqual(result.headers.get("X-Marker"), "present")

    def test_long_response_header(self):
        # verifies that a header longer than the maximum size of a
        # single one reaches the wire, the formatting of the envelope
        # is bounded by the remaining capacity of its buffer
        result = urllib.request.urlopen(self._url("/long-header"), timeout=5)
        self.assertEqual(result.read(), b"long")
        self.assertEqual(result.headers.get("X-Long"), "v" * 4000)

    def test_oversized_body(self):
        # verifies that a payload beyond the maximum allowed size is
        # refused, handing a truncated one to the application would
        # have it processed as valid but incomplete data
        payload = b"o" * (17 * 1024 * 1024)
        result = self._request_data("/size", payload)
        self.assertEqual(result.status, 413)
        self.assertEqual(result.read(), b"Payload Too Large")

    def test_no_content(self):
        # verifies that a status that carries no payload discards the
        # body that has been provided by the application
        result = urllib.request.urlopen(self._url("/no-content"), timeout=5)
        self.assertEqual(result.status, 204)
        self.assertEqual(result.read(), b"")
        self.assertEqual(result.headers.get("Transfer-Encoding"), None)

    def test_status_message(self):
        # verifies that the status message is derived from the code
        # provided by the application, as asgi carries no reason
        result = self._request("/missing")
        self.assertEqual(result.status, 404)
        self.assertEqual(result.reason, "Not Found")

    def test_application_error(self):
        # verifies that an application that raises before starting
        # the response has an internal error produced for it
        result = self._request("/boom")
        self.assertEqual(result.status, 500)
        self.assertEqual(result.read(), b"Internal Server Error")

    def test_application_error_after_start(self):
        # verifies that an application that raises after starting the
        # response has the stream terminated instead of hanging
        result = urllib.request.urlopen(self._url("/raise-after"), timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"partial")

    def test_application_never_closes(self):
        # verifies that an application that returns without closing
        # the response has the stream terminated for it
        result = urllib.request.urlopen(self._url("/never-close"), timeout=5)
        self.assertEqual(result.read(), b"open")

    def test_bad_header(self):
        # verifies that a header carrying a control character is
        # rejected, avoiding the splitting of the response
        result = urllib.request.urlopen(self._url("/bad-header"), timeout=5)
        self.assertEqual(result.read(), b"rejected")
        self.assertEqual(result.headers.get("Injected"), None)

    def test_bad_status(self):
        # verifies that a status outside of the valid range is
        # rejected instead of reaching the wire
        result = urllib.request.urlopen(self._url("/bad-status"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_body_before_start(self):
        # verifies that a payload sent before the start of the
        # response is rejected as an out of order message
        result = urllib.request.urlopen(self._url("/body-first"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_double_start(self):
        # verifies that the response may only be started once, a
        # second attempt is reported as a problem
        result = urllib.request.urlopen(self._url("/double-start"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_body_after_complete(self):
        # verifies that no payload may follow the one that closed
        # the response, the stream has already been terminated
        result = urllib.request.urlopen(self._url("/after-complete"), timeout=5)
        self.assertEqual(result.read(), b"done")

    def test_unknown_message(self):
        # verifies that a message type that is not part of the http
        # scope is rejected as an unexpected one
        result = urllib.request.urlopen(self._url("/unknown-message"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_keep_alive(self):
        # verifies that two requests may be issued over a single
        # connection, the handler is reset between them, the
        # reopening of it is disabled so that a closed one fails
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        connection.auto_open = 0
        connection.connect()
        connection.request("GET", "/plain", headers={"Connection": "keep-alive"})
        result = connection.getresponse()
        self.assertEqual(result.read(), b"plain")
        self.assertEqual(result.headers.get("Connection"), "keep-alive")
        connection.request(
            "GET", "/echo", body=b"again", headers={"Connection": "keep-alive"}
        )
        self.assertEqual(connection.getresponse().read(), b"got:again")
        connection.close()

    def test_concurrent_keep_alive(self):
        # drives a series of requests over several connections that are
        # kept alive, so that the writing of the responses completes
        # from the polling of the service rather than from the calling
        # of it, the tearing down of the context runs with the global
        # interpreter lock released in that path
        failures = []

        def hammer(count):
            connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=15)
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

        threads = [threading.Thread(target=hammer, args=(30,)) for _ in range(4)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join(timeout=30)

        self.assertEqual(failures, [])
        self.assertEqual([thread.is_alive() for thread in threads], [False] * 4)
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.read(), b"plain")

    def test_keep_alive_by_default(self):
        # verifies that a request of the most recent version of the
        # protocol keeps the connection alive without asking for it
        # and closes it when it does ask, which is what the
        # specification requires of each of the two cases
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        connection.auto_open = 0
        connection.connect()
        connection.request("GET", "/plain")
        result = connection.getresponse()
        self.assertEqual(result.read(), b"plain")
        self.assertEqual(result.headers.get("Connection"), "keep-alive")

        # sends a second message over the very same connection, which
        # only reaches the server because the first one kept it alive
        connection.request("GET", "/plain", headers={"Connection": "close"})
        result = connection.getresponse()
        self.assertEqual(result.read(), b"plain")
        self.assertEqual(result.headers.get("Connection"), "close")
        connection.close()

    def test_multiplexed_streams(self):
        # verifies that several requests travelling at the same time on
        # one connection of the most recent version of the protocol are
        # each answered on the stream they came in on, an application
        # answers on a clock of its own and by then the connection is
        # likely serving another of the messages that travel on it
        nghttp = shutil.which("nghttp")
        if nghttp is None:
            self.skipTest("no client of the most recent version is available")

        # the client opens a single connection and asks for every one
        # of the resources on a stream of its own, which is what makes
        # the answers of them travel at the same time
        paths = ["/scope?a=1", "/scope?a=2", "/scope?a=3", "/scope?a=4"]
        process = subprocess.Popen(
            [nghttp] + [self._url(path) for path in paths],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        output = process.communicate(timeout=30)[0].decode()
        self.assertEqual(process.returncode, 0)

        # every one of the answers carries the query of the request it
        # belongs to, one that had been written on another stream would
        # carry the query of that one twice over and leave the query of
        # the stream it belonged to out of the answers altogether
        for index in range(1, len(paths) + 1):
            self.assertEqual(output.count("a=%d" % index), 1)

        # the answers name the version that served them, which is the
        # most recent one as the client asked for nothing else
        self.assertEqual(output.count("'2'"), len(paths))

    def test_system_exit(self):
        # verifies that an application requesting a system exit does
        # not terminate the process that is hosting the server
        result = self._request("/exit")
        self.assertEqual(result.status, 500)
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.read(), b"plain")

    def test_send_not_dictionary(self):
        # verifies that a message that is not a mapping is rejected
        # instead of being inspected for a type
        result = urllib.request.urlopen(self._url("/send-not-dict"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_send_without_type(self):
        # verifies that a message that carries no type is rejected
        # as it may not be dispatched
        result = urllib.request.urlopen(self._url("/send-no-type"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_header_not_bytes(self):
        # verifies that a header that is not a pair of byte strings
        # is rejected, as the specification requires them
        result = urllib.request.urlopen(self._url("/header-not-bytes"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_body_not_bytes(self):
        # verifies that a payload that is not a byte string is
        # rejected instead of reaching the wire
        result = urllib.request.urlopen(self._url("/body-not-bytes"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_similar_header(self):
        # verifies that a header whose name is as long as the content
        # length one is not mistaken for it, the payload stays framed
        result = urllib.request.urlopen(self._url("/similar-header"), timeout=5)
        self.assertEqual(result.read(), b"similar")
        self.assertEqual(result.headers.get("X-Content-Type"), "one")
        self.assertEqual(result.headers.get("Transfer-Encoding"), "chunked")

    def test_short_header(self):
        # verifies that a header that is not a complete pair is
        # rejected instead of reaching the wire half formed
        result = urllib.request.urlopen(self._url("/short-header"), timeout=5)
        self.assertEqual(result.read(), b"rejected")

    def test_empty_header_value(self):
        # verifies that a request header carrying an empty value does
        # not leave the name of it pending in the context
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        connection.request(
            "GET", "/headers-in", headers={"X-Empty": "", "X-Custom-Value": "custom"}
        )
        result = connection.getresponse()
        self.assertEqual(
            ast.literal_eval(result.read().decode()), [(b"x-custom-value", b"custom")]
        )
        connection.close()

    def test_multiplexed_released(self):
        # verifies that the objects of a request are released once the
        # stream that carried it is over, several of them travel at the
        # same time on one connection and each one carries a context of
        # its own that must not outlive the stream
        nghttp = shutil.which("nghttp")
        if nghttp is None:
            self.skipTest("no client of the most recent version is available")

        def request(count):
            paths = [self._url("/scope?a=%d" % index) for index in range(count)]
            process = subprocess.Popen(
                [nghttp] + paths, stdout=subprocess.PIPE, stderr=subprocess.PIPE
            )
            process.communicate(timeout=30)
            self.assertEqual(process.returncode, 0)

        def objects():
            gc.collect()
            return len(gc.get_objects())

        # a first round warms whatever is built once and kept, the
        # count is only meaningful once that has settled
        request(8)
        time.sleep(0.5)
        initial = objects()
        request(40)
        time.sleep(1.0)
        retained = objects() - initial
        self.assertTrue(
            retained < 200,
            "retained %d objects across 40 multiplexed streams" % retained,
        )

    def test_pending_writes_released(self):
        # verifies that the payloads queued in a connection that is
        # dropped before they reach the wire are released, they are
        # never handed back through the callback of the connection
        def drop(count):
            for _ in range(count):
                connection = socket.create_connection(
                    ("127.0.0.1", self.port), timeout=5
                )
                connection.sendall(b"GET /flood HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
                time.sleep(0.02)
                connection.setsockopt(
                    socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0)
                )
                connection.close()
                time.sleep(0.02)

        def futures():
            gc.collect()
            return sum(
                1 for item in gc.get_objects() if type(item).__name__ == "Future"
            )

        drop(5)
        time.sleep(0.5)
        initial = futures()
        drop(30)
        time.sleep(1.0)
        retained = futures() - initial
        self.assertTrue(
            retained < 10,
            "retained %d futures across 30 dropped connections" % retained,
        )

    @unittest.skipIf(
        sys.platform == "win32",
        "the platform carries no interrupt of the kind this drives",
    )
    def test_keyboard_interrupt(self):
        # verifies that an interrupt stops the serving loop, the
        # signals are only handled in the main thread and so it is the
        # one that runs the loop while another raises the interrupt,
        # the other platform is left out of it because the signal it
        # raises is not one of the system there and the loop is left
        # waiting on an interrupt that never reaches it
        state = {"raised": False, "expired": False}
        finished = threading.Event()

        async def application(scope, receive, send):
            # refuses the lifespan scope so that the opening of the
            # service is immediate, the interrupt has to reach the
            # serving loop rather than the wait of the protocol
            if scope["type"] != "http":
                raise NotImplementedError(scope["type"])
            await receive()
            await asyncio.sleep(2.0)
            await send({"type": "http.response.start", "status": 200, "headers": []})
            await send({"type": "http.response.body", "body": b"plain"})

        server = viriatum.Server(application, port=self.port + 30, interface="asgi")

        def request():
            # keeps a request in flight so that a task stays pending
            # and the loop is advancing the interpreter, which is the
            # window where the interrupt used to be discarded
            try:
                urllib.request.urlopen(
                    "http://127.0.0.1:%d/" % (self.port + 30), timeout=10
                )
            except Exception:
                pass

        def interrupt():
            # waits for the service to be listening before raising the
            # interrupt, so that it never reaches the opening of it
            for _ in range(200):
                try:
                    connection = socket.create_connection(
                        ("127.0.0.1", self.port + 30), timeout=1
                    )
                    connection.close()
                    break
                except Exception:
                    time.sleep(0.05)
            threading.Thread(target=request, daemon=True).start()
            time.sleep(0.5)
            state["raised"] = True
            signal.raise_signal(signal.SIGINT)

            # stops the server in case the interrupt is ignored, so
            # that the test fails instead of hanging forever
            if not finished.wait(timeout=10.0):
                state["expired"] = True
                server.stop()

        threading.Thread(target=interrupt, daemon=True).start()
        try:
            self.assertRaises(KeyboardInterrupt, server.serve_forever)
        finally:
            finished.set()
        self.assertTrue(state["raised"])
        self.assertFalse(state["expired"], "the interrupt was ignored")

    def test_serve_helper(self):
        # verifies that the helper builds a server with the provided
        # arguments and runs its loop until it is stopped
        created = []
        original = viriatum.Server

        class Capturing(object):
            def __init__(self, *args, **kwargs):
                self.server = original(*args, **kwargs)
                created.append(self.server)

            def serve_forever(self):
                return self.server.serve_forever()

        viriatum.Server = Capturing
        try:
            thread = threading.Thread(
                target=viriatum.serve,
                args=(self._application,),
                kwargs=dict(port=self.port + 20, interface="asgi"),
                daemon=True,
            )
            thread.start()
            url = "http://127.0.0.1:%d/plain" % (self.port + 20)
            for _ in range(100):
                try:
                    self.assertEqual(
                        urllib.request.urlopen(url, timeout=1).read(), b"plain"
                    )
                    break
                except AssertionError:
                    raise
                except Exception:
                    time.sleep(0.1)
            else:
                raise AssertionError("server did not become ready")
        finally:
            viriatum.Server = original
        self.assertEqual(len(created), 1)
        self.assertTrue(created[0].asgi)
        created[0].stop()
        thread.join(timeout=10)
        self.assertFalse(thread.is_alive())

    def test_concurrent_requests(self):
        # verifies that two slow requests genuinely overlap, which is
        # the whole point of driving the loop from the polling one
        results = []

        def issue():
            result = urllib.request.urlopen(self._url("/slow"), timeout=15)
            results.append(result.read())

        initial = time.time()
        threads = [threading.Thread(target=issue) for _ in range(2)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join(timeout=15)
        elapsed = time.time() - initial

        self.assertEqual(results, [b"slow", b"slow"])
        self.assertTrue(elapsed < 0.7, "requests took %.2f seconds" % elapsed)

    def _request_data(self, path, data):
        # issues a request carrying a payload returning the response
        # even when the status of it is one of the error ones
        try:
            return urllib.request.urlopen(self._url(path), data=data, timeout=30)
        except urllib.error.HTTPError as error:
            return error

    def _request(self, path):
        # issues a request returning the response even when the status
        # of it is one of the error ones (not raising)
        try:
            return urllib.request.urlopen(self._url(path), timeout=5)
        except urllib.error.HTTPError as error:
            return error


class LegacyTest(unittest.TestCase):
    """
    Test suite for the second version of the interface, the one
    where the application is a double callable.
    """

    OFFSET = 20

    @classmethod
    def setUpClass(cls):
        # creates the server for the legacy application defined below
        # and runs its loop in a separate thread
        cls.port = PORT + cls.OFFSET
        cls.server = viriatum.Server(cls._application, port=cls.port, interface="asgi")
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        for _ in range(100):
            try:
                urllib.request.urlopen(cls._url("/plain"), timeout=1).read()
                return
            except Exception:
                time.sleep(0.1)
        raise AssertionError("server did not become ready")

    @classmethod
    def tearDownClass(cls):
        cls.server.stop()
        cls.thread.join(timeout=10)

    @classmethod
    def _url(cls, path):
        return "http://127.0.0.1:%d%s" % (cls.port, path)

    @staticmethod
    def _application(scope):
        # the scope is taken by the outer callable and only then are
        # the two callables provided, the double callable shape
        async def application(receive, send):
            if scope["type"] == "lifespan":
                while True:
                    message = await receive()
                    if message["type"] == "lifespan.startup":
                        await send({"type": "lifespan.startup.complete"})
                    elif message["type"] == "lifespan.shutdown":
                        await send({"type": "lifespan.shutdown.complete"})
                        return
            message = await receive()
            if scope["path"] == "/scope":
                body = repr(
                    (scope["type"], scope["asgi"]["version"], scope["method"])
                ).encode("utf-8")
            elif scope["path"] == "/echo":
                body = b"got:" + message["body"]
            else:
                body = b"plain"
            await send(
                {
                    "type": "http.response.start",
                    "status": 200,
                    "headers": [(b"content-type", b"text/plain")],
                }
            )
            await send({"type": "http.response.body", "body": body})

        return application

    def test_simple_request(self):
        result = urllib.request.urlopen(self._url("/plain"), timeout=5)
        self.assertEqual(result.status, 200)
        self.assertEqual(result.read(), b"plain")

    def test_scope_version(self):
        # verifies that the scope reports the version of the interface
        # that is being used for the calling of the application
        result = urllib.request.urlopen(self._url("/scope"), timeout=5)
        self.assertEqual(
            ast.literal_eval(result.read().decode()), ("http", "2.0", "GET")
        )

    def test_receive_body(self):
        result = urllib.request.urlopen(self._url("/echo"), data=b"payload", timeout=5)
        self.assertEqual(result.read(), b"got:payload")

    def test_lifespan(self):
        # verifies that the lifespan protocol is driven through the
        # double callable shape as well, the server is already up
        self.assertTrue(self.thread.is_alive())


class LifespanTest(unittest.TestCase):
    """
    Test suite for the lifespan scope, each of the cases runs a
    server of its own as the protocol spans the complete serving.
    """

    def test_lifespan(self):
        # verifies that both the startup and the shutdown events
        # reach an application that implements the protocol
        events = []

        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                await self._plain(receive, send)
                return
            while True:
                message = await receive()
                events.append(message["type"])
                if message["type"] == "lifespan.startup":
                    await send({"type": "lifespan.startup.complete"})
                elif message["type"] == "lifespan.shutdown":
                    await send({"type": "lifespan.shutdown.complete"})
                    return

        self._serve(application, PORT + 1)
        self.assertEqual(events, ["lifespan.startup", "lifespan.shutdown"])

    def test_lifespan_slow_startup(self):
        # verifies that a startup that awaits real work is waited for
        # rather than being abandoned, an application that connects to
        # a database on startup depends on it
        events = []

        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                await self._plain(receive, send)
                return
            while True:
                message = await receive()
                if message["type"] == "lifespan.startup":
                    await asyncio.sleep(0.3)
                    events.append("startup")
                    await send({"type": "lifespan.startup.complete"})
                elif message["type"] == "lifespan.shutdown":
                    await asyncio.sleep(0.1)
                    events.append("shutdown")
                    await send({"type": "lifespan.shutdown.complete"})
                    return

        initial = time.time()
        self._serve(application, PORT + 7)
        self.assertEqual(events, ["startup", "shutdown"])
        self.assertTrue(time.time() - initial >= 0.3, "startup was not waited for")

    def test_lifespan_loop_failure(self):
        # verifies that a loop that is unable to advance does not hang
        # the opening of the service, the wait gives up and the
        # requests are served without any lifespan handling
        events = []

        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                await self._plain(receive, send)
                return
            while True:
                message = await receive()
                events.append(message["type"])
                if message["type"] == "lifespan.startup":
                    await send({"type": "lifespan.startup.complete"})

        def sleep(*args, **kwargs):
            raise RuntimeError("intentional failure")

        original = asyncio.sleep
        asyncio.sleep = sleep
        try:
            self._serve(application, PORT + 8)
        finally:
            asyncio.sleep = original
        self.assertEqual(events, [])

    def test_lifespan_shutdown_on_interrupt(self):
        # verifies that the shutdown of the lifespan is run when the
        # serving is ended by an interrupt, the exception raised by
        # the signal has to be saved while it takes place
        events = []
        state = {"expired": False}
        finished = threading.Event()

        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                await self._plain(receive, send)
                return
            while True:
                message = await receive()
                events.append(message["type"])
                if message["type"] == "lifespan.startup":
                    await send({"type": "lifespan.startup.complete"})
                elif message["type"] == "lifespan.shutdown":
                    await send({"type": "lifespan.shutdown.complete"})
                    return

        server = viriatum.Server(application, port=PORT + 9, interface="asgi")

        def interrupt():
            for _ in range(200):
                try:
                    connection = socket.create_connection(
                        ("127.0.0.1", PORT + 9), timeout=1
                    )
                    connection.close()
                    break
                except Exception:
                    time.sleep(0.05)
            signal.raise_signal(signal.SIGINT)
            if not finished.wait(timeout=10.0):
                state["expired"] = True
                server.stop()

        threading.Thread(target=interrupt, daemon=True).start()
        try:
            self.assertRaises(KeyboardInterrupt, server.serve_forever)
        finally:
            finished.set()
        self.assertFalse(state["expired"], "the interrupt was ignored")
        self.assertEqual(events, ["lifespan.startup", "lifespan.shutdown"])

    def test_lifespan_unsupported(self):
        # verifies that an application that refuses the lifespan
        # scope is served anyway, as the specification requires
        async def application(scope, receive, send):
            if scope["type"] == "lifespan":
                raise NotImplementedError("no lifespan here")
            await self._plain(receive, send)

        self._serve(application, PORT + 2)

    def test_lifespan_startup_failed(self):
        # verifies that an application that fails the startup event
        # aborts the serving, reporting the problem to the caller
        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                return
            await receive()
            await send(
                {"type": "lifespan.startup.failed", "message": "intentional failure"}
            )
            while True:
                await receive()

        server = viriatum.Server(application, port=PORT + 3)
        self.assertRaises(RuntimeError, server.serve_forever)

    def test_lifespan_unknown_message(self):
        # verifies that a message that is not part of the lifespan
        # protocol is refused, the startup completes anyway
        failures = []

        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                await self._plain(receive, send)
                return
            await receive()
            try:
                await send({"type": "lifespan.nonsense"})
            except ValueError:
                failures.append("unknown-message")
            await send({"type": "lifespan.startup.complete"})
            while True:
                message = await receive()
                if message["type"] == "lifespan.shutdown":
                    await send({"type": "lifespan.shutdown.complete"})
                    return

        self._serve(application, PORT + 5)
        self.assertEqual(failures, ["unknown-message"])

    def test_lifespan_not_a_coroutine(self):
        # verifies that a plain function forced into the asgi
        # interface is refused, both at the lifespan and the request
        def application(scope, receive, send):
            return None

        server = viriatum.Server(application, port=PORT + 6, interface="asgi")
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        url = "http://127.0.0.1:%d/plain" % (PORT + 6)
        for _ in range(100):
            try:
                urllib.request.urlopen(url, timeout=1).read()
                raise AssertionError("request should have failed")
            except urllib.error.HTTPError as error:
                self.assertEqual(error.status, 500)
                break
            except AssertionError:
                raise
            except Exception:
                time.sleep(0.1)
        else:
            raise AssertionError("server did not become ready")
        server.stop()
        thread.join(timeout=10)
        self.assertFalse(thread.is_alive())

    def test_lifespan_shutdown_failed(self):
        # verifies that a failure of the shutdown event does not
        # prevent the serving from ending properly
        async def application(scope, receive, send):
            if scope["type"] != "lifespan":
                await self._plain(receive, send)
                return
            while True:
                message = await receive()
                if message["type"] == "lifespan.startup":
                    await send({"type": "lifespan.startup.complete"})
                elif message["type"] == "lifespan.shutdown":
                    await send(
                        {
                            "type": "lifespan.shutdown.failed",
                            "message": "intentional failure",
                        }
                    )
                    return

        self._serve(application, PORT + 4)

    @staticmethod
    async def _plain(receive, send):
        await receive()
        await send({"type": "http.response.start", "status": 200, "headers": []})
        await send({"type": "http.response.body", "body": b"plain"})

    def _serve(self, application, port):
        # runs the provided application for the time it takes to
        # answer a single request, then stops the server
        server = viriatum.Server(application, port=port)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        url = "http://127.0.0.1:%d/plain" % port
        for _ in range(100):
            try:
                self.assertEqual(
                    urllib.request.urlopen(url, timeout=1).read(), b"plain"
                )
                break
            except AssertionError:
                raise
            except Exception:
                time.sleep(0.1)
        else:
            raise AssertionError("server did not become ready")
        server.stop()
        thread.join(timeout=10)
        self.assertFalse(thread.is_alive())


class WebsocketTest(ServerCase):
    """
    Test suite for the websocket scope, it runs a server of its
    own as the application routes both of the scopes.
    """

    OFFSET = 10

    def test_handshake(self):
        # verifies that the handshake is answered with the accept
        # value derived from the key provided by the client
        client = WebSocketClient(self.port)
        self.assertEqual(client.status, 101)
        self.assertEqual(client.header("Upgrade"), "websocket")
        self.assertEqual(client.header("Connection"), "Upgrade")
        self.assertEqual(client.header("Sec-WebSocket-Accept"), client.accept)
        client.close()

    def test_scope(self):
        # verifies that the scope of an upgraded connection carries
        # both the websocket type and the proposed subprotocols
        client = WebSocketClient(
            self.port, path="/ws-scope?a=1", protocols="chat, json"
        )
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x1)
        self.assertEqual(
            ast.literal_eval(payload.decode("utf-8")),
            ("websocket", "ws", "/ws-scope", b"a=1", ["chat", "json"]),
        )
        client.close()

    def test_partial_upgrade(self):
        # verifies that an upgrade value that merely resembles the
        # websocket one is not handled as such, it would otherwise
        # receive a handshake response for an unrelated protocol
        connection = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        connection.putrequest("GET", "/plain", skip_accept_encoding=True)
        connection.putheader("Upgrade", "w1234567t")
        connection.putheader("Connection", "Upgrade")
        connection.putheader("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==")
        connection.putheader("Sec-WebSocket-Version", "13")
        connection.endheaders()
        result = connection.getresponse()
        self.assertEqual(result.status, 200)
        connection.close()

    def test_accept_headers(self):
        # verifies that the headers set by the application are part of
        # the response of the handshake, a framework depends on them
        # for the setting of the cookies of the connection
        client = WebSocketClient(self.port, path="/ws-headers")
        self.assertEqual(client.status, 101)
        self.assertEqual(client.header("Set-Cookie"), "session=1")
        self.assertEqual(client.header("X-Extra"), "yes")
        client.close()

    def test_accept_bad_header(self):
        # verifies that a header carrying a control character is
        # rejected, avoiding the splitting of the handshake response
        client = WebSocketClient(self.port, path="/ws-bad-header")
        self.assertEqual(client.status, 101)
        self.assertEqual(client.receive(), (0x1, b"rejected"))
        self.assertEqual(client.header("Injected"), None)
        client.close()

    def test_subprotocol(self):
        # verifies that the subprotocol selected by the application
        # is part of the response of the handshake
        client = WebSocketClient(self.port, path="/ws-protocol", protocols="chat")
        self.assertEqual(client.status, 101)
        self.assertEqual(client.header("Sec-WebSocket-Protocol"), "chat")
        client.close()

    def test_text_message(self):
        client = WebSocketClient(self.port)
        client.send(0x1, "hello".encode("utf-8"))
        self.assertEqual(client.receive(), (0x1, b"echo:hello"))
        client.close()

    def test_binary_message(self):
        client = WebSocketClient(self.port)
        client.send(0x2, b"\x00\xff")
        self.assertEqual(client.receive(), (0x2, b"bin:\x00\xff"))
        client.close()

    def test_empty_message(self):
        client = WebSocketClient(self.port)
        client.send(0x2, b"")
        self.assertEqual(client.receive(), (0x2, b"bin:"))
        client.close()

    def test_extended_length_message(self):
        # verifies that a payload beyond the base length variant is
        # properly unmasked and reassembled by the server
        payload = bytes(index % 256 for index in range(1000))
        client = WebSocketClient(self.port)
        client.send(0x2, payload)
        self.assertEqual(client.receive(), (0x2, b"bin:" + payload))
        client.close()

    def test_grown_message(self):
        # verifies that both the reception buffer and the reassembly
        # one grow beyond their initial capacity when required
        payload = bytes(index % 256 for index in range(10000))
        client = WebSocketClient(self.port)
        client.send(0x2, payload[:5000], fin=False)
        client.send(0x0, payload[5000:])
        self.assertEqual(client.receive(), (0x2, b"bin:" + payload))
        client.close()

    def test_buffered_frames(self):
        # verifies that a reception carrying a complete frame plus the
        # beginning of another one keeps only the incomplete part
        client = WebSocketClient(self.port)
        mask = b"\x01\x02\x03\x04"
        first = "one".encode("utf-8")
        second = "two".encode("utf-8")
        frame = (
            bytes([0x81, 0x83])
            + mask
            + bytes(first[index] ^ mask[index % 4] for index in range(3))
        )
        client.send_raw(frame + bytes([0x81, 0x83]) + mask[:2])
        self.assertEqual(client.receive(), (0x1, b"echo:one"))
        client.send_raw(
            mask[2:] + bytes(second[index] ^ mask[index % 4] for index in range(3))
        )
        self.assertEqual(client.receive(), (0x1, b"echo:two"))
        client.close()

    def test_refused_messages(self):
        # verifies that the messages that are out of order or unknown
        # are refused, the application collects the failures
        client = WebSocketClient(self.port, path="/ws-refused")
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x1)
        self.assertEqual(
            payload,
            (
                b"send-before-accept,unknown-message,control-subprotocol,"
                b"long-subprotocol,double-accept"
            ),
        )
        client.close()

    def test_application_returns_unaccepted(self):
        # verifies that an application that returns without ever
        # accepting has the handshake refused for it
        client = WebSocketClient(self.port, path="/ws-silent")
        self.assertEqual(client.status, 403)
        client.close()

    def test_payload_not_bytes(self):
        # verifies that a binary payload that is not a byte string is
        # rejected instead of being framed
        client = WebSocketClient(self.port, path="/ws-not-bytes")
        self.assertEqual(client.receive(), (0x1, b"rejected"))
        client.close()

    def test_application_returns(self):
        # verifies that an application that returns without closing
        # the connection has it closed for it
        client = WebSocketClient(self.port, path="/ws-return")
        self.assertEqual(client.status, 101)
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1000)
        client.close()

    def test_fragmented_message(self):
        # verifies that a message split over several frames is
        # reassembled before reaching the application
        client = WebSocketClient(self.port)
        client.send(0x1, b"abc", fin=False)
        client.send(0x0, b"def", fin=False)
        client.send(0x0, b"ghi")
        self.assertEqual(client.receive(), (0x1, b"echo:abcdefghi"))
        client.close()

    def test_ping_pong(self):
        # verifies that a ping is answered with the corresponding
        # pong without the application ever noticing it
        client = WebSocketClient(self.port)
        client.send(0x9, b"payload")
        self.assertEqual(client.receive(), (0xA, b"payload"))
        client.send(0x1, "after".encode("utf-8"))
        self.assertEqual(client.receive(), (0x1, b"echo:after"))
        client.close()

    def test_pong_ignored(self):
        # verifies that an unsolicited pong is silently discarded
        # instead of being handed to the application
        client = WebSocketClient(self.port)
        client.send(0xA, b"unsolicited")
        client.send(0x1, "after".encode("utf-8"))
        self.assertEqual(client.receive(), (0x1, b"echo:after"))
        client.close()

    def test_close_by_client(self):
        # verifies that the closing handshake is answered with the
        # corresponding close frame carrying a normal code
        client = WebSocketClient(self.port)
        client.send(0x8, struct.pack("!H", 1000))
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1000)
        client.close()

    def test_close_by_application(self):
        # verifies that the application is able to close the
        # connection carrying both a code and a reason
        client = WebSocketClient(self.port, path="/ws-close")
        client.send(0x1, "bye".encode("utf-8"))
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 4001)
        self.assertEqual(payload[2:], b"done")
        client.close()

    def test_rejected(self):
        # verifies that an application that closes before accepting
        # has the handshake refused instead of completed
        client = WebSocketClient(self.port, path="/ws-reject")
        self.assertEqual(client.status, 403)
        client.close()

    def test_unmasked_frame(self):
        # verifies that a frame that is not masked is rejected as a
        # violation of the protocol, closing the connection
        client = WebSocketClient(self.port)
        client.send_raw(bytes([0x81, 0x02]) + b"hi")
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1002)
        client.close()

    def test_oversized_frame(self):
        # verifies that a frame announcing a payload beyond the
        # maximum allowed one is rejected before being buffered
        client = WebSocketClient(self.port)
        client.send_raw(
            bytes([0x82, 0xFF]) + struct.pack("!Q", 1 << 32) + b"\x01\x02\x03\x04"
        )
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1002)
        client.close()

    def test_unexpected_continuation(self):
        # verifies that a continuation frame that continues nothing
        # is rejected as a violation of the protocol
        client = WebSocketClient(self.port)
        client.send(0x0, b"orphan")
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1002)
        client.close()

    def test_interleaved_message(self):
        # verifies that a new message may not start while another
        # one is still being reassembled
        client = WebSocketClient(self.port)
        client.send(0x1, b"first", fin=False)
        client.send(0x1, b"second")
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1002)
        client.close()

    def test_invalid_text_payload(self):
        # verifies that a textual message that is not properly
        # encoded closes the connection with the invalid code
        client = WebSocketClient(self.port)
        client.send(0x1, b"\xff\xfe")
        opcode, payload = client.receive()
        self.assertEqual(opcode, 0x8)
        self.assertEqual(struct.unpack("!H", payload[:2])[0], 1007)
        client.close()

    def test_partial_frame(self):
        # verifies that a frame that arrives split over several
        # receptions is only handled once it is complete
        client = WebSocketClient(self.port)
        client.send_raw(bytes([0x81, 0x85, 0x01]))
        time.sleep(0.2)
        payload = "hello".encode("utf-8")
        mask = b"\x01\x02\x03\x04"
        client.send_raw(
            mask[1:] + bytes(payload[index] ^ mask[index % 4] for index in range(5))
        )
        self.assertEqual(client.receive(), (0x1, b"echo:hello"))
        client.close()


if __name__ == "__main__":
    unittest.main()
