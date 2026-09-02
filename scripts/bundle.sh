#!/bin/sh
# Hive Viriatum Web Server
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# Assembles the tree that a release carries, the binary together with
# the modules that need no runtime beside them, the configuration and
# the contents, and archives it in the shape of its platform.

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${BUILD:-$ROOT/build.release}
OUTPUT=${OUTPUT:-$ROOT/dist}
VERSION=${VERSION:-unknown}
SUFFIX=${SUFFIX:-}

# the modules that carry no dependency of their own, the ones of the
# interpreters are left out as they bind the tree to the exact library
# of the machine that built it
MODULES=${MODULES:-"diag gif"}

# resolves the name of the platform and of the processor the way the
# rest of the project names them, so that an archive is recognised by
# whoever reads the table of a release
case $(uname -s) in
    Linux*) PLATFORM=linux ;;
    Darwin*) PLATFORM=darwin ;;
    MINGW* | MSYS* | CYGWIN*) PLATFORM=win32 ;;
    *) PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]') ;;
esac

case $(uname -m) in
    x86_64 | amd64) ARCH=x64 ;;
    arm64 | aarch64) ARCH=arm64 ;;
    *) ARCH=$(uname -m) ;;
esac

# the extension that each platform gives to a binary and to a library,
# which decides both what is copied and what the archive is
if [ "$PLATFORM" = "win32" ]; then
    BINARY=viriatum.exe
    LIBRARY=.dll
    PREFIX=
else
    BINARY=viriatum
    LIBRARY=$([ "$PLATFORM" = "darwin" ] && echo .dylib || echo .so)
    PREFIX=lib
fi

NAME=viriatum-$VERSION-$PLATFORM-$ARCH$SUFFIX
TREE=$OUTPUT/$NAME

echo "Assembling $NAME ..."

rm -rf "$TREE"
mkdir -p "$TREE/modules" "$TREE/config" "$TREE/htdocs"

# the binary itself, which is what every other part of the tree is
# found from once it is running
cp "$BUILD/bin/$BINARY" "$TREE/"

# the modules that are carried, a missing one ends the assembling as
# an archive without what it was asked to hold is worse than none, the
# set of them being the one that always builds
for module in $MODULES; do
    SOURCE=$BUILD/modules/${PREFIX}viriatum_mod_${module}${LIBRARY}
    if [ ! -f "$SOURCE" ]; then
        echo "module '$module' was not built, expected it at $SOURCE" >&2
        exit 1
    fi
    cp "$SOURCE" "$TREE/modules/"
done

# the configuration and the contents, both of them read from the tree
# beside the binary once the archive is unpacked
cp "$ROOT/src/viriatum/resources/config/viriatum/viriatum.ini" "$TREE/config/"
cp -R "$ROOT/src/viriatum/resources/html/." "$TREE/htdocs/"
cp "$ROOT/README.md" "$ROOT/LICENSE" "$TREE/" 2> /dev/null || true

# removes what only the building of the tree ever needed, an archive
# carries what is served rather than what makes it
find "$TREE" -name "Makefile.am" -exec rm -f {} +

# archives the tree in the shape the platform is expected to carry,
# the one of windows being the only one that is not a tarball
cd "$OUTPUT"
if [ "$PLATFORM" = "win32" ]; then
    ARCHIVE=$NAME.zip
    rm -f "$ARCHIVE"
    7z a -tzip "$ARCHIVE" "$NAME" > /dev/null
else
    ARCHIVE=$NAME.tar.gz
    rm -f "$ARCHIVE"
    tar -czf "$ARCHIVE" "$NAME"
fi

echo "Wrote $OUTPUT/$ARCHIVE ($(wc -c < "$ARCHIVE" | tr -d ' ') bytes)"
