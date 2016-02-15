FROM hivesolutions/alpine_dev:latest
MAINTAINER Hive Solutions

EXPOSE 9090

ADD . /viriatum

RUN apk update && apk add make autoconf automake libtool pcre-dev
RUN cd /viriatum && ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && make install

CMD ["/usr/bin/viriatum"]
