# Hive Viriatum Web Server
# Copyright (c) 2008-2020 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Web Server.
#
# Hive Viriatum Web Server is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Web Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

include $(top_srcdir)/Common.am

SUBDIRS = resources

bin_PROGRAMS = viriatum

lib_LTLIBRARIES = libviriatum_http.la

TESTS = test.sh

viriatum_CFLAGS = $(AM_CFLAGS)
viriatum_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_http_la_CFLAGS = $(AM_CFLAGS)
libviriatum_http_la_CXXFLAGS = $(AM_CXXFLAGS)

viriatum_LDADD = ../viriatum_commons/libviriatum.a $(INTLLIBS)

libviriatum_http_la_LIBADD = ../viriatum_commons/libviriatum.la $(INTLLIBS)
libviriatum_http_la_LDFLAGS = -version-info $(VIRIATUM_MAJOR_VERSION):$(VIRIATUM_MINOR_VERSION):$(VIRIATUM_MICRO_VERSION)

viriatum_SOURCES = stdafx.c \
viriatum.c \
handlers/handler_default.c \
handlers/handler_dispatch.c \
handlers/handler_file.c \
handlers/handler_proxy.c \
http/http_parser.c \
http/http_request.c \
http/http_util.c \
jni/info.c \
jni/jni_service.c \
jni/start.c \
module/loading.c \
polling/polling_epoll.c\
polling/polling_kqueue.c\
polling/polling_select.c\
stream/stream_http.c \
stream/stream_httpc.c \
stream/stream_io.c \
stream/stream_torrent.c \
system/environment.c \
system/client.c \
system/service.c \
system/torrent.c \
test/simple_test.c \
test/speed_test.c

libviriatum_http_la_SOURCES = stdafx.c \
viriatum.c \
handlers/handler_default.c \
handlers/handler_dispatch.c \
handlers/handler_file.c \
handlers/handler_proxy.c \
http/http_parser.c \
http/http_request.c \
http/http_util.c \
jni/info.c \
jni/jni_service.c \
jni/start.c \
module/loading.c \
polling/polling_epoll.c\
polling/polling_kqueue.c\
polling/polling_select.c\
stream/stream_http.c \
stream/stream_httpc.c \
stream/stream_io.c \
stream/stream_torrent.c \
system/environment.c \
system/client.c \
system/service.c \
system/torrent.c \
test/simple_test.c \
test/speed_test.c
