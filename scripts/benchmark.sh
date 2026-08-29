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
# means anything, the one that drives as hard as it can reports the
# latency of the requests it managed to send and never the one of the
# requests it was held back from sending, which is the omission the
# percentiles of a saturated run quietly leave out
WRK2=${WRK2:-$(command -v wrk2 || true)}
if [ -z "$WRK2" ] && [ -x "$BUILD/wrk2/wrk" ]; then WRK2=$BUILD/wrk2/wrk; fi

# the commit the generator of the fixed rate is pinned to, it is
# unmaintained and carries an interpreter of its own that never
# learnt any of the 64 bit arm targets, so the build of it only ever
# succeeds on the intel machines and the run says when it did not
WRK2_COMMIT=${WRK2_COMMIT:-44a94c17d8e6a0bac8559b53da76848e430cb7a7}

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

# the subject has no image to be run out of unless one was built for
# it, and running it natively against references that are not is the
# one thing this is all meant to avoid, so the references come back
# out of the containers rather than the comparison being spoiled
if [ "$MODE" = "container" ] &&
    ! "$DOCKER" image inspect "$IMAGE_SUBJECT" > /dev/null 2>&1; then
    echo "There is no $IMAGE_SUBJECT image to run the subject out of,"
    echo "  falling back to measuring everything natively instead"
    MODE=native
    REFERENCES=native
fi

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT/runs" "$OUTPUT/logs" "$OUTPUT/www/listing"

echo "Running the harness in the $MODE mode ..."

# builds the server in the release shape, a figure taken out of a
# debug build measures the counters of that build and nothing else,
# the extension is built along with it so that the workloads of the
# interfaces have something to be driven against
if [ ! -x "$BUILD/bin/viriatum" ]; then
    echo "Building the server ..."
    cmake -S "$ROOT" -B "$BUILD" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER="$COMPILER" \
        -DVIRIATUM_BUILD_PYTHON=ON \
        -DPython_EXECUTABLE="$(command -v "$PYTHON")" > "$OUTPUT/logs/build.log" 2>&1
    cmake --build "$BUILD" -j 4 >> "$OUTPUT/logs/build.log" 2>&1
fi

BINARY=${BINARY:-$BUILD/bin/viriatum}

# builds the generator of the fixed rate out of the pinned commit, it
# is packaged by nobody and so it is either built here or absent, a
# failure of the build is never a failure of the run and only ever
# costs the run the percentiles that would have been corrected
if [ -z "$WRK2" ] && command -v git > /dev/null 2>&1; then
    echo "Building the generator of the fixed rate ..."
    if git clone -q https://github.com/giltene/wrk2.git "$BUILD/wrk2" \
        > "$OUTPUT/logs/wrk2.log" 2>&1 &&
        git -C "$BUILD/wrk2" checkout -q "$WRK2_COMMIT" >> "$OUTPUT/logs/wrk2.log" 2>&1 &&
        make -C "$BUILD/wrk2" -j 4 >> "$OUTPUT/logs/wrk2.log" 2>&1; then
        WRK2=$BUILD/wrk2/wrk
    else
        echo "  it did not build here, the tail of the run carries no corrected latency"
    fi
fi

# builds the reference of the proxying workload that ships as a
# framework rather than as a server, it is the slowest thing a run
# builds so the result is kept and only ever built once, a machine
# without the toolchain simply does not measure it
if [ ! -x "$BUILD/pingora/load_balancer" ] && command -v cargo > /dev/null 2>&1; then
    echo "Building the reference that ships as a framework ..."
    if cargo build --release --manifest-path "$ASSETS/pingora/Cargo.toml" \
        --target-dir "$BUILD/pingora/target" > "$OUTPUT/logs/pingora.log" 2>&1; then
        mkdir -p "$BUILD/pingora"
        cp "$BUILD/pingora/target/release/load_balancer" "$BUILD/pingora/load_balancer"
    else
        echo "  it did not build here, the proxying workload carries one reference less"
    fi
fi

# stages the extension next to the package so that the launcher of the
# interfaces is able to import it without the package being installed
MODULE=$(ls "$BUILD"/lib/_viriatum*.so "$BUILD"/lib/_viriatum*.pyd 2> /dev/null | head -1)
if [ -n "$MODULE" ]; then cp "$MODULE" "$ROOT/src/viriatum_python/viriatum/"; fi

# the flags the binary was built with are part of every figure that
# comes out of it, a number that cannot be traced back to a build is
# not a number that may be compared against another one
{
    echo "mode: $MODE"
    echo "references: $REFERENCES"
    echo "machine: $(uname -srm)"
    echo "compiler: $("$COMPILER" --version 2> /dev/null | head -1)"
    echo "flags: $(grep -s CMAKE_C_FLAGS_RELEASE:STRING "$BUILD/CMakeCache.txt" | cut -d= -f2-)"
    echo "banner: $("$BINARY" --version 2> /dev/null | tail -1)"
    echo "duration: ${DURATION}s"
    echo "connections: $CONNECTIONS"
    echo "threads: $THREADS"
    echo "repeats: $REPEATS"
    echo "workers: $WORKERS"
} > "$OUTPUT/environment.txt"
cat "$OUTPUT/environment.txt"

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

# the templates the server builds the listing and the error page out
# of, it looks for them under the root it serves and answers an empty
# body when they are not there, which is not a page the reference can
# be compared against, the workload measured exactly nothing until
# these were put where the server actually looks for them
mkdir -p "$OUTPUT/www/templates"
cp "$ROOT/src/viriatum/resources/html/base/templates/listing.html.tpl" \
    "$OUTPUT/www/templates/listing.html.tpl"
cp "$ROOT/src/viriatum/resources/html/base/templates/error.html.tpl" \
    "$OUTPUT/www/templates/error.html.tpl"

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
# one the workload names and the configuration the one just written,
# the workloads of the interfaces are served by the extension instead
# and go through a launcher of their own
_start_subject() {
    _HANDLER=$1
    _ROLE=$2
    cd "$OUTPUT"

    case $_ROLE in
        wsgi | asgi | asgi-stream)
            PYTHONPATH="$ASSETS:$ROOT/src/viriatum_python" "$PYTHON" \
                "$ASSETS/serve.py" "$_ROLE" "$PORT" \
                < /dev/null > /dev/null 2> "$OUTPUT/logs/subject.log" &
            PID=$!
            _wait "$PORT"
            return 0
            ;;
    esac

    if [ "$MODE" = "container" ]; then
        "$DOCKER" run -d --name viriatum-subject --network host \
            -v "$OUTPUT:/bench" -w /bench "$IMAGE_SUBJECT" \
            viriatum --port="$PORT" --handler="$_HANDLER" \
            --wwwroot=/bench/www --workers="$WORKERS" \
            > "$OUTPUT/logs/subject.id" 2>&1
    else
        "$BINARY" --port="$PORT" --handler="$_HANDLER" \
            --wwwroot="$OUTPUT/www" --workers="$WORKERS" \
            < /dev/null > /dev/null 2> "$OUTPUT/logs/subject.log" &
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

# writes the configuration of the reference that wants a tree of its
# own rather than a single file, the values that decide the comparison
# being the very same ones every other server is given
_render_litespeed() {
    mkdir -p "$OUTPUT/conf/litespeed/conf" "$OUTPUT/conf/litespeed/logs"
    {
        echo "serverName benchmark"
        echo "user nobody"
        echo "group nogroup"
        echo "priority 0"
        echo "enableLve 0"
        echo "autoRestart 0"
        echo "gracefulRestartTimeout 0"
        echo "httpdWorkers $WORKERS"
        echo "indexFiles index.html"
        echo "errorlog \$SERVER_ROOT/logs/error.log { logLevel ERROR }"
        echo "accessLog \$SERVER_ROOT/logs/access.log { logHeaders 0 }"
        echo "tuning {"
        echo "    maxConnections 8192"
        echo "    keepAliveTimeout 65"
        echo "    maxKeepAliveReq 1000000"
        echo "    enableGzipCompress 0"
        echo "    enableBrCompress 0"
        echo "    eventDispatcher epoll"
        echo "}"
        echo "virtualHost benchmark {"
        echo "    vhRoot $_PREFIX/www"
        echo "    docRoot \$VH_ROOT"
        echo "    enableGzip 0"
        echo "    context / {"
        echo "        location \$DOC_ROOT"
        echo "        allowBrowse 1"
        echo "        autoIndex 1"
        echo "    }"
        echo "}"
        echo "listener benchmark {"
        echo "    address *:$PORT_REFERENCE"
        echo "    secure 0"
        echo "    map benchmark *"
        echo "}"
    } > "$OUTPUT/conf/litespeed/httpd_config.conf"
}

# starts a reference out of its image, every one of them is given the
# network of the machine so that it is reached exactly the way the
# subject is and no address translation sits in between
_run_image() {
    "$DOCKER" run -d --name viriatum-reference --network host \
        -v "$OUTPUT:/bench" "$@" > "$OUTPUT/logs/reference.id" 2>&1
}

# reports the pattern that finds a server together with every worker
# it has forked, a reference that renames the title of its processes,
# which most of them do, is never found by the command it was started
# with and would be reported as having consumed nothing at all
_match() {
    case $1 in
        viriatum) echo "viriatum --port=" ;;
        nginx) echo "nginx:" ;;
        caddy) echo "caddy run" ;;
        haproxy) echo "haproxy -f" ;;
        openlitespeed) echo "litespeed" ;;
        pingora) echo "load_balancer" ;;
        gunicorn) echo "gunicorn" ;;
        uvicorn) echo "uvicorn" ;;
        *) echo "$1" ;;
    esac
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
        openlitespeed)
            # the reference that carries a configuration of its own
            # shape, the tree of it is written out whole rather than
            # rendered from a single file the way the others are
            _render_litespeed
            _run_image "$IMAGE_LITESPEED" \
                /usr/local/lsws/bin/litespeed -d -c /bench/conf/litespeed/httpd_config.conf
            ;;
        pingora)
            "$BUILD/pingora/load_balancer" "127.0.0.1:$PORT_REFERENCE" \
                "127.0.0.1:$PORT_UPSTREAM" \
                > "$OUTPUT/logs/reference.log" 2>&1 &
            PID_REFERENCE=$!
            ;;
        gunicorn)
            # the reference is given the shape it was measured to be
            # at its best under rather than the one that mirrors the
            # subject, its worker answering a single request at a time
            # loses every connection past the count of them, and a
            # reference driven in a shape it was never meant for
            # reports the shape and never the reference
            PYTHONPATH=$ASSETS "$PYTHON" -m gunicorn \
                --bind "127.0.0.1:$PORT_REFERENCE" \
                --workers "$WORKERS" --worker-class gthread --threads 32 \
                --backlog 4096 --keep-alive 65 \
                --access-logfile /dev/null --error-logfile /dev/null \
                app:wsgi_app > "$OUTPUT/logs/reference.log" 2>&1 &
            PID_REFERENCE=$!
            ;;
        uvicorn)
            if [ "$_ROLE" = "asgi-stream" ]; then _APP=app:asgi_stream_app; else _APP=app:asgi_app; fi
            PYTHONPATH=$ASSETS "$PYTHON" -m uvicorn \
                --host 127.0.0.1 --port "$PORT_REFERENCE" --workers "$WORKERS" \
                --backlog 4096 --log-level critical --no-access-log "$_APP" \
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
            count = split($2, parts, ":")
            seconds = 0
            if (count == 3) {
                seconds = parts[1] * 3600 + parts[2] * 60 + parts[3]
            } else if (count == 2) {
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

# drives the generator of the fixed rate against the provided target,
# the rate being a share of the throughput that was just measured so
# that the percentiles are taken below the point of saturation, above
# it every figure of the tail is a figure of the queue and not of the
# server that the queue happens to be standing in front of
_latency() {
    _TARGET=$1
    _HEADER=$2
    _REPORTS=$3
    _PEAK=$4

    if [ -z "$WRK2" ]; then return 0; fi

    _RATE=$(echo "$_PEAK $RATE" | awk '{ printf "%d", $1 * $2 / 100 }')
    if [ "$_RATE" -lt 1 ]; then return 0; fi

    if [ -n "$_HEADER" ]; then
        set -- -t "$THREADS" -c "$CONNECTIONS" -d "${DURATION}s" -R "$_RATE" -H "$_HEADER"
    else
        set -- -t "$THREADS" -c "$CONNECTIONS" -d "${DURATION}s" -R "$_RATE"
    fi

    BENCHMARK_REPORT="$_REPORTS/latency.json" "$WRK2" "$@" \
        -s "$ASSETS/report.lua" "$_TARGET" \
        > "$_REPORTS/latency.txt" 2>&1 || true
}

# measures the handling of the connections as two figures of its own,
# the rate at which fresh ones are taken up and the number of requests
# a kept one carries, the accept path and the serving one are separate
# costs and a single figure of throughput hides which of them moved
_connections() {
    _TARGET=$1
    _REPORTS=$2

    # the fresh connections are driven by the generator rather than
    # counted against a clock of whole seconds, a couple of thousand
    # of them are taken up faster than such a clock is able to tell
    # apart and every server would report the very same figure, and a
    # workload that closes every connection of its own accord has
    # measured this already, driving it twice would only walk through
    # the ports of the machine a second time for a figure in hand
    if [ -n "$HEADER" ]; then
        cp "$_REPORTS/repeat-0.json" "$_REPORTS/accept.json" 2> /dev/null || true
    else
        BENCHMARK_REPORT="$_REPORTS/accept.json" wrk -t "$THREADS" -c "$CONNECTIONS" \
            -d "${DURATION}s" -H "Connection: close" -s "$ASSETS/report.lua" \
            "$_TARGET" > /dev/null 2>&1 || true
    fi
    sed -n 's/.*"rps": *\([0-9.]*\).*/\1/p' "$_REPORTS/accept.json" \
        2> /dev/null > "$_REPORTS/accept.txt" || true

    # the requests a single connection carries is what says whether
    # the server holds one open the way it claims to, a server that
    # closes it every time opens a hundred of them for a hundred
    # requests and gives itself away with a figure of one
    curl -s --max-time 60 -w "%{num_connects}\n" \
        $(_repeat 100 "$_TARGET") 2> /dev/null |
        awk '/^[0-9]+$/ { total += $1 }
            END { printf "%.1f\n", (total > 0) ? 100.0 / total : 100.0 }' \
        > "$_REPORTS/reuse.txt"
}

# counts the calls into the kernel that a single request costs, which
# is the figure that most directly explains a gap against a reference
# on the static path, the run that is counted is never one of the
# timed ones, tracing a process slows it by an order of magnitude and
# a figure taken under the tracer would not be a figure of throughput
_syscalls() {
    _REPORTS=$1
    _OF=$3

    if [ -z "$_OF" ] || ! command -v strace > /dev/null 2>&1; then return 0; fi

    strace -c -f -o "$_REPORTS/syscalls.txt" -p "$_OF" 2> /dev/null &
    _TRACER=$!
    sleep 1

    _INDEX=0
    while [ "$_INDEX" -lt 200 ]; do
        curl -s -o /dev/null --max-time 5 "$2" 2> /dev/null || true
        _INDEX=$((_INDEX + 1))
    done

    kill -INT "$_TRACER" 2> /dev/null || true
    wait "$_TRACER" 2> /dev/null || true

    # the total of the tracer is the last of the rows it writes, it is
    # divided by the requests that were driven while it was attached
    awk '/^total|^[[:space:]]*100\.00/ { total = $(NF - 1) }
        END { printf "%.1f\n", total / 200.0 }' \
        "$_REPORTS/syscalls.txt" > "$_REPORTS/syscalls.count" 2> /dev/null || true
}

# repeats the provided target the requested number of times, each one
# of them paired with the sink its body is written to, the client is
# handed the whole list at once and reuses whatever it may of it
_repeat() {
    _INDEX=0
    while [ "$_INDEX" -lt "$1" ]; do
        printf -- "-o /dev/null %s " "$2"
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
python-wsgi-alive|wsgi|file|/||On|
python-asgi-alive|asgi|file|/||On|
python-asgi-stream-alive|asgi-stream|file|/||On|
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

# drives every one of the metrics against a single server that is
# already up and records the result of it, the subject and each of the
# references go through this exactly the same way, a metric taken one
# way on one of them and another way on the other would be comparing
# the harness against itself rather than the servers against each other
_measure() {
    _NAME=$1
    _SERVER=$2
    _AT=$3
    _VERSION=$4

    _MATCH=$(_match "$_SERVER")
    _REPORTS=$OUTPUT/runs/$_NAME.$_SERVER
    _TARGET=http://127.0.0.1:$_AT$PATH_
    mkdir -p "$_REPORTS"

    set -- $(_sample "$_MATCH")
    _RSS_BEFORE=$1
    _CPU_BEFORE=$2

    _drive "$_TARGET" "$HEADER" "$_REPORTS"

    set -- $(_sample "$_MATCH")
    _RSS_AFTER=$1
    _CPU_AFTER=$2

    # the processor time is reported against a thousand requests so
    # that the workloads may be read against one another, the memory
    # is the highest of the two samples taken around the run
    _REQUESTS=$(_values "$_REPORTS" requests | awk '{ total += $1 } END { print total + 0 }')
    _RSS=$(echo "$_RSS_BEFORE $_RSS_AFTER" | awk '{ print ($1 > $2) ? $1 : $2 }')
    _CPU=$(echo "$_CPU_BEFORE $_CPU_AFTER $_REQUESTS" | awk \
        '{ if ($3 > 0) printf "%.3f", ($2 - $1) * 1000000.0 / $3; else print 0 }')

    # the remaining metrics are all taken outside of a timed run, the
    # tail below saturation, the handling of the connections and the
    # calls into the kernel each disturb a run of throughput
    _PEAK=$(_values "$_REPORTS" rps | _stats | awk '{ print $1 }')
    _latency "$_TARGET" "$HEADER" "$_REPORTS" "$_PEAK"
    _connections "$_TARGET" "$_REPORTS"
    # the calls into the kernel are counted for the subject and for
    # every reference alike, the figure of one only says anything
    # about the serving when it is read against the figure of another
    if [ "$_SERVER" = "viriatum" ]; then _OWNER=$PID; else _OWNER=$PID_REFERENCE; fi
    _syscalls "$_REPORTS" "$_TARGET" "$_OWNER"

    _record "$_NAME" "$_SERVER" "$_REPORTS" "$_VERSION"
}

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
    # faster than a server does and would read as a win, and one that
    # served nothing at all is never a figure either, whatever the
    # reason it served nothing happened to have been
    _REQUESTS=$(_values "$_REPORTS" requests | awk '{ total += $1 } END { print total + 0 }')
    _VALID=$(echo "$_SOCKET $_REQUESTS" | awk \
        '{ print ($2 <= 0 || $1 * 1000 > $2) ? "false" : "true" }')

    # the tail is only ever read out of the run that was held at a
    # fixed rate, the percentiles of a saturated run leave out every
    # request the generator was held back from sending
    if [ -s "$_REPORTS/latency.json" ]; then
        _P50=$(sed -n 's/.*"p50": *\([0-9]*\).*/\1/p' "$_REPORTS/latency.json")
        _P99=$(sed -n 's/.*"p99": *\([0-9]*\).*/\1/p' "$_REPORTS/latency.json")
        _P999=$(sed -n 's/.*"p999": *\([0-9]*\).*/\1/p' "$_REPORTS/latency.json")
        _CORRECTED=true
    else
        _P50=0
        _P99=0
        _P999=0
        _CORRECTED=false
    fi

    _ACCEPT=$(cat "$_REPORTS/accept.txt" 2> /dev/null || echo 0)
    _REUSE=$(cat "$_REPORTS/reuse.txt" 2> /dev/null || echo 0)
    _CALLS=$(cat "$_REPORTS/syscalls.count" 2> /dev/null || echo 0)

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
        echo "  \"cpu_ms_per_k\": ${_CPU:-0},"
        echo "  \"latency_corrected\": $_CORRECTED,"
        echo "  \"latency_p50_us\": ${_P50:-0},"
        echo "  \"latency_p99_us\": ${_P99:-0},"
        echo "  \"latency_p999_us\": ${_P999:-0},"
        echo "  \"accept_rps\": ${_ACCEPT:-0},"
        echo "  \"requests_per_connection\": ${_REUSE:-0},"
        echo "  \"syscalls_per_request\": ${_CALLS:-0}"
        echo "}"
    } > "$OUTPUT/runs/$_NAME.$_SERVER.json"

    if [ "$_VALID" = "true" ]; then
        printf "  %-12s %12.0f req/s  %8.2f MB/s\n" "$_SERVER" "$_RPS" \
            "$(echo "$_TRANSFER" | awk '{ printf "%.2f", $1 / 1048576 }')"
    else
        printf "  %-12s %12.0f req/s  discarded, %s connections were lost\n" \
            "$_SERVER" "$_RPS" "$_SOCKET"
        echo "    the machine is holding $(_waiting) closed sockets, a workload that"
        echo "    closes every connection needs a wider range of ports than this to run"
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
            < /dev/null > /dev/null 2> "$OUTPUT/logs/upstream.log" &
        PID_UPSTREAM=$!
        _wait "$PORT_UPSTREAM"
    fi

    _configure "$HANDLER" "$TEMPLATE" "$PROXY"
    _start_subject "$HANDLER" "$ROLE"
    _measure "$NAME" viriatum "$PORT" "$VERSION"
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

        _settle
        _start_reference "$REFERENCE" "$ROLE"
        _measure "$NAME" "$REFERENCE" "$PORT_REFERENCE" "$(_version "$REFERENCE")"
        _stop_reference
    done

    _stop_upstream
done < "$OUTPUT/workloads.txt"

echo

# builds the markdown table of the run, it is written to the step
# summary of the workflow so that the numbers show up on the run page,
# the comparison against the baseline happens inside it as well
if [ -f "$BASELINE" ]; then
    "$PYTHON" "$ROOT/scripts/benchmark_table.py" "$OUTPUT" "$BASELINE" > "$OUTPUT/summary.md"
else
    "$PYTHON" "$ROOT/scripts/benchmark_table.py" "$OUTPUT" > "$OUTPUT/summary.md"
fi
cat "$OUTPUT/summary.md"

echo
echo "Benchmark reports written to $OUTPUT"

# the run reports and never gates, a hosted runner is far too noisy
# for a figure of performance to be allowed to fail a build, the
# comparison against the baseline is there to be read and not obeyed
exit 0
