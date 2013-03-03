# [Viriatum Web Server](http://viriatum.com)

The Viriatum Web Server is simple and lightweight web server aimed at providing a simple yet powerfull solution
for static file serving, reverse proxy serving and dynamic language handling.

The current implementation support both x86/x64 and ARM architectures.

## Building

### Automium

Building viriatum using automium is quite simple even for complex builds like cross compilation.
Just use the apropriate 'build.json' located under 'scripts/build' and under such directory execute:

    atm.sh

For cross compilation (eg: arm-rasp-linux) use the following command:

    atm.sh --cross=arm-rasp-linux-gnueabi

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

### Unix 32bit from 64bit

    apt-get libc6-dev-i386
    export CC="cc -m32"
    ./configure
    make && make install

### Android

Building viriatum for android involves cross compilation and the android ndk toolkit must be used.

* Check the instructions for downloading the android ndk from the [official website](http://developer.android.com/sdk/ndk/)
* Unpack the android-ndk package file `tar -xvf android-ndk-r[x]-linux-x86.tar.bz2`
* Create the standalone toolchain `android-ndk-r[x]/build/tools/make-standalone-toolchain.sh --platform=android-4 --install-dir=/opt/android-toolchain`
* `export PATH=/opt/android-toolchain/bin:$PATH`
* `./configure --host=arm-linux-androideabi --target=arm-linux-androideabi --prefix=/opt/android-root`
* `make && make install`

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

* In order to compile to WINVER <= 0x0500 (Windows 2000 or older) disable IPv6 support (#undef VIRIATUM_IP6)
 
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

    --with-mysql --with-mysqli --with-gmp --with-openssl --enable-bcmath

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

## Benchmarking

* Most of the information presented in [gwan benchmark](http://gwan.ch/en_apachebench_httperf.html) is applicable for benchmarking Viriatum

For more information regarding benchmarking please check the list of [todo](doc/todo.md) information.

## Debugging

* For windows debugging use [Visual Leak Detector](http://vld.codeplex.com) using the `#include <vld.h>` statement

## License

Viriatum is an open-source project licensed under the [GNU General Public License Version 3](http://www.gnu.org/licenses/gpl.html).
