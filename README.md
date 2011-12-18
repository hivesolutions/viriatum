# [Viriatum Web Server](http://viriatum.com)
The Viriatum Web Server is simple and lightweight web server aimed at providing a simple yet powerfull solution for static file serving, proxy access and dynamic language access.

## Building

### Unix

For building under unix simple instructions are used:

* `./configure`
* `make && make install`

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

Under construction

* `apt-get install mingw32 mingw32-binutils gcc-mingw32`
* ` ./configure --host=i586-mingw32msvc --build=i686-linux --prefix=/opt/i586-mingw32`
* `make && make install`

## Features

There are a lot of possible building features to enable

* `--with-moduleroot=path` - Sets the path to be used to load the modules
* `--with-wwwroot=path` - Sets the path to be used to install and serve the default content
* `--enable-debug` - Enables the extra debugging capabilities in viriatum

## License

Viriatum is an open-source project licensed under the [GNU General Public License Version 3](http://www.gnu.org/licenses/gpl.html).