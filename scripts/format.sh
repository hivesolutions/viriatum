#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Runs the formatter of the project over every source of the tree and
# reports the ones it would rewrite, applying the result in place when
# it is asked to fix them rather than to check them.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUTPUT=${OUTPUT:-$ROOT/format}
FORMAT=${FORMAT:-clang-format}
FIX=${FIX:-0}

# the fixing of the sources is asked for either through the variable
# of the environment or through the single argument of the script
if [ "$1" = "--fix" ]; then FIX=1; fi

if ! command -v "$FORMAT" > /dev/null 2>&1; then
    echo "formatting requires clang-format, '$FORMAT' is not on the path" >&2
    exit 1
fi

cd "$ROOT"

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT"

echo "Running $("$FORMAT" --version) ..."

FILES=0
DIRTY=0
LINES=0

echo "## Formatting" > "$OUTPUT/summary.md"
echo "" >> "$OUTPUT/summary.md"
echo "| file | lines |" >> "$OUTPUT/summary.md"
echo "| --- | ---: |" >> "$OUTPUT/summary.md"

# walks the sources that the repository tracks, the ones that are
# generated or vendored are never part of that set
for FILE in $(git ls-files "*.c" "*.h"); do
    FILES=$((FILES + 1))

    if [ "$FIX" = "1" ]; then
        "$FORMAT" -i "$FILE"
        continue
    fi

    COUNT=$("$FORMAT" "$FILE" | diff -u "$FILE" - | grep -c "^[-+][^-+]" || true)
    if [ "$COUNT" -eq 0 ]; then continue; fi

    DIRTY=$((DIRTY + 1))
    LINES=$((LINES + COUNT))
    printf "| \`%s\` | %s |\n" "$FILE" "$COUNT" >> "$OUTPUT/summary.md"
    "$FORMAT" "$FILE" | diff -u "$FILE" - >> "$OUTPUT/format.diff" || true
done

if [ "$FIX" = "1" ]; then
    echo "Formatted $FILES sources in place"
    exit 0
fi

if [ "$DIRTY" -eq 0 ]; then
    echo "| none | 0 |" >> "$OUTPUT/summary.md"
fi

cat "$OUTPUT/summary.md"
echo "Checked $FILES sources, $DIRTY of them are not formatted"

# the run fails whenever a source is left unformatted, the diff of it
# is kept so that the change may be read rather than guessed at
if [ "$DIRTY" -ne 0 ]; then
    echo "Run './scripts/format.sh --fix' to format them" >&2
    exit 1
fi

echo "Formatting reports written to $OUTPUT"
