#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Drives the server and the reference ones through the very same
# workloads on the very same machine and reports them side by side,
# the number that survives a noisy runner being the ratio of one
# against the other rather than either of them on its own.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.benchmark}
OUTPUT=${OUTPUT:-$ROOT/benchmark}
BASELINE=${BASELINE:-$ROOT/scripts/benchmark/baseline.json}
PYTHON=${PYTHON:-python3}
DURATION=${DURATION:-5}
CONNECTIONS=${CONNECTIONS:-64}
THREADS=${THREADS:-4}
RATE=${RATE:-60}
REPEATS=${REPEATS:-3}
WORKERS=${WORKERS:-2}
PORT=${PORT:-9410}
COMPILER=${CC:-cc}

# the ports of the pieces that are started around the subject, a
# reference and the upstream take one of their own so that none of
# them ever contends with the subject for the same one
PORT_REFERENCE=$((PORT + 1))
PORT_UPSTREAM=$((PORT + 2))

# the directory that carries the fixtures and the configurations of
# the reference servers, everything the run needs that it does not
# generate for itself
ASSETS=$ROOT/scripts/benchmark

# the connections that the measurement of the accept path opens, it
# is a count rather than a duration because the connections are the
# thing being measured and not the seconds they take
ACCEPTS=${ACCEPTS:-2000}

# the seconds a run waits for the ports of the machine to come back
# between one workload and the next, together with the count of the
# ones still held that a measurement is allowed to start on top of
SETTLE=${SETTLE:-60}
WAITING=${WAITING:-2000}

# the workloads the harness is asked to drive, an empty value stands
# for every one of them, a name filters the run down to it
ONLY=${ONLY:-}

if ! command -v wrk > /dev/null 2>&1; then
    echo "benchmarking requires the wrk tool" >&2
    exit 1
fi

# the generator that holds a fixed rate is the only one whose tail
# means anything, a machine without it still reports the throughput
# and says in the report that no corrected percentile was taken
WRK2=${WRK2:-$(command -v wrk2 || true)}
if [ -z "$WRK2" ] && [ -x "$BUILD/wrk2/wrk" ]; then WRK2=$BUILD/wrk2/wrk; fi

# the images the references are pinned to, so that a figure may always
# be traced back to the exact build that produced it, the pulling of
# them belongs to the workflow and never to a run, a run that pulled
# would sit on a slow registry for minutes before measuring anything
IMAGE_NGINX=${IMAGE_NGINX:-nginx:1.27-alpine}
IMAGE_CADDY=${IMAGE_CADDY:-caddy:2.8-alpine}
IMAGE_HAPROXY=${IMAGE_HAPROXY:-haproxy:3.0-alpine}
IMAGE_LITESPEED=${IMAGE_LITESPEED:-litespeedtech/openlitespeed:1.8.1-lsphp81}
IMAGE_SUBJECT=${IMAGE_SUBJECT:-viriatum-benchmark:local}

DOCKER=${DOCKER:-$(command -v docker || true)}
if [ -n "$DOCKER" ] && ! "$DOCKER" info > /dev/null 2>&1; then DOCKER=""; fi

# the references run either out of an image or out of the binary that
# the machine carries, and whichever of the two it is the subject is
# run the same way, a subject reached over one stack and a reference
# reached over another measure the stacks and never the servers, which
# is exactly what a container on a machine whose daemon lives inside a
# virtual one of its own would end up doing
if [ -n "$DOCKER" ] && "$DOCKER" image inspect "$IMAGE_NGINX" > /dev/null 2>&1; then
    REFERENCES=${REFERENCES:-container}
else
    REFERENCES=${REFERENCES:-native}
fi

# the subject only ever runs inside a container when it has to, which
# is when the references do and the machine is not the one the daemon
# runs on, on linux the two share a kernel and a network already
if [ "$REFERENCES" = "container" ] && [ "$(uname -s)" != "Linux" ]; then
    MODE=${MODE:-container}
else
    MODE=${MODE:-native}
fi

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT/runs" "$OUTPUT/logs" "$OUTPUT/www/listing"

echo "Running the harness in the $MODE mode ..."

# builds the server in the release shape, a figure taken out of a
# debug build measures the counters of that build and nothing else
if [ ! -x "$BUILD/bin/viriatum" ]; then
    echo "Building the server ..."
    cmake -S "$ROOT" -B "$BUILD" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER="$COMPILER" > "$OUTPUT/logs/build.log" 2>&1
    cmake --build "$BUILD" --target viriatum -j 4 >> "$OUTPUT/logs/build.log" 2>&1
fi

BINARY=${BINARY:-$BUILD/bin/viriatum}

# ---------------------------------------------------------------------------
# the fixtures of the run
# ---------------------------------------------------------------------------

# the three sizes of the static workload, the small one fits inside a
# single packet, the middle one is the illustration that ships with
# the project and the large one is generated instead of committed
cp "$ASSETS/www/small.html" "$OUTPUT/www/small.html"
cp "$ROOT/src/viriatum/resources/html/base/resources/images/illustration/main-illustration.png" \
    "$OUTPUT/www/mid.png"
"$PYTHON" -c "
import sys

with open(sys.argv[1], 'wb') as file:
    file.write(b'viriatum' * (1024 * 1024 // 8))
" "$OUTPUT/www/large.bin"

# the directory the listing workload walks, it carries enough entries
# that the building of the page is what ends up being measured
INDEX=0
while [ "$INDEX" -lt 32 ]; do
    cp "$OUTPUT/www/small.html" "$OUTPUT/www/listing/entry-$INDEX.html"
    INDEX=$((INDEX + 1))
done

SIZE_SMALL=$(wc -c < "$OUTPUT/www/small.html" | tr -d ' ')
SIZE_MID=$(wc -c < "$OUTPUT/www/mid.png" | tr -d ' ')
SIZE_LARGE=$(wc -c < "$OUTPUT/www/large.bin" | tr -d ' ')

# ---------------------------------------------------------------------------
# the pieces that live around a measurement
# ---------------------------------------------------------------------------

# everything the run starts, taken down by the trap no matter how the
# run itself ends up finishing
PID=""
PID_REFERENCE=""
PID_UPSTREAM=""

_stop_subject() {
    if [ -n "$PID" ]; then
        kill "$PID" 2> /dev/null || true
        wait "$PID" 2> /dev/null || true
        PID=""
    fi
    if [ -n "$DOCKER" ]; then
        "$DOCKER" rm -f viriatum-subject > /dev/null 2>&1 || true
    fi
}

_stop_reference() {
    if [ -n "$PID_REFERENCE" ]; then
        kill "$PID_REFERENCE" 2> /dev/null || true
        wait "$PID_REFERENCE" 2> /dev/null || true
        PID_REFERENCE=""
    fi
    # the reference that forks a set of workers of its own leaves them
    # behind when only the one that was started is taken down
    pkill -f "$OUTPUT/conf/" > /dev/null 2>&1 || true
    if [ -n "$DOCKER" ]; then
        "$DOCKER" rm -f viriatum-reference > /dev/null 2>&1 || true
    fi
}

_stop_upstream() {
    if [ -n "$PID_UPSTREAM" ]; then
        kill "$PID_UPSTREAM" 2> /dev/null || true
        wait "$PID_UPSTREAM" 2> /dev/null || true
        PID_UPSTREAM=""
    fi
    if [ -n "$DOCKER" ]; then
        "$DOCKER" rm -f viriatum-upstream > /dev/null 2>&1 || true
    fi
}

_stop() {
    _stop_subject
    _stop_reference
    _stop_upstream
}
trap _stop EXIT INT TERM

# waits for the provided port to answer, a server that never comes up
# is a failure of the run rather than a reported figure of zero
_wait() {
    _INDEX=0
    while [ "$_INDEX" -lt 100 ]; do
        if curl -s -o /dev/null --max-time 1 "http://127.0.0.1:$1/" 2> /dev/null; then return 0; fi
        sleep 0.1
        _INDEX=$((_INDEX + 1))
    done
    echo "nothing came up on port $1" >&2
    return 1
}

# writes the configuration the subject reads for the workload at hand,
# the server picks the file up out of the directory it is started in
_configure() {
    _HANDLER=$1
    _TEMPLATE=$2
    _PROXY=$3
    {
        echo "[general]"
        echo "host = 127.0.0.1"
        echo "port = $PORT"
        echo "ip6 = Off"
        echo "ssl = Off"
        echo "workers = $WORKERS"
        echo "index = index.html"
        echo "use_template = $_TEMPLATE"
        echo "handler = $_HANDLER"
        if [ -n "$_PROXY" ]; then
            echo
            echo "[location:proxy]"
            echo "path = /"
            echo "handler = proxy"
            echo "proxy_pass = $_PROXY"
        fi
    } > "$OUTPUT/viriatum.ini"
}

# starts the subject for the workload at hand, the handler being the
# one the workload names and the configuration the one just written
_start_subject() {
    _HANDLER=$1
    cd "$OUTPUT"
    if [ "$MODE" = "container" ]; then
        "$DOCKER" run -d --name viriatum-subject --network host \
            -v "$OUTPUT:/bench" -w /bench "$IMAGE" \
            viriatum --port="$PORT" --handler="$_HANDLER" \
            --wwwroot=/bench/www --workers="$WORKERS" \
            > "$OUTPUT/logs/subject.id" 2>&1
    else
        "$BINARY" --port="$PORT" --handler="$_HANDLER" \
            --wwwroot="$OUTPUT/www" --workers="$WORKERS" \
            < /dev/null > "$OUTPUT/logs/subject.log" 2>&1 &
        PID=$!
    fi
    _wait "$PORT"
}

# ---------------------------------------------------------------------------
# the reference servers
# ---------------------------------------------------------------------------

# writes the configuration of a reference out of the template of it,
# the values that decide the comparison are the ones the subject is
# run with and are put in place here rather than being written twice
_render() {
    sed -e "s|@PORT@|$PORT_REFERENCE|g" \
        -e "s|@UPSTREAM@|$PORT_UPSTREAM|g" \
        -e "s|@WORKERS@|$WORKERS|g" \
        -e "s|@ROOT@|$_PREFIX|g" \
        "$ASSETS/conf/$1" > "$OUTPUT/conf/$2"
}

# starts a reference out of its image, every one of them is given the
# network of the machine so that it is reached exactly the way the
# subject is and no address translation sits in between
_run_image() {
    "$DOCKER" run -d --name viriatum-reference --network host \
        -v "$OUTPUT:/bench" "$@" > "$OUTPUT/logs/reference.id" 2>&1
}

# reports the version of a reference, the exact build that produced a
# figure is part of the figure and is recorded next to it
_version() {
    case $1 in
        nginx) nginx -v 2>&1 | sed 's|.*/||' ;;
        caddy) caddy version 2> /dev/null | head -1 | awk '{ print $1 }' ;;
        haproxy) haproxy -v 2> /dev/null | head -1 | awk '{ print $3 }' ;;
        gunicorn) "$PYTHON" -c "import gunicorn; print(gunicorn.__version__)" 2> /dev/null ;;
        uvicorn) "$PYTHON" -c "import uvicorn; print(uvicorn.__version__)" 2> /dev/null ;;
        *) echo unknown ;;
    esac
}

# says whether a reference may be driven at all on this machine, one
# whose image and binary are both missing is skipped rather than
# failing the run, in the very same spirit the modules are skipped by
# the coverage script when their dependency is not around
_has_reference() {
    case $1 in
        nginx) _IMAGE=$IMAGE_NGINX; _BINARY=nginx ;;
        caddy) _IMAGE=$IMAGE_CADDY; _BINARY=caddy ;;
        haproxy) _IMAGE=$IMAGE_HAPROXY; _BINARY=haproxy ;;
        openlitespeed) _IMAGE=$IMAGE_LITESPEED; _BINARY=__none__ ;;
        pingora) _IMAGE=__none__; _BINARY=$BUILD/pingora/load_balancer ;;
        gunicorn | uvicorn)
            "$PYTHON" -c "import $1" > /dev/null 2>&1 && return 0
            return 1
            ;;
        *) return 1 ;;
    esac
    if [ "$REFERENCES" = "container" ] && [ -n "$DOCKER" ] &&
        "$DOCKER" image inspect "$_IMAGE" > /dev/null 2>&1; then
        return 0
    fi
    if [ -x "$_BINARY" ] || command -v "$_BINARY" > /dev/null 2>&1; then return 0; fi
    return 1
}

# starts the reference that stands for the role of the workload at
# hand, the configuration of it having been matched against the one
# the subject was given so that the comparison is of the servers
_start_reference() {
    _REF=$1
    _ROLE=$2

    # the paths inside a container and the ones on the machine differ,
    # the configuration is written against whichever of the two the
    # reference is about to be started under
    if [ "$REFERENCES" = "container" ]; then _PREFIX=/bench; else _PREFIX=$OUTPUT; fi
    mkdir -p "$OUTPUT/conf"
    cp "$ASSETS/conf/mime.types" "$OUTPUT/conf/mime.types"

    case $_REF in
        nginx)
            if [ "$_ROLE" = "proxy" ]; then
                _render nginx-proxy.conf nginx.conf
            elif [ "$_ROLE" = "default" ]; then
                _render nginx-default.conf nginx.conf
            else
                _render nginx.conf nginx.conf
            fi
            if [ "$REFERENCES" = "container" ]; then
                _run_image "$IMAGE_NGINX" nginx -c /bench/conf/nginx.conf
            else
                nginx -c "$OUTPUT/conf/nginx.conf" -e /dev/null \
                    > "$OUTPUT/logs/reference.log" 2>&1 &
                PID_REFERENCE=$!
            fi
            ;;
        caddy)
            if [ "$_ROLE" = "proxy" ]; then
                _render Caddyfile.proxy Caddyfile
            elif [ "$_ROLE" = "default" ]; then
                _render Caddyfile.default Caddyfile
            else
                _render Caddyfile Caddyfile
            fi
            if [ "$REFERENCES" = "container" ]; then
                _run_image "$IMAGE_CADDY" caddy run --config /bench/conf/Caddyfile
            else
                XDG_CONFIG_HOME=$OUTPUT XDG_DATA_HOME=$OUTPUT \
                    caddy run --config "$OUTPUT/conf/Caddyfile" \
                    > "$OUTPUT/logs/reference.log" 2>&1 &
                PID_REFERENCE=$!
            fi
            ;;
        haproxy)
            _render haproxy.cfg haproxy.cfg
            if [ "$REFERENCES" = "container" ]; then
                _run_image "$IMAGE_HAPROXY" haproxy -f /bench/conf/haproxy.cfg
            else
                haproxy -f "$OUTPUT/conf/haproxy.cfg" \
                    > "$OUTPUT/logs/reference.log" 2>&1 &
                PID_REFERENCE=$!
            fi
            ;;
        gunicorn)
            PYTHONPATH=$ASSETS "$PYTHON" -m gunicorn \
                --bind "127.0.0.1:$PORT_REFERENCE" --workers "$WORKERS" \
                --access-logfile /dev/null --error-logfile /dev/null \
                --keep-alive 65 app:wsgi_app \
                > "$OUTPUT/logs/reference.log" 2>&1 &
            PID_REFERENCE=$!
            ;;
        uvicorn)
            if [ "$_ROLE" = "asgi-stream" ]; then _APP=app:asgi_stream_app; else _APP=app:asgi_app; fi
            PYTHONPATH=$ASSETS "$PYTHON" -m uvicorn \
                --host 127.0.0.1 --port "$PORT_REFERENCE" --workers "$WORKERS" \
                --log-level critical --no-access-log "$_APP" \
                > "$OUTPUT/logs/reference.log" 2>&1 &
            PID_REFERENCE=$!
            ;;
        *) return 1 ;;
    esac

    _wait "$PORT_REFERENCE"
}

# ---------------------------------------------------------------------------
# the driving of a measurement
# ---------------------------------------------------------------------------

# reports the median together with the quartiles of the values fed to
# it, a single noisy sample never moves a median and the spread of the
# samples is what says whether the median may be trusted at all
_stats() {
    sort -n | awk '
        function percentile(quantile,   position, lower, upper, fraction) {
            position = (quantile / 100.0) * (count - 1) + 1
            lower = int(position)
            upper = lower + 1
            fraction = position - lower
            if (upper > count) { return values[count] }
            return values[lower] + fraction * (values[upper] - values[lower])
        }
        { count = NR; values[NR] = $1 }
        END {
            if (count == 0) { print "0 0 0"; exit }
            printf "%.3f %.3f %.3f\n", percentile(50), percentile(25), percentile(75)
        }'
}

# reports the resident memory and the consumed processor time of the
# subject together with every worker it has forked, the two of them
# are what says whether a win of throughput was paid for elsewhere
_sample() {
    _MATCH=$1
    if [ "$MODE" = "container" ]; then
        "$DOCKER" stats --no-stream --format "{{.MemUsage}}" viriatum-subject 2> /dev/null |
            awk '{ printf "%.0f 0\n", $1 * 1024 }'
        return 0
    fi
    # shellcheck disable=SC2009
    ps -A -o rss=,time=,command= 2> /dev/null | grep "$_MATCH" | grep -v grep | awk '
        {
            split($2, parts, ":")
            seconds = 0
            if (length(parts) == 3) {
                seconds = parts[1] * 3600 + parts[2] * 60 + parts[3]
            } else if (length(parts) == 2) {
                seconds = parts[1] * 60 + parts[2]
            }
            memory += $1
            processor += seconds
        }
        END { printf "%.0f %.3f\n", memory, processor }'
}

# drives the generator against the provided target, the run before the
# measured ones is thrown away so that a cold cache and a server whose
# structures have not yet grown never land inside a reported figure
_drive() {
    _TARGET=$1
    _HEADER=$2
    _REPORTS=$3

    if [ -n "$_HEADER" ]; then
        set -- -t "$THREADS" -c "$CONNECTIONS" -d "${DURATION}s" -H "$_HEADER"
    else
        set -- -t "$THREADS" -c "$CONNECTIONS" -d "${DURATION}s"
    fi

    BENCHMARK_REPORT=/dev/null wrk "$@" -s "$ASSETS/report.lua" \
        "$_TARGET" > /dev/null 2>&1 || true

    _INDEX=0
    while [ "$_INDEX" -lt "$REPEATS" ]; do
        BENCHMARK_REPORT="$_REPORTS/repeat-$_INDEX.json" wrk "$@" \
            -s "$ASSETS/report.lua" "$_TARGET" \
            > "$_REPORTS/repeat-$_INDEX.txt" 2>&1 || true
        _INDEX=$((_INDEX + 1))
    done
}

# reads a single number out of the reports of a measurement, their
# shape is fixed by the hook that writes them so that a plain match is
# enough and no parser of its own has to be carried into the shell
_values() {
    grep -h "\"$2\":" "$1"/repeat-*.json 2> /dev/null |
        sed -n 's/.*: *\([0-9.]*\).*/\1/p'
}

# sums the values of every key that the provided pattern matches over
# all of the repeats of a measurement
_sum() {
    grep -h -E "$2" "$1"/repeat-*.json 2> /dev/null |
        sed -n 's/.*: *\([0-9]*\).*/\1/p' |
        awk '{ total += $1 } END { printf "%d\n", total }'
}

# reports the sockets that are sitting in the state a closed one is
# held in, a workload that closes every connection walks through the
# ephemeral ports of the machine and leaves them all behind
_waiting() {
    if command -v ss > /dev/null 2>&1; then
        ss -tan state time-wait 2> /dev/null | grep -c . || true
    else
        netstat -an -p tcp 2> /dev/null | grep -c TIME_WAIT || true
    fi
}

# waits for the ports of the machine to come back before the next of
# the measurements is driven, a workload started on an exhausted table
# measures the exhaustion and never the server, which showed up as a
# run of two requests a second sitting behind sixty failed connections
_settle() {
    _INDEX=0
    while [ "$_INDEX" -lt "$SETTLE" ]; do
        if [ "$(_waiting)" -lt "$WAITING" ]; then return 0; fi
        sleep 1
        _INDEX=$((_INDEX + 1))
    done
    echo "  the ports of the machine did not come back within ${SETTLE}s" >&2
}

# ---------------------------------------------------------------------------
# the workloads
# ---------------------------------------------------------------------------

# one row per workload, carrying the role that decides which of the
# references stand for it, the handler it is served by, the path it
# asks for, the header that decides whether the connection is kept,
# the state of the template engine and the upstream of a proxy
_workloads() {
    cat << EOF
static-small-alive|static|file|/small.html||On|
static-small-close|static|file|/small.html|Connection: close|On|
static-mid-alive|static|file|/mid.png||On|
static-mid-close|static|file|/mid.png|Connection: close|On|
static-large-alive|static|file|/large.bin||On|
static-large-close|static|file|/large.bin|Connection: close|On|
default-alive|default|default|/||On|
default-close|default|default|/|Connection: close|On|
listing-template|listing|file|/listing/||On|
listing-plain|listing|file|/listing/||Off|
error-template|error|file|/missing||On|
error-plain|error|file|/missing||Off|
proxy-alive|proxy|dispatch|/small.html||On|http://127.0.0.1:$PORT_UPSTREAM/
proxy-close|proxy|dispatch|/small.html|Connection: close|On|http://127.0.0.1:$PORT_UPSTREAM/
EOF
}

# the references that stand for each of the roles, a reference is only
# ever put on a workload it is actually able to serve, haproxy holds
# no file of its own and pingora is a framework rather than a server,
# so the two of them are only ever measured in front of an upstream
_role_references() {
    case $1 in
        static | listing | error) echo "nginx caddy openlitespeed" ;;
        default) echo "nginx caddy" ;;
        proxy) echo "nginx haproxy caddy pingora" ;;
        wsgi) echo "gunicorn" ;;
        asgi | asgi-stream) echo "uvicorn" ;;
        *) echo "" ;;
    esac
}

echo "Fixtures are $SIZE_SMALL, $SIZE_MID and $SIZE_LARGE bytes"
echo "Driving $REPEATS runs of ${DURATION}s over $CONNECTIONS connections"
echo

# ---------------------------------------------------------------------------
# the run
# ---------------------------------------------------------------------------

# records one measured pair as a report of its own, the assembly of
# every one of them into a table is left to the builder of it
_record() {
    _NAME=$1
    _SERVER=$2
    _REPORTS=$3
    _VERSION=$4

    set -- $(_values "$_REPORTS" rps | _stats)
    _RPS=$1
    _RPS_LOW=$2
    _RPS_HIGH=$3

    set -- $(_values "$_REPORTS" transfer_bps | _stats)
    _TRANSFER=$1

    # the two kinds of error are counted apart, the ones of the socket
    # say that the machine ran out of something and make a figure
    # worthless, the ones of the status are what the workload of the
    # error page is asking for and are perfectly expected there
    _SOCKET=$(_sum "$_REPORTS" '"(connect|read|write|timeout)":')
    _STATUS=$(_sum "$_REPORTS" '"status":')

    # a measurement that lost a noticeable part of its connections is
    # never reported as a figure, an exhausted port table answers far
    # faster than a server does and would read as a win
    _REQUESTS=$(_values "$_REPORTS" requests | awk '{ total += $1 } END { print total + 0 }')
    _VALID=$(echo "$_SOCKET $_REQUESTS" | awk \
        '{ print ($1 * 1000 > $2) ? "false" : "true" }')

    {
        echo "{"
        echo "  \"workload\": \"$_NAME\","
        echo "  \"server\": \"$_SERVER\","
        echo "  \"version\": \"$_VERSION\","
        echo "  \"valid\": $_VALID,"
        echo "  \"rps\": $_RPS,"
        echo "  \"rps_low\": $_RPS_LOW,"
        echo "  \"rps_high\": $_RPS_HIGH,"
        echo "  \"transfer_bps\": $_TRANSFER,"
        echo "  \"errors_socket\": ${_SOCKET:-0},"
        echo "  \"errors_status\": ${_STATUS:-0},"
        echo "  \"peak_rss_kb\": ${_RSS:-0},"
        echo "  \"cpu_ms_per_k\": ${_CPU:-0}"
        echo "}"
    } > "$OUTPUT/runs/$_NAME.$_SERVER.json"

    if [ "$_VALID" = "true" ]; then
        printf "  %-12s %12.0f req/s  %8.2f MB/s\n" "$_SERVER" "$_RPS" \
            "$(echo "$_TRANSFER" | awk '{ printf "%.2f", $1 / 1048576 }')"
    else
        printf "  %-12s %12.0f req/s  discarded, %s connections were lost\n" \
            "$_SERVER" "$_RPS" "$_SOCKET"
    fi
}

VERSION=$("$BINARY" --version 2> /dev/null | tail -1 | sed 's/viriatum - //;s/ (.*//')

# the rows are walked out of a file rather than out of a pipe, a pipe
# would run the loop inside a subshell of its own and the identifiers
# of the servers it starts would never reach the trap that stops them
_workloads > "$OUTPUT/workloads.txt"

while IFS='|' read -r NAME ROLE HANDLER PATH_ HEADER TEMPLATE PROXY; do
    if [ -n "$ONLY" ] && [ "$NAME" != "$ONLY" ]; then continue; fi

    echo "Workload $NAME"
    REPORTS=$OUTPUT/runs/$NAME.viriatum
    mkdir -p "$REPORTS"

    # the ports the workload before this one walked through are given
    # the time to come back, a measurement driven on top of them is
    # measuring the table of the machine and not the server at all
    _settle

    # the upstream of the proxy workload is held constant across every
    # server under test, so that what is measured is the proxying and
    # never the thing that sits behind it, it answers out of the
    # cheapest handler there is because an upstream that saturates
    # before the proxies do reports its own ceiling for all of them,
    # which it did while it was serving the requests off the disk
    if [ -n "$PROXY" ]; then
        "$BINARY" --port="$PORT_UPSTREAM" --handler=default \
            --workers="$WORKERS" \
            < /dev/null > "$OUTPUT/logs/upstream.log" 2>&1 &
        PID_UPSTREAM=$!
        _wait "$PORT_UPSTREAM"
    fi

    _configure "$HANDLER" "$TEMPLATE" "$PROXY"
    _start_subject "$HANDLER"

    set -- $(_sample "viriatum --port=$PORT ")
    RSS_BEFORE=$1
    CPU_BEFORE=$2

    _drive "http://127.0.0.1:$PORT$PATH_" "$HEADER" "$REPORTS"

    set -- $(_sample "viriatum --port=$PORT ")
    RSS_AFTER=$1
    CPU_AFTER=$2

    # the processor time is reported against a thousand requests so
    # that the workloads may be read against one another, the memory
    # is the highest of the two samples that were taken around the run
    REQUESTS=$(_values "$REPORTS" requests | awk '{ total += $1 } END { print total + 0 }')
    _RSS=$(echo "$RSS_BEFORE $RSS_AFTER" | awk '{ print ($1 > $2) ? $1 : $2 }')
    _CPU=$(echo "$CPU_BEFORE $CPU_AFTER $REQUESTS" | awk \
        '{ if ($3 > 0) printf "%.3f", ($2 - $1) * 1000000.0 / $3; else print 0 }')

    _record "$NAME" viriatum "$REPORTS" "$VERSION"

    _stop_subject

    # every reference that stands for the role of this workload is
    # driven through the very same shape right after the subject and
    # on the very same machine, the two figures only mean anything
    # next to one another and never on their own
    for REFERENCE in $(_role_references "$ROLE"); do
        if ! _has_reference "$REFERENCE"; then
            echo "  $REFERENCE is not available on this machine, skipping it"
            continue
        fi

        REPORTS=$OUTPUT/runs/$NAME.$REFERENCE
        mkdir -p "$REPORTS"

        _settle
        _start_reference "$REFERENCE" "$ROLE"

        set -- $(_sample "$OUTPUT/conf/")
        RSS_BEFORE=$1
        CPU_BEFORE=$2

        _drive "http://127.0.0.1:$PORT_REFERENCE$PATH_" "$HEADER" "$REPORTS"

        set -- $(_sample "$OUTPUT/conf/")
        RSS_AFTER=$1
        CPU_AFTER=$2

        REQUESTS=$(_values "$REPORTS" requests | awk '{ total += $1 } END { print total + 0 }')
        _RSS=$(echo "$RSS_BEFORE $RSS_AFTER" | awk '{ print ($1 > $2) ? $1 : $2 }')
        _CPU=$(echo "$CPU_BEFORE $CPU_AFTER $REQUESTS" | awk \
            '{ if ($3 > 0) printf "%.3f", ($2 - $1) * 1000000.0 / $3; else print 0 }')

        _record "$NAME" "$REFERENCE" "$REPORTS" "$(_version "$REFERENCE")"

        _stop_reference
    done

    _stop_upstream
done < "$OUTPUT/workloads.txt"

echo
echo "Benchmark reports written to $OUTPUT"
