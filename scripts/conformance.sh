#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Runs the h2spec conformance suite against the server, over both the
# cleartext form of HTTP/2 and the negotiated one, and compares the
# number of the cases that pass against the recorded baseline.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.conformance}
OUTPUT=${OUTPUT:-$ROOT/conformance}
BASELINE=${BASELINE:-$ROOT/scripts/conformance.baseline}
PORT=${PORT:-9291}
PORT_SSL=${PORT_SSL:-9292}
BINARY=${BINARY:-$BUILD/bin/viriatum}

if ! command -v h2spec > /dev/null 2>&1; then
    echo "conformance requires the h2spec tool" >&2
    exit 1
fi

if ! command -v curl > /dev/null 2>&1; then
    echo "conformance requires the curl tool" >&2
    exit 1
fi

# builds the server in the release shape, the conformance of it is
# measured over the binary that is actually shipped
if [ ! -x "$BINARY" ]; then
    echo "Building the server ..."
    cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$BUILD" --target viriatum -j 4
fi

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT/www" "$OUTPUT/cert"
echo "viriatum" > "$OUTPUT/www/index.html"

# generates the certificate of the run, the one that ships with the
# project carries a key that a library of the current generation
# refuses, and the negotiation of the protocol needs a handshake
if command -v openssl > /dev/null 2>&1; then
    openssl req -x509 -newkey rsa:2048 -nodes -days 1 \
        -subj "/CN=localhost" \
        -keyout "$OUTPUT/cert/server.key" \
        -out "$OUTPUT/cert/server.crt" > /dev/null 2>&1
fi

# the identifiers of the servers that are started along the run, they
# are taken down by the trap no matter how the run ends
PID=""
PID_SSL=""

_stop() {
    if [ -n "$PID" ]; then kill "$PID" 2> /dev/null || true; fi
    if [ -n "$PID_SSL" ]; then kill "$PID_SSL" 2> /dev/null || true; fi
}
trap _stop EXIT INT TERM

# waits for the provided port to accept a connection, a server that
# never comes up is a failure of the run rather than of the suite
_wait() {
    INDEX=0
    while [ "$INDEX" -lt 50 ]; do
        # the readiness is decided by a complete exchange rather than
        # by the socket merely accepting, one that only accepts is up
        # before it is able to serve and the suite that follows it
        # then races the setting up of the transport, which showed up
        # as the synchronisation of the settings failing now and then
        if curl -s -k -o /dev/null --max-time 2 "${2:-http}://127.0.0.1:$1/"; then
            return 0
        fi
        sleep 0.2
        INDEX=$((INDEX + 1))
    done
    echo "the server never came up on port $1" >&2
    return 1
}

# runs the suite with the provided arguments and reports the counts of
# it, the output of the run is kept for the reading of a failure
_run() {
    NAME=$1
    shift
    if ! h2spec "$@" -P /index.html > "$OUTPUT/$NAME.txt" 2>&1; then
        echo "the suite reported a failure over $NAME" >&2
    fi
    tail -1 "$OUTPUT/$NAME.txt"
}

cd "$OUTPUT"

echo "Starting the server on port $PORT ..."
"$BINARY" --port="$PORT" --wwwroot="$OUTPUT/www" < /dev/null > "$OUTPUT/server.log" 2>&1 &
PID=$!
_wait "$PORT"

echo "Running the conformance suite over h2c ..."
H2C=$(_run h2c -h 127.0.0.1 -p "$PORT")
echo "$H2C"

# the negotiated form is only reachable when the transport is built
# into the binary and a certificate could be generated for it
H2=""
if [ -f "$OUTPUT/cert/server.crt" ] && head -1 "$OUTPUT/server.log" | grep -q ssl; then
    echo "Starting the server on port $PORT_SSL ..."
    "$BINARY" --port="$PORT_SSL" --ssl --wwwroot="$OUTPUT/www" < /dev/null > "$OUTPUT/server-ssl.log" 2>&1 &
    PID_SSL=$!
    _wait "$PORT_SSL" https

    echo "Running the conformance suite over h2 ..."
    H2=$(_run h2 -t -k -h 127.0.0.1 -p "$PORT_SSL")
    echo "$H2"
fi

_stop
PID=""
PID_SSL=""

# gathers the numbers of both of the runs out of the line that closes
# each one of them, the shape of it is fixed by the tool
_count() {
    echo "$1" | sed -n "s/.*[, ]\([0-9][0-9]*\) $2.*/\1/p"
}

PASSED=$(_count "$H2C" passed)
FAILED=$(_count "$H2C" failed)
PASSED=${PASSED:-0}
FAILED=${FAILED:-0}

if [ -n "$H2" ]; then
    PASSED_SSL=$(_count "$H2" passed)
    FAILED_SSL=$(_count "$H2" failed)
    PASSED_SSL=${PASSED_SSL:-0}
    FAILED_SSL=${FAILED_SSL:-0}
else
    PASSED_SSL=0
    FAILED_SSL=0
fi

# builds the table of the run, it is written to the step summary of
# the workflow so that the numbers show up on the run page
{
    echo "## Conformance"
    echo
    echo "| transport | result |"
    echo "| --- | --- |"
    echo "| \`h2c\` | $H2C |"
    if [ -n "$H2" ]; then echo "| \`h2\` | $H2 |"; fi
} > "$OUTPUT/summary.md"
cat "$OUTPUT/summary.md"

# the recorded baseline is the number of the cases that passed the
# last time the suite was accepted, a run below it is a regression
if [ -f "$BASELINE" ]; then
    EXPECTED=$(cat "$BASELINE")
else
    EXPECTED=0
fi

echo "Cases passed over h2c: $PASSED, baseline is $EXPECTED"

if [ "$FAILED" -ne 0 ] || [ "$FAILED_SSL" -ne 0 ]; then
    echo "The conformance suite reported failures" >&2
    exit 1
fi

if [ "$PASSED" -lt "$EXPECTED" ]; then
    echo "The conformance suite passed fewer cases than the baseline" >&2
    exit 1
fi

echo "Conformance reports written to $OUTPUT"
