#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Builds the targets that drive the decoders of the protocol over the
# bytes an engine produces and runs them for a bounded amount of
# time, keeping the corpus so that a later run starts from it.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.fuzz}
OUTPUT=${OUTPUT:-$ROOT/fuzz}
CORPUS=${CORPUS:-$OUTPUT/corpus}
SECONDS_RUN=${SECONDS_RUN:-60}
RUNS=${RUNS:--1}
COMPILER=${CC:-clang}

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_C_COMPILER="$COMPILER" \
    -DCMAKE_BUILD_TYPE=Release \
    -DVIRIATUM_FUZZ=ON
cmake --build "$BUILD" --target viriatum_fuzz_http2 -j 4

mkdir -p "$CORPUS"

# seeds the corpus with the shapes that a peer opens a connection
# with, an engine reaches the rest of them from these
if [ -z "$(ls -A "$CORPUS" 2> /dev/null)" ]; then
    printf '\000\000\000\004\000\000\000\000\000' > "$CORPUS/settings"
    printf '\000\000\006\004\000\000\000\000\000\000\003\000\000\000\144' > "$CORPUS/settings-value"
    printf '\000\000\010\006\000\000\000\000\000\000\000\000\000\000\000\000\000' > "$CORPUS/ping"
    printf '\000\000\005\002\000\000\000\000\001\000\000\000\000\020' > "$CORPUS/priority"
    printf '\202\206\204\101\017www.example.com' > "$CORPUS/block"
    printf '\010\002\062\060\060' > "$CORPUS/status"
fi

# the amount of time and the number of the runs are both bounded so
# that a job of the integration never hangs on it
"$BUILD/bin/viriatum_fuzz_http2" \
    -max_total_time="$SECONDS_RUN" \
    -runs="$RUNS" \
    -max_len=16384 \
    -print_final_stats=1 \
    "$CORPUS" 2>&1 | tee "$OUTPUT/fuzz.log"

echo "Fuzzing reports written to $OUTPUT"
