# [Viriatum Web Server](http://viriatum.com)

The Viriatum Web Server is simple and lightweight web server aimed at providing a simple yet powerfull solution for static file serving, proxy access and dynamic language access.

## Building

### Source Control Repository

If you're going to build viriatum using the git repository you first need to generate the configure files using autoconf.

* `./autogen.sh`

### Unix

For building under unix simple instructions are used:

* `./configure`
* `make && make install`

In order to provide a correct (system wide) configuration path use:

* `./configure --sysconfdir=/etc`

### Unix 32bit from 64bit

* `apt-get libc6-dev-i386`
* `export CC="cc -m32"`
* `./configure`
* `make && make install`

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

* `apt-get install mingw32 mingw32-binutils`
* `./configure --host=i586-mingw32msvc --build=i686-linux --prefix=/opt/i586-mingw32`
* `make && make install`

For building using the mingw-w64 toolchain for 64 and 32 bit options.

* `apt-get install mingw-w64 gcc-mingw-w64 binutils-mingw-w64`
* `./configure --host=x86_64-w64-mingw32 --build=i686-linux --prefix=/opt/x86_64-w64-mingw32`
* `./configure --host=i686-w64-mingw32 --build=i686-linux --prefix=/opt/i686-w64-mingw32`
* `make && make install`

## Features

There are a lot of possible building features to enable

* `--with-moduleroot=path` - Sets the path to be used to load the modules
* `--with-wwwroot=path` - Sets the path to be used to install and serve the default content
* `--enable-debug` - Enables the extra debugging capabilities in viriatum
* `--enable-defaults` - Enables the default paths in the viriatum server, ignoring wwwroot and moduleroot at runtime
* `--disable-ipv6` - Disables the support for the ipv6 protocol stack

## Notes

* In order to compile to WINVER <= 0x0500 (Windows 2000 or older) disable IPv6 support (#undef VIRIATUM_IP6)

## Modules

There are a series of modules for the viriatum server in order to compile then
some rules apply.
Current modules include:

* `mod_lua` - For interaction with the lua interpreter
* `mod_php` - For interaction with the php interpreter (complex compilation)

## PHP Module

Must compile the php interpreter with special environment variables set, and then must compile with support for embedding.

* `export CFLAGS="-I/usr/local/include/php -I/usr/local/include/php/main -I/usr/local/include/php/TSRM -I/usr/local/include/php/Zend"`
* `./configure --enable-embed`

## WSGI Module

Must compile the php interpreter with special environment variables set to point to the correct headers directory.

* `export CFLAGS="-I/usr/include/python2.7"`

## FreeBSD

FreeBSD ignores the usr local directory by default so it must be included in order to conform with dependencies.

* `setenv CFLAGS "-L/usr/local/lib -I/usr/local/include"`

## Debugging

* For windows debugging use [Visual Leak Detector](http://vld.codeplex.com) using the `#include <vld.h>` statement

## License

Viriatum is an open-source project licensed under the [GNU General Public License Version 3](http://www.gnu.org/licenses/gpl.html).