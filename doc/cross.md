# Cross Compilation

## Raspberry Pi

These cross compilation steps are meant to be used for an Ubuntu Linux stystem.

### Toolchain

http://www.bootc.net/archives/2012/05/26/how-to-build-a-cross-compiler-for-your-raspberry-pi/

### OpenSSL

    export cross=arm-unknown-linux-
    export PATH=/home/joamag/x-tools/arm-unknown-linux-gnueabi/bin:$PATH
    ./config --prefix=/opt/arm-unknown-linux-gnueabi
    ./Configure dist --prefix=/opt/arm-unknown-linux-gnueabi
    make CC="arm-unknown-linux-gnueabi-gcc" AR="arm-unknown-linux-gnueabi-ar r" RANLIB="arm-unknown-linux-gnueabi-ranlib"

### PHP

    export LDFLAGS="-ldl"
    ./configure --host=arm-unknown-linux-gnueabi --build=arm --prefix=/opt/arm-unknown-linux-gnueabi\
        -enable-embed=static --disable-libxml --disable-dom --disable-simplexml --disable-xml\
        --disable-xmlreader --disable-xmlwriter --disable-phar --without-pear --without-iconv\
        --with-config-file-path=/usr/lib
 
### Python

    export CFLAGS="-L/opt/arm-unknown-linux-gnueabi/lib -I/opt/arm-unknown-linux-gnueabi/include\
        -I/opt/arm-unknown-linux-gnueabi/include/php -I/opt/arm-unknown-linux-gnueabi/include/php/main\
        -I/opt/arm-unknown-linux-gnueabi/include/php/TSRM -I/opt/arm-unknown-linux-gnueabi/include/php/Zend"

http://randomsplat.com/id5-cross-compiling-python-for-embedded-linux.html

### Lua
