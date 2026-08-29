#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Runs the interoperability checks of HTTP/2 against the server, the
# clients of them being the ones a deployment actually meets, and
# reports the protocol that each one of them ends up speaking.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.interop}
OUTPUT=${OUTPUT:-$ROOT/interop}
PORT=${PORT:-9293}
PORT_SSL=${PORT_SSL:-9294}
BINARY=${BINARY:-$BUILD/bin/viriatum}

# builds the server in the release shape, the clients are pointed at
# the binary that is actually shipped
if [ ! -x "$BINARY" ]; then
    echo "Building the server ..."
    cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$BUILD" --target viriatum -j 4
fi

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT/www" "$OUTPUT/cert"
echo "viriatum" > "$OUTPUT/www/index.html"

if command -v openssl > /dev/null 2>&1; then
    openssl req -x509 -newkey rsa:2048 -nodes -days 1 \
        -subj "/CN=localhost" \
        -keyout "$OUTPUT/cert/server.key" \
        -out "$OUTPUT/cert/server.crt" > /dev/null 2>&1
fi

PID=""
PID_SSL=""

_stop() {
    if [ -n "$PID" ]; then kill "$PID" 2> /dev/null || true; fi
    if [ -n "$PID_SSL" ]; then kill "$PID_SSL" 2> /dev/null || true; fi
}
trap _stop EXIT INT TERM

_wait() {
    INDEX=0
    while [ "$INDEX" -lt 50 ]; do
        if command -v nc > /dev/null 2>&1; then
            if nc -z 127.0.0.1 "$1" > /dev/null 2>&1; then return 0; fi
        else
            if curl -s -o /dev/null --max-time 1 "http://127.0.0.1:$1/"; then return 0; fi
        fi
        sleep 0.2
        INDEX=$((INDEX + 1))
    done
    echo "the server never came up on port $1" >&2
    return 1
}

# the number of the checks that have run and of the ones that did not
# hold, the second is what decides the status of the run
CHECKS=0
FAILED=0

# records the result of a single check, comparing what the client has
# reported against what it was expected to report
_check() {
    NAME=$1
    EXPECTED=$2
    ACTUAL=$3
    CHECKS=$((CHECKS + 1))
    if [ "$ACTUAL" = "$EXPECTED" ]; then
        echo "  ok    $NAME ($ACTUAL)"
        echo "| \`$NAME\` | $ACTUAL | ok |" >> "$OUTPUT/summary.md"
    else
        echo "  not ok $NAME (expected $EXPECTED, got $ACTUAL)"
        echo "| \`$NAME\` | $ACTUAL | expected $EXPECTED |" >> "$OUTPUT/summary.md"
        FAILED=$((FAILED + 1))
    fi
}

{
    echo "## Interoperability"
    echo
    echo "| client | result | status |"
    echo "| --- | --- | --- |"
} > "$OUTPUT/summary.md"

cd "$OUTPUT"

echo "Starting the server on port $PORT ..."
"$BINARY" --port="$PORT" --wwwroot="$OUTPUT/www" < /dev/null > "$OUTPUT/server.log" 2>&1 &
PID=$!
_wait "$PORT"

echo "Running the interoperability checks over h2c ..."

# the prior knowledge form opens with the preface and expects the
# server to be speaking HTTP/2 from the very first byte
_check "curl --http2-prior-knowledge" 2 "$(
    curl -s -o /dev/null -w '%{http_version}' \
        --http2-prior-knowledge "http://127.0.0.1:$PORT/index.html"
)"

# the very same port keeps serving the older version of the protocol,
# the two are told apart by the bytes that open the connection
_check "curl --http1.1" 1.1 "$(
    curl -s -o /dev/null -w '%{http_version}' \
        --http1.1 "http://127.0.0.1:$PORT/index.html"
)"

_check "curl --http2-prior-knowledge (status)" 200 "$(
    curl -s -o /dev/null -w '%{http_code}' \
        --http2-prior-knowledge "http://127.0.0.1:$PORT/index.html"
)"

if command -v nghttp > /dev/null 2>&1; then
    _check "nghttp" "viriatum" "$(nghttp "http://127.0.0.1:$PORT/index.html" 2> /dev/null | tr -d '\n')"
fi

# the multiplexing is exercised by a client that carries several
# streams over each one of the connections it opens
if command -v h2load > /dev/null 2>&1; then
    _check "h2load" "0" "$(
        h2load -n 100 -c 4 -m 10 "http://127.0.0.1:$PORT/index.html" 2> /dev/null |
            sed -n 's/.*, \([0-9][0-9]*\) failed.*/\1/p'
    )"
fi

# the negotiated form is only reachable when the transport is built
# into the binary and a certificate could be generated for it
if [ -f "$OUTPUT/cert/server.crt" ] && head -1 "$OUTPUT/server.log" | grep -q ssl; then
    echo "Starting the server on port $PORT_SSL ..."
    "$BINARY" --port="$PORT_SSL" --ssl --wwwroot="$OUTPUT/www" < /dev/null > "$OUTPUT/server-ssl.log" 2>&1 &
    PID_SSL=$!
    _wait "$PORT_SSL"

    echo "Running the interoperability checks over h2 ..."

    _check "curl --http2" 2 "$(
        curl -s -k -o /dev/null -w '%{http_version}' \
            --http2 "https://127.0.0.1:$PORT_SSL/index.html"
    )"

    # the order the peer announces is the one honoured, so a client
    # that asks for the older version is served it
    _check "curl --http1.1 (tls)" 1.1 "$(
        curl -s -k -o /dev/null -w '%{http_version}' \
            --http1.1 "https://127.0.0.1:$PORT_SSL/index.html"
    )"

    if command -v openssl > /dev/null 2>&1; then
        _check "openssl -alpn h2" "h2" "$(
            echo | openssl s_client -connect "127.0.0.1:$PORT_SSL" -alpn h2 2> /dev/null |
                sed -n 's/^ALPN protocol: //p'
        )"
        _check "openssl -alpn http/1.1" "http/1.1" "$(
            echo | openssl s_client -connect "127.0.0.1:$PORT_SSL" -alpn http/1.1 2> /dev/null |
                sed -n 's/^ALPN protocol: //p'
        )"
    fi
fi

_stop
PID=""
PID_SSL=""

cat "$OUTPUT/summary.md"
echo "Ran $CHECKS checks, $FAILED of them did not hold"

if [ "$FAILED" -ne 0 ]; then exit 1; fi

echo "Interoperability reports written to $OUTPUT"
