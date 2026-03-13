FROM --platform=linux/amd64 alpine:latest AS builder

# install build dependencies for viriatum and PHP 8
RUN apk update && apk add \
    build-base make autoconf automake libtool \
    pcre-dev openssl-dev php84-dev php84-embed

ADD . /viriatum
RUN rm -rf /viriatum/.git

# build and install viriatum (with PCRE for regex-based dispatch routing)
RUN cd /viriatum && \
    ./autogen.sh && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make && make install

# build and install mod_php module
# php-config84 --ldflags returns a wrong path on Alpine, so we locate
# the directory containing libphp.so directly
RUN cd /viriatum/modules/mod_php && \
    ./autogen.sh && \
    export PHP_LIB_DIR=$(dirname $(find /usr/lib -name "libphp.so" 2>/dev/null | head -1)) && \
    export PHP_INCLUDES="$(php-config84 --includes)" && \
    export CFLAGS="-fcommon -D_GNU_SOURCE ${PHP_INCLUDES}" && \
    export CPPFLAGS="-D_GNU_SOURCE ${PHP_INCLUDES}" && \
    export LDFLAGS="-L${PHP_LIB_DIR}" && \
    ./configure --prefix=/usr && \
    make && make install

FROM --platform=linux/amd64 alpine:latest

LABEL version="1.0"
LABEL maintainer="Hive Solutions <development@hive.pt>"

EXPOSE 9090

RUN apk add --no-cache pcre libssl3 libcrypto3 php84-embed

COPY --from=builder /usr/bin/viriatum /usr/bin/viriatum
COPY --from=builder /usr/lib/libviriatum.so.1.0.0 /usr/lib/libviriatum.so.1.0.0
COPY --from=builder /usr/lib/libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so.1.0.0
COPY --from=builder /usr/lib/viriatum/modules/libviriatum_mod_php.so /usr/lib/viriatum/modules/libviriatum_mod_php.so
COPY --from=builder /etc/viriatum /etc/viriatum
COPY --from=builder /var/viriatum /var/viriatum

RUN ln -s libviriatum.so.1.0.0 /usr/lib/libviriatum.so.1 && \
    ln -s libviriatum.so.1.0.0 /usr/lib/libviriatum.so && \
    ln -s libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so.1 && \
    ln -s libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so && \
    ln -sf /usr/lib/php84/libphp.so /usr/lib/libphp.so

RUN addgroup -S viriatum && adduser -S viriatum -G viriatum && \
    chown -R viriatum:viriatum /etc/viriatum /var/viriatum
USER viriatum

CMD ["/usr/bin/viriatum"]
