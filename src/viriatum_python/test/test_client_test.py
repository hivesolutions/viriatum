#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Test suite for the client that drives a fixed set of requests at a
server, driving it against a listener of the test itself so that what
reaches the other end of the socket is read back and compared against
what the client was handed.

The messages the client carries are written as text and the socket
takes bytes, so what is exercised here is the encoding of them at the
boundary, which is the step a client that was never ported past the
second version of the language leaves out.

Run from the project root with:
    python -m unittest discover -s src/viriatum_python/test
"""

from contextlib import redirect_stdout
from io import StringIO
from os.path import abspath, dirname, join
from socket import AF_INET, SOCK_STREAM, setdefaulttimeout, socket
from sys import path
from threading import Thread
from unittest import TestCase, main

path.insert(
    0, join(dirname(abspath(__file__)), "..", "..", "..", "scripts", "util", "all")
)

import client_test

SIZE = 4096
""" The size of the buffer the listener reads through, larger than
anything the client of a test is ever going to send """

TIMEOUT = 5.0
""" The seconds either of the two ends is allowed to wait on the
other, a client that writes less than it was going to would leave the
two of them waiting forever and take the whole of the suite with it """


class ClientTestTest(TestCase):
    """
    Test suite for the client of the requests, exercising the writing
    of the messages it carries and the reading of the answer.
    """

    def setUp(self) -> None:
        # opens the listener the client is going to be pointed at and
        # keeps the port it was given, an ephemeral one so that a
        # suite that runs twice in a row never finds it still held
        self.listener = socket(AF_INET, SOCK_STREAM)
        self.listener.bind(("127.0.0.1", 0))
        self.listener.listen(1)
        self.listener.settimeout(TIMEOUT)
        self.port = self.listener.getsockname()[1]
        self.received = b""
        self.waited = False

        # every socket that is opened from here on is held to the same
        # wait, the one the client opens for itself included, so that
        # an exchange that never completes fails the test rather than
        # leaving it sitting on a socket nothing is going to write to
        setdefaulttimeout(TIMEOUT)

        # points the client at the listener and takes the pause
        # between the writes away, the separation of the messages is
        # what the pause is for and no server is reading these
        self.port_previous = client_test.PORT
        self.sleep_previous = client_test.WRITE_SLEEP
        client_test.PORT = self.port
        client_test.WRITE_SLEEP = 0

    def tearDown(self) -> None:
        setdefaulttimeout(None)
        client_test.PORT = self.port_previous
        client_test.WRITE_SLEEP = self.sleep_previous
        self.listener.close()

    def _accept(self, expected: int, answer: bytes = b"") -> Thread:
        # answers the connection of the client on a thread of its own,
        # reading the number of bytes it was told to expect and then
        # closing, the client reads until the other end closes and the
        # two of them would otherwise each wait on the other
        def serve() -> None:
            connection, _address = self.listener.accept()
            try:
                while len(self.received) < expected:
                    try:
                        data = connection.recv(SIZE)
                    except TimeoutError:
                        self.waited = True
                        break
                    if not data:
                        break
                    self.received += data
                if answer:
                    connection.send(answer)
            finally:
                connection.close()

        thread = Thread(target=serve)
        thread.daemon = True
        thread.start()
        return thread

    def _call(self, messages: list[str], answer: bytes = b"") -> str:
        # drives the client at the listener and hands back what it
        # wrote, which is where whatever it read shows up, the two
        # ends are held to the wait so that neither of them is able to
        # leave the other one sitting on a socket forever
        expected = len("".join(messages).encode("utf-8"))
        thread = self._accept(expected, answer)
        stream = StringIO()
        with redirect_stdout(stream):
            client_test.call(messages)
        thread.join(TIMEOUT)
        self.assertFalse(self.waited)
        self.assertFalse(thread.is_alive())
        return stream.getvalue()

    def test_messages(self) -> None:
        # the messages the client carries are text and the socket only
        # ever takes bytes, one carrying bytes already would be sent
        # without the step that this exists to exercise
        for messages in (
            client_test.SIMPLE,
            client_test.COMPLEX,
            client_test.PIPELINING,
        ):
            for message in messages:
                self.assertIsInstance(message, str)

    def test_call(self) -> None:
        # what reaches the other end is the message as it was written,
        # encoded rather than refused, which is what the client did
        # for as long as it was handing text to the socket
        self._call(["GET / HTTP/1.1\r\n\r\n"])
        self.assertEqual(self.received, b"GET / HTTP/1.1\r\n\r\n")

    def test_call_messages(self) -> None:
        # the sets the client ships with are the ones it is driven
        # through, a request written in one go, one broken across
        # several writes and a run of them written back to back
        messages = client_test.SIMPLE + client_test.COMPLEX + client_test.PIPELINING
        self._call(messages)
        self.assertEqual(self.received, "".join(messages).encode("utf-8"))

    def test_call_order(self) -> None:
        # the messages arrive in the order they were handed over, a
        # request broken across writes is only ever read back by a
        # server when the parts of it reach it the way it was cut
        self._call(["Hos", "t: viriatum\r\n"])
        self.assertEqual(self.received, b"Host: viriatum\r\n")

    def test_call_answer(self) -> None:
        # the client reads until the other end closes, an answer that
        # is written before the closing is read rather than the client
        # being left waiting on a socket nothing else is going to use,
        # and what it read is what it writes out, so that a client
        # that stopped reading altogether is told apart from one that
        # read and was handed nothing
        written = self._call(
            ["GET / HTTP/1.1\r\n\r\n"], answer=b"HTTP/1.1 200 OK\r\n\r\n"
        )
        self.assertEqual(self.received, b"GET / HTTP/1.1\r\n\r\n")
        self.assertIn("HTTP/1.1 200 OK", written)

    def test_call_empty(self) -> None:
        # a client handed nothing writes nothing and still reads the
        # other end out, the connecting of it being all that happened
        self._call([])
        self.assertEqual(self.received, b"")


if __name__ == "__main__":
    main()
