FROM ubuntu:latest
MAINTAINER Hive Solutions

EXPOSE 9090

ADD . /viriatum

RUN apt-get update && apt-get install -y -q autoconf automake
RUN cd /viriatum && ./autogen.sh && make install

CMD viriatum
