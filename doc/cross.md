# Cross Compilation

## Raspberry Pi

These cross compilation steps are meant to be used for an Ubuntu Linux stystem.

### Toolchain

    CPU Architecture: ARM
    Operative System: Linux
    Toolchain Path: /opt/${CT_TARGET}

http://www.bootc.net/archives/2012/05/26/how-to-build-a-cross-compiler-for-your-raspberry-pi/

### OpenSSL

    export cross=arm-unknown-linux-
    export PATH=/opt/arm-unknown-linux-gnueabi/bin:$PATH
    ./config --prefix=/opt/arm-unknown-linux-gnueabi
    ./Configure dist --prefix=/opt/arm-unknown-linux-gnueabi
    make CC="arm-unknown-linux-gnueabi-gcc" AR="arm-unknown-linux-gnueabi-ar"\
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

http://randomsplat.com/id5-cross-compiling-python-for-embedded-linux.html

### Lua

Some change to the building files is required in order to build a static based version of the lua
interpreter.
