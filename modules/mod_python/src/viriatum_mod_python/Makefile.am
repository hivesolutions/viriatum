# Hive Viriatum Modules
# Copyright (c) 2008-2026 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Modules.
#
# Hive Viriatum Modules is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Modules is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

include $(top_srcdir)/Common.am

lib_LIBRARIES = libviriatum_mod_python.a

lib_LTLIBRARIES = libviriatum_mod_python.la

libviriatum_mod_python_a_CFLAGS = $(AM_CFLAGS)
libviriatum_mod_python_a_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_mod_python_la_CFLAGS = $(AM_CFLAGS)
libviriatum_mod_python_la_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_mod_python_la_LDFLAGS = -version-info $(MOD_PYTHON_MAJOR_VERSION):$(MOD_PYTHON_MINOR_VERSION):$(MOD_PYTHON_MICRO_VERSION)

libviriatum_mod_python_a_SOURCES = stdafx.c \
module/asgi.c \
module/entry.c \
module/extension.c \
module/handler.c \
../../../../src/viriatum_python/module/handler_asgi.c \
../../../../src/viriatum_python/module/loop.c \
../../../../src/viriatum/http/websocket.c

libviriatum_mod_python_la_SOURCES = stdafx.c \
module/asgi.c \
module/entry.c \
module/extension.c \
module/handler.c \
../../../../src/viriatum_python/module/handler_asgi.c \
../../../../src/viriatum_python/module/loop.c \
../../../../src/viriatum/http/websocket.c
