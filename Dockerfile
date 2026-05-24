FROM alpine:latest AS builder

ADD . /viriatum
RUN rm -rf /viriatum/.git
RUN apk update && apk add build-base make autoconf automake libtool pcre-dev openssl-dev
RUN cd /viriatum && ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && make install

FROM alpine:latest

LABEL version="1.0"
LABEL maintainer="Hive Solutions <development@hive.pt>"
LABEL org.opencontainers.image.title="viriatum"
LABEL org.opencontainers.image.description="Hive Viriatum - lightweight modular HTTP server written in C"
LABEL org.opencontainers.image.source="https://github.com/hivesolutions/viriatum"
LABEL org.opencontainers.image.url="https://github.com/hivesolutions/viriatum"
LABEL org.opencontainers.image.vendor="Hive Solutions"
LABEL org.opencontainers.image.licenses="Apache-2.0"

EXPOSE 9090

RUN apk add --no-cache pcre libssl3 libcrypto3

COPY --from=builder /usr/bin/viriatum /usr/bin/viriatum
COPY --from=builder /usr/lib/libviriatum.so.1.0.0 /usr/lib/libviriatum.so.1.0.0
COPY --from=builder /usr/lib/libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so.1.0.0
COPY --from=builder /etc/viriatum /etc/viriatum
COPY --from=builder /var/viriatum /var/viriatum

RUN ln -s libviriatum.so.1.0.0 /usr/lib/libviriatum.so.1 && \
    ln -s libviriatum.so.1.0.0 /usr/lib/libviriatum.so && \
    ln -s libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so.1 && \
    ln -s libviriatum_http.so.1.0.0 /usr/lib/libviriatum_http.so

RUN addgroup -S viriatum && adduser -S viriatum -G viriatum && \
    chown -R viriatum:viriatum /etc/viriatum /var/viriatum
USER viriatum

CMD ["/usr/bin/viriatum"]
