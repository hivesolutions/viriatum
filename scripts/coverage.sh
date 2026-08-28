#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Builds the server with coverage instrumentation, runs the core,
# the module and the python test suites and reports the resulting
# coverage of the complete C tree of the project.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.coverage}
OUTPUT=${OUTPUT:-$ROOT/coverage}
PYTHON=${PYTHON:-python3}
THRESHOLD=${THRESHOLD:-30}
PYTHON_THRESHOLD=${PYTHON_THRESHOLD:-90}
COMPILER=${CC:-clang}

# selects the toolchain that gathers the coverage from the name of
# the compiler in use, clang writes the native profiles of llvm and
# gcc writes the gcov ones, both end up in the same set of reports
case $(basename "$COMPILER") in
    *clang*) TOOLCHAIN=llvm ;;
    *gcc*) TOOLCHAIN=gcov ;;
    *)
        echo "coverage requires either clang or gcc, '$COMPILER' is neither" >&2
        exit 1
        ;;
esac

# resolves the tools of the selected toolchain, the ones of llvm ship
# either on the path or behind the developer tools wrapper of macos
if [ "$TOOLCHAIN" = "llvm" ]; then
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
elif ! command -v gcovr > /dev/null 2>&1; then
    echo "coverage requires the gcovr tool" >&2
    exit 1
fi

rm -rf "$BUILD" "$OUTPUT"
mkdir -p "$OUTPUT"

# builds every target with the counters of the compiler enabled, the
# modules and the python extension included so that the handlers they
# carry are measured together with the core, a module whose dependency
# is missing from the machine is simply skipped by the build
cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_C_COMPILER="$COMPILER" \
    -DVIRIATUM_COVERAGE=ON \
    -DVIRIATUM_BUILD_MODULES=ON \
    -DVIRIATUM_BUILD_PYTHON=ON \
    -DPython_EXECUTABLE="$(command -v "$PYTHON")"
cmake --build "$BUILD" -j 4

# stages the extension next to the package so that the test suites are
# able to import it without the package having to be installed
MODULE=$(ls "$BUILD"/lib/_viriatum*.so "$BUILD"/lib/_viriatum*.pyd 2>/dev/null | head -1)
cp "$MODULE" "$ROOT/src/viriatum_python/viriatum/"

# gathers the binaries that are going to be run, the ones of the
# modules only exist when their dependencies were available
BINARIES="$BUILD/bin/viriatum"
for name in viriatum_mod_lua_test viriatum_mod_wsgi_test; do
    if [ -x "$BUILD/bin/$name" ]; then BINARIES="$BINARIES $BUILD/bin/$name"; fi
done

# runs the core suite and then the one of every module that has been
# built, each of them writing a raw profile of its own, the core is
# the only binary that takes the test flag as it also serves requests
for binary in $BINARIES; do
    name=$(basename "$binary")
    if [ "$name" = "viriatum" ]; then
        LLVM_PROFILE_FILE="$OUTPUT/$name.profraw" "$binary" --test \
            --test-format=markdown --test-output="$OUTPUT/tests.md"
    else
        LLVM_PROFILE_FILE="$OUTPUT/$name.profraw" "$binary"
    fi
done

# runs the python suite, the profile of the extension is written by
# the very same counters that the core has been built with
PYTHONPATH="$ROOT/src/viriatum_python" \
    LLVM_PROFILE_FILE="$OUTPUT/python.profraw" "$PYTHON" -m coverage run \
    --source viriatum --data-file "$OUTPUT/.coverage" \
    -m unittest discover -s "$ROOT/src/viriatum_python/test"

# the measured sources are the complete C tree of the project minus
# the test cases themselves, which would otherwise inflate the numbers,
# note that the test directory of the commons is kept as it holds the
# runner and that one ships as part of the library
SOURCES=$(
    find "$ROOT/src/viriatum" "$ROOT/src/viriatum_commons" "$ROOT/src/viriatum_python" \
        "$ROOT/modules" -name "*.c" \
        -not -path "$ROOT/src/viriatum/test/*" \
        -not -path "*/viriatum_mod_*/test/*"
)

if [ "$TOOLCHAIN" = "llvm" ]; then
    # every binary is inspected as an object of its own, each of them
    # carries a copy of the instrumented core and of the commons
    OBJECTS=""
    for binary in $BINARIES; do OBJECTS="$OBJECTS -object $binary"; done

    $PROFDATA merge -sparse "$OUTPUT"/*.profraw -o "$OUTPUT/viriatum.profdata"
    $COV report "$MODULE" $OBJECTS \
        -instr-profile="$OUTPUT/viriatum.profdata" $SOURCES > "$OUTPUT/summary.txt"
    cat "$OUTPUT/summary.txt"
    $COV show "$MODULE" $OBJECTS \
        -instr-profile="$OUTPUT/viriatum.profdata" -format=text $SOURCES \
        > "$OUTPUT/lines.txt"

    # exports the same numbers in a machine readable form, they are the
    # ones used for the building of the markdown table below
    $COV export "$MODULE" $OBJECTS \
        -instr-profile="$OUTPUT/viriatum.profdata" -summary-only $SOURCES \
        > "$OUTPUT/native.json"
else
    # gcov writes its counters next to the objects of the build, the
    # reports are produced out of the complete build directory and the
    # summary is exported under the shape the table builder expects
    GCOVR="gcovr --root $ROOT --filter $ROOT/src/ --filter $ROOT/modules/ --exclude .*/src/viriatum/test/.* --exclude .*/viriatum_mod_.*/test/.*"
    $GCOVR --output "$OUTPUT/summary.txt" "$BUILD"
    cat "$OUTPUT/summary.txt"
    $GCOVR --txt-metric line --output "$OUTPUT/lines.txt" "$BUILD"
    $GCOVR --json-summary-pretty --output "$OUTPUT/native.json" "$BUILD"
fi

"$PYTHON" -m coverage json --data-file "$OUTPUT/.coverage" \
    -o "$OUTPUT/python.json" -q || true

# builds the markdown table of the coverage, it is written to the step
# summary of the workflow so that the numbers show up on the run page,
# the status is only acted upon once every report has been produced
set +e
"$PYTHON" "$ROOT/scripts/coverage_table.py" "$OUTPUT" "$THRESHOLD" > "$OUTPUT/summary.md"
NATIVE_STATUS=$?
set -e
cat "$OUTPUT/summary.md"

# reports the coverage of the pure python surface of the package, it
# carries a threshold of its own as it is a much smaller surface than
# the complete C tree measured above, note that the status is captured
# instead of being piped, a pipe would hide the failure of the report
set +e
"$PYTHON" -m coverage report --data-file "$OUTPUT/.coverage" \
    --fail-under "$PYTHON_THRESHOLD" > "$OUTPUT/python.txt"
PYTHON_STATUS=$?
set -e
cat "$OUTPUT/python.txt"
cat "$OUTPUT/python.txt" >> "$OUTPUT/summary.txt"

echo "Coverage reports written to $OUTPUT"

# fails the run whenever either of the two surfaces has ended up below
# the threshold that has been set for it
if [ "$NATIVE_STATUS" -ne 0 ] || [ "$PYTHON_STATUS" -ne 0 ]; then exit 1; fi
