# [![Viriatum Web Server](res/logo.png)](http://viriatum.hive.pt)

The Viriatum Web Server is simple and lightweight web server aimed at providing a simple yet powerful solution
for static file serving, reverse proxy serving and dynamic language handling.

The current implementation support both x86/x64 and ARM architectures.

## Building

### Automium

Building viriatum using automium is quite simple even for complex builds like cross compilation.
Just use the appropriate 'build.json' located under 'scripts/build' and under such directory execute:

    atm

For production purposes one should use the proper optimization flags:

    atm --cflags=-O3

For cross compilation (eg: arm-rasp-linux) use the following command:

    atm --cflags=-O3 --cross=arm-rasp-linux-gnueabi

If you want to know more about cross compilation please refer to the [Cross Compilation](doc/cross.md) document.

### CMake

CMake is the recommended build system for local development and IDE integration.

    cmake -B build
    cmake --build build

The binary is placed under `build/bin/viriatum`.

For a debug build (enables debug logging with file/line info and `V_DEBUG` output):

    cmake -DCMAKE_BUILD_TYPE=Debug -B build
    cmake --build build

Debug logging can also be enabled independently of the optimization level:

    cmake -D VIRIATUM_DEBUG=ON -B build
    cmake --build build

To set a custom www root path (equivalent to `--with-wwwroot` in Autoconf):

    cmake -D VIRIATUM_WWW_ROOT=/var/viriatum/www -B build
    cmake --build build

If the project has Conan-managed dependencies, install them first:

    conan install . --output-folder=build --build=missing -s build_type=Debug
    cmake -B build -DCMAKE_BUILD_TYPE=Debug --toolchain build/conan_toolchain.cmake
    cmake --build build

### Autoconf / Automake

If you're going to build viriatum using the git repository you first need to generate the configure
files using autoconf.

    ./autogen.sh

### Unix

For building under unix simple instructions are used:

    ./configure
    make && make install

In order to provide a correct (system wide) configuration path use:

    ./configure --sysconfdir=/etc

In order to get the most performance out of viriatum set the CFLAGS variable for optimization:

    CFLAGS="-O3" ./configure

### Unix 32bit from 64bit

    apt-get libc6-dev-i386
    export CC="cc -m32"
    ./configure
    make && make install

### Android

Building viriatum for android involves cross compilation and the android ndk toolkit must be used.

Check the instructions for downloading the android ndk from the [official website](http://developer.android.com/sdk/ndk/),
then unpack the android-ndk package file into the current directory.

The recommended NDK version is `r8e` as compatibility is ensured for that version. NDK is currently
known to have problems compiling some of the packages (including PHP, Python and Lua). To download that
version of the NDK for Linux use this [link](http://dl.google.com/android/ndk/android-ndk-r8e-linux-x86_64.tar.bz2).

Because of problems building the static versions of PHP, Python, Lua, etc. some of the modules are
currently not compatible with Android.

The `r8d` or preivous versions of the NDK are meant to be run only under x86 machines and should
be used with care unnder x86-64 based machines.

    tar -xvf android-ndk-r[x]-linux-[arch].tar.bz2

Create the standalone toolchain using the script for that purpose.

    android-ndk-r[x]/build/tools/make-standalone-toolchain.sh --system=linux-x86_64\
        --platform=android-4 --install-dir=/opt/android-toolchain

And then you may use the "just" created toolchain to build Viriatum with:

    export PATH=/opt/android-toolchain/bin:$PATH
    export CFLAGS="-L/opt/android-toolchain/lib -I/opt/android-toolchain/include\
        -L/opt/android-root/lib -I/opt/android-root/include"
    ./configure --host=arm-linux-androideabi --build=arm --prefix=/opt/android-root
    make && make install

### Windows

For building using the default mingw32 toolchain.

    apt-get install mingw32 mingw32-binutils
    ./configure --host=i586-mingw32msvc --build=i686-linux --prefix=/opt/i586-mingw32
    make && make install

For building using the mingw-w64 toolchain for 64 and 32 bit options.

    apt-get install mingw-w64 gcc-mingw-w64 binutils-mingw-w64
    ./configure --host=x86_64-w64-mingw32 --build=i686-linux --prefix=/opt/x86_64-w64-mingw32
    ./configure --host=i686-w64-mingw32 --build=i686-linux --prefix=/opt/i686-w64-mingw32
    make && make install

#### Notes

* In order to compile to `WINVER <= 0x0500` (Windows 2000 or older) disable IPv6 support (`#undef VIRIATUM_IP6`)

### FreeBSD

FreeBSD ignores the /usr/local directory by default so it must be included in order to conform with dependencies.

    setenv CFLAGS "-L/usr/local/lib -I/usr/local/include"

## Features

There are a lot of possible building features to enable

* `--with-moduleroot=path` - Sets the path to be used to load the modules
* `--with-wwwroot=path` - Sets the path to be used to install and serve the default content
* `--enable-debug` - Enables the extra debugging capabilities in Viriatum
* `--enable-defaults` - Enables the default paths in the viriatum server, ignoring wwwroot and moduleroot at runtime
* `--disable-ipv6` - Disables the support for the IPv6 protocol stack
* `--disable-epoll` - Disables the support for the epoll mechanism
* `--enable-mpool`- Enables the memory pool support (optimized for windows only)
* `--enable-prefork` - Enables the prefork support so that viriatum can create workers

## Modules

There are a series of modules for the viriatum server that are used to extend functionality
of the base server, in order to compile then some rules apply.
Current modules include:

* `mod_lua` - For interaction with the Lua interpreter
* `mod_php` - For interaction with the PHP interpreter (complex compilation)
* `mod_wsgi` - For interaction with the Python interpreter using the [WSGI](http://wsgi.org) specification.

### Lua Module

For an Ubuntu/Debian environment the Lua 5.1 development packages must be included using:

    apt-get install liblua5.1-0-dev

On Alpine Linux:

    apk add lua5.1-dev

### PHP Module

Requires PHP 8.x compiled with the embed SAPI. On Alpine Linux the simplest approach is to use the
system packages:

    apk add php84-dev php84-embed

When building PHP from source, compile with embed SAPI support (the library is now named `libphp.so`
instead of the old `libphp5.so`):

    ./configure --enable-embed

For static linking, useful for package distribution:

    export CFLAGS="-fpic"
    ./configure --enable-embed=static --disable-libxml --disable-dom --disable-simplexml\
        --disable-xml --disable-xmlreader --disable-xmlwriter --without-pear --without-iconv

The PHP module must then be compiled with `CFLAGS` pointing to the PHP include directories.
The easiest way is to use `php-config`:

    export CFLAGS="$(php-config --includes)"

Or manually:

    export CFLAGS="-I/usr/local/include/php -I/usr/local/include/php/main\
        -I/usr/local/include/php/TSRM -I/usr/local/include/php/Zend"

Additional information about PHP compilation flags may be found [here](https://www.php.net/manual/en/configure.about.php).

### WSGI Module

Requires Python 3 development headers and shared library. On Ubuntu/Debian:

    apt-get install python3-dev

On Alpine Linux:

    apk add python3-dev

On macOS (Homebrew):

    brew install python3

The module is compiled with flags from `python3-config`:

    export CFLAGS="$(python3-config --includes)"
    export LDFLAGS="$(python3-config --ldflags --embed)"

## Docker

It's possible to run viriatum inside a Docker container. Three Dockerfiles are provided:

* `Dockerfile` - Core server only (~11 MB)
* `Dockerfile.php` - Core + mod_php with PHP 8.4 (~22 MB)
* `Dockerfile.all` - All modules: diag, gif, lua, php, wsgi (~112 MB)

To build and run:

    docker build -t viriatum .
    docker run -p 9090:9090 viriatum

To build the full image with all modules:

    docker build -f Dockerfile.all -t viriatum-all .
    docker run -p 9090:9090 viriatum-all

## Benchmarking

* Most of the information presented in [gwan benchmark](http://gwan.ch/en_apachebench_httperf.html) is applicable for benchmarking Viriatum

For more information regarding benchmarking please check the list of [todo](doc/todo.md) information.

## Debugging

* For windows debugging use [Visual Leak Detector](http://vld.codeplex.com) using the `#include <vld.h>` statement
* In linux/unix environments use [Valgrind](http://valgrind.org) with `valgrind --tool=memcheck --leak-check=full viriatum`

## Profiling

* On windows systems the tool to uses is the [Very Sleepy](http://www.codersnotes.com/sleepy)
* For linux/unix machines the best approach is to go with either [Sysprof](http://sysprof.com/) or the more expensive
[Zoom](http://www.rotateright.com) from rotate right

## Versioning

The current version numbering in viriatum follows the following wildcard based structure
`${MAJ}.${MIN}.${MIC}${STA}${STA_VER}` and in order to change any of these values the
`definitions.h`  and the `build.json` files must be changed.

In case any one of the version numbers or the stage value changes a new tag must be created in the
git repository, so that the version is correctly identified for the repository contributors.

## License

Viriatum is currently licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/).

## Build Automation

[![Build Status](https://app.travis-ci.com/hivesolutions/viriatum.svg?branch=master)](https://travis-ci.com/github/hivesolutions/viriatum)
[![Build Status GitHub](https://github.com/hivesolutions/viriatum/workflows/Main%20Workflow/badge.svg)](https://github.com/hivesolutions/viriatum/actions)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](https://www.apache.org/licenses/)
