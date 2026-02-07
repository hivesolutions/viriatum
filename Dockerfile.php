FROM hivesolutions/alpine_dev:latest

LABEL version="1.0"
LABEL maintainer="Hive Solutions <development@hive.pt>"

EXPOSE 9090

ENV PHP_VERSION=5.6.40

# install build dependencies for viriatum and PHP 5
RUN apk update && apk add make autoconf automake libtool pcre-dev wget

# download and build PHP 5 with embed SAPI (static)
RUN wget https://museum.php.net/php5/php-${PHP_VERSION}.tar.gz && \
    tar xzf php-${PHP_VERSION}.tar.gz && \
    cd php-${PHP_VERSION} && \
    CFLAGS="-fpic" ./configure \
        --enable-embed=static \
        --disable-libxml \
        --disable-dom \
        --disable-simplexml \
        --disable-xml \
        --disable-xmlreader \
        --disable-xmlwriter \
        --without-pear \
        --without-iconv \
        --without-openssl && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf php-${PHP_VERSION}*

ADD . /viriatum
RUN rm -rf /viriatum/.git

# build and install viriatum (with PCRE for regex-based dispatch routing)
RUN cd /viriatum && \
    ./autogen.sh && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make && make install

# build and install mod_php module
RUN cd /viriatum/modules/mod_php && \
    ./autogen.sh && \
    CFLAGS="-I/usr/local/include/php -I/usr/local/include/php/main \
            -I/usr/local/include/php/TSRM -I/usr/local/include/php/Zend" \
    ./configure --prefix=/usr && \
    make && make install

CMD ["/usr/bin/viriatum"]
