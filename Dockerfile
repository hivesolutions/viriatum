FROM alpine:latest AS builder

ADD . /viriatum
RUN rm -rf /viriatum/.git
RUN apk update && apk add build-base make autoconf automake libtool pcre-dev openssl-dev
RUN cd /viriatum && ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && make install

FROM alpine:latest

LABEL version="1.0"
LABEL maintainer="Hive Solutions <development@hive.pt>"

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

CMD ["/usr/bin/viriatum"]
