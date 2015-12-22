FROM ubuntu:latest
MAINTAINER Hive Solutions

EXPOSE 9090

ADD . /viriatum

RUN apt-get update && apt-get install -y -q make autoconf automake libtool libpcre3-dev
RUN cd /viriatum && ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && make install

CMD viriatum
