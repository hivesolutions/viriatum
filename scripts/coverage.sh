#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Builds the server with coverage instrumentation, runs both the core
# and the python test suites and reports the resulting coverage.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.coverage}
OUTPUT=${OUTPUT:-$ROOT/coverage}
PYTHON=${PYTHON:-python3}
THRESHOLD=${THRESHOLD:-90}

# resolves the tools of the llvm suite, they ship either on the path
# or behind the developer tools wrapper of macos
if command -v llvm-profdata > /dev/null 2>&1; then
    PROFDATA=llvm-profdata
    COV=llvm-cov
elif xcrun --find llvm-profdata > /dev/null 2>&1; then
    PROFDATA="xcrun llvm-profdata"
    COV="xcrun llvm-cov"
else
    echo "coverage requires the llvm-profdata and llvm-cov tools" >&2
    exit 1
fi

rm -rf "$BUILD" "$OUTPUT"
mkdir -p "$OUTPUT"

# builds every target with the counters of the compiler enabled, the
# python extension included so that its handler is measured as well
cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_C_COMPILER="${CC:-clang}" \
    -DVIRIATUM_COVERAGE=ON \
    -DVIRIATUM_BUILD_MODULES=OFF \
    -DVIRIATUM_BUILD_PYTHON=ON \
    -DPython_EXECUTABLE="$(command -v "$PYTHON")"
cmake --build "$BUILD" -j 4

# stages the extension next to the package so that the test suites are
# able to import it without the package having to be installed
MODULE=$(ls "$BUILD"/lib/_viriatum*.so "$BUILD"/lib/_viriatum*.pyd 2>/dev/null | head -1)
cp "$MODULE" "$ROOT/src/viriatum_python/viriatum/"

# runs the core suite and then the python one, each of them writing a
# raw profile of its own into the output directory
LLVM_PROFILE_FILE="$OUTPUT/core.profraw" "$BUILD/bin/viriatum" --test
PYTHONPATH="$ROOT/src/viriatum_python" \
    LLVM_PROFILE_FILE="$OUTPUT/python.profraw" "$PYTHON" -m coverage run \
    --source viriatum --data-file "$OUTPUT/.coverage" \
    -m unittest discover -s "$ROOT/src/viriatum_python/test"

# merges the raw profiles and reports the coverage of the sources that
# have been written for the asgi support, both objects are inspected
# as each of them carries a copy of the instrumented core
SOURCES=$(find "$ROOT/src/viriatum_python" -name "*.c")
SOURCES="$SOURCES $ROOT/src/viriatum/http/websocket.c"

$PROFDATA merge -sparse "$OUTPUT"/*.profraw -o "$OUTPUT/viriatum.profdata"
$COV report "$MODULE" -object "$BUILD/bin/viriatum" \
    -instr-profile="$OUTPUT/viriatum.profdata" $SOURCES \
    | tee "$OUTPUT/summary.txt"
$COV show "$MODULE" -object "$BUILD/bin/viriatum" \
    -instr-profile="$OUTPUT/viriatum.profdata" -format=text $SOURCES \
    > "$OUTPUT/lines.txt"

# exports the same numbers in a machine readable form, they are the
# ones used for the building of the markdown table below
$COV export "$MODULE" -object "$BUILD/bin/viriatum" \
    -instr-profile="$OUTPUT/viriatum.profdata" -summary-only $SOURCES \
    > "$OUTPUT/native.json"
"$PYTHON" -m coverage json --data-file "$OUTPUT/.coverage" \
    -o "$OUTPUT/python.json" -q || true

# builds the markdown table of the coverage, it is written to the step
# summary of the workflow so that the numbers show up on the run page
"$PYTHON" "$ROOT/scripts/coverage_table.py" "$OUTPUT" "$THRESHOLD" \
    > "$OUTPUT/summary.md"
cat "$OUTPUT/summary.md"

# reports the coverage of the pure python surface of the package, the
# threshold applies to it as it does to the sources of the extension
"$PYTHON" -m coverage report --data-file "$OUTPUT/.coverage" \
    --fail-under "$THRESHOLD" | tee -a "$OUTPUT/summary.txt"

echo "Coverage reports written to $OUTPUT"
