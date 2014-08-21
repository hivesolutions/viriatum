# [Viriatum Web Server](http://viriatum.hive.pt)

The Viriatum Web Server is simple and lightweight web server aimed at providing a simple yet powerful solution
for static file serving, reverse proxy serving and dynamic language handling.

The current implementation support both x86/x64 and ARM architectures.

## Building

### Automium

Building viriatum using automium is quite simple even for complex builds like cross compilation.
Just use the apropriate 'build.json' located under 'scripts/build' and under such directory execute:

    atm.sh

For cross compilation (eg: arm-rasp-linux) use the following command:

    atm.sh --cflags=-O3 --cross=arm-rasp-linux-gnueabi

If you want to know more about cross compilation please refer to the [Cross Compilation](doc/cross.md) document.

### Source Control Repository

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

The recommended NDK version is r6 as compatibility is ensures for that version. Newer versions are
known to have problems compiling some of the packages (including PHP). To download that version of
the NDK for Linux using this [link](http://dl.google.com/android/ndk/android-ndk-r6-linux-x86.tar.bz2).

    tar -xvf android-ndk-r[x]-linux-x86.tar.bz2

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
* `--enable-debug` - Enables the extra debugging capabilities in viriatum
* `--enable-defaults` - Enables the default paths in the viriatum server, ignoring wwwroot and moduleroot at runtime
* `--disable-ipv6` - Disables the support for the ipv6 protocol stack
* `--disable-epoll` - Disables the support for the epoll mechanism
* `--enable-mpool`- Enables the memory pool support (optimized for windows only)
* `--enable-prefork` - Enables the prefork support so that viriatum can create workers

## Modules

There are a series of modules for the viriatum server that are used to extend functionaly
of the base server, in order to compile then some rules apply.
Current modules include:

* `mod_lua` - For interaction with the Lua interpreter
* `mod_php` - For interaction with the PHP interpreter (complex compilation)
* `mod_wsgi` - For interaction with the Python interpreter using the [WSGI](http://wsgi.org) specification.

### Lua Module

For an ubuntu linux environment the lua 5.1 development packages must be included using:

    apt-get install liblua5.1-0-dev

### PHP Module

Must compile the PHP interpreter with support for embeding, this should create the library with the propr SAPI
symbols loaded.

    ./configure --enable-embed

For static linking of the PHP library, useful for package distribution the following command may be used so
that no external library dependencies are created. Note that the `-fPIc` flag must be set to allow the creation
of static library for position independent code.

    export CFLAGS="-fpic"
    ./configure --enable-embed=static --disable-libxml --disable-dom --disable-simplexml\
        --disable-xml --disable-xmlreader --disable-xmlwriter --without-pear --without-iconv

In order to configure the PHP interpreter to locate the php.ini file in the correct location use:

    --with-config-file-path=/usr/lib

Additional libraries may be linked and a typical compilation of the PHP distribution would include the following
flags required by most of the applications.

    --enable-bcmath --with-mysql --with-mysqli --with-gmp --with-openssl

Additional information about the compilation flags may be found [here](http://php.net/manual/en/configure.about.php).

The PHP module must then be compiled with `CFLAGS` environment variable set to point to the proper include directories
so that the module code is able to compiled agains these header files.

    export CFLAGS="-I/usr/local/include/php -I/usr/local/include/php/main\
        -I/usr/local/include/php/TSRM -I/usr/local/include/php/Zend"

### WSGI Module

For an ubuntu linux environment the python development packages must be included using:

    apt-get install python2.7-dev

Must compile viriatum with special environment variables set to point to the correct headers directory.

    export CFLAGS="-I/usr/include/python2.7"
    export CFLAGS="-I/System/Library/Frameworks/Python.framework/Headers"

## Docker

It's possible to run viriatum inside a docker container and pre-built images 
have been created under the public repository.

To use viriatum using these pre-built images use the following command:

    docker run -d joamag/devel /usr/sbin/viriatum

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

Viriatum is an open-source project licensed under the [GNU General Public License Version 3](http://www.gnu.org/licenses/gpl.html).
