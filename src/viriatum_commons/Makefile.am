# Hive Viriatum Commons
# Copyright (c) 2008-2020 Hive Solutions Lda.
#
# This file is part of Hive Viriatum Commons.
#
# Hive Viriatum Commons is free software: you can redistribute it and/or modify
# it under the terms of the Apache License as published by the Apache
# Foundation, either version 2.0 of the License, or (at your option) any
# later version.
#
# Hive Viriatum Commons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# Apache License for more details.
#
# You should have received a copy of the Apache License along with
# Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

# __author__    = João Magalhães <joamag@hive.pt>
# __version__   = 1.0.0
# __revision__  = $LastChangedRevision$
# __date__      = $LastChangedDate$
# __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
# __license__   = Apache License, Version 2.0

include $(top_srcdir)/Common.am

lib_LIBRARIES = libviriatum.a

lib_LTLIBRARIES = libviriatum.la

libviriatum_a_CFLAGS = $(AM_CFLAGS)
libviriatum_a_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_la_CFLAGS = $(AM_CFLAGS)
libviriatum_la_CXXFLAGS = $(AM_CXXFLAGS)

libviriatum_la_LDFLAGS = -version-info $(VIRIATUM_MAJOR_VERSION):$(VIRIATUM_MINOR_VERSION):$(VIRIATUM_MICRO_VERSION)

libviriatum_a_SOURCES = stdafx.c \
checksum/crc_32.c \
checksum/md5.c \
checksum/sha1.c \
compression/huffman.c \
crypto/rc4.c \
debug/error.c \
debug/logging.c \
encoding/base_64.c \
encoding/bencoding.c \
encoding/percent.c \
formats/passwd.c \
formats/url.c \
ini/ini.c \
io/file.c \
jni/info.c \
jni/start.c \
memory/memory_pool.c \
sorting/quicksort.c \
stream/bit_stream.c \
stream/file_stream.c \
stream/io_stream.c \
stream/memory_stream.c \
structures/array_list.c \
structures/buffer.c \
structures/hash_map.c \
structures/iterator.c \
structures/linked_buffer.c \
structures/linked_list.c \
structures/priority_queue.c \
structures/sort_map.c \
structures/string.c \
structures/string_buffer.c \
structures/type.c \
system/_thread_win32.c \
template/engine.c \
template/handler.c \
test/unit.c \
thread/thread_pool.c \
util/arguments_util.c

libviriatum_la_SOURCES = stdafx.c \
checksum/crc_32.c \
checksum/md5.c \
checksum/sha1.c \
compression/huffman.c \
debug/error.c \
debug/logging.c \
encoding/base_64.c \
encoding/bencoding.c \
encoding/percent.c \
formats/passwd.c \
formats/url.c \
ini/ini.c \
io/file.c \
jni/info.c \
jni/start.c \
memory/memory_pool.c \
sorting/quicksort.c \
stream/bit_stream.c \
stream/file_stream.c \
stream/io_stream.c \
stream/memory_stream.c \
structures/array_list.c \
structures/buffer.c \
structures/hash_map.c \
structures/iterator.c \
structures/linked_buffer.c \
structures/linked_list.c \
structures/priority_queue.c \
structures/sort_map.c \
structures/string.c \
structures/string_buffer.c \
structures/type.c \
system/_thread_win32.c \
template/engine.c \
template/handler.c \
test/unit.c \
thread/thread_pool.c \
util/arguments_util.c
