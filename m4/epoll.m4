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

# checks for the epoll
AC_CHECK_HEADER(sys/epoll.h, have_epoll_include=true, have_epoll_include=false)

# checks if the user wants epoll
AC_ARG_ENABLE(epoll, AC_HELP_STRING([--disable-epoll], [disable epoll() support]), wants_epoll="$enableval", wants_epoll="true")

# in case the user wants epoll and the system has epoll
if test "x$have_epoll_include" = "xtrue" && test "x$wants_epoll" = "xtrue"; then
    AC_MSG_CHECKING(for epoll system call)
    AC_RUN_IFELSE(
        [AC_LANG_SOURCE([
            #include <stdint.h>
            #include <sys/param.h>
            #include <sys/types.h>
            #include <sys/syscall.h>
            #include <sys/epoll.h>
            #include <unistd.h>

            int epoll_create(int size) {
                return (syscall(__NR_epoll_create, size));
            }

            int main (int argc, char **argv) {
                int epfd;
                epfd = epoll_create(256);
                exit(epfd == -1 ? 1 : 0);
            }
        ])], have_epoll=true, have_epoll=false, have_epoll=true)
    AC_MSG_RESULT($have_epoll)
fi
