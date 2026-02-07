FROM --platform=linux/amd64 alpine:latest AS builder

ENV PHP_VERSION=5.6.40

# install build dependencies for viriatum and PHP 5
RUN apk update && apk add \
    build-base make autoconf automake libtool \
    pcre-dev wget

# download and build PHP 5 with embed SAPI (static)
RUN wget https://museum.php.net/php5/php-${PHP_VERSION}.tar.gz && \
    tar xzf php-${PHP_VERSION}.tar.gz && \
    cd php-${PHP_VERSION} && \
    CFLAGS="-fpic -std=gnu89 -w" ./configure \
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
    CFLAGS="-fcommon -I/usr/local/include/php -I/usr/local/include/php/main \
            -I/usr/local/include/php/TSRM -I/usr/local/include/php/Zend" \
    ./configure --prefix=/usr && \
    make && make install

FROM --platform=linux/amd64 alpine:latest

LABEL version="1.0"
LABEL maintainer="Hive Solutions <development@hive.pt>"

EXPOSE 9090

RUN apk add --no-cache pcre

COPY --from=builder /usr/bin/viriatum /usr/bin/viriatum
COPY --from=builder /usr/lib/libviriatum.so.1.0.0 /usr/lib/libviriatum.so.1.0.0
COPY --from=builder /usr/lib/libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so.1.0.0
COPY --from=builder /usr/lib/viriatum/modules/libviriatum_mod_php.so /usr/lib/viriatum/modules/libviriatum_mod_php.so
COPY --from=builder /etc/viriatum /etc/viriatum
COPY --from=builder /var/viriatum /var/viriatum

RUN ln -s libviriatum.so.1.0.0 /usr/lib/libviriatum.so.1 && \
    ln -s libviriatum.so.1.0.0 /usr/lib/libviriatum.so && \
    ln -s libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so.1 && \
    ln -s libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so

CMD ["/usr/bin/viriatum"]
