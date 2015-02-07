# Cross Compilation

## Raspberry Pi

These cross compilation steps are meant to be used for an Ubuntu Linux stystem.

### Toolchain

In order to create the toolchain for the raspberry pi environment the easiest way to go is by
using the [crosstool-ng](http://crosstool-ng.org) utility meant to support the creation of the
toolchains for embedded devices.

A series of commands exist to control the creation of the toolchain but the most important ones
are the following.

    ct-ng menuconfig
    ct-ng build

The configuration for the raspberry toolchain is the following.

    Crosstool-NG version: 1.19.0
    CPU Architecture: ARM
    Operative System: Linux
    Toolchain Path: /opt/${CT_TARGET}
    Tuple's vendor string: rasp
    Compilers: gcc and g++

### OpenSSL

    export cross=arm-rasp-linux-
    export PATH=/opt/arm-rasp-linux-gnueabi/bin:$PATH
    ./config --prefix=/opt/arm-rasp-linux-gnueabi
    ./Configure dist --prefix=/opt/arm-rasp-linux-gnueabi
    make CC="arm-rasp-linux-gnueabi-gcc" AR="arm-rasp-linux-gnueabi-ar r"\
        RANLIB="arm-rasp-linux-gnueabi-ranlib"
    make install

### PCRE

    export PATH=/opt/arm-rasp-linux-gnueabi/bin:$PATH
    ./configure --host=arm-rasp-linux-gnueabi --build=arm --prefix=/opt/arm-rasp-linux-gnueabi\
        --disable-shared --enable-static --disable-cpp
    make && make install

### PHP

    export PATH=/opt/arm-rasp-linux-gnueabi/bin:$PATH
    export LDFLAGS="-ldl"
    ./configure --host=arm-rasp-linux-gnueabi --build=arm --prefix=/opt/arm-rasp-linux-gnueabi\
        -enable-embed=static --disable-libxml --disable-dom --disable-simplexml --disable-xml\
        --disable-xmlreader --disable-xmlwriter --disable-phar --without-pear --without-iconv\
        --with-config-file-path=/usr/lib
    make && make install

### Python

    ./configure
    make python Parser/pgen
    mv python hostpython
    mv Parser/pgen Parser/hostpgen
    make distclean

    wget https://raw.github.com/hivesolutions/patches/master/python/Python-2.7.3-xcompile.patch
    patch -p1 < Python-2.7.3-xcompile.patch

    export PATH=/opt/arm-rasp-linux-gnueabi/bin:$PATH

    CC=arm-rasp-linux-gnueabi-gcc CXX=arm-rasp-linux-gnueabi-g++\
        AR=arm-rasp-linux-gnueabi-ar RANLIB=arm-rasp-linux-gnueabi-ranlib\
        ./configure --host=arm-rasp-linux-gnueabi --build=arm --prefix=/opt/arm-rasp-linux-gnueabi --enable-shared

    make HOSTPYTHON=./hostpython HOSTPGEN=./Parser/hostpgen BLDSHARED="arm-rasp-linux-gnueabi-gcc -shared"\
        CROSS_COMPILE=arm-rasp-linux-gnueabi- CROSS_COMPILE_TARGET=yes HOSTARCH=arm-rasp-linux-gnueabi BUILDARCH=arm

    make install HOSTPYTHON=./hostpython BLDSHARED="arm-rasp-linux-gcc -shared"\
        CROSS_COMPILE=arm-rasp-linux-gnueabi- CROSS_COMPILE_TARGET=yes prefix=/opt/arm-rasp-linux-gnueabi

### Lua

    wget https://raw.github.com/hivesolutions/patches/master/lua/lua-5.1.5-xcompile.patch
    patch -p1 < lua-5.1.5-xcompile.patch

    export PATH=/opt/arm-rasp-linux-gnueabi/bin:$PATH
    make linux CC="arm-rasp-linux-gnueabi-gcc" AR="arm-rasp-linux-gnueabi-ar rcu"\
        RANLIB="arm-rasp-linux-gnueabi-ranlib" CFLAGS="-I/opt/arm-rasp-linux-gnueabi/include\
        -L/opt/arm-rasp-linux-gnueabi/lib -R/opt/arm-rasp-linux-gnueabi/lib"
    make install INSTALL_TOP=/opt/arm-rasp-linux-gnueabi

### General

    export PATH=/opt/arm-rasp-linux-gnueabi/bin:$PATH
    ./configure --host=arm-rasp-linux-gnueabi --build=arm --prefix=/opt/arm-rasp-linux-gnueabi
    make && make install
