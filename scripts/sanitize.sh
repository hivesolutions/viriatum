#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Builds the tree under the address sanitizer and drives the suite of
# the tests through it, so that a memory error fails the run and the
# allocations that are left behind are compared against the ones that
# are already known, never letting them grow.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.sanitize}
OUTPUT=${OUTPUT:-$ROOT/sanitize}
BASELINE=${BASELINE:-$ROOT/scripts/sanitize.baseline}
COMPILER=${CC:-clang}

# the leak part of the sanitizer only runs on some of the platforms,
# on the others the memory errors are still caught and the leaks are
# simply not looked at, the integration runs it where it does run
LEAKS=1
case "$(uname -s)" in
    Darwin) LEAKS=0 ;;
esac

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="$COMPILER" \
    -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
cmake --build "$BUILD" --target viriatum -j 4

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT"

echo "Running the suite under the sanitizer ..."

# the run is never allowed to stop the process on the first error, the
# complete report of it is what the comparison below is made against,
# the status of it is kept so that a run that never got as far as the
# tests is told from one that reported on them
STATUS=0
ASAN_OPTIONS="detect_leaks=$LEAKS:halt_on_error=0" \
    "$BUILD/bin/viriatum" --test > "$OUTPUT/tests.txt" 2>&1 || STATUS=$?

RESULT=$(sed -n 's/^Ran \(.*\)$/\1/p' "$OUTPUT/tests.txt" | tail -1)
FAILED=$(grep -c "^not ok" "$OUTPUT/tests.txt" || true)

# an error of the memory is never tolerated, whatever its shape, the
# leaks are reported by a sanitizer of their own and are the only ones
# that are compared against a number rather than simply refused
ERRORS=$(grep -c "ERROR: AddressSanitizer:" "$OUTPUT/tests.txt" || true)
LEAKED=$(sed -n 's/^SUMMARY: AddressSanitizer: [0-9]* byte(s) leaked in \([0-9]*\) allocation(s).*/\1/p' "$OUTPUT/tests.txt" | tail -1)
BYTES=$(sed -n 's/^SUMMARY: AddressSanitizer: \([0-9]*\) byte(s) leaked in [0-9]* allocation(s).*/\1/p' "$OUTPUT/tests.txt" | tail -1)
if [ -z "$LEAKED" ]; then LEAKED=0; BYTES=0; fi

ALLOWED=0
if [ -f "$BASELINE" ]; then ALLOWED=$(tr -d " \t\r\n" < "$BASELINE"); fi

{
    echo "## Sanitizer"
    echo ""
    echo "| measure | value |"
    echo "| --- | ---: |"
    echo "| tests | $RESULT |"
    echo "| memory errors | $ERRORS |"
    if [ "$LEAKS" = "1" ]; then
        echo "| allocations leaked | $LEAKED |"
        echo "| bytes leaked | $BYTES |"
        echo "| allocations allowed | $ALLOWED |"
    else
        echo "| allocations leaked | not measured on this platform |"
    fi
} > "$OUTPUT/summary.md"

cat "$OUTPUT/summary.md"

# a run that never reached the end of the suite has reported on
# nothing at all, so the counts above carry no meaning and the run
# fails on the status of the process instead
if [ -z "$RESULT" ]; then
    echo "The suite did not run to the end, it exited with $STATUS" >&2
    tail -40 "$OUTPUT/tests.txt" >&2
    exit 1
fi

if [ "$FAILED" -ne 0 ]; then
    echo "The suite reported $FAILED failing tests" >&2
    exit 1
fi

if [ "$ERRORS" -ne 0 ]; then
    echo "The sanitizer reported $ERRORS memory errors" >&2
    grep -A 12 "ERROR: AddressSanitizer:" "$OUTPUT/tests.txt" | head -40 >&2
    exit 1
fi

# the number is lowered as the leaks are closed and never raised to
# make a run pass, which is the very same rule the conformance and
# the coverage of the project are held to
if [ "$LEAKS" = "1" ] && [ "$LEAKED" -gt "$ALLOWED" ]; then
    echo "Allocations leaked: $LEAKED, which is above the $ALLOWED of the baseline" >&2
    exit 1
fi

if [ "$LEAKS" = "1" ]; then
    echo "Allocations leaked: $LEAKED, baseline is $ALLOWED"
fi

echo "Sanitizer reports written to $OUTPUT"
