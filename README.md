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
* Unpack the android-ndk package file into the `android-ndk` directory
* Create the standalone toolchain `android-ndk/build/tools/make-standalone-toolchain.sh --platform=android-4 --install-dir=/opt/android-toolchain`
* `export PATH=/opt/android-toolchain/bin:$PATH`
* `./configure --target=arm-linux-androideabi --host=arm-linux-androideabi --prefix=/opt/android-root`
* `make && make install`

### Windows

Under construction

* `export PATH=/opt/android-toolchain/bin:$PATH`
* `./configure --target=i586-mingw32msvc --host=i586-mingw32msvc --prefix=/opt/android-root`
* `make && make install`

## Features

There are a lot of possible building features to enable

* `--enable-debug` - Enables the extra debugging capabilities in viriatum

## License

Viriatum is an open-source project licensed under the [GNU General Public License Version 3](http://www.gnu.org/licenses/gpl.html).