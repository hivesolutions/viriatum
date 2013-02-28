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

    Crosstool-NG version: 1.17.0
    CPU Architecture: ARM
    Operative System: Linux
    Toolchain Path: /opt/${CT_TARGET}

### OpenSSL

    export cross=arm-unknown-linux-
    export PATH=/opt/arm-unknown-linux-gnueabi/bin:$PATH
    ./config --prefix=/opt/arm-unknown-linux-gnueabi
    ./Configure dist --prefix=/opt/arm-unknown-linux-gnueabi
    make CC="arm-unknown-linux-gnueabi-gcc" AR="arm-unknown-linux-gnueabi-ar r"\
        RANLIB="arm-unknown-linux-gnueabi-ranlib"
    make install

### PCRE

    ./configure --host=arm-unknown-linux-gnueabi --build=arm --prefix=/opt/arm-unknown-linux-gnueabi\
        --disable-shared --enable-static --disable-cpp
    make && make install

### PHP

    export LDFLAGS="-ldl"
    ./configure --host=arm-unknown-linux-gnueabi --build=arm --prefix=/opt/arm-unknown-linux-gnueabi\
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
    wget http://randomsplat.com/wp-content/uploads/2012/10/Python-2.7.3-xcompile.patch
    patch -p1 < Python-2.7.3-xcompile.patch
    
    CC=arm-unknown-linux-gnueabi-gcc CXX=arm-unknown-linux-gnueabi-g++\
        AR=arm-unknown-linux-gnueabi-ar RANLIB=arm-unknown-linux-gnueabi-ranlib\
        ./configure --host=arm-unknown-linux-gnueabi --build=arm --prefix=/opt/arm-unknown-linux-gnueabi
    
    make HOSTPYTHON=./hostpython HOSTPGEN=./Parser/hostpgen BLDSHARED="arm-unknown-linux-gcc -shared"\
        CROSS_COMPILE=arm-unknown-linux- CROSS_COMPILE_TARGET=yes HOSTARCH=arm-unknown-linux-gnueabi BUILDARCH=arm
    
    make install HOSTPYTHON=./hostpython BLDSHARED="arm-unknown-linux-gcc -shared"\
        CROSS_COMPILE=arm-unknown-linuxx- CROSS_COMPILE_TARGET=yes prefix=/opt/arm-unknown-linux-gnueabi

http://randomsplat.com/id5-cross-compiling-python-for-embedded-linux.html

### Lua

Some change to the building files is required in order to build a static based version of the lua
interpreter.
