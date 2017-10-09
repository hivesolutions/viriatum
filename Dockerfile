FROM hivesolutions/alpine_dev:latest

LABEL version="1.0"
LABEL maintainer="Hive Solutions <development@hive.pt>"

EXPOSE 9090

ADD . /viriatum

RUN apk update && apk add make autoconf automake libtool pcre-dev
RUN cd /viriatum && ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && make install

CMD ["/usr/bin/viriatum"]
